@echo off
title OmniOS Setup
echo ========================================
echo   OmniOS - Kurulum Araci
echo ========================================
echo.

REM Python kontrol
python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [HATA] Python bulunamadi! Python 3.8+ yukleyin.
    echo https://www.python.org/downloads/
    pause
    exit /b 1
)
echo [OK] Python bulundu

REM Git kontrol
git --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [UYARI] Git bulunamadi. Sadece lokal calisabilir.
) else (
    echo [OK] Git bulundu
)

REM Testleri calistir
echo.
echo Testler calistiriliyor...
python src/tests/test_engine.py
if %errorlevel% neq 0 (
    echo [HATA] Testler basarisiz!
    pause
    exit /b 1
)
echo [OK] Tum testler gecti

REM C kutuphanesini derle
echo.
echo C kutuphanesi derleniyor...
where gcc >nul 2>&1
if %errorlevel% equ 0 (
    cd core
    mingw32-make.exe 2>nul || make 2>nul
    if %errorlevel% equ 0 (
        echo [OK] C kutuphanesi derlendi
    ) else (
        echo [UYARI] C derleme basarisiz (opsiyonel)
    )
    cd ..
) else (
    echo [UYARI] GCC bulunamadi (C kutuphanesi atlandi)
)

echo.
echo ========================================
echo   Kurulum Tamamlandi!
echo ========================================
echo.
echo CLI ile baslatmak icin:  python src/main_improved.py
echo Web sim:                https://mehmetcetincakmak32-bit.github.io/OmniOS/
echo Test:                   python src/tests/test_engine.py
echo Benchmark:              python src/tests/benchmark.py
echo.
pause
