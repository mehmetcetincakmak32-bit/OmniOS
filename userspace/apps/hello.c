/*
 * OmniOS Userspace Hello World
 * Sample userspace program using OmniOS system calls
 */

#include "../lib/libc.h"

void _start(void) {
    printf("========================================\n");
    printf("  OmniOS Userspace Program\n");
    printf("========================================\n\n");

    printf("Hello from OmniOS userspace!\n");
    printf("PID: %d\n", sys_getpid());
    printf("Uptime: %d ms\n", sys_gettime());

    printf("\n");
    printf("OmniOS API Test:\n");
    printf("  syscall(SYS_GETPID) = %d\n", sys_getpid());
    printf("  syscall(SYS_GETTIME) = %d\n", sys_gettime());

    printf("\n");
    printf("Opening /etc/version...\n");
    int fd = sys_open("/etc/version", O_RDONLY);
    if (fd >= 0) {
        printf("  File opened: fd=%d\n", fd);
        sys_close(fd);
    } else {
        printf("  File not found (expected)\n");
    }

    printf("\n");

    /* Create a test file */
    printf("Creating /tmp/test.txt...\n");
    fd = sys_open("/tmp/test.txt", O_WRONLY | O_CREAT);
    if (fd >= 0) {
        const char* msg = "OmniOS was here!\n";
        sys_write(fd, msg, 17);
        printf("  Written %d bytes\n", 17);
        sys_close(fd);
    }

    printf("\n");
    printf("========================================\n");
    printf("  Program completed successfully.\n");
    printf("========================================\n");

    sys_exit(0);
}
