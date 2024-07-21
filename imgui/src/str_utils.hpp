#pragma once

#include <string>

template<typename T>
[[nodiscard]] T toLower(const T& input) {
    T str = input;
    for (auto& c : str) {
        if (std::is_same_v<T, std::wstring>) {
            if (std::iswupper(static_cast<std::wint_t>(c))) {
                c = std::move(std::towlower(static_cast<std::wint_t>(c)));
            }
        } else if (std::is_same_v<T, std::string>) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                c = std::move(std::tolower(static_cast<unsigned char>(c)));
            }
        } else {
            return T();
        }
    }
    return str;
}

inline bool contains(const std::string& s1, const std::string& s2) {
    return toLower(s1).find(toLower(s2)) != std::string::npos;
}

int PriceFormat(double value, char* buff, int size, void*);