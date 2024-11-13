#include <iostream>
#include <fstream>
#include "util.cpp"
using namespace std;

// organization for markov chains/transition matrices
class Chain {
public:

    // asset list for numerical indexing (avoiding hash functions for as long as possible)
    vector<string> asset_list;

    // price matrix (price_mat[asset index][day])
    vector<vector<int> > price_mat;

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
            cout << raw_price_string << endl;
        }
    }
};