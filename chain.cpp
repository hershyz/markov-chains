#include <iostream>
#include <fstream>
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

    int positive_positive;
    int positive_neutral;
    int positive_negative;
    int neutral_positive;
    int neutral_neutral;
    int neutral_negative;
    int negative_positive;
    int negative_neutral;
    int negative_negative;

    Summary() : positive_positive(0), positive_neutral(0), positive_negative(0),
                neutral_positive(0), neutral_neutral(0), neutral_negative(0),
                negative_positive(0), negative_neutral(0), negative_negative(0) {}
};

// organization for markov chain
class Chain {
public:

    // split ratios for training and backtest (80-20 by default)
    double training_split = 0.8;
    double backtest_split = 0.2;

    // asset list for numerical indexing (avoiding hash functions for as long as possible)
    vector<string> asset_list;

    // price matrix (price_mat[asset index][day index])
    vector<vector<double> > price_mat;

    // chain (chain[asset index][interval index])
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

        // know at which day to stop
        int end_training_day = training_split * price_mat[0].size();

        // sliding window bounds
        int l = 0;
        int r = day_interval - 1;

        // calculate momentum per interval
        while (r < end_training_day) {

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

                    cout << asset_a_state << " " << asset_b_state << endl;

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
};