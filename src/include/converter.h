#pragma once

#include <string>
#include <stdexcept> // For std::invalid_argument, std::out_of_range

namespace BaseConverter {

/**
 * Converts a number string from one base to another.
 *
 * @param value_str The number string to convert (e.g., "FF", "101", "42").
 * @param base_from The base of the input number string (2-36).
 * @param base_to The target base for the output string (2-36).
 * @return The converted number string in the target base.
 * @throws std::invalid_argument if bases are out of range (2-36),
 *         if value_str contains invalid characters for base_from,
 *         or if value_str is empty.
 * @throws std::out_of_range if the number is too large to fit in a long long during conversion.
 */
std::string convert_base(const std::string& value_str, int base_from, int base_to);

} // namespace BaseConverter 