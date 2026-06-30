/* OmniOS — kernel/arch/x86_64/cpu.h */
/* x86_64 CPU detection — multi-core, topology, features */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_CPU_H
#define OMNIOS_CPU_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_CPUS 256

/* ── CPU features bitmask (CPUID leaf 1 → EDX, ECX) ─────────────── */

typedef struct {
    /* Standard features */
    bool fpu;        /* x87 FPU */
    bool mmx;        /* MMX */
    bool sse;        /* SSE */
    bool sse2;       /* SSE2 */
    bool sse3;       /* SSE3 */
    bool ssse3;      /* SSSE3 */
    bool sse4_1;     /* SSE4.1 */
    bool sse4_2;     /* SSE4.2 */
    bool avx;        /* AVX */
    bool avx2;       /* AVX2 */
    bool avx512f;    /* AVX-512 Foundation */
    bool avx512bw;   /* AVX-512 Byte/Word */
    bool avx512dq;   /* AVX-512 Dword/Qword */
    bool avx512vl;   /* AVX-512 Vector Length */
    bool aes;        /* AES-NI */
    bool sha;        /* SHA-NI */
    bool rdrand;     /* RDRAND */
    bool rdseed;     /* RDSEED */

    /* Virtualization */
    bool svm;        /* AMD SVM */
    bool vmx;        /* Intel VMX */

    /* Security */
    bool smep;       /* Supervisor Mode Execution Prevention */
    bool smap;       /* Supervisor Mode Access Prevention */
    bool umip;       /* User Mode Instruction Prevention */
    bool pku;        /* Protection Keys for Userspace */
    bool ospke;      /* OS has enabled PKU */
    bool nx;         /* No-Execute page table bit */

    /* Memory management */
    bool pae;        /* Physical Address Extension */
    bool pse;        /* Page Size Extension */
    bool pge;        /* Page Global Enable */
    bool pcid;       /* Process-Context Identifiers */
    bool invpcid;    /* Invalidate PCID */
    bool fsgsbase;   /* RDFSGSBASE instructions */
    bool mtrr;       /* Memory Type Range Registers */
    bool pat;        /* Page Attribute Table */
    bool x2apic;     /* x2APIC */

    /* Power management */
    bool acpi;       /* ACPI via CPUID */
    bool hwp;        /* Hardware P-States (Speed Shift) */
    bool eist;       /* Enhanced Intel SpeedStep */
    bool turbo;      /* Turbo Boost */

    /* Cache / TLB */
    uint32_t cache_line_size;
    uint32_t l1d_cache_kb;
    uint32_t l1i_cache_kb;
    uint32_t l2_cache_kb;
    uint32_t l3_cache_kb;

    /* Topology */
    uint32_t apic_id;         /* Local APIC ID (xAPIC) */
    uint32_t x2apic_id;       /* x2APIC ID */
    uint32_t package_id;      /* Physical package/socket */
    uint32_t core_id;         /* Core within package */
    uint32_t thread_id;       /* SMT thread within core */
    uint32_t core_count;      /* Total cores in package */
    uint32_t thread_count;    /* Total threads in package */
} cpu_features_t;

/* ── Per-CPU control block ───────────────────────────────────────── */

typedef struct {
    uint32_t     apic_id;
    uint32_t     cpu_index;
    bool         online;
    bool         boot_cpu;

    /* State */
    uint64_t     idle_ticks;
    uint64_t     irq_count;
    uint64_t     context_switches;

    /* Stack */
    uintptr_t    kernel_stack;
    uintptr_t    kernel_stack_top;

    /* Cached features */
    cpu_features_t features;

    /* Scheduler data (pointer to runqueue, etc.) */
    void        *sched_data;

    /* TSS for this CPU (needed for IST stacks) */
    uintptr_t    tss_addr;

    /* Per-CPU link in global list */
    uint32_t     next;
} percpu_t;

/* ── Public API ──────────────────────────────────────────────────── */

void cpu_detect_all(void);
void cpu_detect_features(cpu_features_t *feat);
void cpu_print_info(const cpu_features_t *feat);
bool cpu_has_fsgsbase(void);

/* Topology helpers */
uint32_t cpu_get_count(void);
uint32_t cpu_get_apic_id(uint32_t cpu_index);
percpu_t *cpu_get_percpu(uint32_t cpu_index);
percpu_t *cpu_get_current(void);

/* Feature testers */
static inline bool cpu_feature_avx(const cpu_features_t *c) { return c->avx; }
static inline bool cpu_feature_avx2(const cpu_features_t *c) { return c->avx2; }
static inline bool cpu_feature_smep(const cpu_features_t *c) { return c->smep; }
static inline bool cpu_feature_smap(const cpu_features_t *c) { return c->smap; }
static inline bool cpu_feature_umip(const cpu_features_t *c) { return c->umip; }
static inline bool cpu_feature_fsgsbase(const cpu_features_t *c) { return c->fsgsbase; }
static inline bool cpu_feature_x2apic(const cpu_features_t *c) { return c->x2apic; }

#endif /* OMNIOS_CPU_H */
