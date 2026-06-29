"""OmniOS Distributed Systems Module"""
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Callable, Any, Set
from collections import defaultdict
import uuid
import time
import hashlib
import json
from threading import Lock, RLock
from .logger import get_logger


class NodeRole(Enum):
    LEADER = "leader"
    FOLLOWER = "follower"
    CANDIDATE = "candidate"
    OBSERVER = "observer"


class ConsensusState(Enum):
    IDLE = "idle"
    ELECTING = "electing"
    REPLICATING = "replicating"
    COMMITTED = "committed"
    FAILED = "failed"


@dataclass
class NodeInfo:
    node_id: str
    address: str
    port: int
    capabilities: Set[str]
    last_heartbeat: float = 0
    role: NodeRole = NodeRole.FOLLOWER
    term: int = 0
    voted_for: Optional[str] = None
    last_log_index: int = 0
    last_log_term: int = 0


@dataclass
class LogEntry:
    index: int
    term: int
    command: Dict[str, Any]
    timestamp: float
    checksum: str


class RaftConsensus:
    def __init__(self, node_id: str, cluster_nodes: List[NodeInfo]):
        self.node_id = node_id
        self.nodes = {n.node_id: n for n in cluster_nodes}
        self.logger = get_logger()
        
        self.current_term = 0
        self.voted_for: Optional[str] = None
        self.log: List[LogEntry] = []
        self.commit_index = 0
        self.last_applied = 0
        
        self.state = ConsensusState.IDLE
        self.role = NodeRole.FOLLOWER
        self.leader_id: Optional[str] = None
        
        self.election_timeout = 150  # ms
        self.heartbeat_interval = 50  # ms
        self.last_heartbeat = time.time()
        self.last_contact = {nid: time.time() for nid in self.nodes}
        
        self._lock = RLock()
        self._election_timer = 0

    def start_election(self):
        with self._lock:
            self.current_term += 1
            self.role = NodeRole.CANDIDATE
            self.voted_for = self.node_id
            self.logger.info(f"Node {self.node_id} starting election for term {self.current_term}")
            
            votes = 1  # Vote for self
            for nid, node in self.nodes.items():
                if nid != self.node_id:
                    if self._request_vote(nid):
                        votes += 1
            
            if votes > len(self.nodes) // 2:
                self.become_leader()
            else:
                self.role = NodeRole.FOLLOWER

    def _request_vote(self, target_id: str) -> bool:
        # Simulate RPC
        target = self.nodes.get(target_id)
        if not target:
            return False
        
        last_log_index = len(self.log) - 1
        last_log_term = self.log[-1].term if self.log else 0
        
        # Vote granted if:
        # 1. term >= candidate's term
        # 2. voted_for is None or candidate_id
        # 2. candidate's log is at least as up-to-date
        if target.term > self.current_term:
            self.current_term = target.term
            self.role = NodeRole.FOLLOWER
            return False
            
        return True

    def become_leader(self):
        self.role = NodeRole.LEADER
        self.leader_id = self.node_id
        self.logger.info(f"Node {self.node_id} became leader for term {self.current_term}")
        
        # Initialize leader state
        for nid in self.nodes:
            if nid != self.node_id:
                node = self.nodes[nid]
                node.last_log_index = len(self.log)
                node.last_log_term = self.log[-1].term if self.log else 0
        
        self.send_heartbeats()

    def send_heartbeats(self):
        if self.role != NodeRole.LEADER:
            return
            
        with self._lock:
            for nid, node in self.nodes.items():
                if nid != self.node_id:
                    self._send_append_entries(nid)

    def _send_append_entries(self, target_id: str) -> bool:
        target = self.nodes.get(target_id)
        if not target:
            return False
            
        prev_log_index = target.last_log_index
        prev_log_term = target.last_log_term
        
        entries = self.log[prev_log_index + 1:] if prev_log_index < len(self.log) else []
        
        # Simulate RPC response
        success = self._replicate_to(target_id, entries)
        if success:
            target.last_log_index = len(self.log) - 1
            target.last_log_term = self.log[-1].term if self.log else 0
            
        return success

    def _replicate_to(self, target_id: str, entries: List[LogEntry]) -> bool:
        # Simulate replication
        return True

    def propose(self, command: Dict[str, Any]) -> bool:
        with self._lock:
            if self.role != NodeRole.LEADER:
                return False
                
            entry = LogEntry(
                index=len(self.log),
                term=self.current_term,
                command=command,
                timestamp=time.time(),
                checksum=self._compute_checksum(command)
            )
            
            self.log.append(entry)
            
            # Replicate to majority
            replicated = 1
            for nid in self.nodes:
                if nid != self.node_id:
                    if self._replicate_to(nid, [entry]):
                        replicated += 1
            
            if replicated > len(self.nodes) // 2:
                self.commit_index = len(self.log) - 1
                self._apply_committed()
                return True
                
            return False

    def _apply_committed(self):
        while self.last_applied < self.commit_index:
            self.last_applied += 1
            entry = self.log[self.last_applied]
            self._apply_command(entry.command)

    def _apply_command(self, command: Dict[str, Any]):
        pass

    def _compute_checksum(self, command: Dict[str, Any]) -> str:
        return hashlib.sha256(json.dumps(command, sort_keys=True).encode()).hexdigest()[:16]

    def handle_append_entries(self, leader_id: str, term: int, prev_log_index: int, 
                              prev_log_term: int, entries: List[LogEntry], 
                              leader_commit: int) -> bool:
        with self._lock:
            if term < self.current_term:
                return False
                
            if term > self.current_term:
                self.current_term = term
                self.role = NodeRole.FOLLOWER
                
            self.leader_id = leader_id
            self.last_heartbeat = time.time()
            
            # Check log consistency
            if prev_log_index >= 0:
                if prev_log_index >= len(self.log):
                    return False
                if self.log[prev_log_index].term != prev_log_term:
                    return False
                    
            # Append new entries
            if entries:
                self.log = self.log[:prev_log_index + 1] + entries
                
            if leader_commit > self.commit_index:
                self.commit_index = min(leader_commit, len(self.log) - 1)
                self._apply_committed()
                
            return True


