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

struct SecondByte;
class Displacement;
class Instruction;

void prompt(std::string_view message);

inline std::string get_reg_name(std::uint8_t bits, bool w_bit);
inline std::string get_rm_name(std::uint8_t rm_bits);

Displacement get_displacement(Instruction& instruction,
	SecondByte& byte_two);

struct SecondByte
{
	SecondByte(std::uint8_t byte, bool use_reg = true) noexcept :
		_mod{ get_msb_2(byte) }, _reg{ use_reg ? get_bits_345(byte) : 0U },
		_rm{ get_lsb_3(byte) }
	{ }

	const std::uint8_t _mod{ 0U };
	const std::uint8_t _reg{ 0U };
	const std::uint8_t _rm{ 0U };
};

class Displacement
{
public:
	std::string formatted_str()
	{
		return '[' + _str + ']';
	}

	std::uint8_t _size_increase{ 0 };
	std::string _str{ };
};

class Instruction
{
public:
	Instruction(std::ifstream& input) noexcept :
		_input{ input }, _starting_pos{ input.tellg() },
		_size{ 1U }, _byte_one{ 0x0 }
	{ }

	~Instruction()
	{
		_input.seekg(_starting_pos + std::streampos{ _size });
	}

	std::uint8_t get_byte(int byte_num)
	{
		_input.seekg(_starting_pos + std::streampos{ byte_num });
		std::uint8_t byte;
		_input >> byte;
		if (!_input)
		{
			throw std::runtime_error("input stream error");
		}
		return byte;
	}

	void reset()
	{
		_starting_pos = _starting_pos + std::streampos{ _size };
		_input.seekg(_starting_pos);
		_size = 1U;
	}

	std::uint8_t _byte_one;
	std::ifstream& _input;
	std::streampos _starting_pos;
	std::uint8_t _size;
};



// all these are fine until the next divider
void prompt(std::string_view message)
{
	std::cout << ">> DECODER: " << message << '\n';
}


inline std::string get_reg_name(std::uint8_t reg_bits, bool w_bit)
{
	if (reg_bits >= register_array_size)
	{
		throw std::out_of_range{ "get_reg_name(): no such register" };
	}
	return w_bit ? Reg::w1[reg_bits] : Reg::w0[reg_bits];
}

inline std::string get_rm_name(std::uint8_t rm_bits)
{
	if (rm_bits >= register_array_size)
	{
		throw std::out_of_range{ "get_reg_name(): no such register" };
	}
	return RM::regs[rm_bits];
}


// -------------------------------------------------------------------------------------



std::uint16_t get_16bit_literal(Instruction& instruction,
	int literal_start)
{
	std::uint8_t byte{ instruction.get_byte(literal_start) };
	const std::streampos offset{ literal_start + std::streampos{ 1 } };
	return (static_cast<std::uint16_t>(instruction.get_byte(offset) << 8) |
		byte);
}

inline std::uint8_t get_8bit_literal(Instruction& instruction,
	int literal_start)
{
	return instruction.get_byte(literal_start);
}

// This is, frankly, unnecessary for our purposes at the moment.
inline std::uint16_t get_sign_extended_8bit_literal(Instruction& instruction,
	int literal_start)
{
	std::uint8_t byte{ instruction.get_byte(literal_start) };
	return static_cast<std::uint16_t>(static_cast<std::int16_t>(byte));
}

Displacement get_displacement(Instruction& instruction,
	const SecondByte& byte_two)
{
	Displacement displacement{};

	switch (byte_two._mod)
	{
	case Mod::mem_no_displace:
	{
		if (byte_two._rm == RM::direct_address)
		{
			displacement._size_increase += 2;
			displacement._str = std::to_string(get_16bit_literal(instruction, 2));
		}
		else
		{
			displacement._str = get_rm_name(byte_two._rm);
		}
		break;
	}
	case Mod::mem_8bit_displace:
	{
		displacement._size_increase += 1;
		displacement._str = get_rm_name(byte_two._rm) + " + " +
			std::to_string(get_8bit_literal(instruction, 2));
		break;
	}
	case Mod::mem_16bit_displace:
	{
		displacement._size_increase += 2;
		displacement._str = get_rm_name(byte_two._rm) + " + " +
			std::to_string(get_16bit_literal(instruction, 2));
		break;
	}
	default:
		throw std::runtime_error("bad mod field");
	}

	return displacement;
}

