"""
OmniOS Time-Travel Debugging System
System snapshots, deterministic replay, and UI time-travel debugging
"""
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Any, Callable, Set
from collections import defaultdict, deque
import uuid
import time
import hashlib
import json
import copy
import threading
from pathlib import Path
from .logger import get_logger


class SnapshotType(Enum):
    FULL = "full"                    # Complete system state
    INCREMENTAL = "incremental"      # Delta from previous
    CHECKPOINT = "checkpoint"        # Named checkpoint
    TIME_TRAVEL = "time_travel"      # Time-travel specific


class SnapshotStatus(Enum):
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    COMPLETED = "completed"
    FAILED = "failed"
    EXPIRED = "expired"


class ReplayMode(Enum):
    DETERMINISTIC = "deterministic"  # Exact replay
    FAST_FORWARD = "fast_forward"    # Skip to point
    STEP = "step"                    # Instruction-level step
    REVERSE = "reverse"              # Reverse execution


@dataclass
class SystemState:
    """Complete system state for snapshotting"""
    timestamp: float
    snapshot_id: str
    snapshot_type: SnapshotType
    
    # Kernel state
    kernel_state: Dict = field(default_factory=dict)
    process_table: Dict = field(default_factory=dict)
    thread_table: Dict = field(default_factory=dict)
    
    # Memory state
    memory_map: Dict = field(default_factory=dict)
    heap_state: Dict = field(default_factory=dict)
    stack_states: Dict = field(default_factory=dict)
    
    # Capability state
    capability_tables: Dict = field(default_factory=dict)
    security_domains: Dict = field(default_factory=dict)
    
    # Device state
    device_states: Dict = field(default_factory=dict)
    irq_state: Dict = field(default_factory=dict)
    
    # Network state
    network_state: Dict = field(default_factory=dict)
    socket_table: Dict = field(default_factory=dict)
    
    # File system state
    vfs_state: Dict = field(default_factory=dict)
    open_files: Dict = field(default_factory=dict)
    
    # IPC state
    ipc_channels: Dict = field(default_factory=dict)
    message_queues: Dict = field(default_factory=dict)
    
    # UI state (for time-travel UI debugging)
    ui_state: Dict = field(default_factory=dict)
    render_tree: Dict = field(default_factory=dict)
    animation_states: Dict = field(default_factory=dict)
    
    # Execution trace
    execution_trace: List[Dict] = field(default_factory=list)
    instruction_pointer: int = 0
    
    # Integrity
    merkle_root: str = ""
    checksum: str = ""

    def compute_merkle_root(self) -> str:
        """Compute Merkle root for integrity verification"""
        data = json.dumps(self.to_dict(), sort_keys=True, default=str)
        return hashlib.sha256(data.encode()).hexdigest()

    def compute_checksum(self) -> str:
        """Compute simple checksum"""
        data = json.dumps(self.to_dict(), sort_keys=True, default=str)
        return hashlib.md5(data.encode()).hexdigest()

    def to_dict(self) -> Dict:
        return {
            k: v for k, v in self.__dict__.items() 
            if not k.startswith('_')
        }


@dataclass
class Snapshot:
    """System snapshot with metadata"""
    id: str
    type: SnapshotType
    status: SnapshotStatus
    state: SystemState
    parent_id: Optional[str] = None
    children_ids: List[str] = field(default_factory=list)
    tags: List[str] = field(default_factory=list)
    created_at: float = field(default_factory=time.time)
    expires_at: Optional[float] = None
    size_bytes: int = 0
    compression_ratio: float = 1.0
    metadata: Dict = field(default_factory=dict)