class CRDT:
    """Conflict-free Replicated Data Type base class"""
    
    def merge(self, other: 'CRDT') -> 'CRDT':
        raise NotImplementedError
    
    def to_dict(self) -> Dict:
        raise NotImplementedError
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'CRDT':
        raise NotImplementedError


class GCounter(CRDT):
    """Grow-only Counter"""
    
    def __init__(self, node_id: str):
        self.node_id = node_id
        self.counts: Dict[str, int] = defaultdict(int)
    
    def increment(self, node_id: str = None):
        nid = node_id or self.node_id
        self.counts[nid] += 1
    
    def value(self) -> int:
        return sum(self.counts.values())
    
    def merge(self, other: 'GCounter') -> 'GCounter':
        result = GCounter(self.node_id)
        all_nodes = set(self.counts.keys()) | set(other.counts.keys())
        for nid in all_nodes:
            result.counts[nid] = max(self.counts.get(nid, 0), other.counts.get(nid, 0))
        return result
    
    def to_dict(self) -> Dict:
        return {"type": "GCounter", "counts": dict(self.counts)}
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'GCounter':
        c = cls("")
        c.counts = defaultdict(int, data.get("counts", {}))
        return c


class PNCounter(CRDT):
    """Positive-Negative Counter"""
    
    def __init__(self, node_id: str):
        self.positive = GCounter(node_id)
        self.negative = GCounter(node_id)
    
    def increment(self):
        self.positive.increment()
    
    def decrement(self):
        self.negative.increment()
    
    def value(self) -> int:
        return self.positive.value() - self.negative.value()
    
    def merge(self, other: 'PNCounter') -> 'PNCounter':
        result = PNCounter(self.node_id)
        result.positive = self.positive.merge(other.positive)
        result.negative = self.negative.merge(other.negative)
        return result


