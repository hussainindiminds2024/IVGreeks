#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

#include "option_analytics.h"

// ─────────────────────────────────────────────
//  Safe Input Helpers
// ─────────────────────────────────────────────

double getDouble(
    const std::string& prompt,
    double minVal = 0.0
) {

    double val;

    while (true) {

        std::cout << prompt;

        if (std::cin >> val && val >= minVal)
            return val;

        std::cout
            << "Invalid input. Enter value >= "
            << minVal
            << "\n";

        std::cin.clear();

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}

int getChoice(
    const std::string& prompt,
    int lo,
    int hi
) {

    int val;

    while (true) {

        std::cout << prompt;

        if (std::cin >> val && val >= lo && val <= hi)
            return val;

        std::cout
            << "Invalid choice. Enter between "
            << lo
            << " and "
            << hi
            << "\n";

        std::cin.clear();

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────

int main() {

    std::cout << "\n";
    std::cout << "=====================================\n";
    std::cout << "     IV + GREEKS ANALYTICS ENGINE    \n";
    std::cout << "=====================================\n";

    // ─────────────────────────────────────────
    //  Segment
    // ─────────────────────────────────────────

    std::cout << "\n";
    std::cout << "Select Segment\n";
    std::cout << "1. Equity (Black-Scholes)\n";
    std::cout << "2. Commodity (Black-76)\n";

    int segmentChoice =
        getChoice("\nChoice: ", 1, 2);

    std::string segment =
        (segmentChoice == 1)
        ? "equity"
        : "commodity";

    // ─────────────────────────────────────────
    //  Option Type
    // ─────────────────────────────────────────

    std::cout << "\n";
    std::cout << "Select Option Type\n";
    std::cout << "1. Call\n";
    std::cout << "2. Put\n";

    int optionChoice =
        getChoice("\nChoice: ", 1, 2);

    bool isCall =
        (optionChoice == 1);

    // ─────────────────────────────────────────
    //  Inputs
    // ─────────────────────────────────────────

    double S;
    double K;
    double r;
    double q = 0.0;
    double T;
    double marketPrice;

    std::cout << "\nINPUTS\n";
    std::cout << "-----------------------------\n";

    if (segment == "equity") {

        S = getDouble(
            "Spot Price (S): ",
            0.0001
        );
    }
    else {

        S = getDouble(
            "Futures Price (F): ",
            0.0001
        );
    }

    K = getDouble(
        "Strike Price (K): ",
        0.0001
    );

    marketPrice = getDouble(
        "Market Option Price: ",
        0.0001
    );

    double days = getDouble(
        "Days To Expiry: ",
        1.0
    );

    T = days / 365.0;

    r = getDouble(
        "Risk-Free Rate (%) : ",
        0.0
    );

    r /= 100.0;

    if (segment == "equity") {

        q = getDouble(
            "Dividend Yield (%) : ",
            0.0
        );

        q /= 100.0;
    }

    // ─────────────────────────────────────────
    //  Iteration Logging
    // ─────────────────────────────────────────

    std::cout << "\n";
    std::cout << "Show Newton Iterations?\n";
    std::cout << "1. Yes\n";
    std::cout << "2. No\n";

    bool verbose =
        (
            getChoice("\nChoice: ", 1, 2)
            == 1
        );

    // ─────────────────────────────────────────
    //  Calculate
    // ─────────────────────────────────────────

    auto res =
        OptionAnalytics::calculate(
            S,
            K,
            r,
            q,
            T,
            marketPrice,
            isCall,
            segment,
            0.30,
            1e-6,
            100,
            verbose
        );

    // ─────────────────────────────────────────
    //  Print Result
    // ─────────────────────────────────────────

    OptionAnalytics::print(res);

    return 0;
}