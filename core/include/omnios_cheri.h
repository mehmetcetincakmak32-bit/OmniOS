/*
 * OmniOS CHERI Capability Security
 * Hardware-enforced capability-based security for OmniOS v2.0
 * Compatible with ARMv9-A (Morello) and RISC-V CHERI
 */

#ifndef OMNOS_CHERI_H
#define OMNOS_CHERI_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ================================================================
 * CHERI Capability Types
 * ================================================================ */

#if defined(__CHERI_PURE_CAPABILITY__) || defined(__CHERI__)
    #define OMNOS_CHERI_NATIVE 1
    #include <cheri/cheri.h>
    #include <cheriintrin.h>
#else
    #define OMNOS_CHERI_NATIVE 0
    /* Software capability emulation for non-CHERI platforms */
    typedef struct {
        void*       address;
        size_t      length;
        uint64_t    perms;
        uint64_t    type;
        uint64_t    seal;
        uint64_t    otype;
    } __capability;
#endif

/* Capability permissions (monotonic - can only be reduced) */
typedef enum {
    CAP_PERM_READ        = 1ULL << 0,
    CAP_PERM_WRITE       = 1ULL << 1,
    CAP_PERM_EXECUTE     = 1ULL << 2,
    CAP_PERM_LOAD        = 1ULL << 3,    /* Load capability */
    CAP_PERM_STORE       = 1ULL << 4,    /* Store capability */
    CAP_PERM_LOAD_STORE  = 1ULL << 5,    /* Load/store capability */
    CAP_PERM_STORE_LOCAL = 1ULL << 6,    /* Store local capability */
    CAP_PERM_GLOBAL      = 1ULL << 7,    /* Global capability */
    CAP_PERM_SEAL        = 1ULL << 8,    /* Seal capability */
    CAP_PERM_UNSEAL      = 1ULL << 9,    /* Unseal capability */
    CAP_PERM_SYSTEM      = 1ULL << 10,   /* System capability */
    CAP_PERM_LOAD_GLOBAL = 1ULL << 11,   /* Load global capability */
    CAP_PERM_LOAD_MUTABLE= 1ULL << 12,   /* Load mutable capability */
    CAP_PERM_STORE_CAP   = 1ULL << 13,   /* Store capability */
    CAP_PERM_ENTER       = 1ULL << 14,   /* Enter (for sentry capabilities) */
    CAP_PERM_CALL        = 1ULL << 15,   /* Call (for sentry capabilities) */

    CAP_PERM_ALL         = 0xFFFF,
} CapabilityPerm;

/* Capability types */
typedef enum {
    CAP_TYPE_DATA        = 0,   /* Data capability */
    CAP_TYPE_CODE        = 1,   /* Code (sentry) capability */
    CAP_TYPE_SENTRY      = 2,   /* Sentry (sealed code) capability */
    CAP_TYPE_SEALED      = 3,   /* Sealed capability */
    CAP_TYPE_PCC         = 4,   /* Program Counter Capability */
    CAP_TYPE_DDC         = 5,   /* Default Data Capability */
    CAP_TYPE_STACK       = 6,   /* Stack capability */
} CapabilityType;

/* Capability rights (for sealing/unsealing) */
typedef uint64_t cap_rights_t;

#define CAP_RIGHTS_NONE     0
#define CAP_RIGHTS_READ     CAP_PERM_READ
#define CAP_RIGHTS_WRITE    CAP_PERM_WRITE
#define CAP_RIGHTS_EXECUTE  CAP_PERM_EXECUTE
#define CAP_RIGHTS_RW       (CAP_PERM_READ | CAP_PERM_WRITE)
#define CAP_RIGHTS_RX       (CAP_PERM_READ | CAP_PERM_EXECUTE)
#define CAP_RIGHTS_RWX      (CAP_PERM_READ | CAP_PERM_WRITE | CAP_PERM_EXECUTE)

/* Capability seal type */
typedef uint64_t cap_seal_type_t;

#define CAP_SEAL_TYPE_ANY    0
#define CAP_SEAL_TYPE_CODE   1
#define CAP_SEAL_TYPE_DATA   2

