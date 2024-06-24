// This does not exhaustively define each operation.
// It is simply meant to save on repetitive typing.

#ifdef X

X(mov, "mov", MOD_R_RM, 2, 0x88)
X(mov_imm, "mov", RX_0, 2, 0xc6)

X(cmov, "cmov", BASIC, 0, -1)

X(add, "add", MOD_R_RM, 2, 0x00)


X(call, "call", OFFSET, 1, 0xe8)
X(jmp, "jmp", OFFSET, 1, 0xe9)
X(ret, "ret", BASIC, 0, 0xc3)


X(push, "push", REG_RM, 1, 0x50)
X(pop, "pop", REG_RM, 1, 0x58)


X(syscall, "syscall", PREFIX_0x0f, 0, 0x05)
X(breakpoint, "int3", BASIC, 0, 0xcc)
X(nop, "nop", BASIC, 0, 0x90)

#endif

/*
mov,
mov_reg_imm,
mov_reg_scale,
mov_scale_imm,
mov_mem_imm,
mov_index_imm,

cmov,

add,
add_reg_imm,
add_reg_scale,
add_scale_imm,
add_mem_imm,
add_index_imm,

call,
jmp,
ret,

push,
push_mem,
push_imm,
pop,
pop_mem,

syscall,
breakpoint,
nop,
*/
