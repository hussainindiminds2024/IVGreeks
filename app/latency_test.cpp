#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

#include "option_analytics.h"

using Clock = std::chrono::high_resolution_clock;

struct OptionRow {

    double S;
    double K;
    double r;
    double q;
    double T;
    double marketPrice;

    bool isCall;
};

double percentile(
    std::vector<double>& v,
    double p
) {

    size_t idx =
        static_cast<size_t>(
            p * v.size()
        );

    return v[idx];
}

int main() {

    constexpr int NUM_OPTIONS = 100000;

    std::vector<OptionRow> chain;
    chain.reserve(NUM_OPTIONS);

    std::mt19937 rng(42);

    std::uniform_real_distribution<> spotDist(20000, 26000);
    std::uniform_real_distribution<> strikeDist(20000, 26000);
    std::uniform_real_distribution<> priceDist(50, 500);
    std::uniform_real_distribution<> tDist(1, 30);

    // ─────────────────────────────────────────
    //  Generate chain
    // ─────────────────────────────────────────

    for (int i = 0; i < NUM_OPTIONS; ++i) {

        chain.push_back({
            spotDist(rng),
            strikeDist(rng),
            0.06,
            0.00,
            tDist(rng) / 365.0,
            priceDist(rng),
            i % 2 == 0
        });
    }

    // ─────────────────────────────────────────
    //  Latency storage
    // ─────────────────────────────────────────

    std::vector<double> latenciesNs;
    latenciesNs.reserve(NUM_OPTIONS);

    double checksum = 0.0;

    // ─────────────────────────────────────────
    //  Benchmark
    // ─────────────────────────────────────────

    auto totalStart = Clock::now();

    for (const auto& row : chain) {

        auto start = Clock::now();

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

        auto end = Clock::now();

        checksum += res.iv;

        auto ns =
            std::chrono::duration_cast<
                std::chrono::nanoseconds
            >(end - start).count();

        latenciesNs.push_back(
            static_cast<double>(ns)
        );
    }

    auto totalEnd = Clock::now();

    // ─────────────────────────────────────────
    //  Sort for percentile analysis
    // ─────────────────────────────────────────

    std::sort(
        latenciesNs.begin(),
        latenciesNs.end()
    );

    // ─────────────────────────────────────────
    //  Percentiles
    // ─────────────────────────────────────────

    double p50  = percentile(latenciesNs, 0.50);
    double p90  = percentile(latenciesNs, 0.90);
    double p95  = percentile(latenciesNs, 0.95);
    double p99  = percentile(latenciesNs, 0.99);
    double p999 = percentile(latenciesNs, 0.999);

    double min =
        latenciesNs.front();

    double max =
        latenciesNs.back();

    auto totalNs =
        std::chrono::duration_cast<
            std::chrono::nanoseconds
        >(totalEnd - totalStart).count();

    // ─────────────────────────────────────────
    //  Output
    // ─────────────────────────────────────────

    std::cout << "\n";
    std::cout << "=====================================\n";
    std::cout << "        LATENCY DISTRIBUTION         \n";
    std::cout << "=====================================\n";

    std::cout << "Options          : "
              << NUM_OPTIONS
              << "\n";

    std::cout << "Checksum         : "
              << checksum
              << "\n\n";

    std::cout << "Min   (ns)       : "
              << min
              << "\n";

    std::cout << "p50   (ns)       : "
              << p50
              << "\n";

    std::cout << "p90   (ns)       : "
              << p90
              << "\n";

    std::cout << "p95   (ns)       : "
              << p95
              << "\n";

    std::cout << "p99   (ns)       : "
              << p99
              << "\n";

    std::cout << "p99.9 (ns)       : "
              << p999
              << "\n";

    std::cout << "Max   (ns)       : "
              << max
              << "\n\n";

    std::cout << "Total Time (ms)  : "
              << totalNs / 1e6
              << "\n";

    std::cout << "=====================================\n";

    return 0;
}