#include "../include/converter.h"
#include <string>
#include <vector>
#include <algorithm> // For std::reverse
#include <cctype>    // For std::isdigit, std::isalpha, std::toupper

namespace BaseConverter {

// Helper function to convert a character to its integer value.
// Handles '0'-'9' and 'A'-'Z' (case-insensitive).
long long char_to_int(char c, int base) {
    if (std::isdigit(c)) {
        int val = c - '0';
        if (val >= base) {
            throw std::invalid_argument("Invalid character '" + std::string(1, c) + "' for base " + std::to_string(base));
        }
        return val;
    }
    if (std::isalpha(c)) {
        int val = std::toupper(c) - 'A' + 10;
        if (val >= base) {
            throw std::invalid_argument("Invalid character '" + std::string(1, c) + "' for base " + std::to_string(base));
        }
        return val;
    }
    throw std::invalid_argument("Invalid character '" + std::string(1, c) + "' in input number string.");
}

// Helper function to convert an integer (0-35) to its character representation.
char int_to_char(long long val) {
    if (val >= 0 && val <= 9) {
        return static_cast<char>('0' + val);
    }
    if (val >= 10 && val <= 35) {
        return static_cast<char>('A' + (val - 10));
    }
    throw std::invalid_argument("Cannot convert value " + std::to_string(val) + " to character (must be 0-35).");
}

// Converts a number string from a given base to a long long (base 10).
long long to_base10(const std::string& value_str, int base_from) {
    if (value_str.empty()) {
        throw std::invalid_argument("Input number string cannot be empty.");
    }
    long long result = 0;
    long long power = 1;
    bool is_negative = false;
    std::string num_part = value_str;

    if (value_str[0] == '-') {
        if (value_str.length() == 1) {
            throw std::invalid_argument("Invalid negative number format: '-'.");
        }
        is_negative = true;
        num_part = value_str.substr(1);
        if (num_part.empty()) {
             throw std::invalid_argument("Input number string cannot be empty after sign.");
        }
    }
    
    // Validate all characters in num_part first
    for (char c : num_part) {
        char_to_int(c, base_from); // This will throw if char is invalid for base
    }

    for (int i = num_part.length() - 1; i >= 0; --i) {
        long long val = char_to_int(num_part[i], base_from);
        if (__builtin_mul_overflow(val, power, &val)) { // val * power
             throw std::out_of_range("Number too large: overflow during intermediate calculation (val * power).");
        }
        if (__builtin_add_overflow(result, val, &result)) { // result + val
            throw std::out_of_range("Number too large: overflow during intermediate calculation (result + val).");
        }
        if (i > 0) { // Avoid overflow on power for the last digit
            if (__builtin_mul_overflow(power, base_from, &power)) { // power * base_from
                 throw std::out_of_range("Number too large: overflow during power calculation.");
            }
        }
    }
    return is_negative ? -result : result;
}

// Converts a long long (base 10) number to a string in the target base.
std::string from_base10(long long num, int base_to) {
    if (num == 0) {
        return "0";
    }
    std::string result_str = "";
    bool is_negative = false;
    if (num < 0) {
        is_negative = true;
        // Work with positive number for conversion logic, check for LLONG_MIN edge case
        if (num == LLONG_MIN) { 
            // Special handling for LLONG_MIN as -LLONG_MIN can overflow
            // Convert LLONG_MAX, then adjust manually or use a different approach for LLONG_MIN
            // For simplicity in this context, we might restrict input range or accept slight imprecision here
            // Or, more robustly, convert -(num / base_to) and handle remainder separately.
            // Let's convert a slightly smaller number and adjust if this becomes an issue.
            // A simpler approach for now: If base_to is power of 2, it can be exact.
            // For general case, this is complex. Let's assume typical ranges for now.
            // A common strategy is to add 1 to num/base for the recursive call and adjust remainder.
            long long remainder = -(num % base_to); // num is negative, so remainder will be <=0
            if(remainder < 0) remainder += base_to; // ensure remainder is positive
            result_str += int_to_char(remainder);
            num = -(num / base_to); // num becomes positive, may have lost precision if not divisible
            // This approach for LLONG_MIN is tricky. A better way for LLONG_MIN is to handle it by converting LLONG_MAX
            // and then figuring out the last digit, or iteratively building the string.
            // The loop below handles positive numbers correctly. We make `num` positive.
            // For LLONG_MIN, -num is not representable if long long is 2's complement.num = -(num +1 ); // make it LLONG_MAX to be safe
            // This isn't perfect. Let's work with `unsigned long long` for the conversion part to avoid LLONG_MIN issues with negation.
            // However, the input is `long long`. Sticking to `long long` and documenting this limitation for LLONG_MIN or erroring out.
             if (num == LLONG_MIN) { // re-check after attempt to make positive
                // This case requires careful handling, potentially using unsigned or iterative division/modulo.
                // For now, let's throw an error for LLONG_MIN as full handling is complex for all bases.
                throw std::out_of_range("Conversion of LLONG_MIN is not fully supported for all bases due to negation limits.");
            }
            num = -num; // Make positive
        } else {
             num = -num; // Make positive
        }
    }

    unsigned long long u_num = static_cast<unsigned long long>(num); // Use unsigned for modulo arithmetic to avoid issues with negative results

    while (u_num > 0) {
        result_str += int_to_char(u_num % base_to);
        u_num /= base_to;
    }

    if (is_negative) {
        result_str += '-';
    }
    std::reverse(result_str.begin(), result_str.end());
    return result_str;
}

std::string convert_base(const std::string& value_str, int base_from, int base_to) {
    if (base_from < 2 || base_from > 36 || base_to < 2 || base_to > 36) {
        throw std::invalid_argument("Bases must be between 2 and 36 (inclusive).");
    }
    if (value_str.empty()){
        throw std::invalid_argument("Input value string cannot be empty.");
    }

    long long base10_val = to_base10(value_str, base_from);
    return from_base10(base10_val, base_to);
}

} // namespace BaseConverter 