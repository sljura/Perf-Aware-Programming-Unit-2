#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <sstream>
#include <utility>

#include <filesystem>
#include <stdexcept>
// So many includes.
// I shoulda written this in C.

#include "get_bits.h"
#include "codes.h"

void prompt(std::string_view message);

inline std::string get_reg_name(std::uint8_t bits, bool w_bit);
inline std::string get_rm_name(std::uint8_t rm_bits);
inline std::string displacement_str(std::uint16_t byte, std::uint8_t rm_bits) noexcept;

void prompt(std::string_view message)
{
	std::cout << ">> DECODER: " << message << '\n';
}

inline std::string get_reg_name(std::uint8_t reg_bits, bool w_bit)
{
	if (reg_bits >= register_array_size)
	{
		throw std::out_of_range{ "no such register" };
	}
	return w_bit ? Reg::w1[reg_bits] : Reg::w0[reg_bits];
}

inline std::string get_rm_name(std::uint8_t rm_bits)
{
	if (rm_bits >= register_array_size)
	{
		throw std::out_of_range{ "no such register" };
	}
	return RegMem::regs[rm_bits];
}

inline std::string displacement_str(std::uint16_t byte, std::uint8_t rm_bits) noexcept
{
	if (byte)
	{
		return '[' + get_rm_name(rm_bits) + " + " + std::to_string(byte) + ']';
	}
	else
	{
		return '[' + get_rm_name(rm_bits) + ']';
	}
}

std::uint16_t get_literal(std::ifstream& input, bool is_16bits)
{
	std::uint8_t byte;
	input >> byte;
	if (!input)
	{
		throw std::runtime_error("File stream bad");
	}
	std::uint16_t final_number{ static_cast<std::uint16_t>(byte) };

	if (is_16bits)
	{
		input >> byte;
		if (!input)
		{
			throw std::runtime_error("File stream bad");
		}
		final_number = final_number | (static_cast<std::uint16_t>(byte) << 8);
	}

	return final_number;
}

std::string mov_imm_to_reg(std::uint8_t byte, std::ifstream& input)
{
	try
	{
		bool w_bit{ bool_bit_3(byte) };
		return "mov " + get_reg_name(get_lsb_3(byte), w_bit) + 
			", " + std::to_string(get_literal(input, w_bit)) + '\n';
	}
	catch (...)
	{
		return "";
	}
}

std::string mov_regmem_to_regmem(std::uint8_t byte, std::ifstream& input)
{
	try
	{
		std::uint8_t byte_two;
		input >> byte_two;
		if (!input)
		{
			return "";
		}

		std::string source{};
		std::string destination{};

		bool d_bit{ bool_bit_1(byte) };
		bool w_bit{ bool_bit_0(byte) };

		std::uint8_t rm_bits{ get_lsb_3(byte_two) };
		std::string reg_name{ get_reg_name(get_bits345(byte_two), w_bit) };
		std::uint8_t mod{ get_msb_2(byte_two) };

		if (mod == Mod::reg)
		{
			source = d_bit ? get_reg_name(rm_bits, w_bit) : reg_name;
			destination = d_bit ? reg_name : get_reg_name(rm_bits, w_bit);
		}
		else 
		{
			std::uint16_t literal;
			if (mod == Mod::mem_no_displace && rm_bits == RegMem::direct_address)
			{
				// special case
			}
			else
			{
				switch (mod)
				{
				case Mod::mem_no_displace:
					literal = 0U;
					break;
				case Mod::mem_8bit_displace:
					literal = get_literal(input, false);
					break;
				case Mod::mem_16bit_displace:
					literal = get_literal(input, true);
				default:
					return "";
				}

				destination = (d_bit ? reg_name : displacement_str(literal, rm_bits));
				source = (d_bit ? displacement_str(literal, rm_bits) : reg_name);
			}
		}

		return { "mov " + destination + ", " + source + '\n' };
	}
	catch (...)
	{
		return "";
	}
}

//It's in (probably not great) C++, so it reads from the ifstream into an `std::uint8_t` variable. 

std::string decoded_file_str(std::ifstream& input, std::uintmax_t file_size)
{
	std::string input_str;

	// Reserve space to prevent costly reallocations. Assume
	// 3 byte instructions on average, and one displacement
	// per instruction.
	// 
	// Each instruction thus has:
	//     * one three char instruction (3)
	//     * a space (1)
	//     * a two char register name (2)
	//     * a comma and a space (2)
	//     * a bracket, two char register name, space, plus sign, space, two char
	//       displacement, and bracket (9)
	//     * a newline (1)
	// Total chars (and bytes): 18
	//
	// If the string ends up being bigger, it can be reallocated.
	// These averages are guesses as to what's most likely to be
	// the mean space required per instruction, and may be wrong.
	// (Okay, they're probably wrong.)
	input_str.reserve((file_size / 3) * 18);
	
	std::uint8_t byte;
	//size_t counter{ 0 };
	while (input >> byte)
	{
		/*
		// Little debug helper:
		std::cout << "Byte " << counter << ": " << byte << ' ' << std::to_string(byte) << '\n';
		++counter;
		*/

		// Check longer opcodes first.

		std::uint8_t masked_byte{ static_cast<std::uint8_t>(byte >> 2) };

		if (masked_byte == Opcode::mov_regmem_to_regmem)
		{
			input_str.append(mov_regmem_to_regmem(byte, input));
			continue;
		}

		masked_byte = masked_byte >> 2;
		if (masked_byte == Opcode::mov_imm_to_reg)
		{
			input_str.append(mov_imm_to_reg(byte, input));
			continue;
		}
	}

	// Remove final newline character.
	input_str.pop_back();
	return input_str;
}

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		prompt("No files given.");
		return 0;
	}

	for (int count{ 1 }; count < argc; ++count)
	{
		std::string file_name{ argv[count] };
		std::filesystem::path file_path{ file_name };
		if (!std::filesystem::exists(file_path))
		{
			prompt(file_name + " not found.");
			continue;
		}

		std::uintmax_t file_size{ std::filesystem::file_size(file_path) };
		std::ifstream input;
		input.open(file_name, std::ios::binary);
		if (!file_size || !input)
		{
			prompt("Couldn't open " + file_name + " for reading.");
		}

		std::string message{ "Decoding " + file_name + "..." };

		// Tidy up name for output file.
		size_t pos{ file_name.find_last_of('.') };
		if (pos != std::string::npos)
		{
			file_name = file_name.substr(0, pos);
		}

		std::ofstream output{ file_name.append("_decode.txt") };
		if (!output)
		{
			prompt("Couldn't open " + file_name + " for writing decoded instructions.");
			continue;
		}

		prompt(message);
		output << std::move(decoded_file_str(input, file_size));
		prompt("Decoded.");
	}

	return 0;
}