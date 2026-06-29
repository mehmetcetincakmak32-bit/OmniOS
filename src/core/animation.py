"""OmniOS Animation System"""
from enum import Enum
from dataclasses import dataclass
from typing import Callable, Optional, List, Dict, Any
import time
import threading
from .logger import get_logger


class EasingFunction(Enum):
    LINEAR = "linear"
    EASE_IN = "ease_in"
    EASE_OUT = "ease_out"
    EASE_IN_OUT = "ease_in_out"
    BOUNCE = "bounce"
    ELASTIC = "elastic"
    BACK = "back"


@dataclass
class AnimationConfig:
    duration: float = 300.0
    easing: EasingFunction = EasingFunction.EASE_IN_OUT
    delay: float = 0.0
    repeat: int = 0
    reverse: bool = False


class AnimationState(Enum):
    IDLE = "idle"
    RUNNING = "running"
    PAUSED = "paused"
    COMPLETED = "completed"
    CANCELLED = "cancelled"


class Animation:
    def __init__(self, target: Any, property_name: str, from_value: float, to_value: float, config: AnimationConfig):
        self.target = target
        self.property_name = property_name
        self.from_value = from_value
        self.to_value = to_value
        self.config = config
        self.state = AnimationState.IDLE
        self._start_time = 0.0
        self._elapsed = 0.0
        self._paused_time = 0.0
        self._current_value = from_value
        self._repeat_count = 0
        self._reversed = False
        self._on_update: Optional[Callable[[float], None]] = None
        self._on_complete: Optional[Callable[[], None]] = None
        self._on_cancel: Optional[Callable[[], None]] = None

    def start(self):
        if self.state == AnimationState.RUNNING:
            return
        self.state = AnimationState.RUNNING
        self._start_time = time.time() - self._paused_time
        self._repeat_count = 0
        self._reversed = False

    def pause(self):
        if self.state == AnimationState.RUNNING:
            self.state = AnimationState.PAUSED
            self._paused_time = time.time() - self._start_time

    def resume(self):
        if self.state == AnimationState.PAUSED:
            self.state = AnimationState.RUNNING
            self._start_time = time.time() - self._paused_time

    def cancel(self):
        self.state = AnimationState.CANCELLED
        if self._on_cancel:
            self._on_cancel()

    def update(self, dt: float) -> bool:
        if self.state != AnimationState.RUNNING:
            return False

        elapsed = time.time() - self._start_time
        if elapsed < self.config.delay / 1000.0:
            return True

        progress = min(1.0, (elapsed - self.config.delay / 1000.0) / (self.config.duration / 1000.0))
        eased = self._ease(progress)

        if self._reversed:
            eased = 1.0 - eased

        self._current_value = self.from_value + (self.to_value - self.from_value) * eased

        if self._on_update:
            self._on_update(self._current_value)

        if progress >= 1.0:
            if self.config.repeat > 0 and self._repeat_count < self.config.repeat:
                self._repeat_count += 1
                if self.config.reverse:
                    self._reversed = not self._reversed
                self._start_time = time.time()
                return True
            else:
                self.state = AnimationState.COMPLETED
                if self._on_complete:
                    self._on_complete()
                return False
        return True

    def _ease(self, t: float) -> float:
        if self.config.easing == EasingFunction.LINEAR:
            return t
        elif self.config.easing == EasingFunction.EASE_IN:
            return t * t
        elif self.config.easing == EasingFunction.EASE_OUT:
            return 1 - (1 - t) * (1 - t)
        elif self.config.easing == EasingFunction.EASE_IN_OUT:
            return 2 * t * t if t < 0.5 else 1 - (-2 * t + 2) ** 2 / 2
        elif self.config.easing == EasingFunction.BOUNCE:
            if t < 1 / 2.75:
                return 7.5625 * t * t
            elif t < 2 / 2.75:
                t -= 1.5 / 2.75
                return 7.5625 * t * t + 0.75
            elif t < 2.5 / 2.75:
                t -= 2.25 / 2.75
                return 7.5625 * t * t + 0.9375
            else:
                t -= 2.625 / 2.75
                return 7.5625 * t * t + 0.984375
        elif self.config.easing == EasingFunction.ELASTIC:
            if t == 0 or t == 1:
                return t
            return -(2 ** (10 * (t - 1))) * math.sin((t - 0.1) * (2 * math.pi) / 0.4)
        elif self.config.easing == EasingFunction.BACK:
            c = 1.70158
            return t * t * ((c + 1) * t - c)
        return t

    def on_update(self, callback: Callable[[float], None]):
        self._on_update = callback
        return self

    def on_complete(self, callback: Callable[[], None]):
        self._on_complete = callback
        return self

    def on_cancel(self, callback: Callable[[], None]):
        self._on_cancel = callback
        return self


