/* Author: nibble 
 * --------------
 * Licensed under GPLv2
 * This file is part of radare
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include "dietpe.h"
#include "dietpe_static.h"
#include "dietpe_types.h"

static PE_DWord dietpe_aux_rva_to_offset(dietpe_bin *bin, PE_DWord rva) {
	pe_image_section_header *shdrp;
	PE_DWord section_base;
	int i, section_size;

	shdrp = bin->section_header;
	for (i = 0; i < bin->nt_headers->file_header.NumberOfSections; i++, shdrp++) {
		section_base = shdrp->VirtualAddress;
		section_size = shdrp->Misc.VirtualSize;
		if (rva >= section_base && rva < section_base + section_size)
			return shdrp->PointerToRawData + (rva - section_base);
	}
		
	return 0;
}

int dietpe_close(fd) {
	close(fd);

	return 0;
}

int dietpe_get_entrypoint(dietpe_bin *bin, dietpe_entrypoint *entrypoint) {
	entrypoint->rva = bin->nt_headers->optional_header.AddressOfEntryPoint;
	entrypoint->offset = dietpe_aux_rva_to_offset(bin, bin->nt_headers->optional_header.AddressOfEntryPoint);

	return 0;
}

int dietpe_get_exports(dietpe_bin *bin, int fd, dietpe_export *export) {
	PE_DWord functions_offset, names_offset, ordinals_offset, function_rva, name_rva, name_offset;
	PE_Word function_ordinal;
	dietpe_export *exportp;
	char function_name[PE_NAME_LENGTH], forwarder_name[PE_NAME_LENGTH], dll_name[PE_NAME_LENGTH], export_name[PE_NAME_LENGTH];
	int i;

	pe_image_data_directory *data_dir_export = &bin->nt_headers->optional_header.DataDirectory[PE_IMAGE_DIRECTORY_ENTRY_EXPORT];
	PE_DWord export_dir_rva = data_dir_export->VirtualAddress;
	int export_dir_size = data_dir_export->Size;
	
	if (dietpe_init_exports(bin, fd) == -1)
		return -1;
	
	lseek(fd, dietpe_aux_rva_to_offset(bin, bin->export_directory->Name), SEEK_SET);
    read(fd, dll_name, PE_NAME_LENGTH);

	functions_offset = dietpe_aux_rva_to_offset(bin, bin->export_directory->AddressOfFunctions);
	names_offset = dietpe_aux_rva_to_offset(bin, bin->export_directory->AddressOfNames);
	ordinals_offset = dietpe_aux_rva_to_offset(bin, bin->export_directory->AddressOfOrdinals);

	exportp = export;
	for (i = 0; i < bin->export_directory->NumberOfNames; i++, exportp++) {
		lseek(fd, functions_offset + i * sizeof(PE_DWord), SEEK_SET);
		read(fd, &function_rva, sizeof(PE_DWord));
		lseek(fd, ordinals_offset + i * sizeof(PE_Word), SEEK_SET);
		read(fd, &function_ordinal, sizeof(PE_Word));
		lseek(fd, names_offset + i * sizeof(PE_DWord), SEEK_SET);
		read(fd, &name_rva, sizeof(PE_DWord));
		name_offset = dietpe_aux_rva_to_offset(bin, name_rva);

		if (name_offset) {
			lseek(fd, name_offset, SEEK_SET);
			read(fd, function_name, PE_NAME_LENGTH);
		} else {
			snprintf(function_name, PE_NAME_LENGTH, "Ordinal_%i", function_ordinal);
		}
		
		snprintf(export_name, PE_NAME_LENGTH, "%s_%s", dll_name, function_name);

		if (function_rva >= export_dir_rva && function_rva < (export_dir_rva + export_dir_size)) {
			lseek(fd, dietpe_aux_rva_to_offset(bin, function_rva), SEEK_SET);
			read(fd, forwarder_name, PE_NAME_LENGTH);
		} else {
			snprintf(forwarder_name, PE_NAME_LENGTH, "NONE");
		}

		exportp->rva = function_rva;
		exportp->offset = dietpe_aux_rva_to_offset(bin, function_rva);
		exportp->ordinal = function_ordinal;
		memcpy(exportp->forwarder, forwarder_name, PE_NAME_LENGTH);
		memcpy(exportp->name, export_name, PE_NAME_LENGTH);
	}

	return 0;
}

int dietpe_get_exports_count(dietpe_bin *bin, int fd) {
	if (dietpe_init_exports(bin, fd) == -1)
		return -1;
	
	return bin->export_directory->NumberOfNames;
}

int dietpe_get_file_alignment(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.FileAlignment;
}

PE_DWord dietpe_get_image_base(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.ImageBase;
}

int dietpe_get_imports(dietpe_bin *bin, int fd, dietpe_import *import) {
	pe_image_import_directory *import_dirp;
	PE_DWord import_lookup_table;
	PE_Word import_hint, import_ordinal;
	dietpe_import *importp;
	char dll_name[PE_NAME_LENGTH], import_name[PE_NAME_LENGTH], name[PE_NAME_LENGTH];
	int import_dirs_count = dietpe_get_import_dirs_count(bin), i, j;
	
	if (dietpe_init_imports(bin, fd) == -1)
		return -1;

	import_dirp = bin->import_directory;
	importp = import;
	for (i = 0; i < import_dirs_count; i++, import_dirp++) {
		lseek(fd, dietpe_aux_rva_to_offset(bin, import_dirp->Name), SEEK_SET);
    	read(fd, dll_name, PE_NAME_LENGTH);
		for (j = 0, import_lookup_table = 1; import_lookup_table; j++) {
			lseek(fd, dietpe_aux_rva_to_offset(bin, import_dirp->Characteristics) + j * sizeof(PE_DWord), SEEK_SET);
    		read(fd, &import_lookup_table, sizeof(PE_DWord));

			if (import_lookup_table & 0x80000000) {
				import_ordinal = import_lookup_table & 0x7fffffff;
				import_hint = 0;
				snprintf(import_name, PE_NAME_LENGTH, "%s_Ordinal_%i", dll_name, import_ordinal);
			} else if (import_lookup_table) {
				import_ordinal = 0;
				lseek(fd, dietpe_aux_rva_to_offset(bin, import_lookup_table), SEEK_SET);
    			read(fd, &import_hint, sizeof(PE_Word));
    			read(fd, name, PE_NAME_LENGTH);
				snprintf(import_name, PE_NAME_LENGTH, "%s_%s", dll_name, name);
			}

			if (import_lookup_table) {
				memcpy(importp->name, import_name, PE_NAME_LENGTH);
				importp->rva = import_dirp->FirstThunk + j * sizeof(PE_DWord);
				importp->offset = dietpe_aux_rva_to_offset(bin, import_dirp->FirstThunk) + j * sizeof(PE_DWord);
				importp->hint = import_hint;
				importp->ordinal = import_ordinal;
				importp++;
			}
		}
	}
	
	return 0;
}

int dietpe_get_imports_count(dietpe_bin *bin, int fd) {
	pe_image_import_directory *import_dirp;
	PE_DWord import_lookup_table;
	int import_dirs_count = dietpe_get_import_dirs_count(bin), imports_count = 0, i, j;

	if (dietpe_init_imports(bin, fd) == -1)
		return -1;

	import_dirp = bin->import_directory;
	for (i = 0; i < import_dirs_count; i++, import_dirp++) {
		for (j = 0, import_lookup_table = 1; import_lookup_table; j++) {
			lseek(fd, dietpe_aux_rva_to_offset(bin, import_dirp->Characteristics) + j * sizeof(PE_DWord), SEEK_SET);
    		read(fd, &import_lookup_table, sizeof(PE_DWord));
			
			if (import_lookup_table)
				imports_count++;
		}
	}

	return imports_count;
}

static int dietpe_get_import_dirs_count(dietpe_bin *bin) {
	pe_image_data_directory *data_dir_import = &bin->nt_headers->optional_header.DataDirectory[PE_IMAGE_DIRECTORY_ENTRY_IMPORT];

	return (int) (data_dir_import->Size / sizeof(pe_image_import_directory) - 1);
}

int dietpe_get_image_size(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.SizeOfImage;
}

int dietpe_get_machine(dietpe_bin *bin) {
	return bin->nt_headers->file_header.Machine;
}

int dietpe_get_pe_type(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.Magic;
}

int dietpe_get_section_alignment(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.SectionAlignment;
}

int dietpe_get_sections(dietpe_bin *bin, dietpe_section *section) {
	pe_image_section_header *shdrp;
	dietpe_section *sectionp;
	int i, sections_count = dietpe_get_sections_count(bin);

	shdrp = bin->section_header;
	sectionp = section;
	for (i = 0; i < sections_count; i++, shdrp++, sectionp++) {
		memcpy(sectionp->name, shdrp->Name, PE_IMAGE_SIZEOF_SHORT_NAME);
		sectionp->rva = shdrp->VirtualAddress;
		sectionp->size = shdrp->SizeOfRawData;
		sectionp->offset = shdrp->PointerToRawData;
		sectionp->characteristics = shdrp->Characteristics;
	}

	return 0;
}

int dietpe_get_sections_count(dietpe_bin *bin) {
	return bin->nt_headers->file_header.NumberOfSections;
}

int dietpe_get_subsystem(dietpe_bin *bin) {
	return bin->nt_headers->optional_header.Subsystem;
}

static int dietpe_init(dietpe_bin *bin, int fd) {
	lseek(fd, 0, SEEK_SET);
    bin->dos_header = malloc(sizeof(pe_image_dos_header));
    read(fd, bin->dos_header, sizeof(pe_image_dos_header));

	lseek(fd, bin->dos_header->e_lfanew, SEEK_SET);
    bin->nt_headers = malloc(sizeof(pe_image_nt_headers));
    read(fd, bin->nt_headers, sizeof(pe_image_nt_headers));

	if (bin->nt_headers->file_header.SizeOfOptionalHeader != 224)
		return -1;
	
	int sections_size = sizeof(pe_image_section_header) * bin->nt_headers->file_header.NumberOfSections;
	lseek(fd, bin->dos_header->e_lfanew + sizeof(pe_image_nt_headers), SEEK_SET);
    bin->section_header = malloc(sections_size);
    read(fd, bin->section_header, sections_size);

	return 0;
}

static int dietpe_init_exports(dietpe_bin *bin, int fd) {
	pe_image_data_directory *data_dir_export = &bin->nt_headers->optional_header.DataDirectory[PE_IMAGE_DIRECTORY_ENTRY_EXPORT];
	PE_DWord export_dir_offset = dietpe_aux_rva_to_offset(bin, data_dir_export->VirtualAddress);

	if (export_dir_offset == 0)
		return -1;

	lseek(fd, export_dir_offset, SEEK_SET);
    bin->export_directory = malloc(sizeof(pe_image_export_directory));
    read(fd, bin->export_directory, sizeof(pe_image_export_directory));

	return 0;
}

static int dietpe_init_imports(dietpe_bin *bin, int fd) {
	pe_image_data_directory *data_dir_import = &bin->nt_headers->optional_header.DataDirectory[PE_IMAGE_DIRECTORY_ENTRY_IMPORT];
	PE_DWord import_dir_offset = dietpe_aux_rva_to_offset(bin, data_dir_import->VirtualAddress);
	int import_dir_size = data_dir_import->Size;

	if (import_dir_offset == 0)
		return -1;

	lseek(fd, import_dir_offset, SEEK_SET);
    bin->import_directory = malloc(import_dir_size);
    read(fd, bin->import_directory, import_dir_size);

	return 0;
}

int dietpe_open(dietpe_bin *bin, const char *file) {
    int fd;

    if ((fd = open(file, O_RDONLY)) == -1)
		return -1;
    
    if (dietpe_init(bin, fd) == -1)
		return -1;

    return fd;
}