/* ================================================================
 * Capability Structure (Software Emulation / Native Mapping)
 * ================================================================ */

#if OMNOS_CHERI_NATIVE
    typedef __capability cap_ptr_t;
    typedef __capability cap_code_ptr_t;
#else
    /* Software capability representation */
    typedef struct {
        uintptr_t base;       /* Base address */
        size_t    length;     /* Length/bounds */
        uintptr_t offset;     /* Current offset within bounds */
        uint64_t  perms;      /* Permissions bitmask */
        uint64_t  type;       /* Capability type */
        uint64_t  seal;       /* Seal type */
        uint64_t  otype;      /* Object type */
        uint64_t  epoch;      /* Epoch for revocation */
    } cap_ptr_t;

    typedef cap_ptr_t cap_code_ptr_t;
#endif

/* ================================================================
 * Capability Operations (Native / Emulated)
 * ================================================================ */

/* Capability bounds checking */
static inline bool cap_in_bounds(cap_ptr_t cap, size_t offset, size_t size) {
#if OMNOS_CHERI_NATIVE
    return cheri_bounds_get(cap) >= offset + size && cheri_offset_get(cap) <= offset;
#else
    return cap.offset + offset + size <= cap.length;
#endif
}

/* Capability permission checking */
static inline bool cap_has_perm(cap_ptr_t cap, CapabilityPerm perm) {
#if OMNOS_CHERI_NATIVE
    return cheri_perms_get(cap) & perm;
#else
    return cap.perms & perm;
#endif
}

/* Get capability base address */
static inline uintptr_t cap_get_base(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_base_get(cap);
#else
    return cap.base;
#endif
}

/* Get capability length */
static inline size_t cap_get_length(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_length_get(cap);
#else
    return cap.length;
#endif
}

/* Get capability offset */
static inline size_t cap_get_offset(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_offset_get(cap);
#else
    return cap.offset;
#endif
}

/* Get capability permissions */
static inline uint64_t cap_get_perms(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_perms_get(cap);
#else
    return cap.perms;
#endif
}

/* Get capability tag (validity) */
static inline bool cap_is_valid(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_tag_get(cap);
#else
    return cap.base != 0 && cap.length > 0;
#endif
}

/* ================================================================
 * Capability Construction / Derivation
 * ================================================================ */

/* Create a new capability with reduced bounds */
static inline cap_ptr_t cap_bound(cap_ptr_t cap, size_t new_length) {
#if OMNOS_CHERI_NATIVE
    return cheri_bounds_set(cap, new_length);
#else
    cap_ptr_t result = cap;
    if (new_length < cap.length) {
        result.length = new_length;
        if (result.offset > new_length) result.offset = new_length;
    }
    return result;
#endif
}

/* Create a new capability with reduced permissions */
static inline cap_ptr_t cap_perms_and(cap_ptr_t cap, uint64_t perms) {
#if OMNOS_CHERI_NATIVE
    return cheri_perms_and(cap, perms);
#else
    cap_ptr_t result = cap;
    result.perms &= perms;
    return result;
#endif
}

/* Create a new capability with offset */
static inline cap_ptr_t cap_offset(cap_ptr_t cap, size_t offset) {
#if OMNOS_CHERI_NATIVE
    return cheri_offset_set(cap, offset);
#else
    cap_ptr_t result = cap;
    if (offset < cap.length) {
        result.offset = offset;
    }
    return result;
#endif
}

/* Create a capability from base, length, permissions */
static inline cap_ptr_t cap_create(void* base, size_t length, uint64_t perms, CapabilityType type) {
#if OMNOS_CHERI_NATIVE
    return cheri_cap_create(base, length, perms);
#else
    cap_ptr_t cap = {0};
    cap.base = (uintptr_t)base;
    cap.length = length;
    cap.offset = 0;
    cap.perms = perms;
    cap.type = type;
    cap.seal = 0;
    cap.otype = 0;
    cap.epoch = 1;
    return cap;
#endif
}

/* ================================================================
 * Capability Sealing / Unsealing
 * ================================================================ */

