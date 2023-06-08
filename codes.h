#pragma once
#include <cstdint>
#include <string>

namespace Opcode
{
	// 4 bits
	inline constexpr std::uint8_t mov_imm_to_reg{ 0x0B };

	// 6 bits
	inline constexpr std::uint8_t mov_regmem_to_regmem{ 0x22 };

	// 7 bits
	inline constexpr std::uint8_t mov_imm_to_regmem{ 0x63 }; 
	inline constexpr std::uint8_t mov_mem_to_acc{ 0x50 };
	inline constexpr std::uint8_t mov_acc_to_mem{ 0x51 };

	// 8 bits
	inline constexpr std::uint8_t mov_regmem_to_seg{ 0x8E };
	inline constexpr std::uint8_t mov_seg_to_regmem{ 0x8C };
}

inline constexpr size_t register_array_size{ 8 };

namespace Reg
{
	inline constexpr const char* w0[register_array_size]
		{ "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
	inline constexpr const char* w1[register_array_size]
		{ "ax", "cx", "dx", "bx", "sp", "bp", "si", "bi" };
}

namespace RegMem
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

