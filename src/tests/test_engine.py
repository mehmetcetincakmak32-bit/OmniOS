"""OmniOS Core Engine Testleri"""
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from src.core.engine import OmniOSEngine, SystemState
from src.core.mode_manager import Mode, ModeManager
from src.core.process_manager import ProcessManager, Process, ProcessState
from src.core.gesture_engine import GestureEngine, Gesture
from src.core.api_translator import APITranslator
from src.core.memory_manager import MemoryManager


def test_engine_boot():
    engine = OmniOSEngine()
    assert engine.state == SystemState.IDLE, f"IDLE olmali: {engine.state}"
    assert engine.mode_manager.current_mode == Mode.NORMAL, "Normal mod baslamali"
    print("[OK] Engine baslangic durumu dogru")


def test_engine_launch_app():
    engine = OmniOSEngine()
    events = []
    engine.on("launch", lambda d: events.append(d))

    result = engine.launch_app("Chrome")
    assert result, "Chrome baslatilabilmeli"
    assert engine.state == SystemState.RUNNING, "RUNNING olmali"
    assert len(events) > 0
    print("[OK] Uygulama baslatma basarili")


def test_engine_launch_nonexistent():
    engine = OmniOSEngine()
    result = engine.launch_app("OlmayanUygulama")
    assert not result, "Olmayan uygulama baslatilamaz"
    print("[OK] Olmayan uygulama baslatilamiyor")


def test_mode_toggle():
    engine = OmniOSEngine()
    assert engine.mode_manager.current_mode == Mode.NORMAL

    engine.toggle_mode()
    assert engine.mode_manager.current_mode == Mode.FLOW, "Flow moda gecmeli"

    engine.toggle_mode()
    assert engine.mode_manager.current_mode == Mode.NORMAL, "Normal moda donmeli"
    print("[OK] Mod gecisi basarili")


def test_mode_set():
    manager = ModeManager()
    manager.set_mode(Mode.FLOW)
    assert manager.current_mode == Mode.FLOW
    manager.set_mode(Mode.NORMAL)
    assert manager.current_mode == Mode.NORMAL
    print("[OK] Mod set etme basarili")


def test_process_create():
    pm = ProcessManager()
    p = pm.create("TestApp", "Android")

    assert p.name == "TestApp"
    assert p.platform == "Android"
    assert p.state == ProcessState.RUNNING
    assert len(pm.active_processes) == 1
    print(f"[OK] Process olusturma: {p}")


def test_process_kill():
    pm = ProcessManager()
    p = pm.create("KillMe", "iOS")
    assert p.is_active

    result = pm.kill("KillMe")
    assert result
    assert not p.is_active
    assert p.state == ProcessState.TERMINATED
    print("[OK] Process sonlandirma basarili")


def test_process_stats():
    pm = ProcessManager()
    pm.create("App1", "Android")
    pm.create("App2", "iOS")
    pm.create("App3", "Android")
    pm.kill("App1")

    stats = pm.get_stats()
    assert stats["active"] == 2
    assert stats["terminated"] >= 1
    print(f"[OK] Process istatistikleri: {stats['active']} aktif")


def test_gesture_recognition():
    ge = GestureEngine()
    g = ge.recognize("swipe_up")
    assert g == Gesture.SWIPE_UP
    print("[OK] Jest tanima basarili")


def test_gesture_execution():
    ge = GestureEngine()
    from src.core.mode_manager import Mode

    action = ge.execute(Gesture.SWIPE_UP, Mode.NORMAL)
    assert action == "open_app_menu"
    print(f"[OK] Jest calistirma: {action}")


def test_gesture_hints():
    ge = GestureEngine()
    hints = ge.get_gesture_hints("flow")
    assert len(hints) > 0
    print(f"[OK] Jest ipuclari: {len(hints)} adet")


def test_api_translator():
    t = APITranslator()
    android = t.ios_to_android("UIView")
    assert android == "android.view.View"

    ios = t.android_to_ios("android.widget.TextView")
    assert ios == "UILabel"
    print(f"[OK] API ceviri: UIView -> {android}")


def test_memory_manager():
    mm = MemoryManager(4096)
    mm.allocate("proc1", 512, "TestApp")
    assert mm.used == 512
    assert mm.available == 4096 - 512

    mm.allocate("proc2", 256, "PinnedTest", pin=True)
    assert mm.pinned == 256

    mm.free("proc1")
    assert mm.used == 0
    assert mm.pinned == 256
    print(f"[OK] Bellek yonetimi: {mm.usage_percent}% kullaniliyor")


def test_memory_pressure():
    mm = MemoryManager(100)

    mm.allocate("pinned1", 85, "PinnedApp", pin=True)
    assert mm.used == 0
    assert mm.pinned == 85
    assert mm.available == 15
    assert mm.get_status() == "warning"

    result = mm.allocate("too_big", 50, "TooBig")
    assert not result
    print(f"[OK] Bellek basinci: {mm.get_status()}, kullanilabilir: {mm.available}MB")


def test_plugin_system():
    from src.core.plugin_system import PluginManager
    pm = PluginManager()
    plugins = pm.get_all_plugins()
    print(f"[OK] Plugin sistemi: {len(plugins)} plugin bulundu")


def test_system_info():
    engine = OmniOSEngine()
    info = engine.get_system_info()
    assert info["state"] == "IDLE"
    assert info["mode"] == "normal"
    assert info["total_apps"] == 18
    print(f"[OK] Sistem bilgisi: {info['state']}, {info['total_apps']} uygulama")


def test_full_workflow():
    engine = OmniOSEngine()
    events = []
    engine.on("launch", lambda d: events.append(d))
    engine.on("mode", lambda d: events.append(d))
    engine.on("gesture", lambda d: events.append(d))

    engine.launch_app("YouTube")
    engine.toggle_mode()
    engine.process_gesture("swipe_up")
    engine.process_gesture("double_tap")

    assert engine.state == SystemState.RUNNING
    assert engine.mode_manager.current_mode == Mode.FLOW
    assert len(events) >= 4, f"En az 4 event olmali: {len(events)}"
    print(f"[OK] Tam is akisi: {len(events)} event, mod={engine.mode_manager.current_mode.value}")


if __name__ == "__main__":
    import pytest
    import sys
    print("OmniOS Core Engine Testleri");

    sys.exit(pytest.main([__file__, "-v"]))
