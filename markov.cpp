#include <iostream>
using namespace std;

// instance-level variables and parameters
string data_path = "historical.csv";
double negative_momentum_threshold = -3.50;         // (-inf, negative_momentum_threshold) = negative momentum
double positive_momentum_threshold = 3.50;          // (positive_momentum_threshold, inf) = positive momentum
                                                    // [negative_momentum_threshold, positive_momentum_threshold] = neutral momentum
int day_interval = 5;                               // size of interval for which we are assigning the threshold

int main() {
    
}