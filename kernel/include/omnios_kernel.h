/*
 * OmniOS Microkernel
 * omniOS_kernel.h - Main kernel header with all type definitions
 *
 * This is the core of OmniOS - a lightweight microkernel
 * designed for mobile devices with dual-mode support.
 */

#ifndef OMNOS_KERNEL_H
#define OMNOS_KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ================================================================
 * Architecture-Independent Types
 * ================================================================ */

typedef uint32_t tid_t;       /* Thread ID */
typedef uint32_t pid_t;       /* Process ID */
typedef uint32_t uid_t;       /* User ID */
typedef uint32_t gid_t;       /* Group ID */
typedef int32_t  status_t;    /* Status code */
typedef uint32_t irq_t;       /* Interrupt request number */
typedef uint64_t time_t;      /* Time in milliseconds */

/* NULL pointer */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* ================================================================
 * Status / Error Codes
 * ================================================================ */

#define STATUS_SUCCESS          0
#define STATUS_ERROR           -1
#define STATUS_TIMEOUT         -2
#define STATUS_BUSY            -3
#define STATUS_INVALID         -4
#define STATUS_NOT_FOUND       -5
#define STATUS_PERM_DENIED     -6
#define STATUS_NO_MEMORY       -7
#define STATUS_NOT_IMPLEMENTED -8
#define STATUS_ALREADY_EXISTS  -9
#define STATUS_WOULD_BLOCK     -10
#define STATUS_INTERRUPTED     -11

/* ================================================================
 * Process / Thread States
 * ================================================================ */

typedef enum {
    THREAD_CREATED   = 0,
    THREAD_READY     = 1,
    THREAD_RUNNING   = 2,
    THREAD_BLOCKED   = 3,
    THREAD_SLEEPING  = 4,
    THREAD_WAITING   = 5,
    THREAD_TERMINATED = 6,
} thread_state_t;

typedef enum {
    PROCESS_CREATED   = 0,
    PROCESS_RUNNING   = 1,
    PROCESS_SLEEPING  = 2,
    PROCESS_ZOMBIE    = 3,
    PROCESS_TERMINATED = 4,
} process_state_t;

/* ================================================================
 * Scheduling Policies
 * ================================================================ */

typedef enum {
    SCHED_FIFO       = 0,   /* First-In-First-Out */
    SCHED_RR         = 1,   /* Round-Robin */
    SCHED_PRIORITY   = 2,   /* Priority-based */
    SCHED_MLFQ       = 3,   /* Multi-Level Feedback Queue */
} sched_policy_t;

#define PRIORITY_MIN        0
#define PRIORITY_DEFAULT    16
#define PRIORITY_MAX        31
#define PRIORITY_REALTIME   32

/* ================================================================
 * Memory Management
 * ================================================================ */

#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGE_MASK           0xFFFFF000

#define VM_READ             0x01
#define VM_WRITE            0x02
#define VM_EXEC             0x04
#define VM_USER             0x08
#define VM_SHARED           0x10
#define VM_IO               0x20

typedef uint32_t page_flags_t;

typedef struct page {
    uintptr_t    phys_addr;
    uintptr_t    virt_addr;
    page_flags_t flags;
    uint32_t     ref_count;
    struct page* next;
} page_t;

/* ================================================================
 * IPC (Inter-Process Communication)
 * ================================================================ */

#define IPC_MAX_MSG_SIZE    4096
#define IPC_MAX_CHANNELS    256

typedef enum {
    IPC_CHANNEL_UNUSED  = 0,
    IPC_CHANNEL_PIPE    = 1,
    IPC_CHANNEL_QUEUE   = 2,
    IPC_CHANNEL_SHARED_MEM = 3,
} ipc_channel_type_t;

typedef struct {
    uint32_t    channel_id;
    ipc_channel_type_t type;
    pid_t       sender;
    pid_t       receiver;
    uint32_t    msg_size;
    uint8_t     data[IPC_MAX_MSG_SIZE];
} ipc_message_t;

typedef struct {
    uint32_t    channel_id;
    ipc_channel_type_t type;
    pid_t       owner;
    pid_t       peer;
    uint32_t    pending_count;
    bool        blocked;
} ipc_channel_t;

/* ================================================================
 * VFS (Virtual File System)
 * ================================================================ */

#define PATH_MAX            4096
#define NAME_MAX            256
#define FILE_MAX_OPEN       128

typedef enum {
    FILE_TYPE_UNKNOWN   = 0,
    FILE_TYPE_REGULAR   = 1,
    FILE_TYPE_DIRECTORY = 2,
    FILE_TYPE_CHAR_DEV  = 3,
    FILE_TYPE_BLOCK_DEV = 4,
    FILE_TYPE_PIPE      = 5,
    FILE_TYPE_SYMLINK   = 6,
    FILE_TYPE_SOCKET    = 7,
} file_type_t;

