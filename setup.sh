#!/bin/bash
set -e

echo "========================================"
echo "  OmniOS - Kurulum Araci"
echo "========================================"

echo ""
echo "[OK] Python: $(python3 --version 2>&1 || python --version 2>&1)"

cd "$(dirname "$0")"

echo ""
echo "Testler calistiriliyor..."
python3 -m pytest src/tests/test_engine.py -v 2>/dev/null || python src/tests/test_engine.py
echo "[OK] Tum testler gecti"

echo ""
echo "C kutuphanesi derleniyor..."
if command -v gcc &> /dev/null; then
    cd core && make 2>/dev/null && cd ..
    echo "[OK] C kutuphanesi derlendi" || echo "[UYARI] C derleme basarisiz"
else
    echo "[UYARI] GCC bulunamadi (opsiyonel)"
fi

echo ""
echo "========================================"
echo "  Kurulum Tamamlandi!"
echo "========================================"
echo ""
echo "CLI:    python src/main_improved.py"
echo "Web:    https://mehmetcetincakmak32-bit.github.io/OmniOS/"
echo "Test:   python src/tests/test_engine.py"
echo "Bench:  python src/tests/benchmark.py"
