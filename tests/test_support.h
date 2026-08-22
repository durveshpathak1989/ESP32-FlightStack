#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>

#define EXPECT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << __FILE__ << ":" << __LINE__ << " failed: " #expr "\n"; \
            return 1; \
        } \
    } while (0)

#define EXPECT_NEAR(value, expected, tolerance) \
    do { \
        if (std::fabs((value) - (expected)) > (tolerance)) { \
            std::cerr << __FILE__ << ":" << __LINE__ << " failed: " #value \
                      << " expected near " << (expected) << " got " << (value) << "\n"; \
            return 1; \
        } \
    } while (0)

inline int testPassed(const char* name)
{
    std::cout << "PASS: " << name << "\n";
    return 0;
}
