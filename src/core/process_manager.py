"""Process Yoneticisi - Uygulama yasam dongusu"""
from enum import Enum, auto
from datetime import datetime
import uuid


class ProcessState(Enum):
    CREATED = auto()
    RUNNING = auto()
    BACKGROUND = auto()
    SUSPENDED = auto()
    TERMINATED = auto()
    CRASHED = auto()


class Process:
    def __init__(self, name: str, platform: str, pid: str = None):
        self.pid = pid or str(uuid.uuid4())[:8]
        self.name = name
        self.platform = platform
        self.state = ProcessState.CREATED
        self.created_at = datetime.now()
        self.updated_at = self.created_at
        self.cpu_usage = 0.0
        self.memory_usage = 0.0
        self.priority = 0
        self._log = []

    def start(self):
        self.state = ProcessState.RUNNING
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Baslatildi")

    def suspend(self):
        self.state = ProcessState.SUSPENDED
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Durduruldu")

    def resume(self):
        self.state = ProcessState.RUNNING
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Devam")

    def background(self):
        self.state = ProcessState.BACKGROUND
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Arkaplana atildi")

    def terminate(self):
        self.state = ProcessState.TERMINATED
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Sonlandirildi")

    def crash(self, reason: str = ""):
        self.state = ProcessState.CRASHED
        self.updated_at = datetime.now()
        self._log.append(f"[{self.updated_at}] Crashed: {reason}")

    @property
    def uptime(self):
        if self.state in (ProcessState.TERMINATED, ProcessState.CRASHED):
            return (self.updated_at - self.created_at).total_seconds()
        return (datetime.now() - self.created_at).total_seconds()

    @property
    def is_active(self):
        return self.state in (ProcessState.RUNNING, ProcessState.BACKGROUND)

    def info(self) -> dict:
        return {
            "pid": self.pid,
            "name": self.name,
            "platform": self.platform,
            "state": self.state.name,
            "uptime": round(self.uptime, 1),
            "cpu": round(self.cpu_usage, 1),
            "memory": round(self.memory_usage, 1),
            "priority": self.priority,
        }

    def __repr__(self):
        return f"<Process {self.pid}:{self.name} [{self.state.name}]>"


class ProcessManager:
    def __init__(self, max_processes: int = 50):
        self._processes = {}
        self._max_processes = max_processes
        self._total_created = 0

    @property
    def active_processes(self) -> list:
        return [p for p in self._processes.values() if p.is_active]

    @property
    def all_processes(self) -> list:
        return list(self._processes.values())

    def create(self, name: str, platform: str = "OmniOS") -> Process:
        self._cleanup_terminated()

        if len(self.active_processes) >= self._max_processes:
            oldest = min(self.active_processes, key=lambda p: p.priority)
            oldest.terminate()

        proc = Process(name, platform)
        proc.start()
        self._processes[proc.pid] = proc
        self._total_created += 1
        return proc

    def get(self, pid: str) -> Process:
        return self._processes.get(pid)

    def find(self, name: str) -> list:
        return [p for p in self._processes.values() if p.name.lower() == name.lower()]

    def kill(self, identifier: str) -> bool:
        proc = self._processes.get(identifier)
        if not proc:
            matches = self.find(identifier)
            if matches:
                proc = matches[0]
        if proc and proc.is_active:
            proc.terminate()
            return True
        return False

    def suspend(self, identifier: str) -> bool:
        proc = self._processes.get(identifier)
        if not proc:
            matches = self.find(identifier)
            if matches:
                proc = matches[0]
        if proc and proc.state.name == "RUNNING":
            proc.suspend()
            return True
        return False

    def kill_all(self):
        for proc in self.active_processes:
            proc.terminate()

    def get_stats(self) -> dict:
        active = self.active_processes
        return {
            "total_created": self._total_created,
            "active": len(active),
            "terminated": sum(1 for p in self._processes.values() if p.state == ProcessState.TERMINATED),
            "suspended": sum(1 for p in self._processes.values() if p.state == ProcessState.SUSPENDED),
            "crashed": sum(1 for p in self._processes.values() if p.state == ProcessState.CRASHED),
            "max_processes": self._max_processes,
        }

    def _cleanup_terminated(self):
        to_remove = [pid for pid, p in self._processes.items()
                     if p.state in (ProcessState.TERMINATED, ProcessState.CRASHED)]
        for pid in to_remove:
            del self._processes[pid]