class AnimationManager:
    def __init__(self):
        self.animations: List[Animation] = []
        self._running = False
        self._thread: Optional[threading.Thread] = None
        self._lock = threading.Lock()
        self.logger = get_logger()

    def start(self):
        if self._running:
            return
        self._running = True
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def stop(self):
        self._running = False
        with self._lock:
            for anim in self.animations:
                anim.cancel()
            self.animations.clear()
        if self._thread:
            self._thread.join(timeout=1.0)

    def _loop(self):
        last_time = time.time()
        while self._running:
            current_time = time.time()
            dt = current_time - last_time
            last_time = current_time

            with self._lock:
                completed = []
                for anim in self.animations:
                    if not anim.update(dt):
                        completed.append(anim)
                for anim in completed:
                    self.animations.remove(anim)

            time.sleep(0.016)

    def animate(self, target: Any, property_name: str, from_value: float, to_value: float, config: AnimationConfig = None) -> Animation:
        config = config or AnimationConfig()
        anim = Animation(target, property_name, from_value, to_value, config)
        with self._lock:
            self.animations.append(anim)
        anim.start()
        return anim

    def animate_to(self, target: Any, property_name: str, to_value: float, config: AnimationConfig = None) -> Animation:
        current = getattr(target, property_name, 0)
        return self.animate(target, property_name, current, to_value, config)

    def fade_in(self, target: Any, duration: float = 300.0) -> Animation:
        return self.animate(target, 'opacity', 0.0, 1.0, AnimationConfig(duration=duration))

    def fade_out(self, target: Any, duration: float = 300.0) -> Animation:
        return self.animate(target, 'opacity', 1.0, 0.0, AnimationConfig(duration=duration))

    def slide_in(self, target: Any, property_name: str, from_pos: float, to_pos: float, duration: float = 300.0) -> Animation:
        return self.animate(target, property_name, from_pos, to_pos, AnimationConfig(duration=duration))

    def scale(self, target: Any, from_scale: float, to_scale: float, duration: float = 200.0) -> Animation:
        return self.animate(target, 'scale', from_scale, to_scale, AnimationConfig(duration=duration))

    def spring(self, target: Any, property_name: str, to_value: float, stiffness: float = 100, damping: float = 10) -> Animation:
        config = AnimationConfig(duration=500, easing=EasingFunction.EASE_OUT)
        anim = self.animate(target, property_name, getattr(target, property_name, 0), to_value, config)
        return anim

    def sequence(self, animations: List[Animation]) -> Animation:
        pass

    def parallel(self, animations: List[Animation]) -> List[Animation]:
        for anim in animations:
            anim.start()
        return animations

    def cancel_all(self):
        with self._lock:
            for anim in self.animations:
                anim.cancel()
            self.animations.clear()

    def cancel_animation(self, animation: Animation):
        with self._lock:
            if animation in self.animations:
                animation.cancel()
                self.animations.remove(animation)

    def get_running_count(self) -> int:
        with self._lock:
            return sum(1 for a in self.animations if a.state == AnimationState.RUNNING)


_global_animation_manager: Optional['AnimationManager'] = None


def get_animation_manager() -> AnimationManager:
    global _global_animation_manager
    if _global_animation_manager is None:
        _global_animation_manager = AnimationManager()
        _global_animation_manager.start()
    return _global_animation_manager


def animate(target: Any, property_name: str, from_value: float, to_value: float, duration: float = 300.0, easing: EasingFunction = EasingFunction.EASE_IN_OUT) -> Animation:
    config = AnimationConfig(duration=duration, easing=easing)
    return get_animation_manager().animate(target, property_name, from_value, to_value, config)


def animate_to(target: Any, property_name: str, to_value: float, duration: float = 300.0, easing: EasingFunction = EasingFunction.EASE_IN_OUT) -> Animation:
    config = AnimationConfig(duration=duration, easing=easing)
    return get_animation_manager().animate_to(target, property_name, to_value, config)


def fade_in(target: Any, duration: float = 300.0) -> Animation:
    return get_animation_manager().fade_in(target, duration)


def fade_out(target: Any, duration: float = 300.0) -> Animation:
    return get_animation_manager().fade_out(target, duration)


def spring_to(target: Any, property_name: str, to_value: float, stiffness: float = 100, damping: float = 10) -> Animation:
    return get_animation_manager().spring(target, property_name, to_value, stiffness, damping)


import math