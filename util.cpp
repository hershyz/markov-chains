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