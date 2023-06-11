#pragma once
#include <cstdint>
#include <string>

namespace Opcode
{
	// 4 bits
	inline constexpr std::uint8_t mov_imm_to_reg{ 0xB0 };

	// 6 bits
	inline constexpr std::uint8_t mov_rm_to_rm{ 0x88 };
	inline constexpr std::uint8_t mov_accumulator{ 0xA0 };
	inline constexpr std::uint8_t add_rm_to_rm{ 0x0 };
	inline constexpr std::uint8_t sub_rm_from_rm{ 0x28 };
	inline constexpr std::uint8_t cmp_rm_to_rm{ 0x38 };

	// for add, sub, cmp
	inline constexpr std::uint8_t op_imm_to_rm{ 0x80 };

	// 7 bits
	inline constexpr std::uint8_t mov_imm_to_rm{ 0xC6 };
	inline constexpr std::uint8_t add_imm_to_acc{ 0x04 };
	inline constexpr std::uint8_t sub_imm_from_acc{ 0x2C };
	inline constexpr std::uint8_t cmp_imm_acc{ 0x3C };

	// 8 bits
	inline constexpr std::uint8_t jnz{ 0x75 };
	inline constexpr std::uint8_t mov_rm_to_seg{ 0x8E };
	inline constexpr std::uint8_t mov_seg_to_rm{ 0x8C };
}

inline constexpr size_t register_array_size{ 8 };

namespace Reg
{
	inline constexpr const char* w0[register_array_size]
		{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
	inline constexpr const char* w1[register_array_size]
		{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" };

	inline constexpr std::uint8_t opname_add{ 0x0 };
	inline constexpr std::uint8_t opname_sub{ 0x5 };
	inline constexpr std::uint8_t opname_cmp{ 0x7 };
}

namespace RM
{
	inline constexpr const char* regs[register_array_size]
		{ "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx" };

	inline constexpr std::uint8_t direct_address{ 0x6 };
}

namespace Mod
{
	inline constexpr std::uint8_t mem_no_displace{ 0x0 };
	inline constexpr std::uint8_t mem_8bit_displace{ 0x1 };
	inline constexpr std::uint8_t mem_16bit_displace{ 0x2 };
	inline constexpr std::uint8_t reg{ 0x3 };
}