typedef struct stat {
    uint32_t    st_dev;
    uint32_t    st_ino;
    uint16_t    st_mode;
    uint32_t    st_nlink;
    uid_t       st_uid;
    gid_t       st_gid;
    uint32_t    st_size;
    time_t      st_atime;
    time_t      st_mtime;
    time_t      st_ctime;
    uint32_t    st_blksize;
    uint32_t    st_blocks;
} stat_t;

typedef struct dirent {
    uint32_t    d_ino;
    char        d_name[NAME_MAX];
    file_type_t d_type;
} dirent_t;

/* File descriptor */
typedef struct file_descriptor {
    uint32_t    fd;
    file_type_t type;
    uint32_t    pos;
    uint32_t    flags;
    void*       private_data;
    bool        in_use;
} fd_t;

/* ================================================================
 * Device Driver Model
 * ================================================================ */

#define DEV_NAME_MAX        32

typedef enum {
    DEV_TYPE_CHAR     = 0,
    DEV_TYPE_BLOCK    = 1,
    DEV_TYPE_NETWORK  = 2,
    DEV_TYPE_FRAMEBUFFER = 3,
    DEV_TYPE_INPUT    = 4,
    DEV_TYPE_AUDIO    = 5,
} dev_type_t;

typedef struct device_ops {
    status_t (*init)(void);
    status_t (*deinit)(void);
    status_t (*read)(uint32_t offset, uint8_t* buffer, uint32_t size);
    status_t (*write)(uint32_t offset, const uint8_t* buffer, uint32_t size);
    status_t (*ioctl)(uint32_t request, void* arg);
    status_t (*irq_handler)(irq_t irq);
} device_ops_t;

typedef struct device {
    char            name[DEV_NAME_MAX];
    dev_type_t      type;
    uint32_t        dev_id;
    device_ops_t*   ops;
    void*           private_data;
    bool            registered;
    struct device*  next;
} device_t;

/* ================================================================
 * System Calls
 * ================================================================ */

#define SYSCALL_MAX_ARGS    6

typedef enum {
    SYS_EXIT        = 0,
    SYS_FORK        = 1,
    SYS_EXEC        = 2,
    SYS_SLEEP       = 3,
    SYS_GETPID      = 4,
    SYS_OPEN        = 5,
    SYS_CLOSE       = 6,
    SYS_READ        = 7,
    SYS_WRITE       = 8,
    SYS_MMAP        = 9,
    SYS_MUNMAP      = 10,
    SYS_SBRK        = 11,
    SYS_IPC_SEND    = 12,
    SYS_IPC_RECV    = 13,
    SYS_SCHED_YIELD = 14,
    SYS_GETTIME     = 15,
    SYS_IOCTL       = 16,
    SYS_PIPE        = 17,
    SYS_DUP         = 18,
    SYS_CHDIR       = 19,
    SYS_MKDIR       = 20,
    SYS_GETDENTS    = 21,
    SYS_STAT        = 22,
    SYS_BRK         = 23,
    SYS_REBOOT      = 24,
    SYS_MAX         = 64,
} syscall_t;

typedef status_t (*syscall_handler_t)(uint32_t args[SYSCALL_MAX_ARGS]);

/* ================================================================
 * Network Stack
 * ================================================================ */

#define ETH_ALEN            6
#define IP_ALEN             4
#define MAX_PACKET_SIZE     1514

typedef struct mac_address {
    uint8_t addr[ETH_ALEN];
} mac_addr_t;

typedef struct ip_address {
    uint8_t addr[IP_ALEN];
} ip_addr_t;

typedef enum {
    NET_PROTO_IPV4 = 0x0800,
    NET_PROTO_ARP  = 0x0806,
    NET_PROTO_IPV6 = 0x86DD,
} net_protocol_t;

typedef enum {
    SOCK_STREAM = 1,
    SOCK_DGRAM  = 2,
    SOCK_RAW    = 3,
} socket_type_t;

/* ================================================================
 * Timer / Clock
 * ================================================================ */

typedef struct timespec {
    time_t tv_sec;
    uint32_t tv_nsec;
} timespec_t;

typedef void (*timer_callback_t)(void* arg);

typedef struct timer {
    uint32_t        timer_id;
    time_t          interval_ms;
    timer_callback_t callback;
    void*           arg;
    bool            periodic;
    bool            active;
    struct timer*   next;
} timer_t;

/* ================================================================
 * Interrupt Handling
 * ================================================================ */

typedef void (*irq_handler_t)(irq_t irq, void* context);

#define IRQ_MAX_HANDLERS    64

/* ================================================================
 * Kernel Configuration
 * ================================================================ */

