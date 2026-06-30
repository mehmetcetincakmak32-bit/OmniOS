# OmniOS

**A mobile operating system — x86_64 SMP development, ARM64 mobile target**

OmniOS is a from-scratch operating system with a native C kernel (x86_64 SMP + ARM64), a driver ecosystem, capability-based security, Android (Waydroid) compatibility, and a Flutter-based system UI.

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  USERSPACE                           │
│  ┌──────────┐ ┌──────────┐ ┌──────────────────────┐ │
│  │  Flutter  │ │  wlroots │ │  Capability Daemon   │ │
│  │  System   │ │ Compositor│ │  (token IPC/auth)   │ │
│  │  UI       │ │          │ │                      │ │
│  └──────────┘ └──────────┘ └──────────────────────┘ │
│  ┌────────────────────────────────────────────────┐ │
│  │  Waydroid (Android container + Binder IPC)     │ │
│  └────────────────────────────────────────────────┘ │
├─────────────────────────────────────────────────────┤
│                  KERNEL                               │
│  ┌─────────────┐ ┌─────────────┐ ┌────────────────┐ │
│  │  x86_64 SMP  │ │  ARM64      │ │  Drivers        │ │
│  │  (dev/QEMU)  │ │  (mobile)   │ │  PCI/VirtIO/   │ │
│  │              │ │             │ │  UFS/Audio/     │ │
│  │  APIC/GIC    │ │  GICv3      │ │  Display/Input  │ │
│  │  ACPI        │ │  Generic    │ │  USB/Modem/NFC  │ │
│  │  SMP         │ │  Timer      │ │  WiFi/BT/Sensors│ │
│  └─────────────┘ └─────────────┘ └────────────────┘ │
│  ┌─────────────┐ ┌─────────────┐ ┌────────────────┐ │
│  │  PMM (Buddy) │ │  VMM (4Lv)  │ │  Scheduler      │ │
│  │  NUMA-aware  │ │  Higher-half│ │  SMP RR + Load  │ │
│  └─────────────┘ └─────────────┘ └────────────────┘ │
├─────────────────────────────────────────────────────┤
│                  ARM64 BSP (SM8250)                    │
│  PM8150 PMIC  MDSS Display  QCA6391 WiFi/BT/GPS      │
│  X55 Modem    NXP PN553 NFC  Synaptics Touch          │
│  SLPI Sensors ADSP Audio    DWC3 USB3                 │
│  UFS 3.0      Fingerprint   USB Gadget (ADB/MTP)     │
└─────────────────────────────────────────────────────┘
```

---

## What Works Now

### Kernel — x86_64 SMP (`ARCH=x86_64`)
- 4-level paging, higher-half VMA, NX/SMEP/SMAP
- GDT (64-bit, TSS with IST), IDT (256 vector), PIC/APIC
- SMP: SIPI AP boot, per-CPU GS.base, per-CPU runqueues
- ACPI: RSDP/RSDT/MADT/FADT, poweroff/reboot
- Physical memory: buddy allocator, 10 orders, NUMA-aware
- Virtual memory: VM regions, demand paging, kernel heap
- Process/thread: PCBs, inline assembly context switch
- Scheduler: SMP round-robin, tick, yield, sleep/wake, load balance
- PCI: config space enumeration, MMIO BARs
- VirtIO: modern PCI transport, descriptor rings

### Kernel — ARM64 Mobile (`ARCH=arm64`)
- ARM64 boot (EL2→EL1, FP/SIMD, page tables, SMP spin-table)
- GICv3, generic timer
- User page tables, EL0 entry via ERET
- Exception vectors (SVC, IRQ, sync handlers)
- initramfs with cpio + ELF loader

### BSP — Snapdragon 865 (SM8250)
- **PMIC**: PM8150 charger, fuel gauge (%, mV, °C)
- **Display**: MDSS framebuffer 1080x2340 32bpp
- **Touch**: Synaptics S3908 (I2C)
- **Modem**: X55 RIL + IMS/VoLTE/WiFi Calling
- **WiFi/BT/GPS**: QCA6391 (SDIO/UART)
- **Audio**: ADSP + WCD938x I2S
- **Sensors**: SLPI accel/gyro/light/proximity
- **Storage**: UFS 3.0 flash
- **USB**: DWC3 OTG + gadget (ADB/MTP)
- **NFC**: NXP PN553
- **Fingerprint**: enroll/authenticate
- **Power**: suspend/resume, cpufreq, thermal
- **RTC**: PM8150 epoch/alarm
- **Input**: event queue, power/volume buttons

### Userspace
- ELF64 loader, cpio initramfs
- System calls: exit/write/read/open/close/sleep/getpid/fork/execve/brk/yield/time/poweroff/reboot
- Device filesystem (`/dev/null`, `/dev/console`, `/dev/tty0`, `/dev/kmsg`)
- Process filesystem (`/proc/cpuinfo`, `/proc/meminfo`, `/proc/uptime`, `/proc/battery`)
- TTY console, kernel log ring buffer
- Pipe IPC, signal handling

### Security
- seccomp-BPF profiles (4 profiles: default, compositor, app, Waydroid)
- Capability token IPC: `capd` daemon + `libomnios_cap` client library
- Unix SOCK_SEQPACKET with rights bitmap (read/write/execute/admin)

### Android Compatibility
- Waydroid binder probing (`/dev/binder`, `/dev/hwbinder`, `/dev/vndbinder`)
- Capability token registration for Waydroid containers

### Flutter System UI
- BasicMessageChannel JSON bridge between Dart and C embedder
- Platform bridge service (`omnios_app/lib/services/platform_bridge.dart`)

### OTA Updates
- A/B slot GPT layout (boot_a, root_a, boot_b, root_b, data, metadata)
- Full update lifecycle: status/prepare/apply/switch/commit/rollback/verify
- dm-verity hash tree verification

---

## Build

### x86_64 (development on QEMU)
```bash
cd kernel
make test                          # run test suite
make ARCH=x86_64 all               # build kernel
make run_qemu                      # launch in QEMU (SMP 4, KVM)
```

### ARM64 (mobile target)
```bash
cd kernel
make ARCH=arm64 CC=aarch64-linux-gnu-gcc all   # cross-compile
make ARCH=arm64 run_qemu_arm64                  # QEMU aarch64 virt
```

### Userspace
```bash
cd userspace/compositor && meson setup build && meson compile -C build
cd omnios_app && flutter pub get && flutter run
scripts/gen_initramfs.sh arm64                  # build initramfs
```

---

## Project Structure

```
OmniOS/
├── kernel/                      # Native C kernel
│   ├── arch/x86_64/             # x86_64 SMP (boot, GDT, IDT, APIC, ACPI, paging)
│   ├── arch/arm64/              # ARM64 (boot, GIC, timer, exceptions, page tables)
│   ├── bsp/sm8250/              # Snapdragon 865 BSP
│   │   └── drivers/             # 14 mobile drivers (PMIC, display, touch, modem, etc.)
│   ├── mm/                      # PMM (buddy) + VMM (4-level)
│   ├── proc/                    # Process/thread management, signals
│   ├── sched/                   # SMP scheduler
│   ├── fs/                      # ELF loader, initramfs, devfs, procfs
│   ├── ipc/                     # Pipe IPC
│   ├── drivers/                 # PCI, VirtIO, TTY
│   ├── include/                 # omnios_kernel.h
│   └── Makefile                 # Build: x86_64 + ARM64, test, QEMU targets
├── userspace/
│   ├── compositor/              # wlroots-based compositor
│   ├── flutter-embedder/        # Flutter engine C embedder (Wayland+EGL)
│   ├── security/                # capd daemon, libomnios_cap, Waydroid binder
│   └── init/                    # PID 1 init (ELF compiled for initramfs)
├── omnios_app/                  # Flutter system UI
│   └── lib/services/            # Platform bridge, gesture, state
├── scripts/                     # build_rootfs, run_qemu, partition_layout, ota_update, gen_initramfs
├── core/                        # C core library (seccomp profiles)
├── .github/workflows/           # CI (10+ jobs: x86_64 + ARM64, compositor, capability, security)
└── third_party/OpenHands        # AI coding agent (submodule)
```

---

## CI/CD

| Job | Status |
|-----|--------|
| python-tests | ✅ Python core engine tests |
| c-build | ✅ C core library compile + test |
| flutter-analyze | ✅ Flutter static analysis |
| kernel-test | ✅ x86_64 kernel build + test suite |
| kernel-arm64-build | ✅ ARM64 cross-compile |
| kernel-config-validate | ✅ Defconfig validation |
| rootfs-build-validate | ✅ Script validation |
| compositor-build | ✅ wlroots compositor build |
| security-profile-test | ✅ seccomp profiles |
| capability-build | ✅ Capability IPC + Waydroid binder |

---

## License

MIT License — See [LICENSE](LICENSE)
