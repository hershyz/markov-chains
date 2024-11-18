#include <iostream>
#include "chain.cpp"
using namespace std;

// instance-level variables and parameters
string csv_path = "historical.csv";
double negative_momentum_threshold = -1.50;         // (-inf, negative_momentum_threshold) = negative momentum
double positive_momentum_threshold = 1.50;          // (positive_momentum_threshold, inf) = positive momentum
                                                    // [negative_momentum_threshold, positive_momentum_threshold] = neutral momentum
int day_interval = 3;                               // size of interval for which we are assigning the threshold

int main() {
    
    // initialize and build markov chain
    Chain chain;
    chain.parse(csv_path);
    chain.build(day_interval, positive_momentum_threshold, negative_momentum_threshold);

    // run greedy backtest
    chain.backtest_greedy(day_interval, 1000.0, positive_momentum_threshold, negative_momentum_threshold);
}