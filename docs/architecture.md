# OmniOS Mimari Dokümanı

## 1. Genel Mimari

OmniOS, **katmanlı (layered) bir mimari** ile inşa edilmiştir. Her katman bir altındaki katmanın hizmetlerini kullanır ve bir üstündeki katmana soyutlanmış hizmet sunar.

```
┌────────────────────────────────────────────────────────────┐
│                     KULLANICI ARAYÜZÜ                       │
│  ┌─────────────────────┐  ┌─────────────────────────────┐  │
│  │     Normal Mod      │  │         Flow Mod            │  │
│  │─────────────────────│  │─────────────────────────────│  │
│  │• App Grid           │  │• Gesture Recognition        │  │
│  │• Widget Engine      │  │• Gesture → Action Mapping   │  │
│  │• Notification Center│  │• Minimal UI Renderer        │  │
│  │• Quick Settings     │  │• Context Engine             │  │
│  │• Lock Screen        │  │• Predictive UI              │  │
│  └──────────┬──────────┘  └──────────────┬──────────────┘  │
│             └──────────────┬──────────────┘                │
│                            ▼                               │
│              ┌────────────────────────┐                    │
│              │     MOD YÖNETİCİSİ     │                    │
│              │────────────────────────│                    │
│              │• Mod State Machine     │                    │
│              │• Transition Manager    │                    │
│              │• Shared State Store    │                    │
│              │• Event Bus             │                    │
│              └────────────┬───────────┘                    │
├───────────────────────────┼────────────────────────────────┤
│                           ▼                                │
│              ┌────────────────────────┐                    │
│              │  UYGULAMA YÖNETİCİSİ   │                    │
│              │────────────────────────│                    │
│              │• App Registry          │                    │
│              │• App Lifecycle Manager │                    │
│              │• Resource Allocator    │                    │
│              │• Sandbox Manager       │                    │
│              └────────────┬───────────┘                    │
├───────────────────────────┼────────────────────────────────┤
│                           ▼                                │
│              ┌────────────────────────┐                    │
│              │   UYUMLULUK KATMANI    │                    │
│              │────────────────────────│                    │
│              │  ┌──────────────────┐  │                    │
│              │  │  App Detector    │  │                    │
│              │  │  (Platform ID)   │  │                    │
│              │  └────────┬─────────┘  │                    │
│              │     ┌─────┴─────┐      │                    │
│              │     ▼           ▼      │                    │
│              │  ┌──────┐  ┌──────┐   │                    │
│              │  │iOS   │  │Android│  │                    │
│              │  │Runtime│  │Runtime│  │                    │
│              │  │(UIKit)│  │(ART)  │  │                    │
│              │  └───┬──┘  └───┬──┘   │                    │
│              └──────┼──────────┼─────┘                    │
├─────────────────────┼──────────┼──────────────────────────┤
│                     ▼          ▼                           │
│              ┌────────────────────────┐                    │
│              │   SİSTEM KATMANI       │                    │
│              │────────────────────────│                    │
│              │• OS Abstraction Layer  │                    │
│              │• Hardware Interface    │                    │
│              │• File System Bridge    │                    │
│              │• Network Manager       │                    │
│              │• Security Module       │                    │
│              └────────────────────────┘                    │
├────────────────────────────────────────────────────────────┤
│                    MEVCUT İŞLETİM SİSTEMİ                   │
│                    (iOS / Android / Diğer)                  │
└────────────────────────────────────────────────────────────┘
```

---

## 2. Katman Detayları

### 2.1 UI Katmanı
Kullanıcı ile etkileşime giren en üst katman.

**Normal Mod Bileşenleri:**
- **App Grid**: Uygulamaların ızgara şeklinde görüntülenmesi
- **Widget Engine**: Boyutlandırılabilir, sürüklenebilir widget'lar
- **Notification Center**: Bildirimleri toplama, gruplama, önceliklendirme
- **Quick Settings**: Sık kullanılan ayarlara hızlı erişim
- **Lock Screen**: Güvenlik ve bildirim önizleme

**Flow Mod Bileşenleri:**
- **Gesture Recognition**: Parmak hareketlerini algılama ve yorumlama
- **Gesture-Action Mapping**: Her jestin bir aksiyona bağlanması
- **Minimal UI Renderer**: Gereksiz UI elemanlarını gizleme
- **Context Engine**: Kullanıcının bağlamına göre içerik sunma

### 2.2 Mod Yöneticisi
İki mod arasındaki geçişleri ve paylaşılan durumu yönetir.

