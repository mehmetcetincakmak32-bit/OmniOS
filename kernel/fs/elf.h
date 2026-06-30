/* OmniOS — kernel/fs/elf.h */
/* ELF64 definitions */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_ELF_H
#define OMNIOS_ELF_H

#include <stdint.h>
#include <stddef.h>

#define ELF_MAGIC       0x464C457F
#define ELFCLASS64      2
#define ET_EXEC         2
#define ET_DYN          3
#define PT_LOAD         1

typedef struct {
    uint32_t magic;
    uint8_t  _class;
    uint8_t  data;
    uint8_t  version;
    uint8_t  osabi;
    uint8_t  abiversion;
    uint8_t  pad[7];
    uint16_t type;
    uint16_t machine;
    uint32_t version2;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __attribute__((packed)) elf64_header_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint64_t align;
} __attribute__((packed)) elf64_phdr_t;

typedef struct {
    void     *data;
    size_t    size;
    uint64_t  entry;
    uintptr_t load_base;
} elf_loaded_t;

int elf_load(const void *binary, size_t size, elf_loaded_t *out);

#endif
