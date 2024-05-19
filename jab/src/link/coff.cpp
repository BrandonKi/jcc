#include "link/coff.h"

// TODO move
#include "misc/crc.h"

#include <fstream>
#include <bit>
#include <filesystem>

#define raw(x) std::to_underlying(x)

using namespace jab;

static void write_file(std::string filepath, std::vector<byte>& bin) {
	auto path = std::filesystem::absolute(filepath);
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(filepath, std::ios::out|std::ios::binary|std::ios::trunc);
    if(file.is_open())
		file.write((char *)bin.data(), bin.size());
    file.close();
}


void Coff::patch_section_offsets() {
	auto raw_data_offset = COFF_HEADER_SIZE + (section_table.size() * SECTION_TABLE_ENTRY_SIZE);
	auto symtab_offset = raw_data_offset + raw_data.size();

	for(auto& section: section_table) {
		section.pointer_to_raw_data += raw_data_offset;
	}

	header.pointer_to_symbol_table = symtab_offset;
}

void Coff::serialize_string(std::vector<byte>& buffer, std::string& string) {
    auto* cstr = string.c_str();
	u32 size = string.size() + 1;
	
    buffer.reserve(buffer.size() + size);
    buffer.insert(buffer.end(), cstr, cstr + size);
}

void CoffHeader::serialize(std::vector<byte>& buffer) {
	append(buffer, raw(machine));
	append(buffer, number_of_sections);
	append(buffer, time_date_stamp);
	append(buffer, pointer_to_symbol_table);
	append(buffer, number_of_symbols);
	append(buffer, size_of_optional_header);
	append(buffer, characteristics.value);
}

void AuxSymbolTableEntry::serialize(std::vector<byte>& buffer) {
	switch(type) {
		case AuxType::function_definition:
			unreachable
		case AuxType::begin_end_function:
			unreachable
		case AuxType::weak_externals:
			unreachable
		case AuxType::files:
			unreachable
		case AuxType::section_definition:
			append(buffer, section_definition.length);
			append(buffer, section_definition.number_of_relocations);
			append(buffer, section_definition.number_of_linenumbers);
			append(buffer, section_definition.check_sum);
			append(buffer, section_definition.number);
			append(buffer, section_definition.selection);
			append(buffer, section_definition.unused_1);
			append(buffer, section_definition.unused_2);
			return;
		default:
			unreachable
	}
}

void CoffReloc::serialize(std::vector<byte>& buffer) {
	append(buffer, virtual_address);
	append(buffer, symbol_table_index);
	append(buffer, raw(reloc_type));
}

void SymbolTableEntry::serialize(std::vector<byte>& buffer) {
	if(name.length > 8) {
		append(buffer, name.zeroes);
		append(buffer, name.offset);
	}
	else {
		append(buffer, std::bit_cast<u64>(name.short_name));
	}

	append(buffer, value);
	append(buffer, section_number);
	append(buffer, raw(type));
	append(buffer, raw(storage_class));
	append(buffer, number_of_aux_symbols);

	for(auto& aux: aux_symbols) {
		aux.serialize(buffer);
	}
}

void SectionTableEntry::serialize(std::vector<byte>& buffer) {
	append(buffer, std::bit_cast<u64>(name));
	append(buffer, virtual_size);
	append(buffer, virtual_address);
	append(buffer, size_of_raw_data);
	append(buffer, pointer_to_raw_data);
	append(buffer, pointer_to_relocations);
	append(buffer, pointer_to_linenumbers);
	append(buffer, number_of_relocations);
	append(buffer, number_of_linenumbers);
	append(buffer, characteristics.value);
}

void Coff::serialize(std::string path) {
	// TODO patch relocs, string table
	patch_section_offsets();

	std::vector<byte> buffer;

	// header
	header.serialize(buffer);
	//section table
	for(auto& section: section_table)
		section.serialize(buffer);
	// raw data
	vec_append(buffer, raw_data);
	// relocations
	for(auto& reloc: relocs)
		reloc.serialize(buffer);
	// symbol table
	for(auto& symbol: symbol_table)
		symbol.serialize(buffer);
	// string table
	append(buffer, string_table.size);
	for(auto& string: string_table.strings)
        // TODO serialize the string, idk if it's null terminated or not
		// just not sure about the exact format in general
		serialize_string(buffer, string);
	
	// TODO write to file
	write_file(path, buffer);
}

