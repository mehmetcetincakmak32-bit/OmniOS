"""OmniOS Benchmark - Performans ve yuk testi"""
import sys
import os
import time
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))

from src.core.engine import OmniOSEngine
from src.core.gesture_engine import Gesture


class Benchmark:
    def __init__(self):
        self.results = {}

    def run_all(self):
        print("=" * 55)
        print("  OmniOS Performance Benchmark")
        print("=" * 55)

        self._bench("Engine Baslangic", self._test_engine_boot)
        self._bench("Uygulama Baslatma (10x)", self._test_app_launch)
        self._bench("Mod Gecisi (100x)", self._test_mode_switch)
        self._bench("Jest Isleme (100x)", self._test_gesture)
        self._bench("Process Yonetimi (50x)", self._test_process_mgmt)
        self._bench("API Ceviri (1000x)", self._test_api_translate)
        self._bench("Bellek Yonetimi (100x)", self._test_memory)
        self._bench("Tam Is Akisi", self._test_full_workflow)

        print("\n" + "=" * 55)
        print("  SONUCLAR")
        print("=" * 55)
        for name, elapsed in sorted(self.results.items(), key=lambda x: x[1]):
            print(f"  {name:<35} {elapsed:>8.3f}s")
        print("=" * 55)

    def _bench(self, name, fn):
        start = time.perf_counter()
        fn()
        elapsed = time.perf_counter() - start
        self.results[name] = elapsed
        status = "OK" if elapsed < 1.0 else "SLOW"
        print(f"  [{status}] {name:<35} {elapsed:.3f}s")

    def _test_engine_boot(self):
        for _ in range(10):
            e = OmniOSEngine()
            _ = e.get_system_info()

    def _test_app_launch(self):
        e = OmniOSEngine()
        apps = ["Chrome", "Safari", "Maps", "Gmail", "YouTube", "Music",
                 "Phone", "Settings", "Camera", "Notes"]
        for name in apps:
            e.launch_app(name)
        assert len(e.process_manager.active_processes) == len(apps)

    def _test_mode_switch(self):
        e = OmniOSEngine()
        for _ in range(100):
            e.toggle_mode()

    def _test_gesture(self):
        e = OmniOSEngine()
        gestures = ["swipe_up", "swipe_down", "swipe_left", "swipe_right",
                     "double_tap", "long_press", "pinch_out"]
        for _ in range(100):
            for g in gestures:
                e.process_gesture(g)

    def _test_process_mgmt(self):
        e = OmniOSEngine()
        pids = []
        for i in range(50):
            e.launch_app(f"App{i}")
        for p in e.process_manager.active_processes[:25]:
            e.kill_app(p.name)
        stats = e.process_manager.get_stats()
        assert stats["active"] + stats["terminated"] <= 50

    def _test_api_translate(self):
        from src.core.api_translator import APITranslator
        t = APITranslator()
        ios_classes = ["UIView", "UIViewController", "UITableView",
                       "UILabel", "UIButton", "UIImageView", "UIColor",
                       "NSData", "NSString", "NSArray"]
        for _ in range(100):
            for cls in ios_classes:
                t.ios_to_android(cls)
                t.android_to_ios(t.ios_to_android(cls))

    def _test_memory(self):
        from src.core.memory_manager import MemoryManager
        mm = MemoryManager(8192)
        for i in range(100):
            mm.allocate(f"proc{i}", i * 10, f"TestApp{i}")
        for i in range(50):
            mm.free(f"proc{i}")
        report = mm.get_report()
        assert report["available_mb"] > 0

    def _test_full_workflow(self):
        e = OmniOSEngine()
        events = []
        e.on("launch", lambda d: events.append(d))
        e.on("mode", lambda d: events.append(d))
        e.on("gesture", lambda d: events.append(d))

        e.launch_app("Chrome")
        e.launch_app("Maps")
        e.toggle_mode()
        for _ in range(5):
            e.process_gesture("swipe_up")
            e.process_gesture("double_tap")
        e.toggle_mode()
        e.kill_app("Chrome")
        e.kill_app("Maps")

        assert len(events) > 10
        assert e.state.name == "IDLE"

    def test_cli_integration(self):
        """CLI uzerinden entegrasyon testi"""
        import subprocess
        result = subprocess.run(
            [sys.executable, "-c", """
import sys, os
sys.path.insert(0, '.')
from src.core.engine import OmniOSEngine
e = OmniOSEngine()
print(e.get_system_info()['state'])
"""],
            capture_output=True, text=True, cwd=os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
        )
        assert "IDLE" in result.stdout
        return True


if __name__ == "__main__":
    b = Benchmark()
    b.run_all()