/* Seal a capability (makes it opaque, can only be unsealed with matching key) */
static inline cap_ptr_t cap_seal(cap_ptr_t cap, cap_ptr_t key) {
#if OMNOS_CHERI_NATIVE
    return cheri_seal(cap, key);
#else
    cap_ptr_t result = cap;
    result.seal = (uintptr_t)key;
    result.type = CAP_TYPE_SEALED;
    return result;
#endif
}

/* Unseal a capability */
static inline cap_ptr_t cap_unseal(cap_ptr_t sealed, cap_ptr_t key) {
#if OMNOS_CHERI_NATIVE
    return cheri_unseal(sealed, key);
#else
    if (sealed.seal == (uintptr_t)key) {
        cap_ptr_t result = sealed;
        result.seal = 0;
        result.type = CAP_TYPE_DATA;
        return result;
    }
    /* Invalid unseal - return invalid capability */
    cap_ptr_t invalid = {0};
    return invalid;
#endif
}

/* Check if capability is sealed */
static inline bool cap_is_sealed(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_is_sealed(cap);
#else
    return cap.seal != 0;
#endif
}

/* Get seal type */
static inline uint64_t cap_get_seal_type(cap_ptr_t cap) {
#if OMNOS_CHERI_NATIVE
    return cheri_seal_type_get(cap);
#else
    return cap.seal;
#endif
}

/* ================================================================
 * Capability Revocation (Epoch-based)
 * ================================================================ */

typedef struct {
    uint64_t current_epoch;
    uint64_t revoked_epochs[64];  /* Bitmap of revoked epochs */
} cap_revocation_t;

static inline void cap_revocation_init(cap_revocation_t* rev) {
    rev->current_epoch = 1;
    for (int i = 0; i < 64; i++) rev->revoked_epochs[i] = 0;
}

static inline void cap_revocation_revoke(cap_revocation_t* rev, uint64_t epoch) {
    if (epoch < 64) {
        rev->revoked_epochs[epoch] = 1;
    }
}

static inline bool cap_is_revoked(const cap_revocation_t* rev, const cap_ptr_t* cap) {
#if OMNOS_CHERI_NATIVE
    /* Native CHERI uses different revocation mechanism */
    return false;
#else
    if (cap->epoch < 64) {
        return rev->revoked_epochs[cap->epoch] != 0;
    }
    return false;
#endif
}

/* ================================================================
 * Capability Memory Allocator
 * ================================================================ */

typedef struct {
    cap_ptr_t heap_cap;
    size_t    total_size;
    size_t    used_size;
    size_t    alignment;
} cap_allocator_t;

/* Initialize capability allocator */
int cap_allocator_init(cap_allocator_t* alloc, size_t size, uint64_t perms);

/* Allocate capability-bounded memory */
cap_ptr_t cap_alloc(cap_allocator_t* alloc, size_t size, uint64_t perms);

/* Free capability-bounded memory */
void cap_free(cap_allocator_t* alloc, cap_ptr_t cap);

/* Get allocator stats */
void cap_allocator_stats(const cap_allocator_t* alloc, size_t* used, size_t* free);

/* ================================================================
 * Capability Process Isolation
 * ================================================================ */

typedef struct {
    cap_ptr_t    code_cap;      /* Code capability (PCC) */
    cap_ptr_t    data_cap;      /* Data capability (DDC) */
    cap_ptr_t    stack_cap;     /* Stack capability */
    cap_ptr_t    heap_cap;      /* Heap capability */
    cap_ptr_t    syscall_cap;   /* Syscall sentry capability */
    cap_revocation_t revocation;
    uint64_t     process_epoch;
    uint32_t     pid;
} cap_process_t;

/* Initialize process capabilities */
int cap_process_init(cap_process_t* proc, uint32_t pid, cap_ptr_t code, cap_ptr_t data, cap_ptr_t stack);

/* Enter process capability domain */
int cap_process_enter(cap_process_t* proc);

/* Exit process capability domain */
void cap_process_exit(cap_process_t* proc);

/* Revoke all capabilities for a process */
void cap_process_revoke(cap_process_t* proc);

