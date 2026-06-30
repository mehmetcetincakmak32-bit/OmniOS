/* OmniOS — kernel/fs/elf.c */
/* ELF64 loader — parse phdrs, map segments */
/* SPDX-License-Identifier: MIT */

#include "elf.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

int elf_load(const void *binary, size_t size, elf_loaded_t *out) {
    if (!binary || !out || size < sizeof(elf64_header_t)) return -1;

    const elf64_header_t *hdr = (const elf64_header_t *)binary;

    if (hdr->magic != ELF_MAGIC)     { printf("[ELF] Bad magic\n"); return -1; }
    if (hdr->_class != ELFCLASS64)   { printf("[ELF] Not 64-bit\n"); return -1; }
    if (hdr->type != ET_EXEC && hdr->type != ET_DYN) {
        printf("[ELF] Not executable\n"); return -1;
    }

    size_t phdr_size = hdr->phentsize * hdr->phnum;
    if (hdr->phoff + phdr_size > size) {
        printf("[ELF] Program headers overflow\n"); return -1;
    }

    const elf64_phdr_t *phdr = (const elf64_phdr_t *)((const uint8_t *)binary + hdr->phoff);

    uintptr_t load_base = (uintptr_t)-1;
    uintptr_t load_end  = 0;

    for (uint16_t i = 0; i < hdr->phnum; i++) {
        if (phdr[i].type != PT_LOAD) continue;

        uintptr_t start = (uintptr_t)phdr[i].vaddr;
        uintptr_t end   = start + phdr[i].memsz;

        if (start < load_base) load_base = start;
        if (end > load_end)    load_end   = end;

        if (phdr[i].offset + phdr[i].filesz > size) {
            printf("[ELF] Segment %u overflow\n", i); return -1;
        }

        const uint8_t *src = (const uint8_t *)binary + phdr[i].offset;
        uint8_t *dst = (uint8_t *)phdr[i].vaddr;

        memcpy(dst, src, phdr[i].filesz);

        if (phdr[i].memsz > phdr[i].filesz) {
            memset(dst + phdr[i].filesz, 0, phdr[i].memsz - phdr[i].filesz);
        }
    }

    out->data      = (void *)binary;
    out->size      = size;
    out->entry     = hdr->entry;
    out->load_base = load_base;

    printf("[ELF] Entry: 0x%llx  Base: 0x%lx  Size: %lu\n",
        (unsigned long long)hdr->entry, (unsigned long)load_base, (unsigned long)(load_end - load_base));
    return 0;
}
