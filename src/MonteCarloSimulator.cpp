#include "MonteCarloSimulator.hpp"
#include "RiskEvaluator.hpp"
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <algorithm>
#include <cmath>

MonteCarloSimulator::MonteCarloSimulator(int simulations, int threads)
    : numSimulations(simulations), numThreads(threads) {}

std::vector<double> MonteCarloSimulator::simulateLosses(const Policyholder& policyholder) const {
    double expectedLoss = RiskEvaluator::calculateExpectedLoss(policyholder);
    if (expectedLoss <= 0.0) {
        return std::vector<double>(numSimulations, 0.0);
    }

    // Parameters for log-normal distribution modeling severity tail risk
    double sigma = 0.75; // Volatility parameter
    double mu = std::log(expectedLoss) - (sigma * sigma / 2.0);

    int simsPerThread = numSimulations / numThreads;
    std::vector<std::future<std::vector<double>>> futures;
    futures.reserve(numThreads);

    for (int i = 0; i < numThreads; ++i) {
        int currentSims = (i == numThreads - 1) ? (numSimulations - i * simsPerThread) : simsPerThread;
        
        futures.push_back(std::async(std::launch::async, [mu, sigma, currentSims, i]() {
            std::vector<double> localLosses;
            localLosses.reserve(currentSims);
            
            // Unique seed per thread using random_device and thread offset
            std::random_device rd;
            std::mt19937 generator(rd() + i * 1337);
            std::lognormal_distribution<double> distribution(mu, sigma);

            for (int j = 0; j < currentSims; ++j) {
                localLosses.push_back(distribution(generator));
            }
            return localLosses;
        }));
    }

    // Aggregate results from all threads
    std::vector<double> allLosses;
    allLosses.reserve(numSimulations);
    for (auto& fut : futures) {
        auto partialLosses = fut.get();
        allLosses.insert(allLosses.end(), partialLosses.begin(), partialLosses.end());
    }

    return allLosses;
}

double MonteCarloSimulator::calculateVaR(const std::vector<double>& simulatedLosses, double confidenceLevel) const {
    if (simulatedLosses.empty()) return 0.0;

    std::vector<double> sortedLosses = simulatedLosses;
    std::sort(sortedLosses.begin(), sortedLosses.end());

    size_t index = static_cast<size_t>(std::ceil(confidenceLevel * sortedLosses.size())) - 1;
    if (index >= sortedLosses.size()) {
        index = sortedLosses.size() - 1;
    }

    return sortedLosses[index];
}