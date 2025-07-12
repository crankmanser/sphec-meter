#pragma once
#include <string>
#include <vector> // Required for std::vector in base64_decode return type
#include <cctype> // Added for isalnum

// Standard Base64 encoding and decoding functions.
// Sourced conceptually from https://stackoverflow.com/a/34571089
// No external libraries needed.

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

// Declaration of encoding function
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

// Declaration of decoding function
std::vector<uint8_t> base64_decode(const std::string& encoded_string);