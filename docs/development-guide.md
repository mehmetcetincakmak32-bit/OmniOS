# OmniOS Geliştirici Rehberi

## Hoş Geldiniz!

OmniOS açık kaynak bir projedir ve her türlü katkıya açığız. İster yeni bir özellik ekleyin, ister hata bildirin, ister dokümantasyonu iyileştirin — her katkı değerlidir.

---

## 1. Başlarken

### 1.1 Depoyu Klonlama
```bash
git clone https://github.com/[kullaniciadi]/OmniOS.git
cd OmniOS
```

### 1.2 Ortamı Hazırlama
```bash
# Python prototip için (opsiyonel)
python --version  # Python 3.8+ olmalı

# Geliştirme araçları
# Flutter: https://flutter.dev/docs/get-started/install
# Swift: Xcode 14+ (macOS)
# Kotlin: Android Studio (opsiyonel)
```

### 1.3 Prototipi Çalıştırma
```bash
cd OmniOS
python src/main_improved.py

# Python testleri
python src/tests/test_engine.py -v

# Dokümantasyon
gitBook pages
```

---

## 2. Proje Yapısı

```
OmniOS/
├── README.md                 # Ana doküman
├── .gitignore
├── requirements.txt
│
├── docs/                     # Dokümantasyon
│   ├── README.md             # Doküman indeksi
│   ├── concept.md            # Konsept dokümanı
│   ├── architecture.md       # Mimari dokümanı
│   ├── features.md           # Özellik listesi
│   ├── technical-requirements.md  # Teknik gereksinimler
│   ├── ui-specs.md           # UI spesifikasyonları
│   ├── development-guide.md  # Bu dosya
│   └── screenshots/          # Ekran görüntüleri
│
├── src/                      # Python prototip
│   ├── main.py               # Ana giriş noktası
│   ├── modes/                # Mod bileşenleri
│   │   ├── normal_mode.py
│   │   └── flow_mode.py
│   ├── ui/                   # UI bileşenleri
│   │   ├── phone_canvas.py
│   │   └── widgets.py
│   ├── apps/                 # Uygulama yönetimi
│   │   └── app_manager.py
│   └── compatibility/        # Uyumluluk katmanı
│       ├── android_layer.py
│       └── ios_layer.py
│
├── native/                   # Native implementasyon (TBD)
│   ├── ios/                  # iOS/Swift bileşenleri
│   ├── android/              # Android/Kotlin bileşenleri
│   └── flutter/              # Flutter cross-platform
│
└── assets/                   # Görsel kaynaklar (TBD)
    ├── icons/
    └── mockups/
```

---

## 3. Nasıl Katkıda Bulunurum?

### 3.1 Hata Bildirme (Bug Report)
1. **Issues** sekmesine gidin
2. "New Issue" tıklayın
3. Şu bilgileri ekleyin:
   - Başlık (kısa ve açıklayıcı)
   - Adımlar (hatayı nasıl tekrarlarım)
   - Beklenen davranış
   - Gerçekleşen davranış
   - Ekran görüntüsü (varsa)
   - Ortam bilgisi (cihaz, OS)

### 3.2 Yeni Özellik Önerme (Feature Request)
1. **Issues** → "New Issue" → "Feature Request"
2. Şunları belirtin:
   - Özelliğin ne olduğu
   - Neden gerekli olduğu
   - Nasıl çalışması gerektiği
   - Referanslar (varsa benzer projeler)

### 3.3 Kod Katkısı (Pull Request)

**Adım Adım:**
1. Repo'yu fork edin
2. Yeni bir branch oluşturun:
   ```bash
   git checkout -b feature/benim-ozelligim
   ```
3. Değişikliklerinizi yapın
4. Commit mesajı yazın:
   ```bash
   git commit -m "feat: yeni özellik açıklaması"
   ```
5. Branch'inizi push edin:
   ```bash
   git push origin feature/benim-ozelligim
   ```
6. GitHub'da Pull Request oluşturun

**Commit Mesajı Formatı:**
- `feat:` — Yeni özellik
- `fix:` — Hata düzeltme
- `docs:` — Dokümantasyon
- `style:` — Kod stili (işlev değişmez)
- `refactor:` — Kod yeniden düzenleme
- `test:` — Test ekleme
- `chore:` — Bakım işleri

---

## 4. Kod Standartları

### 4.1 Python (Prototip)
- PEP 8 standartları
- Tip ipuçları (type hints) kullanın
- Satır uzunluğu: 100 karakter
- Dökümantasyon string'i (docstring) ekleyin

```python
def get_apps_by_platform(self, platform: str) -> list:
    """Platforma göre uygulamaları filtrele.
    
    Args:
        platform: "all", "android", "ios"
        
    Returns:
        Filtrelenmiş uygulama listesi
    """
    ...
```

### 4.2 Flutter/Dart (Native)
- Dart format standardı
- final/const tercih edin
- Widget'ları küçük parçalara bölün
- `// TODO:` etiketleri ile eksikleri işaretleyin

### 4.3 Genel
- Anlamlı değişken isimleri kullanın
- Gereksiz yorum eklemeyin (kod kendini açıklasın)
- Test yazmayı unutmayın
- API değişikliklerinde dokümantasyonu güncelleyin

---

## 5. Geliştirme Süreci

### 5.1 Branch Stratejisi
```
main (kararlı)
  └── develop (geliştirme)
        ├── feature/xyz (yeni özellik)
        ├── fix/abc (hata düzeltme)
        └── docs/readme (dokümantasyon)
```

### 5.2 İş Akışı
```
1. Issue oluştur veya var olanı al
2. Branch oluştur
3. Geliştirmeyi yap
4. Test et
5. Pull Request aç
6. Code review
7. Birleştir (merge)
```

---

## 6. Öncelikli Alanlar

🚨 **Yardıma ihtiyacımız olan alanlar:**

| Alan | Zorluk | Beceri |
|------|--------|--------|
| iOS Uyumluluk Katmanı | Zor | Swift, Runtime |
| Android Uyumluluk Katmanı | Zor | Kotlin, ART |
| Normal Mod UI (Flutter) | Orta | Flutter/Dart |
| Flow Mod UI (Flutter) | Orta | Flutter/Dart |
| Gesture Recognition | Orta | C++ |
| Dokümantasyon Çeviri | Kolay | Çoklu dil |
| UI/UX Tasarım | Orta | Figma, Sketch |
| Test Otomasyonu | Orta | Python, CI/CD |

---

## 7. İletişim

- **GitHub Issues**: Teknik konular ve hatalar
- **Pull Requests**: Kod katkıları
- **Discussions**: Genel tartışmalar ve fikirler

---

## 8. Kod Davranış Kuralları

- Saygılı ve kapsayıcı olun
- Yapıcı geri bildirim verin
- Farklı görüşlere açık olun
- Herkesin katkısına değer verin

---

*Hadi birlikte geleceğin mobil deneyimini inşa edelim! 🚀*
