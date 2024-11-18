#include <iostream>
#include <fstream>
#include "util.cpp"
using namespace std;

// organization for markov chains/transition matrices
class Chain {
public:

    // split ratios for training and backtest (80-20 by default)
    double training_split = 0.8;
    double backtest_split = 0.2;

    // asset list for numerical indexing (avoiding hash functions for as long as possible)
    vector<string> asset_list;

    // price matrix (price_mat[asset index][day])
    vector<vector<double> > price_mat;

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
        string raw_price_string;
        while (getline(file, raw_price_string)) {
            vector<double> price_row = split_double(raw_price_string, ',');
            price_row.erase(price_row.begin());
            for (size_t i = 0; i < price_row.size(); i++) {
                price_mat[i].push_back(price_row[i]);
            }
        }

        for (size_t i = 0; i < price_mat.size(); i++) {
            cout << price_mat[i].size() << endl;
        }
    }
};