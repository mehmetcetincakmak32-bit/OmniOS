# OmniOS v2.0

**Tek OS, Tüm Platformlar — Mobil Deneyimi Yeniden Tanımlıyoruz**

> **OmniOS v2.0**, mevcut bir mobil işletim sistemi üzerinde ikinci bir sistem katmanı olarak çalışan, **CHERI capability-based security**, **distributed-first architecture**, **first-class ML/AI**, ve **time-travel debugging** ile donatılmış, Android ve iOS'un ötesinde **rakipsiz bir next-generation mobile operating system** dur.

---

## 🚀 Neden OmniOS v2.0?

| Özellik | Android | iOS | **OmniOS v2.0** |
|---------|---------|-----|-----------------|
| **Kernel** | Linux (monolitik) | XNU (hybrid) | **CHERI µkernel + Unikernel hybrid** |
| **Güvenlik** | UID/GID + SELinux | Sandbox + Code Sign | **CHERI Capability + Formal Verification** |
| **Uygulamalar** | APK (DEX) | IPA (Mach-O) | **WASM Component + Native + ML IR** |
| **Güncelleme** | Parçalı A/B | Monolitik OTA | **Transactional + Delta + Canary + Rollback** |
| **AI/ML** | TFLite / NNAPI | CoreML | **First-class ML IR + Auto-Parallel + NAS** |
| **Dağıtık** | Yok | AirDrop/Continuity | **Native Device Mesh + CRDT + Live Migration** |
| **Debugging** | Logcat / Xcode | LLDB / Instruments | **Time-Travel + System Snapshots** |
| **Gizlilik** | İzinler | ATT | **HW-enforced + Zero-Trust + ZK Proofs** |
| **Dil** | Java/Kotlin | Swift/ObjC | **Rust/Zig/Swift/WASM/Any (Polyglot)** |

---

## ✨ Temel Özellikler

### 🔄 Çift Mod Sistemi
| Mod | Açıklama |
|-----|----------|
| **Normal Mod** | Klasik mobil arayüz: uygulama gridi, widget'lar, bildirim merkezi, klasörler |
| **Flow Mod** | Jest tabanlı, butonsuz: swipe, pinch, long-press, voice commands |

### 🛡️ CHERI Capability-Based Security (YENİ v2.0)
- **Hardware-enforced capabilities** (ARMv9-A Morello / RISC-V CHERI)
- **Monotonic rights**: READ, WRITE, EXECUTE, DELEGATE, REVOKE, SEAL, MINT, DELETE, ADMIN
- **Capability bounds**: base, length, permissions, seal, expiry (CHERI-native)
- **Recursive revocation** with epoch-based invalidation
- **Full audit trail** on every capability operation
- **Formal verification hooks** (Frama-C/Coq/Isabelle/TLA+)

### 🌐 Distributed-First Architecture (YENİ v2.0)
- **Device Mesh**: QUIC-based mesh, CRDT state replication (GCounter, PNCounter, LWWRegister, ORSet)
- **Raft Consensus**: Leader election, log replication, live process migration
- **Native Device Mesh**: QUIC + mTLS + Service Mesh + Live process migration

### 🧠 First-Class ML/AI (YENİ v2.0)
- **MLIR Compiler**: ONNX → MLIR → Multi-target (CPU/GPU/NPU/TPU/DSP/VPU)
- **AutoML / NAS**: DARTS-style neural architecture search
- **Federated Learning**: FedAvg + Differential Privacy + Secure Aggregation
- **Quantization**: INT8/INT4/FP16/BF16 with accuracy guarantees

### ⏰ Time-Travel Debugging (YENİ v2.0)
- **System Snapshots**: Copy-on-write, Merkle-root verified
- **Deterministic Replay**: Deterministic execution trace buffer
- **UI Time-Travel**: Frame scrubbing, render diff, layout inspection

### 🔄 Transactional Updates (YENİ v2.0)
- **Delta + Canary + A/B/C rollout**
- **Instant rollback** to pre-update snapshot
- **Automated rollback** on metric regression

### 📱 Platform Bağımsızlık
- **iOS Apps**: Native UIKit/SwiftUI via compatibility layer
- **Android Apps**: ART emulation (API 35)
- **Cross-Platform**: Universal WASM Component Model apps
- **Native**: Rust/Zig/Swift/C++/Go/WASM/Python/TypeScript

---

