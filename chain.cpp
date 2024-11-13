#include <iostream>
#include <fstream>
#include "util.cpp"
using namespace std;

// organization for markov chains/transition matrices
class Chain {
public:

    vector<string> asset_list;

    // parse historical csv
    void parse(string csv_path) {

        // populate asset list
        ifstream file(csv_path);
        string first;
        getline(file, first);
        asset_list = split(first, ',');
        asset_list.erase(asset_list.begin());
    }
};