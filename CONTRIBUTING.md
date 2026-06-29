# OmniOS Katkı Rehberi

## Hoş Geldiniz!

OmniOS açık kaynak bir projedir. Kod, dokümantasyon, tasarım, test — her türlü katkı değerlidir.

## Başlarken

```bash
git clone https://github.com/mehmetcetincakmak32-bit/OmniOS.git
cd OmniOS
python src/tests/test_engine.py  # Testlerin geçtiğini doğrula
```

## Katkı Türleri

### 🐞 Hata Bildirimi
- **GitHub Issues** kullanın
- Başlık: kısa ve açıklayıcı
- Adımlar, beklenen/gerçekleşen davranış, ekran görüntüsü

### 💡 Özellik Önerisi
- Yeni bir **Issue** açın
- Ne, neden, nasıl çalışmalı?

### 🧑‍💻 Kod Katkısı
1. Repo'yu fork edin
2. Branch: `feature/aciklama` veya `fix/aciklama`
3. Commit: `feat:`, `fix:`, `docs:`, `refactor:`, `test:`
4. Pull Request gönderin

## Kod Standartları

**Python:**
- PEP 8, satır uzunluğu 100
- Type hints kullanın

**C:**
- snake_case, OMNOS_HEADER_H guards
- -Wall -Wextra ile hatasız derlenmeli

**Flutter/Dart:**
- `flutter analyze` hatasız
- const tercih edin

## Test
```bash
python src/tests/test_engine.py    # Python testleri (pytest)
python src/tests/benchmark.py       # Performans testi
cd core && make test                # C testleri
```

## Lisans
Katkılarınız MIT lisansı altında kabul edilir.
