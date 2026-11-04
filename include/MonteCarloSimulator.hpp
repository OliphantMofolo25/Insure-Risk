#ifndef MONTECARLOSIMULATOR_HPP
#define MONTECARLOSIMULATOR_HPP

#include "Policyholder.hpp"
#include <vector>

class MonteCarloSimulator {
private:
    int numSimulations;
    int numThreads;

public:
    MonteCarloSimulator(int simulations = 10000, int threads = 4);

    // Runs multi-threaded simulations to generate loss scenarios
    std::vector<double> simulateLosses(const Policyholder& policyholder) const;

    // Calculates Value-at-Risk at a given confidence level (e.g., 0.95 for VaR 95%)
    double calculateVaR(const std::vector<double>& simulatedLosses, double confidenceLevel) const;
};

#endif // MONTECARLOSIMULATOR_HPP