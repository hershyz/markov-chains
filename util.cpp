#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

// split a string by the token character
vector<string> split(const string& str, char token) {
    vector<string> res;
    stringstream ss(str);
    string curr;
    while (getline(ss, curr, token)) {
        res.push_back(curr);
    }
    return res;
}

// split into doubles by the token character
vector<double> split_double(const string& str, char token) {
    vector<double> res;
    stringstream ss(str);
    string curr;
    while (getline(ss, curr, token)) {
        try {
            res.push_back(stod(curr));
        } catch (const invalid_argument e) {
            continue;
        }
    }
    return res;
}