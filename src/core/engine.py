"""OmniOS Core Engine - Ana sistem motoru"""
from enum import Enum, auto
from .mode_manager import ModeManager, Mode
from .process_manager import ProcessManager
from .gesture_engine import GestureEngine
from ..compatibility.android_layer import AndroidCompatibilityLayer
from ..compatibility.ios_layer import iOSCompatibilityLayer
from ..apps.app_manager import AppManager


class SystemState(Enum):
    BOOTING = auto()
    IDLE = auto()
    RUNNING = auto()
    SLEEPING = auto()
    SHUTDOWN = auto()


class OmniOSEngine:
    def __init__(self):
        self.state = SystemState.BOOTING
        self.mode_manager = ModeManager()
        self.process_manager = ProcessManager()
        self.gesture_engine = GestureEngine()
        self.app_manager = AppManager()
        self.android_layer = AndroidCompatibilityLayer()
        self.ios_layer = iOSCompatibilityLayer()

        self._listeners = {}
        self._boot_sequence()

    def _boot_sequence(self):
        self.emit("boot", "OmniOS baslatiliyor...")
        self.emit("boot", "Mod yoneticisi yuklendi")
        self.emit("boot", "Uyumluluk katmani hazir")
        self.emit("boot", "Android Runtime: " + self.android_layer.get_info()["status"])
        self.emit("boot", "iOS Runtime: " + self.ios_layer.get_info()["status"])
        self.state = SystemState.IDLE
        self.emit("boot", "OmniOS hazir")

    def launch_app(self, app_name: str) -> bool:
        app = None
        for a in self.app_manager.get_all_apps():
            if a.name.lower() == app_name.lower():
                app = a
                break

        if not app:
            self.emit("error", f"Uygulama bulunamadi: {app_name}")
            return False

        if app.platform == "android" and not self.android_layer.can_run(app):
            self.emit("error", f"Android uyumluluk hatasi: {app_name}")
            return False
        if app.platform == "ios" and not self.ios_layer.can_run(app):
            self.emit("error", f"iOS uyumluluk hatasi: {app_name}")
            return False

        platform_name = {"android": "Android", "ios": "iOS", "cross": "OmniOS"}.get(app.platform, "unknown")
        self.process_manager.create(app.name, platform_name)
        self.state = SystemState.RUNNING
        self.emit("launch", f"[{platform_name}] {app.name} baslatildi")
        return True

    def kill_app(self, app_name: str) -> bool:
        result = self.process_manager.kill(app_name)
        if result:
            self.emit("kill", f"{app_name} durduruldu")
            if not self.process_manager.active_processes:
                self.state = SystemState.IDLE
        return result

    def toggle_mode(self) -> str:
        new_mode = self.mode_manager.toggle()
        self.emit("mode", f"Mod degisti: {new_mode.value}")
        return new_mode.value

    def set_mode(self, mode: str) -> bool:
        try:
            m = Mode(mode)
            self.mode_manager.set_mode(m)
            self.emit("mode", f"Mod: {mode}")
            return True
        except ValueError:
            return False

    def process_gesture(self, gesture_name: str) -> str:
        gesture = self.gesture_engine.recognize(gesture_name)
        if gesture:
            action = self.gesture_engine.execute(gesture, self.mode_manager.current_mode)
            self.emit("gesture", f"Jest: {gesture_name} -> {action}")
            return action
        return "unknown"

    def on(self, event: str, callback):
        if event not in self._listeners:
            self._listeners[event] = []
        self._listeners[event].append(callback)

    def emit(self, event: str, data=None):
        if event in self._listeners:
            for cb in self._listeners[event]:
                cb(data)

    def get_system_info(self) -> dict:
        return {
            "state": self.state.name,
            "mode": self.mode_manager.current_mode.value,
            "active_processes": len(self.process_manager.active_processes),
            "total_apps": len(self.app_manager.get_all_apps()),
            "android_runtime": self.android_layer.get_info()["status"],
            "ios_runtime": self.ios_layer.get_info()["status"],
            "gestures_supported": len(self.gesture_engine.get_supported_gestures()),
        }

    def shutdown(self):
        self.process_manager.kill_all()
        self.state = SystemState.SHUTDOWN
        self.emit("shutdown", "OmniOS kapatiliyor")
