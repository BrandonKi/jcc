#include "arch/x86_64/encode.h"

#include "arch/x86_64/pretty_print.h"

using namespace jab;
using namespace x86_64;

// signifies an opcode extension
#define EXT(x) x

Encoder::Encoder(MCModule* module): module{module} {

}

// TODO remove, this is just for debugging
std::vector<byte> Encoder::raw_bin() {
	std::vector<byte> buf;
	for(auto* fn: module->functions)
		encode_function(buf, fn);

	pretty_print(buf);
	return buf;
}

void Encoder::encode_function(std::vector<byte>& buf, MCFunction* fn) {
	for(auto inst: fn->insts) {
		encode_inst(buf, inst);
	}
}

void Encoder::encode_inst(std::vector<byte>& buf, MCInst inst) {
	using enum Opcode;

	switch(inst.op) {
		case mov:
			encode_mov(buf, inst.reg1, inst.reg2);
			return;
		case mov_reg_imm:
			encode_mov_reg_imm(buf, inst.reg1, inst.extra.imm);
			return;
		case mov_reg_scale:
			encode_mov_reg_scale(buf);
			return;
		case mov_scale_imm:
			encode_mov_scale_imm(buf);
			return;
		case mov_mem_imm:
			encode_mov_mem_imm(buf);
			return;
		case mov_index_imm:
			encode_mov_index_imm(buf);
			return;
		case cmov:
			encode_cmov(buf, inst.reg1, inst.reg2, inst.extra.cond);
			return;

		case add:
			encode_add(buf, inst.reg1, inst.reg2);
			return;
		case add_reg_imm:
			encode_add_reg_imm(buf, inst.reg1, inst.extra.imm);
			return;
		case add_reg_scale:
			encode_add_reg_scale(buf);
			return;
		case add_scale_imm:
			encode_add_scale_imm(buf);
			return;
		case add_mem_imm:
			encode_add_mem_imm(buf);
			return;
		case add_index_imm:
			encode_add_index_imm(buf);
			return;

		case call:
			encode_call(buf);
			return;
		case jmp:
			encode_jmp(buf);
			return;
		case ret:
			encode_ret(buf);
			return;
		
		case push:
			encode_push(buf, inst.reg1);
			return;
		case push_mem:
			encode_push_mem(buf);
			return;
		case push_imm:
			encode_push_imm(buf, inst.extra.imm);
			return;
		case pop:
			encode_pop(buf, inst.reg1);
			return;
		case pop_mem:
			encode_pop_mem(buf);

		case syscall:
			encode_syscall(buf);
			return;
		case breakpoint:
			encode_breakpoint(buf);
			return;
		case nop:
			encode_nop(buf, 1);
			return;
		default:
			unreachable
	}
}

void Encoder::encode_mov(
	std::vector<byte>& buf,
	Register dest,
	Register src
) {
	assert(size(dest) >= size(src));

	auto rex_prefix = get_rex_prefix(dest, src);
	
	switch(size(dest)) {
		case 8:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x88);
			emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x89);
			emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 32:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x89);
			emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 64:
			emit<byte>(buf, rex_w | rex_prefix);
			emit<byte>(buf, 0x89);
			emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 128:
		case 256:
		default:
			unreachable
	}
}

// TODO pick the smallest immediate encoding
void Encoder::encode_mov_reg_imm(std::vector<byte>& buf, Register dest, i64 imm) {
	auto rex_prefix = get_rex_prefix_dest(dest);

	switch(size(dest)) {
		case 8:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0xb0 | id(dest));
			emit<i8>(buf, imm);
			return;
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0xb8 | id(dest));
			emit<i16>(buf, imm);
			return;
		case 32:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0xb8 | id(dest));
			emit<i32>(buf, imm);
			return;
		case 64:
			emit<byte>(buf, rex_w | rex_prefix);
			emit<byte>(buf, 0xb8 | id(dest));
			emit<i64>(buf, imm);
			return;
		case 128:
		case 256:
		default:
			unreachable
	}
}

void Encoder::encode_mov_reg_scale(std::vector<byte>& buf) {

}

void Encoder::encode_mov_scale_imm(std::vector<byte>& buf) {

}

void Encoder::encode_mov_mem_imm(std::vector<byte>& buf) {

}

void Encoder::encode_mov_index_imm(std::vector<byte>& buf) {

}

static byte get_cmov_opcode(Condition cond) {
	using enum Condition;
	
	switch(cond) {
		case above:
			return 0x47;
		case above_equal:
			return 0x43;
		case below:
			return 0x42;
		case below_equal:
			return 0x46;
		case carry:
			return 0x42;
		case equal:
			return 0x44;
		case greater:
			return 0x4f;
		case greater_equal:
			return 0x4d;
		case lesser:
			return 0x4c;
		case lesser_equal:
			return 0x4e;
		default:
			unreachable
			return -1;
	}
} 

void Encoder::encode_cmov(
	std::vector<byte>& buf,
	Register dest,
	Register src,
	Condition cond
) {

	auto rex_prefix = get_rex_prefix(dest, src);
	auto op = get_cmov_opcode(cond);

	switch(size(dest)) {
		case 8:
			unreachable
		case 16:
		case 32:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x0f);
			emit<byte>(buf, op);
			emit<byte>(buf, modrm_direct(id(dest), id(src)));
			return;
		case 64:
			emit<byte>(buf, rex_w | rex_prefix);
			emit<byte>(buf, 0x0f);
			emit<byte>(buf, op);
			emit<byte>(buf, modrm_direct(id(dest), id(src)));
			return;
		case 128:
		case 256:
		default:
			unreachable
	}

}

