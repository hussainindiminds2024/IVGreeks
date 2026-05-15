#include <iostream>
#include <vector>
#include <chrono>
#include <random>

#include "option_analytics.h"

struct OptionRow {

    double S;
    double K;
    double r;
    double q;
    double T;
    double marketPrice;

    bool isCall;
};

int main() {

    constexpr int NUM_OPTIONS = 100000;

    std::vector<OptionRow> chain;
    chain.reserve(NUM_OPTIONS);

    // ─────────────────────────────────────────
    //  Random market data generation
    // ─────────────────────────────────────────

    std::mt19937 rng(42);

    std::uniform_real_distribution<> spotDist(20000, 26000);
    std::uniform_real_distribution<> strikeDist(20000, 26000);
    std::uniform_real_distribution<> ivDist(50, 500);
    std::uniform_real_distribution<> tDist(1, 30);

    for (int i = 0; i < NUM_OPTIONS; ++i) {

        OptionRow row;

        row.S = spotDist(rng);
        row.K = strikeDist(rng);

        row.r = 0.06;
        row.q = 0.00;

        row.T =
            tDist(rng) / 365.0;

        row.marketPrice =
            ivDist(rng);

        row.isCall =
            (i % 2 == 0);

        chain.push_back(row);
    }

    // ─────────────────────────────────────────
    //  Benchmark Start
    // ─────────────────────────────────────────

    auto start =
        std::chrono::high_resolution_clock::now();

    double ivAccumulator = 0.0;

    for (const auto& row : chain) {

        auto res =
            OptionAnalytics::calculate(
                row.S,
                row.K,
                row.r,
                row.q,
                row.T,
                row.marketPrice,
                row.isCall,
                "equity",
                0.30,
                1e-6,
                100,
                false
            );

        ivAccumulator += res.iv;
    }

    auto end =
        std::chrono::high_resolution_clock::now();

    // ─────────────────────────────────────────
    //  Metrics
    // ─────────────────────────────────────────

    auto duration =
        std::chrono::duration_cast<
            std::chrono::microseconds
        >(end - start);

    double totalMs =
        duration.count() / 1000.0;

    double perOptionUs =
        duration.count()
        / static_cast<double>(NUM_OPTIONS);

    double throughput =
        NUM_OPTIONS
        / (totalMs / 1000.0);

    // ─────────────────────────────────────────
    //  Output
    // ─────────────────────────────────────────

    std::cout << "\n";
    std::cout << "=====================================\n";
    std::cout << "        OPTION ENGINE BENCHMARK      \n";
    std::cout << "=====================================\n";

    std::cout << " Options Processed : "
              << NUM_OPTIONS
              << "\n";

    std::cout << " Total Time (ms)   : "
              << totalMs
              << "\n";

    std::cout << " Avg Latency (us)  : "
              << perOptionUs
              << "\n";

    std::cout << " Throughput/sec    : "
              << static_cast<long long>(throughput)
              << "\n";

    std::cout << " IV Checksum       : "
              << ivAccumulator
              << "\n";

    std::cout << "=====================================\n";

    return 0;
}