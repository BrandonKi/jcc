#pragma once

#include "jb.h"

#include <vector>
#include <array>
#include <chrono>

namespace jb {

enum class MachineType : u16 {
    unknown = 0x0,
    am33 = 0x1d3,
    amd64 = 0x8664,
    arm = 0x1c0,
    arm64 = 0xaa64,
    armnt = 0x1c4,
    ebc = 0xebc,
    i386 = 0x14c,
    ia64 = 0x200,
    loongarch32 = 0x6232,
    loongarch64 = 0x6264,
    m32r = 0x9041,
    mips16 = 0x266,
    mipsfpu = 0x366,
    mipsfpu16 = 0x466,
    powerpc = 0x1f0,
    powerpcfp = 0x1f1,
    r4000 = 0x166,
    riscv32 = 0x5032,
    riscv64 = 0x5064,
    riscv128 = 0x5128,
    sh3 = 0x1a2,
    sh3dsp = 0x1a3,
    sh4 = 0x1a6,
    sh5 = 0x1a8,
    thumb = 0x1c2,
    wcemipsv2 = 0x169,
};

enum class CharacteristicFlag : u16 {
    relocs_stripped = 0x0001,
    executable_image = 0x0002,
    line_nums_stripped = 0x0004,
    local_syms_stripped = 0x0008,
    aggressive_ws_trim = 0x0010,
    large_address_aware = 0x0020,
    reserved = 0x0040,
    bytes_reversed_lo = 0x0080,
    _32bit_machine = 0x0100,
    debug_stripped = 0x0200,
    removable_run_from_swap = 0x0400,
    net_run_from_swap = 0x0800,
    system = 0x1000,
    dll = 0x2000,
    up_system_only = 0x4000,
    bytes_reversed_hi = 0x8000,
};

struct CharacteristicFlags {
    u16 value = 0;

    template <typename... Characteristic>
    CharacteristicFlags(Characteristic... flags) : value{0 & flags...} {}
};

constexpr int COFF_HEADER_SIZE = 20;
struct CoffHeader {
    MachineType machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header;
    CharacteristicFlags characteristics;

    void serialize(std::vector<byte> &);
};

enum SectionFlag : u32 {
    reserved_1 = 0x00000000,
    reserved_2 = 0x00000001,
    reserved_3 = 0x00000002,
    reserved_4 = 0x00000004,
    type_no_pad = 0x00000008,
    reserved_5 = 0x00000010,
    cnt_code = 0x00000020,
    cnt_initialized_data = 0x00000040,
    cnt_uninitialized_data = 0x00000080,
    lnk_other = 0x00000100,
    lnk_info = 0x00000200,
    reserved_6 = 0x00000400,
    lnk_remove = 0x00000800,
    lnk_comdat = 0x00001000,
    gprel = 0x00008000,
    mem_purgeable = 0x00020000,
    mem_16bit = 0x00020000,
    mem_locked = 0x00040000,
    mem_preload = 0x00080000,
    align_1bytes = 0x00100000,
    align_2bytes = 0x00200000,
    align_4bytes = 0x00300000,
    align_8bytes = 0x00400000,
    align_16bytes = 0x00500000,
    align_32bytes = 0x00600000,
    align_64bytes = 0x00700000,
    align_128bytes = 0x00800000,
    align_256bytes = 0x00900000,
    align_512bytes = 0x00a00000,
    align_1024bytes = 0x00b00000,
    align_2048bytes = 0x00c00000,
    align_4096bytes = 0x00d00000,
    align_8192bytes = 0x00e00000,
    lnk_nreloc_ovfl = 0x01000000,
    mem_discardable = 0x02000000,
    mem_not_cached = 0x04000000,
    mem_not_paged = 0x08000000,
    mem_shared = 0x10000000,
    mem_execute = 0x20000000,
    mem_read = 0x40000000,
    mem_write = 0x80000000,
};

struct SectionFlags {
    u32 value = 0;

    template <typename... SectionFlag>
    SectionFlags(SectionFlag... flags) : value{(0 | ... | flags)} {}
};

constexpr int SECTION_TABLE_ENTRY_SIZE = 40;
struct SectionTableEntry {
    char name[8] = {0};
    u32 virtual_size = 0;
    u32 virtual_address = 0;
    u32 size_of_raw_data = 0;
    u32 pointer_to_raw_data = 0;
    u32 pointer_to_relocations = 0;
    u32 pointer_to_linenumbers = 0;
    u16 number_of_relocations = 0;
    u16 number_of_linenumbers = 0;
    SectionFlags characteristics{};

