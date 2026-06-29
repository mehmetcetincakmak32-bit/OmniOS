# OmniOS

**Tek OS, Tüm Platformlar — Mobil Deneyimi Yeniden Tanımlıyoruz**

> OmniOS, mevcut bir mobil işletim sistemi üzerinde ikinci bir sistem katmanı olarak çalışan, hem iOS hem Android uygulamalarını çalıştırabilen, iki farklı kullanım modu sunan devrim niteliğinde bir mobil platform konseptidir.

---

## Neden OmniOS?

Günümüzde mobil kullanıcılar iki büyük platform arasında sıkışmış durumda:
- **iOS kullanıcısıysanız**, Android'in özgürlüğünden mahrum kalıyorsunuz
- **Android kullanıcısıysanız**, iOS'un kaliteli uygulama ekosistemine erişemiyorsunuz
- **İkisini de kullanmak istiyorsanız**, iki telefon taşımak zorundasınız

**OmniOS bu sorunu kökünden çözer.**

---

## Temel Özellikler

### Çift Mod Sistemi

| Mod | Açıklama |
|-----|----------|
| **Normal Mod** | Klasik mobil arayüz, uygulama gridi, widget'lar, bildirim merkezi |
| **Flow Mod** | Tamamen jest/hareket tabanlı minimalist arayüz, buton yok |

### Platform Bağımsızlık
- **iOS uygulamaları** — UIKit/SwiftUI uyumluluk katmanı sayesinde native çalıştırma
- **Android uygulamaları** — ART (Android Runtime) emulasyonu ile tam uyum
- **Cross-platform** — Her iki platformda da çalışan evrensel uygulamalar

### Mevcut Sistem Üzerinde Çalışma
- Root/Jailbreak gerektirmez
- Mevcut işletim sistemini değiştirmez
- İkinci bir katman olarak paralel çalışır
- İstenildiğinde devre dışı bırakılabilir

---

## Normal Mod

Klasik mobil arayüz deneyimini sunar:

- **Uygulama Gridi** — 4×4 veya 4×5 düzen, özelleştirilebilir
- **Klasör Desteği** — Uygulamaları gruplama
- **Widget'lar** — Boyutlandırılabilir, sürükle-bırak
- **Bildirim Merkezi** — Zaman bazlı gruplama, öncelik sıralaması
- **Hızlı Ayarlar** — Özelleştirilebilir panel
- **Kilit Ekranı** — Bildirim önizleme, hızlı erişim
- **Tema Desteği** — Açık/Karanlık mod, 16 milyon renk

![Normal Mod Taslak](docs/screenshots/normal-mode.png)

---

## Flow Mod

Tamamen jest tabanlı, butonsuz bir arayüz:

| Jest | İşlev |
|------|-------|
| ⬆️ **Yukarı çek** | Uygulama menüsü (akıllı öneriler) |
| ➡️ **Sağa kaydır** | Son kullanılan uygulamalar |
| ⬅️ **Sola kaydır** | Bildirimler (öncelik sıralı) |
| ⬇️ **Aşağı kaydır** | Bağlamsal hızlı ayarlar |
| ✌️ **Çift dokun** | Ana ekrana dön |
| ✊ **Uzun basma** | Sesli komut modu |
| 🤏 **Parmak yakınlaştırma** | Cover flow uygulama seçici |

![Flow Mod Taslak](docs/screenshots/flow-mode.png)

---

## Mimari Genel Bakış

```
┌─────────────────────────────────────────────────────┐
│                    OmniOS UI Katmanı                 │
│  ┌──────────────────┐  ┌──────────────────────────┐ │
│  │   Normal Mod     │  │       Flow Mod           │ │
│  │  (Widget'lar,    │  │  (Jest motoru, gesture   │ │
│  │   Grid, Paneller)│  │   recognition, animasyon) │ │
│  └──────┬───────────┘  └──────────┬───────────────┘ │
├─────────┼─────────────────────────┼─────────────────┤
│         └──────────┬──────────────┘                 │
│                    ▼                                │
│           ┌────────────────┐                        │
│           │   Mod Yöneticisi  │                     │
│           └────────────────┘                        │
├─────────────────────────────────────────────────────┤
│               Uyumluluk Katmanı                     │
│  ┌──────────────────┐  ┌──────────────────────────┐ │
│  │  Android Runtime │  │   iOS Runtime            │ │
│  │  (ART Emulator)  │  │   (UIKit Emulator)       │ │
│  │  API Seviye 35   │  │   API Seviye 18          │ │
│  └──────┬───────────┘  └──────────┬───────────────┘ │
├─────────┼─────────────────────────┼─────────────────┤
│         └──────────┬──────────────┘                 │
│                    ▼                                │
│           ┌────────────────┐                        │
│           │ Mevcut İşletim  │                       │
│           │ Sistemi (iOS/   │                       │
│           │ Android)        │                       │
│           └────────────────┘                        │
└─────────────────────────────────────────────────────┘
```

Detaylı mimari için: [docs/architecture.md](docs/architecture.md)

---

## Teknik Gereksinimler (Taslak)

### Minimum Donanım
- **İşlemci**: ARM64 (Apple A12+ / Snapdragon 845+)
- **RAM**: 4 GB+
- **Depolama**: 2 GB boş alan
- **Ekran**: 5.5"+, 1080p+

### Hedef platformlar
- iOS 15.0+
- Android 11.0+ (API 30+)

---

## Proje Durumu

| Bileşen | Durum | Açıklama |
|---------|-------|----------|
| Konsept Tasarım | ✅ Tamamlandı | Bu doküman |
| Normal Mod UI Spec | ✅ Tamamlandı | docs/ui-specs.md |
| Flow Mod UI Spec | ✅ Tamamlandı | docs/ui-specs.md |
| Mimari Doküman | ✅ Tamamlandı | docs/architecture.md |
| Python Prototip | 🔧 Temel versiyon | src/ klasörü |
| Android Uyumluluk | 📝 Taslak | Geliştirilmeli |
| iOS Uyumluluk | 📝 Taslak | Geliştirilmeli |
| Native Implementation | ❌ | Katkını bekliyor |

---

## Katkıda Bulunma

Bu proje **topluluk odaklıdır**. Herkes katkıda bulunabilir:

1. **Frontend UI** — Normal/Flow mod arayüz tasarımı
2. **Uyumluluk Katmanı** — iOS/Android runtime geliştirme
3. **Dokümantasyon** — Teknik doküman, çeviri
4. **Test** — Farklı cihaz ve platformlarda test
5. **Tasarım** — UI/UX, ikon seti, animasyon

Detaylı katkı rehberi: [docs/development-guide.md](docs/development-guide.md)

---

## Lisans

**MIT License** — Tamamen açık kaynak, ticari kullanıma izin verir.

---

## İletişim

Proje GitHub üzerinden yönetilmektedir. Issue açabilir, tartışmalara katılabilirsiniz.

---

*OmniOS — Geleceğin mobil deneyimi, bugünden şekilleniyor.*
