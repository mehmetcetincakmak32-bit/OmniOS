"""Mod Yoneticisi - Normal ve Flow mod gecisleri"""
from enum import Enum


class Mode(Enum):
    NORMAL = "normal"
    FLOW = "flow"

    def __str__(self):
        return self.value


class ModeState:
    def __init__(self, mode: Mode):
        self.mode = mode
        self.last_activity = None
        self.settings = {}
        self.history = []

    def record_activity(self, activity: str):
        self.last_activity = activity
        self.history.append(activity)
        if len(self.history) > 50:
            self.history.pop(0)

    def set_setting(self, key: str, value):
        self.settings[key] = value

    def get_setting(self, key: str, default=None):
        return self.settings.get(key, default)


class ModeManager:
    def __init__(self):
        self._current = Mode.NORMAL
        self._states = {
            Mode.NORMAL: ModeState(Mode.NORMAL),
            Mode.FLOW: ModeState(Mode.FLOW),
        }
        self._transition_callbacks = []
        self._transition_history = []

    @property
    def current_mode(self) -> Mode:
        return self._current

    @property
    def current_state(self) -> ModeState:
        return self._states[self._current]

    def toggle(self) -> Mode:
        new = Mode.FLOW if self._current == Mode.NORMAL else Mode.NORMAL
        return self.set_mode(new)

    def set_mode(self, mode: Mode) -> Mode:
        if mode == self._current:
            return self._current

        old_mode = self._current
        self._current = mode
        self._transition_history.append((old_mode, mode))

        for cb in self._transition_callbacks:
            cb(old_mode, mode)

        return self._current

    def on_transition(self, callback):
        self._transition_callbacks.append(callback)

    def get_state(self, mode: Mode = None) -> ModeState:
        if mode is None:
            mode = self._current
        return self._states[mode]

    def is_normal(self) -> bool:
        return self._current == Mode.NORMAL

    def is_flow(self) -> bool:
        return self._current == Mode.FLOW

    def record_activity(self, activity: str):
        self.current_state.record_activity(activity)

    def get_transition_history(self, limit: int = 10) -> list:
        return self._transition_history[-limit:]

    def reset(self):
        self._current = Mode.NORMAL
        for state in self._states.values():
            state.history.clear()
            state.settings.clear()
            state.last_activity = None
