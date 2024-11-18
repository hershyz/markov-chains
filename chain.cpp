#include <iostream>
#include <fstream>
#include <numeric>
#include <deque>
#include "util.cpp"
using namespace std;

// momentum enum
enum Momentum {
    POSITIVE,
    NEUTRAL,
    NEGATIVE
};

// summary struct between two assets
struct Summary {
    int positive_positive = 0;
    int positive_neutral = 0;
    int positive_negative = 0;
    int neutral_positive = 0;
    int neutral_neutral = 0;
    int neutral_negative = 0;
    int negative_positive = 0;
    int negative_neutral = 0;
    int negative_negative = 0;
};

// organization for markov chain
class Chain {
public:

    // asset list for numerical indexing (avoiding hash functions)
    vector<string> asset_list;

    // price matrix (price_mat[asset id][day index])
    vector<vector<double> > price_mat;

    // chain (chain[asset id][interval index])
    vector<vector<Momentum> > chain;

    // transition matrix (transition_mat[asset 1 index][asset 2 index])
    vector<vector<Summary> > transition_mat;

    // parse historical csv
    void parse(string csv_path) {

        // populate asset list
        ifstream file(csv_path);
        string first;
        getline(file, first);
        asset_list = split(first, ',');
        asset_list.erase(asset_list.begin());

        // populate price matrix
        price_mat.resize(asset_list.size());
        chain.resize(asset_list.size());
        string raw_price_string;
        while (getline(file, raw_price_string)) {
            vector<double> price_row = split_double(raw_price_string, ',');
            price_row.erase(price_row.begin());
            for (int i = 0; i < price_row.size(); i++) {
                price_mat[i].push_back(price_row[i]);
            }
        }
    }

    // build markov chain
    void build(int day_interval, double positive_momentum_threshold, double negative_momentum_threshold) {

        // sliding window bounds
        int l = 0;
        int r = day_interval - 1;

        // calculate momentum per interval
        while (r < price_mat[0].size()) {

            for (int asset_index = 0; asset_index < asset_list.size(); asset_index++) {
                double interval_start_price = price_mat[asset_index][l];
                double interval_end_price = price_mat[asset_index][r];
                double percent_diff = (((interval_end_price / interval_start_price) - 1.00) * 100);
                if (percent_diff < negative_momentum_threshold) {
                    chain[asset_index].push_back(NEGATIVE);
                } else if (percent_diff > positive_momentum_threshold) {
                    chain[asset_index].push_back(POSITIVE);
                } else {
                    chain[asset_index].push_back(NEUTRAL);
                }
            }

            l += day_interval;
            r += day_interval;
        }

        //construct condensed transition matrix representing the whole chain
        transition_mat.resize(asset_list.size(), vector<Summary>(asset_list.size()));
        for (int interval_index = 0; interval_index < chain[0].size() - 1; interval_index++) {
            for (int asset_index_a = 0; asset_index_a < asset_list.size(); asset_index_a++) {
                for (int asset_index_b = 0; asset_index_b < asset_list.size(); asset_index_b++) {

                    Momentum asset_a_state = chain[asset_index_a][interval_index];
                    Momentum asset_b_state = chain[asset_index_b][interval_index + 1];

                    if (asset_a_state == POSITIVE && asset_b_state == POSITIVE) {
                        transition_mat[asset_index_a][asset_index_b].positive_positive++;
                    } else if (asset_a_state == POSITIVE && asset_b_state == NEUTRAL) {
                        transition_mat[asset_index_a][asset_index_b].positive_neutral++;
                    } else if (asset_a_state == POSITIVE && asset_b_state == NEGATIVE) {
                        transition_mat[asset_index_a][asset_index_b].positive_negative++;
                    } else if (asset_a_state == NEUTRAL && asset_b_state == POSITIVE) {
                        transition_mat[asset_index_a][asset_index_b].neutral_positive++;
                    } else if (asset_a_state == NEUTRAL && asset_b_state == NEUTRAL) {
                        transition_mat[asset_index_a][asset_index_b].neutral_neutral++;
                    } else if (asset_a_state == NEUTRAL && asset_b_state == NEGATIVE) {
                        transition_mat[asset_index_a][asset_index_b].neutral_negative++;
                    } else if (asset_a_state == NEGATIVE && asset_b_state == POSITIVE) {
                        transition_mat[asset_index_a][asset_index_b].negative_positive++;
                    } else if (asset_a_state == NEGATIVE && asset_b_state == NEUTRAL) {
                        transition_mat[asset_index_a][asset_index_b].negative_neutral++;
                    } else if (asset_a_state == NEGATIVE && asset_b_state == NEGATIVE) {
                        transition_mat[asset_index_a][asset_index_b].negative_negative++;
                    }
                }
            }
        }
    }