void Encoder::encode_add(std::vector<byte>& buf, Register dest, Register src) {
	auto rex_prefix = get_rex_prefix(dest, src);

	switch(size(dest)) {
		case 8:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x00);
            emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x01);
            emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 32:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x01);
            emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 64:
			emit<byte>(buf, rex_w | rex_prefix);
			emit<byte>(buf, 0x01);
            emit<byte>(buf, modrm_direct(id(src), id(dest)));
			return;
		case 128:
		case 256:
		default:
			unreachable
	}

}

void Encoder::encode_add_reg_imm(std::vector<byte>& buf, Register dest, i64 imm) {
	auto rex_prefix = get_rex_prefix_dest(dest);

	switch(size(dest)) {
		case 8:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x80);
			emit<byte>(buf, modrm_direct(EXT(0), dest));
			emit<i8>(buf, imm);
			return;
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x81);
			emit<byte>(buf, modrm_direct(EXT(0), dest));
			emit<i8>(buf, imm);
			return;
		case 32:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x81);
			emit<byte>(buf, modrm_direct(EXT(0), dest));
			emit<i8>(buf, imm);	
			return;
		case 64:
			emit<byte>(buf, rex_w | rex_prefix);
			emit<byte>(buf, 0x81);
			emit<byte>(buf, modrm_direct(EXT(0), dest));
			emit<i8>(buf, imm);			
			return;
		case 128:
		case 256:
		default:
			unreachable
	}

}

void Encoder::encode_add_reg_scale(std::vector<byte>& buf) {

}

void Encoder::encode_add_scale_imm(std::vector<byte>& buf) {

}

void Encoder::encode_add_mem_imm(std::vector<byte>& buf) {

}

void Encoder::encode_add_index_imm(std::vector<byte>& buf) {

}


void Encoder::encode_call(std::vector<byte>& buf) {
	emit<byte>(buf, 0xe8);
	emit<u32>(buf, 0x0);
}

void Encoder::encode_jmp(std::vector<byte>& buf) {

}

void Encoder::encode_ret(std::vector<byte>& buf) {
	emit<byte>(buf, 0xc3);
}


void Encoder::encode_push(std::vector<byte>& buf, Register reg) {
	auto rex_prefix = get_rex_prefix_dest(reg);


	switch(size(reg)) {
		case 8:
			unreachable
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x50 + id(reg));
		case 32:
			unreachable
		case 64:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x50 + id(reg));
			return;
		case 128:
		case 256:
		default:
			unreachable
	}
}

void Encoder::encode_push_mem(std::vector<byte>& buf) {

}

// TODO use smallest immediate
void Encoder::encode_push_imm(std::vector<byte>& buf, i64 imm) {
	emit<byte>(buf, 0x68);
	emit<i32>(buf, imm);
}


void Encoder::encode_pop(std::vector<byte>& buf, Register reg) {
	auto rex_prefix = get_rex_prefix_dest(reg);

	switch(size(reg)) {
		case 8:
			unreachable
		case 16:
			emit<byte>(buf, 0x66);
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x58 + id(reg));
		case 32:
			unreachable
		case 64:
			emit_if_nz<byte>(buf, rex_prefix);
			emit<byte>(buf, 0x58 + id(reg));
			return;
		case 128:
		case 256:
		default:
			unreachable
	}
}

void Encoder::encode_pop_mem(std::vector<byte>& buf) {

}


void Encoder::encode_syscall(std::vector<byte>& buf) {
	emit<byte>(buf, 0x0f);
	emit<byte>(buf, 0x05);
}

void Encoder::encode_breakpoint(std::vector<byte>& buf) {
	emit<byte>(buf, 0xcc);
}

void Encoder::encode_nop(std::vector<byte>& buf, i64 bytes) {
	auto nop_9 = bytes / 9;
	auto nop_rest = bytes % 9;

	while(nop_9 --> 0) {	// as nop_9 goes to 0
		emit<byte>(buf, 0x66);
		emit<byte>(buf, 0x0f);
		emit<byte>(buf, 0x1f);
		emit<byte>(buf, 0x84);
		emit<byte>(buf, 0x00);
		emit<byte>(buf, 0x00);
		emit<byte>(buf, 0x00);
		emit<byte>(buf, 0x00);
		emit<byte>(buf, 0x00);
	}

	switch(nop_rest) {
		case 0:
			return;
		case 1:
			emit<byte>(buf, 0x90);			
			return;
		case 2:
			emit<byte>(buf, 0x66);			
			emit<byte>(buf, 0x90);			
			return;
		case 3:
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x00);			
			return;
		case 4:
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x40);			
			emit<byte>(buf, 0x00);			
			return;
		case 5:
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x44);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			return;
		case 6:
			emit<byte>(buf, 0x66);			
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x44);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			return;
		case 7:
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x80);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			return;
		case 8:
			emit<byte>(buf, 0x0f);			
			emit<byte>(buf, 0x1f);			
			emit<byte>(buf, 0x84);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);			
			emit<byte>(buf, 0x00);
			return;
		default:
			unreachable
	}
}

byte Encoder::get_rex_prefix_dest(Register reg) {
	return is_extended(reg) ? rex_b : 0; 
}

byte Encoder::get_rex_prefix_src(Register reg) {
	return is_extended(reg) ? rex_r : 0; 
}

byte Encoder::get_rex_prefix_index(Register reg) {
	return is_extended(reg) ? rex_x : 0; 
}

byte Encoder::get_rex_prefix(Register dest, Register src) {
	return get_rex_prefix_dest(dest) | get_rex_prefix_src(src);
}

byte Encoder::get_rex_prefix(Register dest, Register src, Register index) {
	return get_rex_prefix(dest, src) | get_rex_prefix_index(index);
}