```
Mod Yöneticisi Durum Makinesi:
                    ┌─────────┐
                    │  BAŞLANGIÇ│
                    └────┬────┘
                         │
                    ┌────▼────┐
              ┌─────│ NORMAL  │──────┐
              │     │  MOD    │      │
              │     └─────────┘      │
              │         │            │
         ┌────▼────┐    │    ┌───────▼───┐
         │  NORMAL │    │    │  FLOW     │
         │  → FLOW │◄───┴───►│  → NORMAL │
         │  GEÇİŞ  │         │  GEÇİŞ   │
         └─────────┘         └───────────┘
              │                    │
              └──────┬─────────────┘
                     ▼
               ┌──────────┐
               │   FLOW   │
               │   MOD    │
               └──────────┘
```

Geçişler sırasında:
- Mevcut modun durumu kaydedilir
- Animasyon oynatılır
- Yeni mod başlatılır
- Kullanıcıya geçiş bildirilmez (sorunsuz geçiş)

### 2.3 Uygulama Yöneticisi
Yüklü uygulamaların kaydını tutar, yaşam döngülerini yönetir.

| İşlev | Açıklama |
|-------|----------|
| App Registry | Yüklü tüm uygulamaların veritabanı |
| Lifecycle | Çalıştırma, duraklatma, durdurma yönetimi |
| Resource | RAM, CPU, pil kullanımı tahsisi |
| Sandbox | Her uygulamanın izole çalışması |

### 2.4 Uyumluluk Katmanı
OmniOS'un en kritik katmanı. iOS ve Android uygulamalarını çalıştırır.

**Uygulama Tanıma Süreci:**
```
Uygulama Dosyası Gelir
        ↓
┌───────────────────┐
│  Dosya Analizi    │
│  - İmza kontrolü  │
│  - Binary tespiti │
│  - API çağrıları  │
└───────────────────┘
        ↓
┌───────────────────┐
│ Platform Tespiti  │
│ - Mach-O → iOS    │
│ - APK/DEX → Android│
│ - Universal → Her │
│   ikisi de        │
└───────────────────┘
        ↓
┌───────────────────┐
│ Runtime Seçimi    │
│ iOS → UIKit EMU   │
│ Android → ART EMU │
└───────────────────┘
        ↓
      ÇALIŞTIR
```

### 2.5 Sistem Katmanı
Mevcut işletim sistemi ile köprü görevi görür.

- **OS Abstraction Layer**: iOS ve Android arasındaki farklılıkları soyutlar
- **Hardware Interface**: Kamera, GPS, sensörlere erişim
- **File System Bridge**: Dosya sistemi erişimini yönetir
- **Security Module**: İzinleri ve güvenlik politikalarını yönetir

---

## 3. Veri Akışları

### 3.1 Uygulama Başlatma
```
Kullanıcı → Uygulama İkonuna Dokunur
    ↓
UI Katmanı → App Manager'a İletir
    ↓
App Manager → Uyumluluk Katmanına Yönlendirir
    ↓
Uyumluluk Katmanı → Platformu Tespit Eder
    ↓
İlgili Runtime → Uygulamayı Başlatır
    ↓
Sistem Katmanı → Kaynakları Tahsis Eder
    ↓
Uygulama Çalışır
```

### 3.2 Bildirim Akışı
```
Mevcut OS → Bildirim Alır
    ↓
Sistem Katmanı → Bildirimi Yakalar
    ↓
Mod Yöneticisi → Aktif Mode İletir
    ↓
Normal Mod → Bildirim Merkezine Ekler
    ↓
Flow Mod → Gesture Hint Gösterir
    ↓
Kullanıcı → Bildirimi Görür
```

---

## 4. Güvenlik Mimarisi

### 4.1 Sandbox (Kum Havuzu)
Her uygulama kendi izole alanında çalışır:
- Kendi dosya sistemine sahiptir
- Diğer uygulamaların verilerine erişemez
- Kaynak kullanımı sınırlandırılmıştır

### 4.2 İzin Sistemi
OmniOS'un kendi izin katmanı mevcuttur:
- Her izin ayrı ayrı sorulur
- İzinler iptal edilebilir
- Platformdan bağımsız izin yönetimi

### 4.3 Veri Şifreleme
- Tüm yerel veriler şifrelenir
- Platformlar arası veri aktarımı şifrelidir
- Biyometrik doğrulama desteği

---

## 5. Performans Hedefleri

| Metrik | Hedef | Açıklama |
|--------|-------|----------|
| UI Tepki Süresi | <16ms | 60fps akıcılık |
| Uygulama Başlatma | <2s | iOS/Android uygulamaları |
| Mod Geçişi | <500ms | Normal ↔ Flow |
| Pil Tüketimi | <%15 artış | Native'e göre |
| RAM Kullanımı | <500MB | OmniOS katmanı |
| Depolama | <2GB | OmniOS kurulumu |