class MerkleTree:
    """Merkle tree for efficient state verification"""
    
    def __init__(self):
        self.leaves: List[str] = []
        self.tree: List[List[str]] = []
    
    def build(self, data_chunks: List[bytes]) -> str:
        """Build Merkle tree from data chunks"""
        self.leaves = [hashlib.sha256(chunk).hexdigest() for chunk in data_chunks]
        self.tree = [self.leaves]
        
        level = self.leaves
        while len(level) > 1:
            next_level = []
            for i in range(0, len(level), 2):
                left = level[i]
                right = level[i + 1] if i + 1 < len(level) else left
                combined = hashlib.sha256((left + right).encode()).hexdigest()
                next_level.append(combined)
            self.tree.append(next_level)
            level = next_level
        
        return level[0] if level else ""
    
    def get_proof(self, index: int) -> List[Dict]:
        """Get Merkle proof for leaf at index"""
        proof = []
        for level in self.tree[:-1]:
            sibling_idx = index ^ 1
            if sibling_idx < len(level):
                proof.append({
                    "hash": level[sibling_idx],
                    "position": "left" if sibling_idx < index else "right"
                })
            index //= 2
        return proof
    
    def verify_proof(self, leaf: str, proof: List[Dict], root: str) -> bool:
        """Verify Merkle proof"""
        current = leaf
        for step in proof:
            if step["position"] == "left":
                current = hashlib.sha256((step["hash"] + current).encode()).hexdigest()
            else:
                current = hashlib.sha256((current + step["hash"]).encode()).hexdigest()
        return current == root


