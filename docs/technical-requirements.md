# OmniOS Teknik Gereksinimler

## 1. Minimum Donanım Gereksinimleri

### 1.1 iOS Cihazlar

| Bileşen | Minimum | Önerilen |
|---------|---------|----------|
| İşlemci | Apple A12 Bionic | Apple A15+ |
| RAM | 4 GB | 6 GB+ |
| Depolama | 2 GB boş alan | 4 GB+ |
| Ekran | 5.5" 1080p | 6.1"+ OLED |
| iOS Sürümü | 15.0 | 17.0+ |
| Desteklenen Cihazlar | iPhone XS/XR+ | iPhone 14+ |

### 1.2 Android Cihazlar

| Bileşen | Minimum | Önerilen |
|---------|---------|----------|
| İşlemci | Snapdragon 845 / Exynos 9810 | Snapdragon 8 Gen 2+ |
| RAM | 4 GB | 8 GB+ |
| Depolama | 2 GB boş alan | 4 GB+ |
| Ekran | 5.5" 1080p | 6.3"+ AMOLED |
| Android Sürümü | 11.0 (API 30) | 14.0+ (API 34+) |
| Google Play Services | Opsiyonel | Opsiyonel |

---

## 2. Yazılım Gereksinimleri

### 2.1 Geliştirme Ortamı

| Araç | Versiyon | Amaç |
|------|----------|------|
| Python | 3.8+ | Prototip geliştirme |
| Flutter/Dart | 3.0+ | Native uygulama (hedef) |
| Swift | 5.5+ | iOS runtime bileşenleri |
| Kotlin | 1.8+ | Android runtime bileşenleri |
| C/C++ | C17/C++20 | Performans kritik bileşenler |

### 2.2 Kütüphane Bağımlılıkları

#### Prototip (Python)
- tkinter (dahili)
- Pillow (opsiyonel, görsel işleme)

#### Native (Gelecek)
- Flutter framework
- Platform channels
- FFI (Foreign Function Interface)

---

## 3. Platformlar Arası Farklılıklar

### 3.1 Dosya Sistemi

| Özellik | iOS | Android |
|---------|-----|---------|
| Uygulama Sandbox | Zorunlu | İsteğe bağlı |
| Dış Depolama | Yok | microSD desteği |
| Dosya Uzantısı Gizleme | Var | Yok |
| App Bundle | .ipa (zip) | .apk/.aab |
| Binary Formatı | Mach-O | DEX/ELF |

### 3.2 İzin Sistemi

| Kategori | iOS | Android | OmniOS |
|----------|-----|---------|--------|
| Kamera | Info.plist | Manifest | Birleşik sistem |
| Konum | Info.plist | Manifest | Birleşik sistem |
| Bildirim | Runtime | Runtime + Manifest | Birleşik sistem |
| Depolama | Sınırlı | Kapsamlı | Birleşik sistem |

### 3.3 API Farklılıkları

OmniOS uyumluluk katmanının çözmesi gereken temel API farklılıkları:

```
iOS UIKit                    Android View
─────────                    ────────────
UIView              →        View
UIViewController    →        Activity/Fragment
UITableView         →        RecyclerView
UICollectionView    →        RecyclerView
UINavigationController →    Navigation Component
UITabBarController  →       BottomNavigationView
UIAlertController   →       AlertDialog
UIScrollView        →       ScrollView/NestedScrollView
WKWebView           →       WebView
```

---

## 4. Performans Kriterleri

### 4.1 UI Performansı

| Metrik | Hedef | Ölçüm Yöntemi |
|--------|-------|---------------|
| Kare Hızı (FPS) | 60 (hedef 120) | Profiler |
| UI Tepki Süresi | <16ms | Touch latency test |
| Animasyon Geçişi | <300ms | Frame capture |
| Scroll Smoothness | Jank-free | Choreographer |

### 4.2 Uygulama Performansı

| Metrik | Native | OmniOS | Fark |
|--------|--------|--------|------|
| Başlatma Süresi | 500ms | <2s | +%300 |
| CPU Kullanımı | %100 | %115 | +%15 |
| RAM Kullanımı | 100MB | 130MB | +%30 |
| Grafik Performansı | %100 | %85 | -%15 |

### 4.3 Pil Tüketimi

| Senaryo | Native | OmniOS | Fark |
|---------|--------|--------|------|
| Boşta (1 saat) | %1 | %1.5 | +%0.5 |
| Sosyal Medya (1 saat) | %8 | %10 | +%2 |
| Oyun (1 saat) | %20 | %25 | +%5 |
| Video (1 saat) | %6 | %8 | +%2 |

---

## 5. Güvenlik Gereksinimleri

### 5.1 Uygulama Güvenliği

- Tüm uygulamalar kum havuzunda (sandbox) çalışmalı
- Uygulamalar arası veri paylaşımı yalnızca Intent/IPC ile
- Her uygulamanın benzersiz kimliği (App ID) olmalı
- Uygulama imzalaması zorunlu

### 5.2 Veri Güvenliği

- Tüm yerel veriler AES-256 ile şifrelenmeli
- Platformlar arası veri aktarımı TLS 1.3
- Biyometrik veriler Secure Enclave/TEE'de saklanmalı
- Hassas veriler için ek şifreleme

### 5.3 Ağ Güvenliği

- Tüm ağ trafiği HTTPS (TLS 1.3)
- Sertifika sabitleme (Certificate Pinning)
- DNS over HTTPS desteği
- VPN entegrasyonu

---

## 6. Uyumluluk Test Matrisi

### 6.1 iOS Uygulama Testleri

| Uygulama Türü | Örnek | Beklenen Durum |
|---------------|-------|----------------|
| UIKit Uygulaması | Safari, Notlar | ✅ Tam uyumlu |
| SwiftUI Uygulaması | Yeni iOS uygulamaları | 🔧 Kısmi uyum |
| Metal Oyun | Genshin Impact | ❌ Geliştirilmeli |
| ARKit Uygulaması | AR Measure | ❌ Geliştirilmeli |
| HealthKit | Apple Health | 🔧 Kısmi uyum |

### 6.2 Android Uygulama Testleri

| Uygulama Türü | Örnek | Beklenen Durum |
|---------------|-------|----------------|
| Jetpack Compose | Modern Android uyg. | 🔧 Kısmi uyum |
| View Binding | Klasik Android uyg. | ✅ Tam uyumlu |
| OpenGL ES | PUBG Mobile | ❌ Geliştirilmeli |
| NDK Uygulaması | Performans odaklı uyg. | 🔧 Kısmi uyum |
| Google Play Servis | Maps, Drive | 🔧 Emulasyon gerekli |

---

*Bu doküman proje geliştikçe güncellenecektir.*