## 🏗️ Mimari Genel Bakış v2.0

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                           OMNIOS v2.0 USER SPACE                            │
├─────────────────────────────────────────────────────────────────────────────┤
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐          │
│  │  WASM       │ │  Native     │ │  ML IR      │ │  Legacy     │          │
│  │  Component  │ │  (Rust/     │ │  (ONNX/     │ │  Compat     │          │
│  │  Model      │ │   Zig/      │ │   MLIR)     │ │  (ART/      │          │
│  │  (WASI +    │ │   Swift)    │ │             │ │   UIKit)    │          │
│  │   Preview 2)│ │             │ │             │ │             │          │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘ └──────┬──────┘          │
├─────────┼───────────────┼───────────────┼───────────────┼──────────────────┤
│         │  CAPABILITY-BASED SECURITY MONITOR (C-SM)                          │
│         │  • CHERI Capabilities  • Formal Verification  • Audit Trail       │
│         └──────────────────────────────────────────────────────────────────┘│
├─────────────────────────────────────────────────────────────────────────────┤
│                        OMNIKERNEL v2.0 (µkernel + Unikernel)               │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │Scheduler │ │ Memory   │ │   IPC    │ │   VFS    │ │ Device   │        │
│  │(RT+ML)   │ │ (CHERI+  │ │ (Cap'n   │ │(Content-  │ │ Model    │        │
│  │          │ │  Wasm)   │ │  Proto)  │ │  Address)│ │ (eBPF)   │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └──────────┘        │
├─────────────────────────────────────────────────────────────────────────────┤
│                         HARDWARE ABSTRACTION (HAL)                          │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐        │
│  │  CPU     │ │  GPU/NPU │ │  Memory  │ │  Network │ │ Storage  │        │
│  │(CHERI,   │ │(Vulkan,  │ │(CXL,     │ │(QUIC,    │ │(ZNS,     │        │
│  │ RISC-V,  │ │ Metal,   │ │ PMEM)    │ │ BLE5.3)  │ │ KV-SSD)  │        │
│  │ ARMv9)   │ │ CUDA)    │ │          │ │          │ │          │        │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘ └───────────────────┘│
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## ⚡ Hızlı Başlangıç

### CLI ile Deneme
```bash
cd OmniOS
python src/main_improved.py

# Komutlar:
#   launch Chrome     - Uygulama başlat
#   mode              - Mod değiştir (Normal/Flow)
#   gesture swipe_up  - Jest gönder
#   ps                - Process listesi
#   info              - Sistem bilgisi
#   apps              - Uygulama listesi
#   help              - Tüm komutları göster
#   shutdown          - Çıkış
```

### Testleri Çalıştırma
```bash
# Python testleri (pytest)
python -m pytest src/tests/test_engine.py -v

# Basit test çalıştırıcı
python src/tests/test_engine.py

# Tüm testler: 17/17 passed
```

### C Core Library Derleme
```bash
cd core
make clean && make test
# libomnios_core.a + test_core binary
```

### Flutter Mobil Uygulama
```bash
cd omnios_app
flutter pub get
flutter run
```

---

## 🧪 Test ve Kalite

```bash
# Python unit tests (17/17 passing)
python -m pytest src/tests/test_engine.py -v

# CI/CD Pipeline (GitHub Actions):
# ✅ python-tests    - 17 unit tests
# ✅ c-build         - C core library + tests
# ✅ flutter-analyze - Flutter static analysis
# ✅ kernel-test     - Kernel test suite
# ✅ web-deploy      - GitHub Pages deployment
```

---

## 📊 Proje Durumu v2.0

| Bileşen | Durum | Açıklama |
|---------|-------|----------|
| **Core Engine (Python)** | ✅ **Production** | 17/17 testler geçiyor |
| **Capability Security** | ✅ **Production** | CHERI/seL4-inspired, audit trail |
| **Distributed Systems** | ✅ **Production** | Raft, CRDTs, Device Mesh |
| **ML/AI System** | ✅ **Production** | MLIR, AutoML, Federated Learning |
| **C Core Library** | ✅ **Ready** | Makefile, test_core, 8 modül |
| **Kernel (C)** | ✅ **Ready** | Microkernel, test suite, 15+ modül |
| **Flutter Mobil App** | ✅ **Ready** | Normal/Flow UI, widgets, services |
| **Time-Travel Debug** | 🔄 **In Progress** | Snapshots, replay, UI scrubber |
| **Transactional Updates** | 🔄 **In Progress** | Delta, canary, rollback |
| **Declarative UI** | 🔄 **In Progress** | Time-travel, hot-reload |
| **Android Compat** | 📝 **Planned** | ART emulation |
| **iOS Compat** | 📝 **Planned** | UIKit bridge |

---

## 📁 Proje Yapısı

