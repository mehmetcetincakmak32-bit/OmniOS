/* OmniOS — kernel/arch/x86_64/cpu.c */
/* x86_64 CPU detection implementation */
/* SPDX-License-Identifier: MIT */

#include "cpu.h"
#include <stdio.h>
#include <string.h>

/* ── Inline CPUID helpers ────────────────────────────────────────── */

static inline void cpuid(uint32_t leaf, uint32_t *a, uint32_t *b,
        uint32_t *c, uint32_t *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(leaf), "c"(0));
}

static inline void cpuid_count(uint32_t leaf, uint32_t subleaf,
        uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    __asm__ volatile("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(leaf), "c"(subleaf));
}

static inline uint64_t read_msr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

/* ── Per-CPU array ───────────────────────────────────────────────── */

static percpu_t percpu_array[MAX_CPUS];
static uint32_t cpu_count = 0;

uint32_t cpu_get_count(void) { return cpu_count; }
uint32_t cpu_get_apic_id(uint32_t idx) {
    if (idx >= cpu_count) return 0xFFFFFFFF;
    return percpu_array[idx].apic_id;
}
percpu_t *cpu_get_percpu(uint32_t idx) {
    if (idx >= cpu_count) return NULL;
    return &percpu_array[idx];
}

/* GS-base points to current percpu block */
percpu_t *cpu_get_current(void) {
    percpu_t *p;
    __asm__ volatile("movq %%gs:0, %0" : "=r"(p));
    return p;
}

/* ── Feature detection ───────────────────────────────────────────── */

void cpu_detect_features(cpu_features_t *feat) {
    memset(feat, 0, sizeof(*feat));

    uint32_t a, b, c, d;

    /* Leaf 0: highest basic leaf + vendor */
    cpuid(0, &a, &b, &c, &d);
    uint32_t max_basic = a;

    /* Leaf 1: feature bits */
    if (max_basic >= 1) {
        cpuid(1, &a, &b, &c, &d);

        /* EDX */
        feat->fpu   = (d >> 0)  & 1;
        feat->pse   = (d >> 3)  & 1;
        feat->pae   = (d >> 6)  & 1;
        feat->pge   = (d >> 13) & 1;
        feat->mtrr  = (d >> 12) & 1;
        feat->pat   = (d >> 16) & 1;
        feat->mmx   = (d >> 23) & 1;
        feat->nx    = (d >> 20) & 1;
        feat->sse   = (d >> 25) & 1;
        feat->sse2  = (d >> 26) & 1;

        /* ECX */
        feat->sse3     = (c >> 0)  & 1;
        feat->ssse3    = (c >> 9)  & 1;
        feat->sse4_1   = (c >> 19) & 1;
        feat->sse4_2   = (c >> 20) & 1;
        feat->aes      = (c >> 25) & 1;
        feat->avx      = (c >> 28) & 1;
        feat->rdrand   = (c >> 30) & 1;
        feat->vmx      = (c >> 5)  & 1;
        feat->svm      = (c >> 2)  & 1;
        feat->smep     = (c >> 7)  & 1;
        feat->smap     = (c >> 20) & 1;
        feat->umip     = (c >> 2)  & 1;
        feat->pcid     = (c >> 17) & 1;
        feat->x2apic   = (c >> 21) & 1;
        feat->fsgsbase = (c >> 24) & 1;
        feat->pku      = (c >> 22) & 1;
        feat->ospke    = (c >> 23) & 1;
        feat->invpcid  = (c >> 10) & 1;

        /* APIC ID from EBX */
        feat->apic_id = (b >> 24) & 0xFF;
        if (feat->x2apic) {
            feat->x2apic_id = (uint32_t)read_msr(0x802); /* APIC_ID reg */
        } else {
            feat->x2apic_id = feat->apic_id;
        }
    }

    /* Leaf 4: cache topology */
    if (max_basic >= 4) {
        uint32_t cache_type, count;
        for (int i = 0; i < 4; i++) {
            cpuid_count(4, i, &a, &b, &c, &d);
            cache_type = a & 0x1F;
            count = (a >> 14) & 0xFFF;
            uint32_t size_kb = ((b >> 12) & 0xFFFFF) * (a & 0xFFF) * count / 1024;
            if (cache_type == 1) {
                feat->l1d_cache_kb = size_kb;
            } else if (cache_type == 2) {
                feat->l1i_cache_kb = size_kb;
            } else if (cache_type == 3) {
                feat->l2_cache_kb = size_kb;
            } else if (cache_type == 4) {
                feat->l3_cache_kb = size_kb;
            }
        }
        feat->cache_line_size = ((b >> 0) & 0xFFF) + 1;
    }

    /* Leaf 7 (subleaf 0): extended feature bits (AVX2, AVX-512, etc.) */
    if (max_basic >= 7) {
        cpuid_count(7, 0, &a, &b, &c, &d);
        feat->avx2    = (b >> 5)  & 1;
        feat->avx512f = (b >> 16) & 1;
        feat->avx512bw= (b >> 30) & 1;
        feat->avx512dq= (b >> 17) & 1;
        feat->avx512vl= (b >> 31) & 1;
        feat->sha     = (b >> 29) & 1;
        feat->rdseed  = (b >> 18) & 1;
        feat->hwp     = (b >> 7)  & 1;
    }

    /* Leaf 0x80000001: extended features (NX, long mode, etc.) */
    uint32_t max_ext;
    cpuid(0x80000000, &max_ext, &b, &c, &d);
    if (max_ext >= 0x80000001) {
        cpuid(0x80000001, &a, &b, &c, &d);
        (void)a;
        feat->eist   = (d >> 7)  & 1;  /* AMD: constant TSC? Actually Intel uses this for SpeedStep */
        feat->turbo  = (d >> 7)  & 1;  /* rough */
    }

    /* Leaf 0x80000008: topology */
    if (max_ext >= 0x80000008) {
        cpuid(0x80000008, &a, &b, &c, &d);
        feat->core_count   = (a & 0xFF) + 1;
        feat->thread_count = ((b >> 12) & 0xF) + 1;
    }

    /* 0x1B: per-PM EFREQ (Intel turbo); not critical */
    (void) a; (void) b; (void) c; (void) d;
}

/* ── Print CPU info ──────────────────────────────────────────────── */

void cpu_print_info(const cpu_features_t *feat) {
    printf("  CPU: APIC %u | %uC/%uT pkg=%u core=%u thr=%u\n",
        feat->apic_id, feat->core_count, feat->thread_count,
        feat->package_id, feat->core_id, feat->thread_id);
    printf("  Features: ");
    if (feat->sse)    printf("SSE ");
    if (feat->sse2)   printf("SSE2 ");
    if (feat->sse3)   printf("SSE3 ");
    if (feat->ssse3)  printf("SSSE3 ");
    if (feat->sse4_1) printf("SSE4.1 ");
    if (feat->sse4_2) printf("SSE4.2 ");
    if (feat->avx)    printf("AVX ");
    if (feat->avx2)   printf("AVX2 ");
    if (feat->avx512f)printf("AVX-512F ");
    printf("\n  Security: ");
    if (feat->smep)   printf("SMEP ");
    if (feat->smap)   printf("SMAP ");
    if (feat->umip)   printf("UMIP ");
    if (feat->nx)     printf("NX ");
    printf("\n  Cache: L1D=%uK L1I=%uK L2=%uK L3=%uk line=%uB\n",
        feat->l1d_cache_kb, feat->l1i_cache_kb,
        feat->l2_cache_kb, feat->l3_cache_kb,
        feat->cache_line_size);
}

/* ── Detect all CPUs (BSP) ───────────────────────────────────────── */

void cpu_detect_all(void) {
    cpu_features_t bsp_feat;
    cpu_detect_features(&bsp_feat);

    /* Bootstrap CPU */
    percpu_t *bsp = &percpu_array[0];
    memset(bsp, 0, sizeof(*bsp));
    bsp->apic_id    = bsp_feat.apic_id;
    bsp->cpu_index  = 0;
    bsp->online     = true;
    bsp->boot_cpu   = true;
    bsp->features   = bsp_feat;
    cpu_count = 1;

    printf("[CPU] Bootstrap processor detected\n");
    cpu_print_info(&bsp_feat);

    /* TODO: Parse ACPI MADT to discover APs */
    cpu_count = bsp_feat.core_count;
    if (cpu_count > MAX_CPUS) cpu_count = MAX_CPUS;

    printf("[CPU] Total CPUs detected: %u\n", cpu_count);
}