class SnapshotManager:
    """Manages system snapshots with time-travel capabilities"""
    
    def __init__(self, max_snapshots: int = 1000, max_size_gb: float = 10.0):
        self.logger = get_logger()
        self.max_snapshots = max_snapshots
        self.max_size_bytes = int(max_size_gb * 1024 * 1024 * 1024)
        
        self.snapshots: Dict[str, Snapshot] = {}
        self.snapshot_order: List[str] = []  # Chronological order
        self.current_size_bytes = 0
        self.merkle_tree = MerkleTree()
        self._lock = threading.RLock()
        
        # Auto-snapshot settings
        self.auto_snapshot_interval = 60  # seconds
        self.last_auto_snapshot = time.time()
        self.auto_snapshot_enabled = True
        
        # Time-travel state
        self.replay_mode = ReplayMode.DETERMINISTIC
        self.replay_position = 0
        self.execution_trace: List[Dict] = []
        self.max_trace_size = 100000
        
        # UI time-travel
        self.ui_snapshots: Dict[str, Dict] = {}
        self.ui_snapshot_order: List[str] = []
        self.max_ui_snapshots = 5000

    def create_snapshot(self, 
                        snapshot_type: SnapshotType = SnapshotType.INCREMENTAL,
                        tags: List[str] = None,
                        metadata: Dict = None) -> Snapshot:
        """Create a new system snapshot"""
        with self._lock:
            snapshot_id = f"snap_{int(time.time() * 1000)}_{uuid.uuid4().hex[:8]}"
            
            # Capture current system state
            state = self._capture_system_state()
            state.snapshot_id = snapshot_id
            state.snapshot_type = snapshot_type
            
            # Compute integrity
            state.merkle_root = state.compute_merkle_root()
            state.checksum = state.compute_checksum()
            
            # Create snapshot
            snapshot = Snapshot(
                id=snapshot_id,
                type=snapshot_type,
                status=SnapshotStatus.COMPLETED,
                state=state,
                parent_id=self.snapshot_order[-1] if self.snapshot_order else None,
                tags=tags or [],
                metadata=metadata or {},
                size_bytes=len(json.dumps(state.to_dict(), default=str).encode()),
            )
            
            self.snapshots[snapshot_id] = snapshot
            self.snapshot_order.append(snapshot_id)
            self.current_size_bytes += snapshot.size_bytes
            
            # Update parent's children
            if snapshot.parent_id and snapshot.parent_id in self.snapshots:
                self.snapshots[snapshot.parent_id].children_ids.append(snapshot_id)
            
            # Cleanup if needed
            self._cleanup_old_snapshots()
            
            self.logger.info(f"Created {snapshot_type.value} snapshot: {snapshot_id}")
            return snapshot
    
    def _capture_system_state(self) -> SystemState:
        """Capture complete system state"""
        # In real implementation, would capture from kernel/userspace
        return SystemState(
            timestamp=time.time(),
            snapshot_id="",
            snapshot_type=SnapshotType.INCREMENTAL,
        )
    
    def _cleanup_old_snapshots(self):
        """Remove old snapshots if limits exceeded"""
        while (len(self.snapshots) > self.max_snapshots or 
               self.current_size_bytes > self.max_size_bytes):
            if not self.snapshot_order:
                break
            oldest_id = self.snapshot_order.pop(0)
            oldest = self.snapshots.pop(oldest_id, None)
            if oldest:
                self.current_size_bytes -= oldest.size_bytes
                # Update parent's children
                if oldest.parent_id and oldest.parent_id in self.snapshots:
                    parent = self.snapshots[oldest.parent_id]
                    if oldest.id in parent.children_ids:
                        parent.children_ids.remove(oldest.id)
    
    def restore_snapshot(self, snapshot_id: str) -> bool:
        """Restore system to a snapshot (time-travel)"""
        with self._lock:
            snapshot = self.snapshots.get(snapshot_id)
            if not snapshot:
                self.logger.error(f"Snapshot not found: {snapshot_id}")
                return False
            
            if snapshot.status != SnapshotStatus.COMPLETED:
                self.logger.error(f"Snapshot not completed: {snapshot_id}")
                return False
            
            self.logger.info(f"Restoring snapshot: {snapshot_id}")
            # In real implementation, would restore kernel/userspace state
            return True
    
    def get_snapshot(self, snapshot_id: str) -> Optional[Snapshot]:
        with self._lock:
            return self.snapshots.get(snapshot_id)
    
    def list_snapshots(self, tag: str = None) -> List[Snapshot]:
        with self._lock:
            snapshots = list(self.snapshots.values())
            if tag:
                snapshots = [s for s in snapshots if tag in s.tags]
            return sorted(snapshots, key=lambda s: s.created_at, reverse=True)
    
    def diff_snapshots(self, from_id: str, to_id: str) -> Dict:
        """Compute diff between two snapshots"""
        with self._lock:
            from_snap = self.snapshots.get(from_id)
            to_snap = self.snapshots.get(to_id)
            
            if not from_snap or not to_snap:
                return {"error": "Snapshot not found"}
            
            # Compare states (simplified)
            return {
                "from": from_id,
                "to": to_id,
                "time_diff": to_snap.created_at - from_snap.created_at,
                "size_diff": to_snap.size_bytes - from_snap.size_bytes,
                "state_changes": self._compute_state_diff(
                    from_snap.state.to_dict(), 
                    to_snap.state.to_dict()
                )
            }
    
    def _compute_state_diff(self, from_state: Dict, to_state: Dict) -> Dict:
        """Compute detailed state diff"""
        changes = {}
        all_keys = set(from_state.keys()) | set(to_state.keys())
        for key in all_keys:
            fv = from_state.get(key)
            tv = to_state.get(key)
            if fv != tv:
                changes[key] = {"from": fv, "to": tv}
        return changes

    # Time-travel execution recording
    def record_execution(self, event: Dict):
        """Record execution event for time-travel replay"""
        with self._lock:
            event["timestamp"] = time.time()
            event["sequence"] = len(self.execution_trace)
            self.execution_trace.append(event)
            
            if len(self.execution_trace) > self.max_trace_size:
                self.execution_trace = self.execution_trace[-self.max_trace_size:]
    
    def get_execution_trace(self, 
                           start_seq: int = 0, 
                           end_seq: int = None) -> List[Dict]:
        with self._lock:
            end = end_seq or len(self.execution_trace)
            return self.execution_trace[start_seq:end]
    
    def replay_execution(self, 
                        mode: ReplayMode = ReplayMode.DETERMINISTIC,
                        start_seq: int = 0,
                        end_seq: int = None) -> List[Dict]:
        """Replay execution trace"""
        trace = self.get_execution_trace(start_seq, end_seq)
        
        if mode == ReplayMode.REVERSE:
            trace = list(reversed(trace))
        elif mode == ReplayMode.STEP:
            # Yield one at a time
            pass
        
        return trace
    
    # UI Time-travel
    def capture_ui_snapshot(self, name: str, ui_state: Dict) -> str:
        """Capture UI state for time-travel debugging"""
        with self._lock:
            snapshot_id = f"ui_{int(time.time() * 1000)}_{uuid.uuid4().hex[:8]}"
            
            self.ui_snapshots[snapshot_id] = {
                "id": snapshot_id,
                "name": name,
                "timestamp": time.time(),
                "state": copy.deepcopy(ui_state),
                "render_tree": ui_state.get("render_tree", {}),
                "animations": ui_state.get("animations", {}),
            }
            
            self.ui_snapshot_order.append(snapshot_id)
            
            # Cleanup
            if len(self.ui_snapshots) > self.max_ui_snapshots:
                oldest = self.ui_snapshot_order.pop(0)
                del self.ui_snapshots[oldest]
            
            return snapshot_id
    
    def get_ui_snapshot(self, snapshot_id: str) -> Optional[Dict]:
        with self._lock:
            return self.ui_snapshots.get(snapshot_id)
    
    def list_ui_snapshots(self) -> List[Dict]:
        with self._lock:
            return [
                {
                    "id": sid,
                    "name": snap["name"],
                    "timestamp": snap["timestamp"]
                }
                for sid in self.ui_snapshot_order
                if (snap := self.ui_snapshots.get(sid))
            ]
    
    def scrub_ui(self, snapshot_id: str) -> Optional[Dict]:
        """Scrub to specific UI state (time-travel UI)"""
        snap = self.get_ui_snapshot(snapshot_id)
        if snap:
            self.logger.info(f"UI scrub to: {snap['name']} at {snap['timestamp']}")
            return snap["state"]
        return None
    
    def compare_ui_snapshots(self, from_id: str, to_id: str) -> Dict:
        """Compare two UI snapshots"""
        from_snap = self.get_ui_snapshot(from_id)
        to_snap = self.get_ui_snapshot(to_id)
        
        if not from_snap or not to_snap:
            return {"error": "Snapshot not found"}
        
        return {
            "from": from_id,
            "to": to_id,
            "render_diff": self._diff_render_trees(
                from_snap.get("render_tree", {}),
                to_snap.get("render_tree", {})
            ),
            "animation_diff": self._diff_animations(
                from_snap.get("animations", {}),
                to_snap.get("animations", {})
            )
        }
    
    def _diff_render_trees(self, from_tree: Dict, to_tree: Dict) -> Dict:
        """Diff two render trees"""
        # Simplified tree diff
        return {
            "added": [],
            "removed": [],
            "modified": [],
            "moved": []
        }
    
    def _diff_animations(self, from_anim: Dict, to_anim: Dict) -> Dict:
        """Diff animation states"""
        return {"changed": [], "started": [], "completed": []}

    # Integrity verification
    def verify_snapshot(self, snapshot_id: str) -> bool:
        """Verify snapshot integrity via Merkle root"""
        with self._lock:
            snapshot = self.snapshots.get(snapshot_id)
            if not snapshot:
                return False
            
            computed_root = snapshot.state.compute_merkle_root()
            return snapshot.state.merkle_root == computed_root
    
    def verify_all(self) -> Dict[str, bool]:
        """Verify all snapshots"""
        with self._lock:
            return {sid: self.verify_snapshot(sid) for sid in self.snapshots}


