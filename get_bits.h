#pragma once
#include <cstdint>

std::uint8_t get_msb_1(std::uint8_t byte);
std::uint8_t get_msb_2(std::uint8_t byte);
std::uint8_t get_msb_3(std::uint8_t byte);
std::uint8_t get_msb_4(std::uint8_t byte);
std::uint8_t get_msb_5(std::uint8_t byte);
std::uint8_t get_msb_6(std::uint8_t byte);
std::uint8_t get_msb_7(std::uint8_t byte);

std::uint8_t get_lsb_3(std::uint8_t byte);

bool bool_bit_0(std::uint8_t byte);
bool bool_bit_1(std::uint8_t byte);
bool bool_bit_3(std::uint8_t byte);

std::uint8_t get_bits345(std::uint8_t byte);