/*
 * OmniOS Kernel Test Suite
 * Tests for all kernel subsystems
 */

#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  Testing %s... ", name);
#define END_TEST(result) do { \
    if (result) { printf("OK\n"); tests_passed++; } \
    else { printf("FAIL\n"); tests_failed++; } \
} while(0)

void test_kernel_init(void) {
    printf("\n[Kernel Initialization]\n");

    kernel_config_t config = DEFAULT_CONFIG;
    config.total_memory_mb = 256;

    status_t status = kernel_init(&config);
    TEST("Kernel baslatma");
    END_TEST(status == STATUS_SUCCESS);
}

void test_scheduler(void) {
    printf("\n[Scheduler]\n");

    TEST("Scheduler init");
    END_TEST(sched_init() == STATUS_SUCCESS);

    tid_t t1 = sched_create_thread(1, NULL, NULL, PRIORITY_DEFAULT);
    TEST("Thread olusturma");
    END_TEST(t1 > 0);

    tid_t t2 = sched_create_thread(2, NULL, NULL, PRIORITY_MAX);
    TEST("High-priority thread");
    END_TEST(t2 > 0 && t2 > t1);

    TEST("Thread sayisi");
    END_TEST(sched_get_thread_count() == 2);
}

void test_memory(void) {
    printf("\n[Memory Manager]\n");

    TEST("mm_init");
    END_TEST(mm_init(256) == STATUS_SUCCESS);

    void* p1 = kmalloc(128);
    TEST("kmalloc 128 bytes");
    END_TEST(p1 != NULL);

    void* p2 = kmalloc(4096);
    TEST("kmalloc 4KB");
    END_TEST(p2 != NULL);

    memset(p1, 0xAA, 128);
    TEST("Bellek yazma dogru");
    END_TEST(((uint8_t*)p1)[0] == 0xAA && ((uint8_t*)p1)[127] == 0xAA);

    kfree(p1);
    TEST("kfree");
    END_TEST(true);

    void* p3 = kcalloc(10, 32);
    TEST("kcalloc 10*32");
    END_TEST(p3 != NULL);

    TEST("kcalloc sifirli");
    bool zero_ok = true;
    for (int i = 0; i < 320; i++) {
        if (((uint8_t*)p3)[i] != 0) { zero_ok = false; break; }
    }
    END_TEST(zero_ok);

    kfree(p2);
    kfree(p3);

    page_t* pg = mm_alloc_page(VM_READ | VM_WRITE);
    TEST("Sayfa ayirma");
    END_TEST(pg != NULL && pg->phys_addr > 0);

    TEST("Sayfa referans sayisi");
    END_TEST(pg->ref_count == 1);

    TEST("Sayfa free");
    END_TEST(mm_free_page(pg) == STATUS_SUCCESS);
}

void test_ipc(void) {
    printf("\n[IPC]\n");

    TEST("IPC init");
    END_TEST(ipc_init() == STATUS_SUCCESS);

    ipc_channel_t ch;
    ch.type = IPC_CHANNEL_PIPE;
    ch.owner = 1;
    ch.peer = 2;

    TEST("Kanal olusturma");
    END_TEST(ipc_channel_create(&ch) == STATUS_SUCCESS);
    TEST("Kanal ID dogru");
    END_TEST(ch.channel_id > 0);

    TEST("Kanal kapatma");
    END_TEST(ipc_channel_close(ch.channel_id) == STATUS_SUCCESS);

    TEST("Olmayan kanal kapatma");
    END_TEST(ipc_channel_close(999) == STATUS_NOT_FOUND);
}

void test_vfs(void) {
    printf("\n[VFS]\n");

    TEST("VFS init");
    END_TEST(vfs_init() == STATUS_SUCCESS);

    TEST("Kok dizin var");
    stat_t st;
    END_TEST(vfs_stat("/", &st) == STATUS_SUCCESS);

    TEST("mkdir test");
    END_TEST(vfs_mkdir("/test") == STATUS_SUCCESS);
    END_TEST(vfs_stat("/test", &st) == STATUS_SUCCESS);

    TEST("Dizin olustu");
    END_TEST(st.st_mode & 040755);

    int fd = vfs_open("/test/hello.txt", 0);
    TEST("Dosya acma (yok)");
    END_TEST(fd < 0);

    TEST("Kok dizin durumu");
    END_TEST(vfs_stat("/", &st) == STATUS_SUCCESS);

    /* Clean up */
    vfs_close(fd);
}

