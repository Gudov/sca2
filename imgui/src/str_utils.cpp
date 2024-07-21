#include "str_utils.hpp"

#include <stdio.h>

int PriceFormat(double value, char* buff, int size, void*) {
    if (value >= 1'000'000) {
        return snprintf(buff, size, "%.2lfM", value / 1'000'000);
    }
    if (value >= 1'000) {
        return snprintf(buff, size, "%.2lfk", value / 1'000);
    }
    return snprintf(buff, size, "%lu", size_t(value));
}