    void serialize(std::vector<byte> &);
};

struct Name {
    u32 length;
    union {
        char short_name[8] = {0};
        struct {
            u32 zeroes;
            u32 offset;
        };
    };
};

enum class StorageClass : u8 {
    null = 0,
    external = 2,
    _static = 3,
    function = 101,
};

enum class SectionNumber : i16 {
    undefined = 0,
    absolute = -1,
    debug = -2,
};

enum class CoffRelocType : u16 {
    // amd64 relocs
    amd64_absolute = 0x0000,
    amd64_addr64 = 0x0001,
    amd64_addr32 = 0x0002,
    amd64_addr32nb = 0x0003,
    amd64_rel32 = 0x0004,
    amd64_rel32_1 = 0x0005,
    amd64_rel32_2 = 0x0006,
    amd64_rel32_3 = 0x0007,
    amd64_rel32_4 = 0x0008,
    amd64_rel32_5 = 0x0009,
    amd64_section = 0x000a,
    amd64_secrel = 0x000b,
    amd64_secrel7 = 0x000c,
    amd64_token = 0x000d,
    amd64_srel32 = 0x000e,
    amd64_pair = 0x000f,
    amd64_sspan32 = 0x0010,

    // arm relocs
    arm_absolute = 0x0000,
    arm_addr32 = 0x0001,
    arm_addr32nb = 0x0002,
    arm_branch24 = 0x0003,
    arm_branch11 = 0x0004,
    arm_rel32 = 0x000a,
    arm_section = 0x000e,
    arm_secrel = 0x000f,
    arm_mov32 = 0x0010,

    // arm64 relocs
    arm64_absolute = 0x0000,
    arm64_addr32 = 0x0001,
    arm64_addr32nb = 0x0002,
    arm64_branch26 = 0x0003,
    arm64_pagebase_rel21 = 0x0004,
    arm64_rel21 = 0x0005,
    arm64_pageoffset_12a = 0x0006,
    arm64_pageoffset_12l = 0x0007,
    arm64_secrel = 0x0007,
    arm64_secrel_low12a = 0x0008,
    arm64_secrel_high12a = 0x0009,
    arm64_secrel_low12l = 0x000a,
    arm64_token = 0x000c,
    arm64_section = 0x000d,
    arm64_addr64 = 0x000e,
    arm64_branch19 = 0x000f,
    arm64_branch14 = 0x0010,
    arm64_rel32 = 0x0011,
};

struct CoffReloc {
    u32 virtual_address;
    u32 symbol_table_index;
    CoffRelocType reloc_type;

    void serialize(std::vector<byte> &);
};

enum class AuxType {
    function_definition,
    begin_end_function,
    weak_externals,
    files,
    section_definition,
};

struct AuxSymbolTableEntry {
    AuxType type;

    union {
        struct {
            u32 length;
            u16 number_of_relocations;
            u16 number_of_linenumbers;
            u32 check_sum;
            u16 number;
            u8 selection;
            u16 unused_1;
            u8 unused_2;
        } section_definition;
    };

    void serialize(std::vector<byte> &);
};

enum class CoffSymbolType : u16 {
    not_function = 0x0,
    function = 0x20,
};

constexpr int SYMBOL_TABLE_ENTRY_SIZE = 18;
struct SymbolTableEntry {
    Name name;
    u32 value = 0;
    i16 section_number = (i16)SectionNumber::undefined;
    CoffSymbolType type = CoffSymbolType::not_function;
    StorageClass storage_class = StorageClass::null;
    u8 number_of_aux_symbols = 0;

    std::vector<AuxSymbolTableEntry> aux_symbols = {};

    void serialize(std::vector<byte> &);
};

struct StringTable {
    u32 size;
    std::vector<std::string> strings;

    void add_string(std::string &);
};

class Coff {
public:
    void serialize(std::string dir = "");

    CoffHeader header;
    std::vector<SectionTableEntry> section_table;
    std::vector<byte> raw_data;
    std::vector<CoffReloc> relocs;
    std::vector<SymbolTableEntry> symbol_table;
    StringTable string_table;

private:
    void patch_section_offsets();
    void serialize_string(std::vector<byte> &, std::string &);
};

Coff to_coff(BinaryFile *);

} // namespace jb
