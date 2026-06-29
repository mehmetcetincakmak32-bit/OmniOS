"""Jest Motoru - Flow mod hareket tanima"""
from enum import Enum
from dataclasses import dataclass
from typing import Callable


class Gesture(Enum):
    SWIPE_UP = "swipe_up"
    SWIPE_RIGHT = "swipe_right"
    SWIPE_LEFT = "swipe_left"
    SWIPE_DOWN = "swipe_down"
    DOUBLE_TAP = "double_tap"
    LONG_PRESS = "long_press"
    PINCH_IN = "pinch_in"
    PINCH_OUT = "pinch_out"
    TAP = "tap"


class ModeKey(Enum):
    NORMAL = "normal"
    FLOW = "flow"


@dataclass
class GestureAction:
    gesture: Gesture
    action: str
    description: str
    context: str = "global"

    def __repr__(self):
        return f"<GestureAction {self.gesture.value}: {self.action}>"


GestureHandler = Callable[[Gesture], str]


class GestureEngine:
    """Flow modunda kullanilacak hareket tanima motoru"""

    _FLOW_ACTIONS = {
        Gesture.SWIPE_UP: GestureAction(Gesture.SWIPE_UP, "open_app_menu", "Uygulama menusu", "flow"),
        Gesture.SWIPE_RIGHT: GestureAction(Gesture.SWIPE_RIGHT, "open_recent_apps", "Son uygulamalar", "flow"),
        Gesture.SWIPE_LEFT: GestureAction(Gesture.SWIPE_LEFT, "open_notifications", "Bildirimler", "flow"),
        Gesture.SWIPE_DOWN: GestureAction(Gesture.SWIPE_DOWN, "open_quick_settings", "Hizli ayarlar", "flow"),
        Gesture.DOUBLE_TAP: GestureAction(Gesture.DOUBLE_TAP, "go_home", "Ana ekran", "flow"),
        Gesture.LONG_PRESS: GestureAction(Gesture.LONG_PRESS, "voice_command", "Sesli komut", "flow"),
        Gesture.PINCH_OUT: GestureAction(Gesture.PINCH_OUT, "app_selector_flow", "Coverflow secici", "flow"),
    }

    _NORMAL_ACTIONS = {
        Gesture.SWIPE_UP: GestureAction(Gesture.SWIPE_UP, "open_app_menu", "Uygulama menusu"),
        Gesture.SWIPE_RIGHT: GestureAction(Gesture.SWIPE_RIGHT, "open_recent_apps", "Son uygulamalar"),
        Gesture.SWIPE_LEFT: GestureAction(Gesture.SWIPE_LEFT, "open_notifications", "Bildirimler"),
        Gesture.SWIPE_DOWN: GestureAction(Gesture.SWIPE_DOWN, "open_quick_settings", "Hizli ayarlar"),
        Gesture.DOUBLE_TAP: GestureAction(Gesture.DOUBLE_TAP, "go_home", "Ana ekran"),
        Gesture.LONG_PRESS: GestureAction(Gesture.LONG_PRESS, "voice_command", "Sesli komut"),
        Gesture.PINCH_OUT: GestureAction(Gesture.PINCH_OUT, "app_selector", "Uygulama secici"),
    }

    def __init__(self):
        self._custom_mappings = {}
        self._history = []
        self._handlers = {}

    def recognize(self, gesture_name: str):
        try:
            return Gesture(gesture_name)
        except ValueError:
            for g in Gesture:
                if g.value == gesture_name or g.name.lower() == gesture_name.lower():
                    return g
        return None

    def get_action(self, gesture: Gesture, mode):
        mode_key = mode.value if hasattr(mode, 'value') else mode
        if mode_key in self._custom_mappings and gesture in self._custom_mappings[mode_key]:
            return self._custom_mappings[mode_key][gesture]

        defaults = self._FLOW_ACTIONS if mode_key == "flow" else self._NORMAL_ACTIONS
        return defaults.get(gesture, GestureAction(gesture, "no_action", "Tanimlanmadi"))

    def execute(self, gesture: Gesture, mode) -> str:
        action = self.get_action(gesture, mode)
        gesture_name = gesture.value
        if gesture_name in self._handlers:
            return self._handlers[gesture_name](gesture)
        self._history.append((gesture, action, mode))
        if len(self._history) > 100:
            self._history.pop(0)
        return action.action

    def register_handler(self, gesture_name: str, handler: GestureHandler):
        self._handlers[gesture_name] = handler

    def remap(self, gesture: Gesture, action: GestureAction, mode: str = "normal"):
        mode_key = ModeKey(mode).value
        if mode_key not in self._custom_mappings:
            self._custom_mappings[mode_key] = {}
        self._custom_mappings[mode_key][gesture] = action

    def get_supported_gestures(self) -> list:
        return [g.value for g in Gesture]

    def get_gesture_hints(self, mode="normal") -> list:
        defaults = self._FLOW_ACTIONS if mode == "flow" else self._NORMAL_ACTIONS
        mappings = self._custom_mappings.get(mode, defaults)
        return [
            {"gesture": g.value, "action": a.action, "description": a.description}
            for g, a in mappings.items()
        ]

    def get_history(self, limit=20) -> list:
        return [
            {"gesture": g.value, "action": a.action, "mode": m.value if hasattr(m, 'value') else str(m)}
            for g, a, m in self._history[-limit:]
        ]
