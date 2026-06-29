"""Bellek Yoneticisi - Kaynak kullanimi ve optimizasyon"""


class MemoryBlock:
    def __init__(self, pid: str, size: int, label: str = ""):
        self.pid = pid
        self.size = size
        self.label = label
        self.pinned = False

    def __repr__(self):
        return f"<MemBlock {self.label or self.pid}: {self.size}MB>"


class MemoryManager:
    def __init__(self, total_memory: int = 4096):
        self.total = total_memory
        self._blocks = {}
        self._threshold_warning = int(total_memory * 0.85)
        self._threshold_critical = int(total_memory * 0.95)

    @property
    def used(self) -> int:
        return sum(b.size for b in self._blocks.values() if not b.pinned)

    @property
    def pinned(self) -> int:
        return sum(b.size for b in self._blocks.values() if b.pinned)

    @property
    def available(self) -> int:
        return self.total - self.used - self.pinned

    @property
    def usage_percent(self) -> float:
        return round((self.used + self.pinned) / self.total * 100, 1)

    def allocate(self, pid: str, size: int, label: str = "", pin: bool = False) -> bool:
        if size > self.available:
            self._try_compact()
            if size > self.available:
                return False

        block = MemoryBlock(pid, size, label)
        block.pinned = pin
        self._blocks[pid] = block
        return True

    def free(self, pid: str) -> bool:
        if pid in self._blocks:
            del self._blocks[pid]
            return True
        return False

    def pin(self, pid: str) -> bool:
        if pid in self._blocks:
            self._blocks[pid].pinned = True
            return True
        return False

    def unpin(self, pid: str) -> bool:
        if pid in self._blocks:
            self._blocks[pid].pinned = False
            return True
        return False

    def get_usage(self, pid: str) -> int:
        block = self._blocks.get(pid)
        return block.size if block else 0

    def get_status(self) -> str:
        if self.usage_percent >= 95:
            return "critical"
        if self.usage_percent >= 85:
            return "warning"
        return "normal"

    def get_report(self) -> dict:
        return {
            "total_mb": self.total,
            "used_mb": self.used,
            "pinned_mb": self.pinned,
            "available_mb": self.available,
            "usage_percent": self.usage_percent,
            "status": self.get_status(),
            "block_count": len(self._blocks),
            "blocks": [{"pid": pid, "size": b.size, "label": b.label, "pinned": b.pinned}
                       for pid, b in self._blocks.items()],
        }

    def _try_compact(self):
        non_pinned = [(pid, b) for pid, b in self._blocks.items() if not b.pinned]
        non_pinned.sort(key=lambda x: x[1].size, reverse=True)
        while non_pinned and self.available < self.total * 0.1:
            pid, block = non_pinned.pop(0)
            del self._blocks[pid]
