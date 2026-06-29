"""OmniOS System Monitor Plugin - Sistem izleme"""
import time
import random
from src.core.plugin_system import OmniOSPlugin


class SystemMonitorPlugin(OmniOSPlugin):
    name = "system_monitor"
    version = "1.0.0"
    description = "Sistem kaynak kullanimini izler"
    author = "OmniOS Team"

    def __init__(self, engine=None):
        super().__init__(engine)
        self._history = []
        self._interval = 5.0
        self._last_collect = 0

    def on_enable(self):
        super().on_enable()
        print(f"[SystemMonitor] Sistem izleme basladi (interval: {self._interval}s)")

    def on_disable(self):
        super().on_disable()
        print(f"[SystemMonitor] Sistem izleme durduruldu ({len(self._history)} kayit)")

    def collect(self) -> dict:
        now = time.time()
        if now - self._last_collect < self._interval:
            return self._history[-1] if self._history else {}

        snapshot = {
            "timestamp": now,
            "cpu_percent": round(random.uniform(10, 95), 1),
            "memory_percent": round(random.uniform(30, 85), 1),
            "active_processes": random.randint(1, 20),
            "total_processes": random.randint(10, 50),
            "battery_percent": round(random.uniform(20, 100), 1),
            "network_rx_mbps": round(random.uniform(0, 100), 1),
            "network_tx_mbps": round(random.uniform(0, 50), 1),
        }

        if self.engine:
            info = self.engine.get_system_info()
            snapshot["active_processes"] = info["active_processes"]

        self._history.append(snapshot)
        if len(self._history) > 360:
            self._history.pop(0)

        self._last_collect = now
        return snapshot

    def get_history(self, limit: int = 60) -> list:
        return self._history[-limit:]

    def get_summary(self) -> dict:
        if not self._history:
            return {"status": "no_data"}

        recent = self._history[-1]
        avg_cpu = sum(h["cpu_percent"] for h in self._history[-10:]) / min(10, len(self._history))

        return {
            "status": "healthy" if recent["cpu_percent"] < 80 else "busy",
            "current_cpu": recent["cpu_percent"],
            "avg_cpu": round(avg_cpu, 1),
            "current_memory": recent["memory_percent"],
            "current_battery": recent["battery_percent"],
            "uptime_seconds": round(time.time() - self._history[0]["timestamp"]),
            "total_snapshots": len(self._history),
        }
