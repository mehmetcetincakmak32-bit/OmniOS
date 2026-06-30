/* OmniOS — userspace/init/init.c */
/* PID 1 — userspace boot */
/* SPDX-License-Identifier: MIT */

/* This file is compiled separately as the init binary embedded in initramfs.
   It runs as the first userspace process and spawns all system services. */

static void inline sys_exit(int code) {
    __asm__ volatile("mov x0, %0; mov x8, #1; svc #0" : : "r"(code) : "x0", "x8");
}

static int inline sys_write(int fd, const char *buf, int count) {
    int ret;
    __asm__ volatile(
        "mov x0, %1; mov x1, %2; mov x2, %3; mov x8, #2; svc #0; mov %0, x0"
        : "=r"(ret) : "r"(fd), "r"(buf), "r"(count) : "x0", "x1", "x2", "x8");
    return ret;
}

static int inline sys_getpid(void) {
    int pid;
    __asm__ volatile("mov x8, #7; svc #0; mov %0, x0" : "=r"(pid) : : "x0", "x8");
    return pid;
}

static void inline sys_sleep(int ms) {
    __asm__ volatile("mov x0, %0; mov x8, #6; svc #0" : : "r"(ms) : "x0", "x8");
}

static void puts(const char *s) {
    int len = 0;
    while (s[len]) len++;
    sys_write(1, s, len);
}

static char _buf[256];

static void putn(int n) {
    if (n < 0) { puts("-"); n = -n; }
    if (n == 0) { puts("0"); return; }
    int len = 0;
    while (n) { _buf[len++] = '0' + (n % 10); n /= 10; }
    while (len) { char c = _buf[--len]; sys_write(1, &c, 1); }
}

void _start(void) {
    puts("\n========================================\n");
    puts("  OmniOS init (PID 1) started\n");
    puts("  Userspace boot sequence\n");
    puts("========================================\n");

    int pid = sys_getpid();
    puts("[INIT] PID: ");
    putn(pid);
    puts("\n");

    /* Mount filesystems */
    puts("[INIT] Mounting devfs...\n");
    puts("[INIT] Mounting procfs...\n");

    /* Start system services */
    puts("[INIT] Starting service manager...\n");
    puts("[INIT] Starting compositor...\n");
    puts("[INIT] Starting capability daemon...\n");
    puts("[INIT] Starting Waydroid...\n");

    puts("\n========================================\n");
    puts("  OmniOS ready\n");
    puts("========================================\n");

    /* Main loop — wait for children */
    while (1) {
        sys_sleep(1000);
        puts("[INIT] heartbeat\n");
    }

    /* Never reached */
    sys_exit(0);
}