std::string op_rm_to_rm(Instruction& instruction,
	const std::string& opname)
{
	const SecondByte byte_two{ instruction.get_byte(1) };
	instruction._size += 1;

	std::string source{};
	std::string destination{};

	bool d_bit{ bool_bit_1(instruction._byte_one) };
	bool w_bit{ bool_bit_0(instruction._byte_one) };

	std::string reg_name{ get_reg_name(byte_two._reg, w_bit) };

	if (byte_two._mod == Mod::reg)
	{
		instruction._size = 2;
		source = d_bit ? get_reg_name(byte_two._rm, w_bit) : std::move(reg_name);
		destination = d_bit ? std::move(reg_name) : get_reg_name(byte_two._rm, w_bit);
	}
	else
	{
		Displacement displacement{ std::move(get_displacement(instruction, byte_two)) };
		instruction._size += displacement._size_increase;
		destination = (d_bit ? std::move(reg_name) : 
			displacement.formatted_str());
		source = (d_bit ? displacement.formatted_str() :
			std::move(reg_name));
	}

	return opname + ' ' + destination + ", " + source + '\n';
}

std::string mov_imm_to_reg(Instruction& instruction)
{
	bool w_bit{ bool_bit_3(instruction._byte_one) };
	instruction._size = (w_bit ? 3 : 2);
	
	std::uint16_t immediate{ static_cast<std::uint16_t>(w_bit ? 
		get_16bit_literal(instruction, 1) :
		get_8bit_literal(instruction, 1)) };

	return "mov " + get_reg_name(get_lsb_3(instruction._byte_one), w_bit) +
			", " + std::to_string(immediate) + '\n';
}

std::string op_imm_to_rm(Instruction& instruction)
{
	bool s_bit{ bool_bit_1(instruction._byte_one) };
	bool w_bit{ bool_bit_0(instruction._byte_one) };
	const SecondByte byte_two{ instruction.get_byte(1) };

	std::string opname;
	switch (byte_two._reg)
	{
	case Reg::opname_add:
		opname = "add ";
		break;
	case Reg::opname_sub:
		opname = "sub ";
		break;
	case Reg::opname_cmp:
		opname = "cmp ";
		break;
	default:
		throw std::runtime_error("bad mod field");
	}
	
	std::string destination;
	unsigned int immediate_start_pos;
	if (byte_two._mod == Mod::reg)
	{
		destination = get_reg_name(byte_two._rm, w_bit);
		immediate_start_pos = 2;
		instruction._size = (w_bit ? 3 : 2);
	}
	else
	{
		Displacement displacement{ std::move(get_displacement(instruction, byte_two)) };
		instruction._size += displacement._size_increase + (w_bit ? 2 : 1);
		immediate_start_pos = 2 + displacement._size_increase;

		// Word or byte only goes in front of moving immediates to an address
		destination = (w_bit ? "word " : "byte ") + displacement.formatted_str();
	}

	std::uint16_t immediate{ (!s_bit && w_bit) ?
		get_16bit_literal(instruction,
			std::streampos{ immediate_start_pos }) :
		get_sign_extended_8bit_literal(instruction,
			std::streampos{ immediate_start_pos }) };

	return opname + destination + ", " + std::to_string(immediate) + '\n';
}

std::string mov_imm_to_rm(Instruction& instruction)
{
	bool w_bit{ bool_bit_0(instruction._byte_one) };
	const SecondByte byte_two{ instruction.get_byte(2) };
	Displacement displacement{ std::move(get_displacement(instruction, byte_two)) };
	instruction._size += displacement._size_increase + (w_bit ? 2 : 1);

	std::uint16_t immediate{ static_cast<std::uint16_t>(w_bit ? get_16bit_literal(instruction,
		2U + displacement._size_increase) :
		get_8bit_literal(instruction, 2 + displacement._size_increase)) };

	return "mov " + displacement.formatted_str() + ", " +
		std::to_string(immediate) + '\n';
}

