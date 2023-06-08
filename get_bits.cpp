#include "get_bits.h"

std::uint8_t get_msb_1(std::uint8_t byte)
{
	return byte >> 7;
}

std::uint8_t get_msb_2(std::uint8_t byte)
{
	return byte >> 6;
}

std::uint8_t get_msb_3(std::uint8_t byte)
{
	return byte >> 5;
}

std::uint8_t get_msb_4(std::uint8_t byte)
{
	return byte >> 4;
}

std::uint8_t get_msb_5(std::uint8_t byte)
{
	return byte >> 3;
}

std::uint8_t get_msb_6(std::uint8_t byte)
{
	return byte >> 2;
}

std::uint8_t get_msb_7(std::uint8_t byte)
{
	return byte >> 1;
}

std::uint8_t get_lsb_3(std::uint8_t byte)
{
	return (byte & 0x7);
}

bool bool_bit_0(std::uint8_t byte)
{
	return (byte & 0x1);
}

bool bool_bit_1(std::uint8_t byte)
{
	return (byte & 0x2);
}

bool bool_bit_3(std::uint8_t byte)
{
	return (byte & 0x8);
}

std::uint8_t get_bits345(std::uint8_t byte)
{
	return (byte & 0x38) >> 3;
}