/* ================================================================
 * Capability-Based IPC
 * ================================================================ */

typedef struct {
    cap_ptr_t send_cap;      /* Capability to send */
    cap_ptr_t recv_cap;      /* Capability to receive */
    cap_ptr_t reply_cap;     /* Capability for reply */
    uint64_t  message_type;
    size_t    payload_size;
} cap_ipc_msg_t;

/* Send capability via IPC */
int cap_ipc_send(cap_ptr_t endpoint, cap_ipc_msg_t* msg);

/* Receive capability via IPC */
int cap_ipc_recv(cap_ptr_t endpoint, cap_ipc_msg_t* msg, uint64_t timeout_ms);

/* Create IPC endpoint with capabilities */
cap_ptr_t cap_ipc_endpoint_create(uint64_t perms);

/* ================================================================
 * Capability Sandbox
 * ================================================================ */

typedef struct {
    cap_ptr_t       allowed_caps[32];
    size_t          cap_count;
    cap_ptr_t       memory_region;
    cap_revocation_t revocation;
    uint64_t        policy_hash;
    bool            network_allowed;
    bool            filesystem_allowed;
    bool            gpu_allowed;
} cap_sandbox_t;

/* Create sandbox with specific capabilities */
int cap_sandbox_create(cap_sandbox_t* sandbox, const cap_ptr_t* caps, size_t count);

/* Enter sandbox */
int cap_sandbox_enter(cap_sandbox_t* sandbox);

/* Exit sandbox */
void cap_sandbox_exit(cap_sandbox_t* sandbox);

/* Check if capability is allowed in sandbox */
bool cap_sandbox_check(cap_sandbox_t* sandbox, cap_ptr_t cap);

/* ================================================================
 * Capability Audit / Verification
 * ================================================================ */

typedef enum {
    CAP_AUDIT_OK = 0,
    CAP_AUDIT_INVALID_TAG,
    CAP_AUDIT_OUT_OF_BOUNDS,
    CAP_AUDIT_PERMISSION_DENIED,
    CAP_AUDIT_REVOKED,
    CAP_AUDIT_SEAL_MISMATCH,
    CAP_AUDIT_TYPE_MISMATCH,
    CAP_AUDIT_EPOCH_MISMATCH,
} cap_audit_result_t;

typedef struct {
    cap_audit_result_t result;
    const char*        message;
    cap_ptr_t          capability;
    uint64_t           expected_perms;
    uint64_t           actual_perms;
} cap_audit_t;

/* Audit capability for security compliance */
cap_audit_t cap_audit(cap_ptr_t cap, uint64_t required_perms, cap_revocation_t* rev);

/* Verify capability chain integrity */
bool cap_verify_chain(const cap_ptr_t* chain, size_t length);

/* ================================================================
 * Capability-Based Memory Protection
 * ================================================================ */

/* Memory protection keys (pkeys) integration */
typedef enum {
    CAP_PKEY_KERNEL    = 0,
    CAP_PKEY_USER      = 1,
    CAP_PKEY_SANDBOX   = 2,
    CAP_PKEY_SHARED    = 3,
    CAP_PKEY_SECURE    = 4,
    CAP_PKEY_MAX       = 15,
} cap_pkey_t;

/* Assign protection key to capability */
int cap_pkey_assign(cap_ptr_t* cap, cap_pkey_t pkey);

/* Check if capability has protection key */
bool cap_pkey_check(cap_ptr_t cap, cap_pkey_t pkey);

/* ================================================================
 * Capability Verification Helpers (for testing/formal verification)
 * ================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* Verify capability invariants */
bool cap_verify_invariants(cap_ptr_t cap);

/* Check capability monotonicity (permissions only decrease) */
bool cap_check_monotonic(cap_ptr_t parent, cap_ptr_t child);

/* Verify no capability leaks */
bool cap_check_no_leaks(const cap_ptr_t* caps, size_t count, const cap_ptr_t* allowed, size_t allowed_count);

#ifdef __cplusplus
}
#endif

#endif /* OMNOS_CHERI_H */