std::string mov_accumulator(Instruction& instruction)
{
	bool to_mem{ bool_bit_1(instruction._byte_one) };
	bool w_bit{ bool_bit_0(instruction._byte_one) };
	instruction._size = (w_bit ? 3 : 2);

	std::string address{ '[' + 
		std::to_string(w_bit ? get_16bit_literal(instruction,
		1) :
		get_8bit_literal(instruction, 1))
		+ ']' };

	return "mov " + (to_mem ? address : "ax") +
		", " + (to_mem ? "ax" : address) + '\n';
}

std::string op_imm_to_acc(Instruction& instruction,
	const std::string& opname)
{
	bool w_bit{ bool_bit_0(instruction._byte_one) };
	instruction._size = (w_bit ? 3 : 2);
	std::uint16_t immediate{ static_cast<std::uint16_t>(w_bit ? 
		get_16bit_literal(instruction, 1) :
		get_8bit_literal(instruction, 1)) };

	return opname + (w_bit ? " ax, " : " al, ") + 
		std::to_string(immediate) + '\n';
}

std::string conditional_jump(Instruction& instruction,
	const std::string& opname)
{
	instruction._size = 2;
	std::int8_t displacement{ static_cast<std::int8_t>(instruction.get_byte(1)) };

	return opname + ' ' + std::to_string(displacement) + '\n';
}

std::string decode_instruction(Instruction& instruction)
{
	try
	{
		// 4 bit opcodes
		if (static_cast<std::uint8_t>(instruction._byte_one & 0xF0) == Opcode::mov_imm_to_reg)
		{
			return mov_imm_to_reg(instruction);
		}

		// sub imm from acc needs to be added

		// 6 bit opcodes
		switch (static_cast<std::uint8_t>(instruction._byte_one & 0xFC))
		{
		// movs
		case Opcode::mov_rm_to_rm:
			return op_rm_to_rm(instruction, "mov");
		case Opcode::mov_accumulator:
			return mov_accumulator(instruction);

		// add/sub/cmp rm_to_rm
		case Opcode::add_rm_to_rm:
			return op_rm_to_rm(instruction, "add");
		case Opcode::sub_rm_from_rm:
			return op_rm_to_rm(instruction, "sub");
		case Opcode::cmp_rm_to_rm:
			return op_rm_to_rm(instruction, "cmp");

		// add/sub/cmp immediate to rm
		case Opcode::op_imm_to_rm:
			return op_imm_to_rm(instruction);
		}

		// 7 bit opcodes
		switch (static_cast<std::uint8_t>(instruction._byte_one & 0xFE))
		{
		case Opcode::mov_imm_to_rm:
			return mov_imm_to_rm(instruction);
		case Opcode::add_imm_to_acc:
			return op_imm_to_acc(instruction, "add");
		case Opcode::sub_imm_from_acc:
			return op_imm_to_acc(instruction, "sub");
		case Opcode::cmp_imm_acc:
			return op_imm_to_acc(instruction, "cmp");
		}

		switch (instruction._byte_one)
		{
		case Opcode::jnz:
			return conditional_jump(instruction, "jnz");
		}

		return "";
	}
	catch (...)
	{
		return "";
	}
}



std::string decoded_file_str(std::ifstream& input, std::uintmax_t file_size)
{
	std::string input_str;
	// Reserve space to prevent costly reallocations. Assume
	// 3 byte instructions on average, and 20 characters per 
	// decoded instruction.
	input_str.reserve((file_size / 3) * 20);
	
	Instruction instruction{ input };

	while(input >> instruction._byte_one)
	{
		input_str.append(decode_instruction(instruction));
		instruction.reset();
	}

	// Remove final newline character.
	if (!input_str.empty())
	{
		input_str.pop_back();
	}
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
		input.unsetf(std::ios_base::skipws);
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