    // greedy backtest (complete sell on all assets with predicted negative momentum and reallocate evenly to all assets with predicted positive momentum)
    void backtest_greedy(int day_interval, double starting_balance, double positive_momentum_threshold, double negative_momentum_threshold) {
        
        // holdings[asset id] = asset value in portfolio
        vector<double> holdings;
        holdings.resize(asset_list.size());
        for (int i = 0; i < holdings.size(); i++) {
            holdings[i] = (starting_balance / holdings.size());
        }

        // last_price[asset id] = last price for asset id
        vector<double> last_price;
        last_price.resize(asset_list.size());
        for (int i = 0; i < last_price.size(); i++) {
            last_price[i] = price_mat[i][day_interval - 1];
        }

        // sliding window bounds
        int l = 0;
        int r = day_interval - 1;

        // sliding window
        while (r < price_mat[0].size()) {

            // update holdings and display portfolio value
            for (int asset_index = 0; asset_index < asset_list.size(); asset_index++) {
                double curr = price_mat[asset_index][r];
                double prev = last_price[asset_index];
                double change = curr / prev;
                holdings[asset_index] *= change;
                last_price[asset_index] = curr;
            }
            double portfolio_value = accumulate(holdings.begin(), holdings.end(), 0.0);
            cout << "day: " << r << ", portfolio value: " << portfolio_value << endl;

            // compute momentums and make predictions about next interval
            struct Distribution {
                int predicted_positive = 0;
                int predicted_neutral = 0;
                int predicted_negative = 0;
            };
            vector<Distribution> predictions;               // predictions[asset id] = Distribution
            predictions.resize(asset_list.size());

            for (int asset_index = 0; asset_index < asset_list.size(); asset_index++) {

                double interval_start_price = price_mat[asset_index][l];
                double interval_end_price = price_mat[asset_index][r];
                double percent_diff = (((interval_end_price / interval_start_price) - 1.00) * 100);
                Momentum curr_momentum = POSITIVE;
                if (percent_diff > positive_momentum_threshold) {
                    curr_momentum = POSITIVE;
                } else if (percent_diff < negative_momentum_threshold) {
                    curr_momentum = NEGATIVE;
                } else {
                    curr_momentum = NEUTRAL;
                }

                for (int next_asset_index = 0; next_asset_index < asset_list.size(); next_asset_index++) {
                    Summary s = transition_mat[asset_index][next_asset_index];
                    if (curr_momentum == POSITIVE) {
                        predictions[next_asset_index].predicted_positive += s.positive_positive;
                        predictions[next_asset_index].predicted_neutral += s.positive_neutral;
                        predictions[next_asset_index].predicted_negative += s.positive_negative;
                    } else if (curr_momentum == NEUTRAL) {
                        predictions[next_asset_index].predicted_positive += s.neutral_positive;
                        predictions[next_asset_index].predicted_neutral += s.neutral_neutral;
                        predictions[next_asset_index].predicted_negative += s.neutral_negative;
                    } else if (curr_momentum == NEGATIVE) {
                        predictions[next_asset_index].predicted_positive += s.negative_positive;
                        predictions[next_asset_index].predicted_neutral += s.negative_neutral;
                        predictions[next_asset_index].predicted_negative += s.negative_negative;
                    }
                }
            }

            deque<int> predicted_negative_queue;              // asset ids predicted negative momentum for the next interval
            deque<int> predicted_positive_queue;              // asset ids predicted positive momentum for the next interval
            for (int asset_index = 0; asset_index < predictions.size(); asset_index++) {
                Distribution d = predictions[asset_index];
                double pos_factor = static_cast<double>(d.predicted_positive) / static_cast<double>(d.predicted_positive + d.predicted_neutral + d.predicted_negative);
                double neg_factor = static_cast<double>(d.predicted_negative) / static_cast<double>(d.predicted_positive + d.predicted_neutral + d.predicted_negative);
                if (pos_factor > neg_factor && pos_factor > 0.33) {
                    predicted_positive_queue.push_back(asset_index);
                }
                if (neg_factor > pos_factor && neg_factor > 0.33) {
                    predicted_negative_queue.push_back(asset_index);
                }
            }

            // reallocate assets
            double to_allocate = 0.0;
            while (!predicted_negative_queue.empty()) {
                int asset_index = predicted_negative_queue.front();
                predicted_negative_queue.pop_front();
                to_allocate += holdings[asset_index];
                holdings[asset_index] = 0;
            }
            to_allocate /= predicted_positive_queue.size();

            while (!predicted_positive_queue.empty()) {
                int asset_index = predicted_positive_queue.front();
                predicted_positive_queue.pop_front();
                holdings[asset_index] += to_allocate;
            }
            
            // update window
            l += day_interval;
            r += day_interval;
        }
    }
};