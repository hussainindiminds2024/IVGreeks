#pragma once

// ─────────────────────────────────────────────
//  option_analytics.h
//
//  Single-header ultra-fast IV + Greeks utility
//
//  Supports:
//    • Black-Scholes (Equity)
//    • Black-76      (Commodity)
//
//  Features:
//    • Implied Volatility
//    • Greeks
//    • Call / Put
//    • Equity / Commodity
//    • Header-only
//    • Inline optimized
//
// ─────────────────────────────────────────────

#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>

namespace OptionAnalytics {

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────

constexpr double PI = 3.14159265358979323846;

// ─────────────────────────────────────────────
//  Greeks container
// ─────────────────────────────────────────────

struct Greeks {

    double delta = 0.0;
    double gamma = 0.0;
    double theta = 0.0;
    double vega  = 0.0;
    double rho   = 0.0;
};

// ─────────────────────────────────────────────
//  Final analytics result
// ─────────────────────────────────────────────

struct Result {

    // Implied volatility
    double iv = 0.0;

    // Theoretical option price
    double theoPrice = 0.0;

    // Black model intermediates
    double d1 = 0.0;
    double d2 = 0.0;

    // Greeks
    Greeks greeks;

    // Solver diagnostics
    int iterations = 0;
    bool converged = false;
};

// ─────────────────────────────────────────────
//  Normal PDF
// ─────────────────────────────────────────────

inline double normPDF(double x) {

    return std::exp(-0.5 * x * x)
         / std::sqrt(2.0 * PI);
}

// ─────────────────────────────────────────────
//  Normal CDF
//  Abramowitz-Stegun approximation
// ─────────────────────────────────────────────

inline double normCDF(double x) {

    static constexpr double a1 =  0.254829592;
    static constexpr double a2 = -0.284496736;
    static constexpr double a3 =  1.421413741;
    static constexpr double a4 = -1.453152027;
    static constexpr double a5 =  1.061405429;
    static constexpr double p  =  0.3275911;

    int sign = (x < 0.0) ? -1 : 1;

    x = std::fabs(x) / std::sqrt(2.0);

    double t =
        1.0 / (1.0 + p * x);

    double y =
        1.0
        - (((((a5 * t + a4) * t + a3)
        * t + a2) * t + a1)
        * t * std::exp(-x * x));

    return 0.5 * (1.0 + sign * y);
}

// ─────────────────────────────────────────────
//  Black-Scholes Price
// ─────────────────────────────────────────────

inline double blackScholesPrice(
    double S,
    double K,
    double r,
    double q,
    double T,
    double sigma,
    bool isCall
) {

    double sqrtT = std::sqrt(T);

    double d1 =
        (
            std::log(S / K)
            + (r - q + 0.5 * sigma * sigma) * T
        )
        / (sigma * sqrtT);

    double d2 = d1 - sigma * sqrtT;

    double eqT = std::exp(-q * T);
    double erT = std::exp(-r * T);

    if (isCall) {

        return
            S * eqT * normCDF(d1)
            - K * erT * normCDF(d2);
    }

    return
        K * erT * normCDF(-d2)
        - S * eqT * normCDF(-d1);
}

// ─────────────────────────────────────────────
//  Black-Scholes Vega
// ─────────────────────────────────────────────

inline double vegaBS(
    double S,
    double K,
    double r,
    double q,
    double T,
    double sigma
) {

    double sqrtT = std::sqrt(T);

    double d1 =
        (
            std::log(S / K)
            + (r - q + 0.5 * sigma * sigma) * T
        )
        / (sigma * sqrtT);

    return
        S
        * std::exp(-q * T)
        * sqrtT
        * normPDF(d1);
}

// ─────────────────────────────────────────────
//  Black-76 Price
// ─────────────────────────────────────────────

inline double black76Price(
    double F,
    double K,
    double r,
    double T,
    double sigma,
    bool isCall
) {

    double sqrtT = std::sqrt(T);

    double d1 =
        (
            std::log(F / K)
            + 0.5 * sigma * sigma * T
        )
        / (sigma * sqrtT);

    double d2 = d1 - sigma * sqrtT;

    double erT = std::exp(-r * T);

    if (isCall) {

        return
            erT
            * (
                F * normCDF(d1)
                - K * normCDF(d2)
            );
    }

    return
        erT
        * (
            K * normCDF(-d2)
            - F * normCDF(-d1)
        );
}

// ─────────────────────────────────────────────
//  Black-76 Vega
// ─────────────────────────────────────────────

inline double vegaBlack76(
    double F,
    double K,
    double r,
    double T,
    double sigma
) {

    double sqrtT = std::sqrt(T);

    double d1 =
        (
            std::log(F / K)
            + 0.5 * sigma * sigma * T
        )
        / (sigma * sqrtT);

    return
        std::exp(-r * T)
        * F
        * sqrtT
        * normPDF(d1);
}

// ─────────────────────────────────────────────
//  Unified IV + Greeks Engine
//
//  segment:
//      "equity"
//      "commodity"
//
//  S = Spot for equity
//  S = Futures for commodity
//
// ─────────────────────────────────────────────

inline Result calculate(
    double S,
    double K,
    double r,
    double q,
    double T,
    double marketPrice,
    bool isCall,
    const std::string& segment = "equity",
    double initSigma = 0.30,
    double tolerance = 1e-6,
    int maxIter = 100,
    bool verbose = false
) {

    Result result;

    double sigma = initSigma;

    double theo = 0.0;
    double vega = 0.0;

    bool converged = false;

    int iter = 0;

    // ─────────────────────────────────────────
    //  Newton-Raphson IV Solver
    // ─────────────────────────────────────────

    for (iter = 1; iter <= maxIter; ++iter) {

        if (segment == "equity") {

            theo =
                blackScholesPrice(
                    S, K, r, q,
                    T, sigma,
                    isCall
                );

            vega =
                vegaBS(
                    S, K, r, q,
                    T, sigma
                );
        }
        else {

            theo =
                black76Price(
                    S, K, r,
                    T, sigma,
                    isCall
                );

            vega =
                vegaBlack76(
                    S, K, r,
                    T, sigma
                );
        }

        double diff =
            theo - marketPrice;

        if (verbose) {

            std::cout
                << "Iter "
                << iter
                << " | Sigma="
                << sigma
                << " | Theo="
                << theo
                << " | Diff="
                << diff
                << "\n";
        }

        if (std::fabs(diff) < tolerance) {

            converged = true;
            break;
        }

        if (vega < 1e-10)
            break;

        sigma -= diff / vega;

        // Safety clamp

        if (sigma < 0.0001)
            sigma = 0.0001;

        if (sigma > 5.0)
            sigma = 5.0;
    }

    // ─────────────────────────────────────────
    //  d1 / d2
    // ─────────────────────────────────────────

    double sqrtT = std::sqrt(T);

    double d1;
    double d2;

    if (segment == "equity") {

        d1 =
            (
                std::log(S / K)
                + (
                    r - q
                    + 0.5 * sigma * sigma
                ) * T
            )
            / (sigma * sqrtT);
    }
    else {

        d1 =
            (
                std::log(S / K)
                + 0.5 * sigma * sigma * T
            )
            / (sigma * sqrtT);
    }

    d2 = d1 - sigma * sqrtT;

    // ─────────────────────────────────────────
    //  Greeks
    // ─────────────────────────────────────────

    Greeks g;

    double npd1 = normPDF(d1);

    if (segment == "equity") {

        double eqT = std::exp(-q * T);
        double erT = std::exp(-r * T);

        g.gamma =
            eqT * npd1
            / (S * sigma * sqrtT);

        g.vega =
            S * eqT * sqrtT * npd1
            / 100.0;

        if (isCall) {

            g.delta =
                eqT * normCDF(d1);

            g.rho =
                K * T * erT * normCDF(d2)
                / 100.0;

            g.theta =
                (
                    -(S * eqT * npd1 * sigma)
                    / (2.0 * sqrtT)

                    - r * K * erT * normCDF(d2)

                    + q * S * eqT * normCDF(d1)
                )
                / 365.0;
        }
        else {

            g.delta =
                -eqT * normCDF(-d1);

            g.rho =
                -K * T * erT * normCDF(-d2)
                / 100.0;

            g.theta =
                (
                    -(S * eqT * npd1 * sigma)
                    / (2.0 * sqrtT)

                    + r * K * erT * normCDF(-d2)

                    - q * S * eqT * normCDF(-d1)
                )
                / 365.0;
        }
    }
    else {

        double erT = std::exp(-r * T);

        g.gamma =
            erT * npd1
            / (S * sigma * sqrtT);

        g.vega =
            erT * S * sqrtT * npd1
            / 100.0;

        if (isCall) {

            g.delta =
                erT * normCDF(d1);

            g.rho =
                -T * theo
                / 100.0;

            g.theta =
                (
                    -(S * erT * npd1 * sigma)
                    / (2.0 * sqrtT)

                    + r * theo
                )
                / 365.0;
        }
        else {

            g.delta =
                -erT * normCDF(-d1);

            g.rho =
                -T * theo
                / 100.0;

            g.theta =
                (
                    -(S * erT * npd1 * sigma)
                    / (2.0 * sqrtT)

                    - r * theo
                )
                / 365.0;
        }
    }

    // ─────────────────────────────────────────
    //  Final result
    // ─────────────────────────────────────────

    result.iv         = sigma;
    result.theoPrice  = theo;
    result.d1         = d1;
    result.d2         = d2;
    result.greeks     = g;
    result.iterations = iter;
    result.converged  = converged;

    return result;
}

// ─────────────────────────────────────────────
//  Pretty printer
// ─────────────────────────────────────────────

inline void print(
    const Result& r
) {

    std::cout
        << std::fixed
        << std::setprecision(6);

    std::cout << "\n===================================\n";

    std::cout << " IV (%)      : "
              << r.iv * 100.0
              << "\n";

    std::cout << " Theo Price  : "
              << r.theoPrice
              << "\n";

    std::cout << " d1          : "
              << r.d1
              << "\n";

    std::cout << " d2          : "
              << r.d2
              << "\n";

    std::cout << "\n";

    std::cout << " Delta       : "
              << r.greeks.delta
              << "\n";

    std::cout << " Gamma       : "
              << r.greeks.gamma
              << "\n";

    std::cout << " Theta       : "
              << r.greeks.theta
              << "\n";

    std::cout << " Vega        : "
              << r.greeks.vega
              << "\n";

    std::cout << " Rho         : "
              << r.greeks.rho
              << "\n";

    std::cout << "\n";

    std::cout << " Iterations  : "
              << r.iterations
              << "\n";

    std::cout << " Converged   : "
              << (r.converged ? "YES" : "NO")
              << "\n";

    std::cout << "===================================\n";
}

} // namespace OptionAnalytics