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
    double positive_proportion;
    double neutral_proportion;
    double negative_proportion;
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

        for (int i = 0; i < asset_list.size(); i++) {
            cout << chain[i].size() << endl;
        }

        //construct condensed transition matrix representing the whole chain
    }
};