typedef struct kernel_config {
    uint32_t    max_processes;
    uint32_t    max_threads;
    uint32_t    total_memory_mb;
    uint32_t    heap_size_mb;
    uint32_t    ticks_per_second;
    sched_policy_t sched_policy;
    uint32_t    quantum_ms;
    char        hostname[64];
    bool        enable_networking;
    bool        enable_mp;
} kernel_config_t;

/* Default configuration */
#define DEFAULT_CONFIG { \
    .max_processes = 256, \
    .max_threads = 1024, \
    .total_memory_mb = 4096, \
    .heap_size_mb = 64, \
    .ticks_per_second = 1000, \
    .sched_policy = SCHED_MLFQ, \
    .quantum_ms = 10, \
    .hostname = "omnios", \
    .enable_networking = true, \
    .enable_mp = false, \
}

/* ================================================================
 * Kernel API Functions
 * ================================================================ */

/* Kernel initialization and lifecycle */
status_t kernel_init(const kernel_config_t* config);
void     kernel_poweroff(void);
void     kernel_reboot(void);
void     kernel_panic(const char* message);

/* Scheduler */
status_t sched_init(void);
tid_t    sched_create_thread(pid_t pid, void (*entry)(void*), void* arg, uint32_t priority);
status_t sched_yield(void);
status_t sched_sleep(time_t ms);
status_t sched_wake(tid_t tid);
pid_t    sched_get_current_pid(void);
tid_t    sched_get_current_tid(void);
void     sched_tick(void);

/* Memory management */
status_t mm_init(uint32_t total_memory_mb);
void*    kmalloc(size_t size);
void*    kcalloc(size_t num, size_t size);
void*    krealloc(void* ptr, size_t size);
void     kfree(void* ptr);
page_t*  mm_alloc_page(page_flags_t flags);
status_t mm_free_page(page_t* page);
void*    mm_map_io(uintptr_t phys_addr, size_t size);

/* IPC */
status_t ipc_init(void);
status_t ipc_channel_create(ipc_channel_t* channel);
status_t ipc_channel_close(uint32_t channel_id);
status_t ipc_send(uint32_t channel_id, const ipc_message_t* msg);
status_t ipc_receive(uint32_t channel_id, ipc_message_t* msg, time_t timeout);

/* VFS */
status_t vfs_init(void);
int      vfs_open(const char* path, uint32_t flags);
status_t vfs_close(int fd);
status_t vfs_read(int fd, uint8_t* buffer, uint32_t size, uint32_t* bytes_read);
status_t vfs_write(int fd, const uint8_t* buffer, uint32_t size, uint32_t* bytes_written);
status_t vfs_stat(const char* path, stat_t* st);
status_t vfs_mkdir(const char* path);
status_t vfs_getdents(int fd, dirent_t* dirp, uint32_t count);

/* Device driver framework */
status_t dev_init(void);
status_t dev_register(device_t* device);
status_t dev_unregister(uint32_t dev_id);
device_t* dev_find(const char* name);
device_t* dev_find_by_id(uint32_t dev_id);

/* System calls */
status_t syscall_init(void);
void     syscall_handler(uint32_t syscall_num, uint32_t args[SYSCALL_MAX_ARGS], status_t* result);

/* Network */
status_t net_init(void);
status_t net_send_packet(const uint8_t* data, uint32_t len, net_protocol_t proto);
status_t net_register_protocol(net_protocol_t proto, void (*handler)(const uint8_t*, uint32_t));

/* Timer */
status_t timer_init(void);
uint32_t timer_create(time_t interval_ms, timer_callback_t callback, void* arg, bool periodic);
status_t timer_cancel(uint32_t timer_id);
time_t   timer_get_uptime(void);

/* Interrupts */
status_t irq_register(irq_t irq, irq_handler_t handler, void* context);
status_t irq_unregister(irq_t irq);
void     irq_enable(void);
void     irq_disable(void);

/* Kernel utility functions */
uint64_t kernel_get_ticks(void);
time_t   kernel_get_uptime_ms(void);

/* Scheduler extra */
uint32_t sched_get_thread_count(void);
uint32_t sched_get_queue_count(uint32_t level);
void     sched_print_info(void);

/* Network extra */
status_t net_interface_add(const char* name, uint32_t ip, uint32_t netmask, uint32_t gateway);
status_t net_receive_packet(const uint8_t* data, uint32_t len, net_protocol_t proto);
int      net_socket_create(socket_type_t type, net_protocol_t proto);
status_t net_socket_bind(int socket_id, uint16_t port);
status_t net_socket_connect(int socket_id, uint32_t ip, uint16_t port);
status_t net_socket_send(int socket_id, const uint8_t* data, uint32_t len);
uint32_t net_get_stats(void);

/* Device extra */
uint32_t dev_get_count(void);

/* Timer extra */
void     timer_tick(void);

#endif /* OMNOS_KERNEL_H */