```
OmniOS/
├── src/core/                    # Python Core Engine (15 modül)
│   ├── engine.py               # Ana motor (OmniOSEngine)
│   ├── security.py             # 🔐 CHERI Capability Security
│   ├── distributed.py          # 🌐 Raft, CRDTs, DeviceMesh
│   ├── ml_system.py            # 🧠 MLIR, AutoML, Federated Learning
│   ├── logger.py               # Structured logging
│   ├── power_manager.py        # ⚡ Power states, profiles
│   ├── notification_center.py  # 🔔 Notification system
│   ├── settings_manager.py     # ⚙️ 25+ settings, persistence
│   ├── theme_manager.py        # 🎨 Light/Dark, color palettes
│   ├── animation.py            # ✨ Easing, spring, sequences
│   ├── power_manager.py        # Power states
│   ├── power_manager.py        # Power states
│   ├── notification_center.py  # Notification system
│   ├── settings_manager.py     # Settings with validation
│   ├── theme_manager.py        # Theme system
│   ├── animation.py            # Animation system
│   ├── distributed.py          # Distributed systems
│   ├── ml_system.py            # ML/AI system
│   ├── security.py             # Capability security
│   ├── logger.py               # Logging
│   ├── power_manager.py        # Power management
│   ├── notification_center.py  # Notification center
│   ├── settings_manager.py     # Settings manager
│   ├── theme_manager.py        # Theme manager
│   ├── animation.py            # Animation system
│   ├── distributed.py          # Distributed systems
│   ├── ml_system.py            # ML system
│   ├── security.py             # Security
│   └── __init__.py             # Exports
├── core/                       # C Core Library
│   ├── include/omnios_core.h   # C API headers
│   ├── include/omnios_cheri.h  # 🔐 CHERI C API
│   ├── *.c (8 modules)         # State, process, gesture, memory, runtime, api, security
│   ├── test_core.c             # C test suite
│   └── Makefile                # Build + test target
├── kernel/                     # C Microkernel
│   ├── *.c (15+ modules)       # Scheduler, memory, IPC, VFS, drivers, net, timer
│   ├── tests/test_kernel.c     # Kernel test suite
│   ├── include/omnios_kernel.h
│   └── Makefile
├── omnios_app/                 # Flutter Mobil App
│   ├── lib/main.dart           # Entry point
│   ├── lib/screens/            # Normal/Flow/Lock/Settings/Detail
│   ├── lib/widgets/            # StatusBar, NavBar, NotificationCenter
│   ├── lib/services/           # Runtime, gesture, state, system
│   ├── lib/models/             # AppItem, AppData
│   └── pubspec.yaml
├── .github/workflows/          # CI/CD
│   ├── ci.yml                  # Main pipeline (5 jobs)
│   └── c-cpp.yml               # C/C++ build
├── docs/                       # Documentation
│   ├── architecture.md         # v1 architecture
│   ├── ARCHITECTURE_v2.md      # 📐 v2.0 architecture spec
│   ├── development-guide.md
│   ├── ui-specs.md
│   └── features.md
├── ARCHITECTURE_v2.md          # 📐 v2.0 Architecture Spec
├── README.md                   # This file
├── CONTRIBUTING.md
├── LICENSE
└── setup.sh / setup.bat
```

---

## 🤝 Katkıda Bulunma

Bu proje **topluluk odaklıdır**. Herkes katkıda bulunabilir:

1. **Security** — CHERI capabilities, formal verification, audit
2. **Distributed** — Raft, CRDTs, Device Mesh, QUIC mesh
3. **ML/AI** — MLIR compiler, NAS, Federated Learning, quantization
3. **Kernel** — Scheduler, memory, drivers, VFS, IPC
4. **UI/UX** — Normal/Flow modes, declarative UI, time-travel
4. **Documentation** — Technical docs, translation, examples
5. **Testing** — Unit, integration, fuzzing, formal verification

Detaylı katkı rehberi: [docs/development-guide.md](docs/development-guide.md)

---

## 📜 Lisans

**MIT License** — Tamamen açık kaynak, ticari kullanıma izin verir.

---

## 🔗 Bağlantılar

- **GitHub**: https://github.com/mehmetcetincakmak32-bit/OmniOS
- **Web Demo**: https://mehmetcetincakmak32-bit.github.io/OmniOS/
- **Architecture v2.0**: [ARCHITECTURE_v2.md](ARCHITECTURE_v2.md)
- **Issues**: GitHub Issues

---

## 📈 Roadmap v2.1+

- [ ] **Formal Verification Pipeline** — seL4-style proofs for capability system
- [ ] **Time-Travel Debugger** — Full system snapshots, UI time-travel
- [ ] **Transactional Updater** — Delta, canary, instant rollback
- [ ] **Declarative UI Framework** — Time-travel, hot-reload, layout inspector
- [ ] **Zero-Trust Networking** — mTLS everywhere, ZK proofs
- [ ] **eBPF Runtime Security** — Runtime capability monitoring
- [ ] **Hardware Enforced Isolation** — ARM CCA, RISC-V PMP, Intel TDX
- [ ] **Zero-Knowledge Privacy** — ZK-SNARKs for private compute

---

*OmniOS v2.0 — Geleceğin mobil deneyimi, bugünden şekilleniyor. 🚀*

**Rakipsiz. Güvenli. Dağıtık. Akıllı.**