class TimeTravelDebugger:
    """High-level time-travel debugging interface"""
    
    def __init__(self):
        self.logger = get_logger()
        self.snapshot_manager = SnapshotManager()
        self.breakpoints: Dict[int, Dict] = {}
        self.watchpoints: Dict[str, Dict] = {}
        self.call_stack: List[Dict] = []
        self.variable_history: Dict[str, List[Dict]] = defaultdict(list)
    
    def set_breakpoint(self, sequence: int, condition: Callable = None):
        """Set breakpoint at execution sequence"""
        self.breakpoints[sequence] = {
            "condition": condition,
            "hit_count": 0,
            "enabled": True
        }
    
    def set_watchpoint(self, variable: str, condition: Callable = None):
        """Set watchpoint on variable"""
        self.watchpoints[variable] = {
            "condition": condition,
            "last_value": None,
            "hit_count": 0
        }
    
    def record_variable(self, name: str, value: Any, sequence: int):
        """Record variable value for history"""
        self.variable_history[name].append({
            "value": value,
            "sequence": sequence,
            "timestamp": time.time()
        })
        # Keep history bounded
        if len(self.variable_history[name]) > 1000:
            self.variable_history[name] = self.variable_history[name][-1000:]
    
    def check_breakpoints(self, sequence: int, context: Dict) -> List[Dict]:
        """Check if any breakpoints hit"""
        hit = []
        for seq, bp in self.breakpoints.items():
            if bp["enabled"] and (seq == seq or (bp["condition"] and bp["condition"](context))):
                bp["hit_count"] += 1
                hit.append({"sequence": seq, "breakpoint": bp})
        return hit
    
    def check_watchpoints(self, variable: str, value: Any, context: Dict) -> bool:
        """Check if watchpoint triggered"""
        if variable in self.watchpoints:
            wp = self.watchpoints[variable]
            if wp["condition"] is None or wp["condition"](value, context):
                wp["hit_count"] += 1
                wp["last_value"] = value
                return True
        return False
    
    def get_variable_history(self, name: str) -> List[Dict]:
        return self.variable_history.get(name, [])
    
    def create_checkpoint(self, name: str, tags: List[str] = None) -> str:
        """Create named checkpoint"""
        return self.snapshot_manager.create_snapshot(
            SnapshotType.CHECKPOINT, 
            tags=(tags or []) + ["checkpoint", name]
        ).id
    
    def time_travel_to(self, snapshot_id: str) -> bool:
        """Travel to specific snapshot"""
        return self.snapshot_manager.restore_snapshot(snapshot_id)
    
    def scrub_to_sequence(self, sequence: int) -> List[Dict]:
        """Scrub execution to specific sequence"""
        trace = self.snapshot_manager.get_execution_trace(0, sequence)
        return trace
    
    def analyze_failure(self, snapshot_id: str) -> Dict:
        """Analyze failure from snapshot"""
        snapshot = self.snapshot_manager.get_snapshot(snapshot_id)
        if not snapshot:
            return {"error": "Snapshot not found"}
        
        # Analyze trace leading to failure
        trace = self.snapshot_manager.get_execution_trace()
        
        return {
            "snapshot_id": snapshot_id,
            "failure_point": trace[-1] if trace else None,
            "trace_length": len(trace),
            "state_summary": {
                "processes": len(snapshot.state.process_table),
                "threads": len(snapshot.state.thread_table),
                "memory_regions": len(snapshot.state.memory_map),
                "capabilities": sum(len(t) for t in snapshot.state.capability_tables.values()),
            }
        }


_global_snapshot_manager: Optional[SnapshotManager] = None
_global_debugger: Optional[TimeTravelDebugger] = None


def get_snapshot_manager() -> SnapshotManager:
    global _global_snapshot_manager
    if _global_snapshot_manager is None:
        _global_snapshot_manager = SnapshotManager()
    return _global_snapshot_manager


def get_time_travel_debugger() -> TimeTravelDebugger:
    global _global_debugger
    if _global_debugger is None:
        _global_debugger = TimeTravelDebugger()
    return _global_debugger