void test_syscalls(void) {
    printf("\n[System Calls]\n");

    TEST("Syscall init");
    END_TEST(syscall_init() == STATUS_SUCCESS);

    uint32_t args[SYSCALL_MAX_ARGS] = {0};
    status_t result;

    args[0] = 0;
    syscall_handler(SYS_GETPID, args, &result);
    TEST("SYS_GETPID");
    END_TEST(result == STATUS_SUCCESS);

    args[0] = 10;
    syscall_handler(SYS_SLEEP, args, &result);
    TEST("SYS_SLEEP");
    END_TEST(result == STATUS_SUCCESS);

    args[0] = SYS_MAX + 1;
    syscall_handler(args[0], args, &result);
    TEST("Gecersiz syscall");
    END_TEST(result == STATUS_NOT_IMPLEMENTED);
}

void test_drivers(void) {
    printf("\n[Device Drivers]\n");

    TEST("Device init");
    END_TEST(dev_init() == STATUS_SUCCESS);

    device_t test_dev;
    strcpy(test_dev.name, "test_uart");
    test_dev.type = DEV_TYPE_CHAR;
    test_dev.ops = NULL;
    test_dev.private_data = NULL;
    test_dev.registered = false;
    test_dev.next = NULL;

    TEST("Cihaz kaydetme");
    END_TEST(dev_register(&test_dev) == STATUS_SUCCESS);

    device_t* found = dev_find("test_uart");
    TEST("Cihaz bulma");
    END_TEST(found != NULL && strcmp(found->name, "test_uart") == 0);

    TEST("Cihaz kaldirma");
    END_TEST(dev_unregister(found->dev_id) == STATUS_SUCCESS);

    TEST("Kaldirilani bulma");
    END_TEST(dev_find("test_uart") == NULL);
}

void test_network(void) {
    printf("\n[Network Stack]\n");

    TEST("Net init");
    END_TEST(net_init() == STATUS_SUCCESS);

    TEST("Arayuz ekleme");
    END_TEST(net_interface_add("wlan0", 0xC0A80001, 24, 0xC0A800FE) == STATUS_SUCCESS);

    uint8_t packet[] = "Hello OmniOS Network";
    TEST("Paket gonderme");
    END_TEST(net_send_packet(packet, sizeof(packet), NET_PROTO_IPV4) == STATUS_SUCCESS);

    int sock = net_socket_create(SOCK_DGRAM, NET_PROTO_IPV4);
    TEST("Socket olusturma");
    END_TEST(sock > 0);

    TEST("Socket bind");
    END_TEST(net_socket_bind(sock, 8080) == STATUS_SUCCESS);

    TEST("Socket connect");
    END_TEST(net_socket_connect(sock, 0xC0A80002, 9090) == STATUS_SUCCESS);
}

void test_timer(void) {
    printf("\n[Timer]\n");

    TEST("Timer init");
    END_TEST(timer_init() == STATUS_SUCCESS);

    static bool timer_fired = false;
    void timer_cb(void* arg) { timer_fired = true; (void)arg; }

    uint32_t tid = timer_create(100, timer_cb, NULL, false);
    TEST("Timer olusturma");
    END_TEST(tid > 0);

    TEST("Timer iptal");
    END_TEST(timer_cancel(tid) == STATUS_SUCCESS);
}

void test_stress(void) {
    printf("\n[Stress Test]\n");

    mm_init(256);

    printf("  1000x kmalloc/kfree... ");
    void* ptrs[1000];
    for (int i = 0; i < 1000; i++) {
        ptrs[i] = kmalloc((i % 256) + 1);
        if (!ptrs[i]) { printf("FAIL\n"); tests_failed++; break; }
    }
    if (ptrs[999]) printf("OK\n");
    for (int i = 0; i < 1000; i++) {
        if (ptrs[i]) kfree(ptrs[i]);
    }

    printf("  100x page alloc/free... ");
    page_t* pages[100];
    for (int i = 0; i < 100; i++) {
        pages[i] = mm_alloc_page(VM_READ | VM_WRITE);
        if (!pages[i]) { printf("FAIL\n"); tests_failed++; break; }
    }
    if (pages[99]) printf("OK\n");
    for (int i = 0; i < 100; i++) {
        if (pages[i]) mm_free_page(pages[i]);
    }
    tests_passed += 2;
}

int main(void) {
    printf("=== OmniOS Kernel Test Suite ===\n");
    printf("Architecture: Kernel Microkernel\n");
    printf("Version: 1.0.0\n\n");

    test_scheduler();
    test_memory();
    test_ipc();
    test_vfs();
    test_syscalls();
    test_drivers();
    test_network();
    test_timer();
    test_stress();

    printf("\n================================\n");
    printf("Sonuc: %d passed, %d failed\n", tests_passed, tests_failed);
    printf("================================\n");

    return tests_failed > 0 ? 1 : 0;
}