class LWWRegister(CRDT):
    """Last-Writer-Wins Register"""
    
    def __init__(self, node_id: str, value: Any = None):
        self.node_id = node_id
        self.value = value
        self.timestamp = 0
        self.node = node_id
    
    def set(self, value: Any):
        self.value = value
        self.timestamp = time.time()
        self.node = self.node_id
    
    def merge(self, other: 'LWWRegister') -> 'LWWRegister':
        if other.timestamp > self.timestamp:
            return other
        return self
    
    def to_dict(self) -> Dict:
        return {"type": "LWWRegister", "value": self.value, "timestamp": self.timestamp, "node": self.node}
    
    @classmethod
    def from_dict(cls, data: Dict) -> 'LWWRegister':
        r = cls("", data.get("value"))
        r.timestamp = data.get("timestamp", 0)
        r.node = data.get("node", "")
        return r


class ORSet(CRDT):
    """Observed-Remove Set"""
    
    def __init__(self, node_id: str):
        self.node_id = node_id
        self.elements: Dict[Any, Dict[str, float]] = {}  # element -> {node: timestamp}
    
    def add(self, element: Any):
        if element not in self.elements:
            self.elements[element] = {}
        self.elements[element][self.node_id] = time.time()
    
    def remove(self, element: Any):
        if element in self.elements:
            self.elements[element][self.node_id] = time.time()
    
    def contains(self, element: Any) -> bool:
        if element not in self.elements:
            return False
        # Element exists if there's an add without a matching remove
        tags = self.elements[element]
        # In full implementation, track add/remove tags separately
        return len(tags) > 0
    
    def value(self) -> Set[Any]:
        return set(self.elements.keys())
    
    def merge(self, other: 'ORSet') -> 'ORSet':
        result = ORSet(self.node_id)
        all_elements = set(self.elements.keys()) | set(other.elements.keys())
        for elem in all_elements:
            tags = {}
            tags.update(self.elements.get(elem, {}))
            tags.update(other.elements.get(elem, {}))
            if tags:
                result.elements[elem] = tags
        return result


class DeviceMesh:
    def __init__(self, node_id: str, local_address: str, port: int):
        self.node_id = node_id
        self.local_address = local_address
        self.port = port
        self.logger = get_logger()
        
        self.peers: Dict[str, NodeInfo] = {}
        self.consensus = RaftConsensus(node_id, [])
        self.crdt_store: Dict[str, CRDT] = {}
        
        self._running = False
        self._lock = Lock()

    def add_peer(self, node_id: str, address: str, port: int, capabilities: Set[str] = None):
        with self._lock:
            self.peers[node_id] = NodeInfo(
                node_id=node_id,
                address=address,
                port=port,
                capabilities=capabilities or set()
            )
        self.logger.info(f"Added peer: {node_id} at {address}:{port}")

    def remove_peer(self, node_id: str):
        with self._lock:
            if node_id in self.peers:
                del self.peers[node_id]
        self.logger.info(f"Removed peer: {node_id}")

    def get_peers(self) -> List[NodeInfo]:
        return list(self.peers.values())

    def propose_state_change(self, command: Dict[str, Any]) -> bool:
        return self.consensus.propose(command)

    def register_crdt(self, name: str, crdt: CRDT):
        self.crdt_store[name] = crdt
        self.logger.info(f"Registered CRDT: {name}")

    def get_crdt(self, name: str) -> Optional[CRDT]:
        return self.crdt_store.get(name)

    def sync_crdt(self, name: str, remote_crdt: CRDT) -> CRDT:
        local = self.crdt_store.get(name)
        if local:
            merged = local.merge(remote_crdt)
            self.crdt_store[name] = merged
            return merged
        return remote_crdt

    def broadcast_state(self):
        state = {}
        for name, crdt in self.crdt_store.items():
            state[name] = crdt.to_dict()
        return state


class DeviceMeshManager:
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
        self.meshes: Dict[str, DeviceMesh] = {}
        self._lock = Lock()
    
    def create_mesh(self, mesh_id: str, node_id: str, address: str, port: int) -> DeviceMesh:
        with self._lock:
            mesh = DeviceMesh(node_id, "0.0.0.0", port)
            self.meshes[mesh_id] = mesh
            return mesh
    
    def get_mesh(self, mesh_id: str) -> Optional[DeviceMesh]:
        return self.meshes.get(mesh_id)
    
    def remove_mesh(self, mesh_id: str):
        with self._lock:
            if mesh_id in self.meshes:
                del self.meshes[mesh_id]


def get_device_mesh_manager() -> DeviceMeshManager:
    return DeviceMeshManager()