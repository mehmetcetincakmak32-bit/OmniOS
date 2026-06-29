"""OmniOS Power Management Module"""
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Callable, Dict, List
from threading import Timer as ThreadingTimer
from .logger import get_logger, LogLevel


class PowerState(Enum):
    ACTIVE = auto()
    IDLE = auto()
    STANDBY = auto()
    SUSPEND = auto()
    HIBERNATE = auto()
    SHUTDOWN = auto()


class WakeReason(Enum):
    USER_INPUT = auto()
    NETWORK = auto()
    TIMER = auto()
    POWER_BUTTON = auto()
    SCHEDULED = auto()


@dataclass
class PowerStats:
    battery_level: int = 100
    is_charging: bool = False
    estimated_time_remaining: int = 480
    screen_on_time: int = 0
    cpu_usage_percent: float = 0.0
    temperature: float = 25.0
    wake_count: int = 0


@dataclass
class PowerProfile:
    name: str
    screen_timeout: int = 30
    cpu_max_freq: float = 1.0
    background_sync: bool = True
    location_services: bool = True
    notifications: bool = True


class PowerManager:
    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialized = False
        return cls._instance

    def __init__(self):
        if self._initialized:
            return
        self._initialized = True
        self.logger = get_logger()
        self.state = PowerState.ACTIVE
        self.stats = PowerStats()
        self.profile = PowerProfile("balanced")
        self.wake_callbacks: List[Callable] = []
        self.sleep_callbacks: List[Callable] = []
        self._idle_timer: Timer = None
        self._screen_timer: Timer = None
        self._last_activity = 0
        self._activity_count = 0

    def set_profile(self, profile: PowerProfile):
        self.profile = profile
        self.logger.info(f"Power profile changed to: {profile.name}")

    def get_profile(self) -> PowerProfile:
        return self.profile

    def get_stats(self) -> PowerStats:
        return self.stats

    def get_state(self) -> PowerState:
        return self.state

    def register_wake_callback(self, callback: Callable):
        self.wake_callbacks.append(callback)

    def register_sleep_callback(self, callback: Callable):
        self.sleep_callbacks.append(callback)

    def activity(self, source: str = "user"):
        self._activity_count += 1
        self._last_activity = 0
        self._reset_idle_timer()
        self._reset_screen_timer()

    def _reset_idle_timer(self):
        if self._idle_timer:
            self._idle_timer.cancel()
        self._idle_timer = ThreadingTimer(
            300,
            self._on_idle_timeout
        )
        self._idle_timer.daemon = True
        self._idle_timer.start()

    def _reset_screen_timer(self):
        if self._screen_timer:
            self._screen_timer.cancel()
        timeout = self.profile.screen_timeout
        self._screen_timer = ThreadingTimer(
            timeout,
            self._on_screen_timeout
        )
        self._screen_timer.daemon = True
        self._screen_timer.start()

    def _on_idle_timeout(self):
        if self.state == PowerState.ACTIVE:
            self.transition_to(PowerState.IDLE)

    def _on_screen_timeout(self):
        if self.state in [PowerState.ACTIVE, PowerState.IDLE]:
            self.transition_to(PowerState.STANDBY)

    def transition_to(self, new_state: PowerState):
        old_state = self.state
        self.state = new_state
        self.logger.info(f"Power state: {old_state.name} -> {new_state.name}")

        if new_state == PowerState.SUSPEND:
            for cb in self.sleep_callbacks:
                cb()
        elif new_state == PowerState.ACTIVE and old_state != PowerState.ACTIVE:
            self.stats.wake_count += 1
            for cb in self.wake_callbacks:
                cb(WakeReason.USER_INPUT)

    def wake(self, reason: WakeReason = WakeReason.USER_INPUT):
        self.transition_to(PowerState.ACTIVE)
        self.activity("wake")
        self.stats.wake_count += 1
        self.logger.info(f"System wake: {reason.name}")

    def sleep(self):
        self.transition_to(PowerState.SUSPEND)

    def shutdown(self):
        self.transition_to(PowerState.SHUTDOWN)
        if self._idle_timer:
            self._idle_timer.cancel()
        if self._screen_timer:
            self._screen_timer.cancel()

    def simulate_battery_drain(self, amount: int = 1):
        self.stats.battery_level = max(0, self.stats.battery_level - amount)
        if self.stats.battery_level <= 5:
            self.logger.critical(f"Kritik batarya seviyesi: {self.stats.battery_level}%")

    def set_charging(self, charging: bool):
        self.stats.is_charging = charging
        if charging:
            self.stats.battery_level = min(100, self.stats.battery_level + 5)
        self.logger.info(f"Sarj durumu: {'Sarj ediliyor' if charging else 'Sarj edilmiyor'}")


def get_power_manager() -> PowerManager:
    return PowerManager()