static u32 get_time() {
	return static_cast<u32>(std::time(nullptr));
}

static StorageClass to_storage_class(SymbolType type) {
	switch(type) {
		case SymbolType::none:
			return StorageClass::null; 
		case SymbolType::function:
			return StorageClass::function;
		case SymbolType::label:
		case SymbolType::internal:
			return StorageClass::_static;
		case SymbolType::external:
			return StorageClass::external;
		case SymbolType::external_def:
		default:
			unreachable
	}
}

static SectionFlags get_characteristics(std::string& name) {
	if(name.starts_with(".text")) {
		return SectionFlags(
			SectionFlag::cnt_code,
			SectionFlag::align_16bytes,
			SectionFlag::mem_execute,
			SectionFlag::mem_read
		);
	} else if(name.starts_with(".data")) {
		return SectionFlags(
			SectionFlag::cnt_initialized_data,
			SectionFlag::align_16bytes,
			SectionFlag::mem_read,
			SectionFlag::mem_write
		);
	} else if(name.starts_with(".bss")) {
		return SectionFlags(
			SectionFlag::cnt_uninitialized_data,
			SectionFlag::align_16bytes,
			SectionFlag::mem_read,
			SectionFlag::mem_write
		);
	} else {
		unreachable
	}
}

void StringTable::add_string(std::string& string) {
    size += string.size() + 1;
	strings.push_back(string);
}

Coff jab::to_coff(BinaryFile* file) {
	Coff coff{};

	// FIXME get target machine type
	coff.header.machine = MachineType::amd64;
	coff.header.time_date_stamp = get_time();
	coff.header.number_of_sections = file->sections.size();
	coff.header.number_of_symbols = file->symbols.size();
	
	for(auto& section: file->sections) {
		if(section.name.size() > 8)
			unreachable	// TODO needs to point into string table

		SectionTableEntry entry{};

		std::memset(entry.name, 0, 8);
		std::memcpy(entry.name, section.name.data(), section.name.size());

		entry.virtual_size = section.virtual_size;
		entry.virtual_address = section.virtual_address;
		entry.size_of_raw_data = section.bin.size();
		// index into raw data, gets patched later
		entry.pointer_to_raw_data = coff.raw_data.size();
		entry.number_of_relocations = section.relocs.size();

		vec_append(coff.raw_data, section.bin);

		entry.characteristics = get_characteristics(section.name);
		
		
		coff.section_table.push_back(entry);
	}

	for(auto& symbol: file->symbols) {
		SymbolTableEntry entry{};

		if(symbol.name.size() > 8)
			unreachable	// TODO needs to point into string table

		entry.name.length = symbol.name.size();
		std::memset(entry.name.short_name, 0, 8);
		std::memcpy(entry.name.short_name, symbol.name.data(), symbol.name.size());

		entry.storage_class = to_storage_class(symbol.type);

		if(entry.storage_class == StorageClass::function) {
			entry.type = CoffSymbolType::function;
			entry.storage_class = StorageClass::external;
			entry.section_number = symbol.section_index;
		}
		
		if(entry.storage_class == StorageClass::_static ) {
			entry.section_number = symbol.section_index;

			auto aux = AuxSymbolTableEntry {AuxType::section_definition};
			aux.section_definition = {(u32)symbol.value, 0, 0, 0, 0, 0, 0, 0};

			aux.section_definition.check_sum = misc::crc(0, file->sections[0].bin);
			
			entry.aux_symbols.push_back(aux);
			entry.number_of_aux_symbols += 1; // nocheckin
			coff.header.number_of_symbols += 1; // nocheckin
		}
//		entry.section_number = 1; // nocheckin

		coff.symbol_table.push_back(entry);
	}

	// TODO relocs, string table
	
	return coff;
}
