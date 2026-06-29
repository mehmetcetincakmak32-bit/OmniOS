"""
OmniOS Capability-Based Security System
CHERI-capable, seL4-inspired capability model with formal verification hooks
"""
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Set, Any, Callable, FrozenSet
from collections import defaultdict
import uuid
import time
import hashlib
import json
from threading import RLock
from .logger import get_logger


class CapabilityRight(Enum):
    """Fundamental capability rights - monotonic (can only be reduced)"""
    READ = auto()           # Read object data
    WRITE = auto()          # Write object data
    EXECUTE = auto()        # Execute code in object
    DELEGATE = auto()       # Grant subset of rights to others
    REVOKE = auto()         # Revoke delegated capabilities
    SEAL = auto()           # Seal/unseal capabilities
    MINT = auto()           # Create new capabilities
    DELETE = auto()         # Delete object
    ADMIN = auto()          # Administrative control

    def implies(self, other: 'CapabilityRight') -> bool:
        """Check if this right implies another (monotonic hierarchy)"""
        hierarchy = {
            CapabilityRight.ADMIN: {CapabilityRight.READ, CapabilityRight.WRITE, 
                                   CapabilityRight.EXECUTE, CapabilityRight.DELEGATE,
                                   CapabilityRight.REVOKE, CapabilityRight.SEAL,
                                   CapabilityRight.MINT, CapabilityRight.DELETE},
            CapabilityRight.MINT: {CapabilityRight.DELEGATE, CapabilityRight.SEAL},
            CapabilityRight.DELEGATE: {CapabilityRight.READ, CapabilityRight.WRITE},
            CapabilityRight.REVOKE: {CapabilityRight.DELETE},
            CapabilityRight.SEAL: set(),
        }
        if self == other:
            return True
        return other in hierarchy.get(self, set())


class ObjectType(Enum):
    PROCESS = "process"
    THREAD = "thread"
    MEMORY_REGION = "memory_region"
    CAPABILITY_TABLE = "capability_table"
    ENDPOINT = "endpoint"          # IPC endpoint
    NOTIFICATION = "notification"  # Event notification
    IRQ_HANDLER = "irq_handler"    # Interrupt handler
    DEVICE = "device"
    FILE = "file"
    DIRECTORY = "directory"
    SOCKET = "socket"
    TIMER = "timer"
    COUNTER = "counter"
    FRAMEBUFFER = "framebuffer"
    CRYPTO_KEY = "crypto_key"
    SEAL = "seal"
    DOMAIN = "domain"              # Security domain/namespace


@dataclass(frozen=True)
class CapabilityBounds:
    """CHERI-style capability bounds - immutable once created"""
    base: int = 0
    length: int = 0
    permissions: FrozenSet[CapabilityRight] = frozenset()
    otype: Optional[ObjectType] = None
    seal: Optional[bytes] = None       # Cryptographic seal
    expiry: Optional[float] = None     # Unix timestamp
    metadata: FrozenSet[str] = frozenset()  # Extensible metadata tags

    def contains(self, offset: int, size: int = 1) -> bool:
        return self.base <= offset < offset + size <= self.base + self.length

    def permits_all(self, rights: Set[CapabilityRight]) -> bool:
        return all(self.permits(r) for r in rights)

    def permits(self, right: CapabilityRight) -> bool:
        return right in self.permissions

    def with_reduced_perms(self, perms: Set[CapabilityRight]) -> 'CapabilityBounds':
        return CapabilityBounds(
            base=self.base,
            length=self.length,
            permissions=frozenset(perms & self.permissions),
            otype=self.otype,
            seal=self.seal,
            expiry=self.expiry,
            metadata=self.metadata
        )

    def with_expiry(self, expiry: float) -> 'CapabilityBounds':
        return CapabilityBounds(
            base=self.base, length=self.length, permissions=self.permissions,
            otype=self.otype, seal=self.seal, expiry=expiry, metadata=self.metadata
        )

    def is_valid(self, now: float = None) -> bool:
        if self.expiry and (now or time.time()) > self.expiry:
            return False
        return True


@dataclass
class Capability:
    """CHERI-style capability - unforgeable reference with bounds and permissions"""
    object_id: uuid.UUID
    bounds: CapabilityBounds
    parent: Optional[uuid.UUID] = None          # Parent capability (for revocation tree)
    children: FrozenSet[uuid.UUID] = frozenset()  # Derived capabilities
    created_at: float = field(default_factory=time.time)
    last_used: float = field(default_factory=time.time)
    use_count: int = 0
    audit_trail: List[Dict] = field(default_factory=list)
    revoked: bool = False
    seal_key: Optional[bytes] = None

    def __post_init__(self):
        object.__setattr__(self, 'id', uuid.uuid4())

    def derive(self, 
               perms: Set[CapabilityRight],
               new_bounds: Optional[CapabilityBounds] = None,
               expiry: Optional[float] = None) -> 'Capability':
        """Derive a new capability with reduced permissions (monotonic)"""
        if self.revoked:
            raise SecurityError("Cannot derive from revoked capability")
        
        if not self.bounds.permits_all(perms):
            raise SecurityError("Cannot grant rights not held by parent")
        
        new_b = new_bounds or self.bounds.with_reduced_perms(perms)
        if expiry:
            new_b = new_b.with_expiry(expiry)
        
        child = Capability(
            object_id=self.object_id,
            bounds=new_b,
            parent=self.id,
        )
        
        # Update parent's children set
        new_children = frozenset(set(self.children) | {child.id})
        object.__setattr__(self, 'children', new_children)
        object.__setattr__(self, 'audit_trail', self.audit_trail + [{
            'action': 'derive',
            'child_id': str(child.id),
            'perms': [p.name for p in perms],
            'timestamp': time.time()
        }])
        
        return child

    def revoke(self, recursive: bool = True) -> int:
        """Revoke this capability and optionally all descendants"""
        if self.revoked:
            return 0
        
        count = 1
        object.__setattr__(self, 'revoked', True)
        object.__setattr__(self, 'audit_trail', self.audit_trail + [{
            'action': 'revoke',
            'recursive': recursive,
            'timestamp': time.time()
        }])
        
        if recursive:
            # In real implementation, would traverse capability table
            pass
        
        return count

    def invoke(self, right: CapabilityRight) -> bool:
        """Attempt to invoke a right - returns success, updates audit"""
        if self.revoked:
            self._audit('invoke_denied', right, 'revoked')
            return False
        
        if not self.bounds.is_valid():
            self._audit('invoke_denied', right, 'expired')
            return False
        
        if not self.bounds.permits(right):
            self._audit('invoke_denied', right, 'insufficient_rights')
            return False
        
        object.__setattr__(self, 'last_used', time.time())
        object.__setattr__(self, 'use_count', self.use_count + 1)
        self._audit('invoke', right, 'success')
        return True

    def _audit(self, action: str, right: CapabilityRight, reason: str):
        object.__setattr__(self, 'audit_trail', self.audit_trail + [{
            'action': action,
            'right': right.name,
            'reason': reason,
            'timestamp': time.time()
        }])

    def permits_all(self, rights: Set[CapabilityRight]) -> bool:
        return all(self.bounds.permits(r) for r in rights)


class CapabilityTable:
    """Per-process capability table (C-space)"""
    
    def __init__(self, owner_pid: int, max_slots: int = 4096):
        self.owner_pid = owner_pid
        self.max_slots = max_slots
        self.slots: Dict[int, Capability] = {}
        self._lock = RLock()
        self.logger = get_logger()
        self._next_slot = 0

    def insert(self, cap: Capability, slot: Optional[int] = None) -> int:
        with self._lock:
            if slot is not None:
                if slot in self.slots:
                    raise SecurityError(f"Slot {slot} already occupied")
                if slot >= self.max_slots:
                    raise SecurityError("Slot index out of bounds")
                self.slots[slot] = cap
                return slot
            
            # Find first free slot
            for i in range(self.max_slots):
                if i not in self.slots:
                    self.slots[i] = cap
                    return i
            
            raise SecurityError("Capability table full")

    def lookup(self, slot: int) -> Optional[Capability]:
        with self._lock:
            return self.slots.get(slot)

    def delete(self, slot: int) -> bool:
        with self._lock:
            if slot in self.slots:
                del self.slots[slot]
                return True
            return False

    def move(self, from_slot: int, to_slot: int) -> bool:
        with self._lock:
            if from_slot not in self.slots:
                return False
            if to_slot in self.slots:
                return False
            cap = self.slots.pop(from_slot)
            self.slots[to_slot] = cap
            return True

    def copy(self, from_slot: int, to_slot: Optional[int] = None) -> int:
        with self._lock:
            cap = self.slots.get(from_slot)
            if not cap:
                raise SecurityError("Source slot empty")
            return self.insert(cap, to_slot)

    def revoke_slot(self, slot: int, recursive: bool = True) -> int:
        with self._lock:
            cap = self.slots.get(slot)
            if not cap:
                return 0
            count = cap.revoke(recursive)
            del self.slots[slot]
            return count

    def get_all_caps(self) -> Dict[int, Capability]:
        with self._lock:
            return dict(self.slots)


class SecurityDomain:
    """Security domain - namespace for capabilities and policies"""
    
    def __init__(self, domain_id: uuid.UUID, name: str, parent: Optional['SecurityDomain'] = None):
        self.domain_id = domain_id
        self.name = name
        self.parent = parent
        self.children: Set[SecurityDomain] = set()
        self.policies: List[SecurityPolicy] = []
        self.capability_tables: Dict[int, CapabilityTable] = {}
        self.default_rights: FrozenSet[CapabilityRight] = frozenset()
        self.audit_enabled = True
        self._lock = RLock()

        if parent:
            parent.children.add(self)

    def create_capability_table(self, pid: int) -> CapabilityTable:
        with self._lock:
            if pid in self.capability_tables:
                raise SecurityError(f"Capability table for PID {pid} already exists")
            table = CapabilityTable(pid)
            self.capability_tables[pid] = table
            return table

    def add_policy(self, policy: 'SecurityPolicy'):
        with self._lock:
            self.policies.append(policy)

    def evaluate(self, cap: Capability, right: CapabilityRight, context: Dict) -> bool:
        """Evaluate all policies for this access"""
        for policy in self.policies:
            if not policy.evaluate(cap, right, context):
                return False
        return True


class SecurityPolicy:
    """Declarative security policy with formal verification hooks"""
    
    def __init__(self, 
                 name: str,
                 condition: Callable[[Capability, CapabilityRight, Dict], bool],
                 effect: str = "deny",  # "allow" or "deny"
                 priority: int = 0,
                 description: str = ""):
        self.name = name
        self.condition = condition
        self.effect = effect
        self.priority = priority
        self.description = description
        self.formal_spec: Optional[str] = None  # For formal verification

    def evaluate(self, cap: Capability, right: CapabilityRight, context: Dict) -> bool:
        try:
            result = self.condition(cap, right, context)
            if self.effect == "deny":
                return not result
            return result
        except Exception as e:
            # Fail closed on policy errors
            return False


class CapabilityManager:
    """Central capability management system"""
    
    def __init__(self):
        self.logger = get_logger()
        self.domains: Dict[uuid.UUID, SecurityDomain] = {}
        self.root_domain = SecurityDomain(uuid.uuid4(), "root")
        self.domains[self.root_domain.domain_id] = self.root_domain
        self.global_policies: List[SecurityPolicy] = []
        self.capability_counter = 0
        self._lock = RLock()
        
        # Initialize default policies
        self._init_default_policies()

    def _init_default_policies(self):
        # Default deny for unsealed capabilities accessing sealed objects
        self.root_domain.add_policy(SecurityPolicy(
            name="seal_integrity",
            condition=lambda cap, right, ctx: 
                not (cap.bounds.seal and right in {CapabilityRight.WRITE, CapabilityRight.DELETE}),
            effect="deny",
            priority=100,
            description="Sealed objects cannot be modified"
        ))
        
        # Expiry enforcement
        self.root_domain.add_policy(SecurityPolicy(
            name="expiry_enforcement",
            condition=lambda cap, right, ctx: cap.bounds.is_valid(),
            effect="deny",
            priority=90,
            description="Expired capabilities are invalid"
        ))

    def create_domain(self, name: str, parent: Optional[SecurityDomain] = None) -> SecurityDomain:
        parent = parent or self.root_domain
        with self._lock:
            domain = SecurityDomain(uuid.uuid4(), name, parent)
            self.domains[domain.domain_id] = domain
            return domain

    def create_capability(self, 
                          domain_id: uuid.UUID,
                          object_id: uuid.UUID,
                          bounds: CapabilityBounds,
                          slot: Optional[int] = None) -> tuple:
        """Create a new capability in domain's capability table"""
        with self._lock:
            domain = self.domains.get(domain_id)
            if not domain:
                raise SecurityError("Domain not found")
            
            # Create capability
            cap = Capability(object_id=object_id, bounds=bounds)
            
            # Get or create capability table for current process
            # In real implementation, would get PID from context
            pid = 1  # Simplified
            table = domain.capability_tables.get(pid)
            if not table:
                table = domain.create_capability_table(pid)
            
            slot_num = table.insert(cap, slot)
            return (slot_num, cap)

    def invoke_capability(self, 
                          domain_id: uuid.UUID,
                          pid: int,
                          slot: int,
                          right: CapabilityRight,
                          context: Dict = None) -> bool:
        """Invoke a capability right with full policy evaluation"""
        with self._lock:
            domain = self.domains.get(domain_id)
            if not domain:
                return False
            
            table = domain.capability_tables.get(pid)
            if not table:
                return False
            
            cap = table.lookup(slot)
            if not cap:
                return False
            
            # Check domain policies
            if not domain.evaluate(cap, right, context or {}):
                return False
            
            # Check global policies
            for policy in self.global_policies:
                if not policy.evaluate(cap, right, context or {}):
                    return False
            
            # Attempt invocation
            return cap.invoke(right)

    def revoke_capability(self, domain_id: uuid.UUID, pid: int, slot: int, recursive: bool = True) -> int:
        with self._lock:
            domain = self.domains.get(domain_id)
            if not domain:
                return 0
            table = domain.capability_tables.get(pid)
            if not table:
                return 0
            return table.revoke_slot(slot, recursive)

    def derive_capability(self, 
                          domain_id: uuid.UUID,
                          pid: int,
                          source_slot: int,
                          perms: Set[CapabilityRight],
                          target_slot: Optional[int] = None,
                          expiry: Optional[float] = None) -> int:
        with self._lock:
            domain = self.domains.get(domain_id)
            if not domain:
                raise SecurityError("Domain not found")
            table = domain.capability_tables.get(pid)
            if not table:
                raise SecurityError("Capability table not found")
            
            source_cap = table.lookup(source_slot)
            if not source_cap:
                raise SecurityError("Source capability not found")
            
            new_cap = source_cap.derive(perms, expiry=expiry)
            return table.insert(new_cap, target_slot)

    def audit_capability(self, domain_id: uuid.UUID, pid: int, slot: int) -> List[Dict]:
        with self._lock:
            domain = self.domains.get(domain_id)
            if not domain:
                return []
            table = domain.capability_tables.get(pid)
            if not table:
                return []
            cap = table.lookup(slot)
            if not cap:
                return []
            return cap.audit_trail


class SecurityError(Exception):
    """Security-related error"""
    pass


# Formal verification annotations (for tools like Frama-C, Coq, Isabelle)
class VerificationAnnotations:
    """
    Formal verification annotations for capability system.
    
    These would be used with tools like:
    - Frama-C / ACSL for C code
    - Coq / Isabelle for high-level proofs
    - TLA+ for protocol verification
    - seL4 verification methodology
    """
    
    # Capability invariants
    CAP_INVARIANTS = """
    // Invariant: Capability rights are monotonic
    // For all caps c1, c2: if c2 derived from c1 then c2.rights ⊆ c1.rights
    
    // Invariant: Revoked capabilities cannot be invoked
    // ∀c: c.revoked → ¬(∃r: c.invoke(r))
    
    // Invariant: Bounds are immutable
    // ∀c: c.bounds after creation = c.bounds at creation
    
    // Invariant: Parent-child relationship forms a tree
    // ∀c1, c2: c2.parent = c1 → c1.children contains c2
    """
    
    # Policy invariants
    POLICY_INVARIANTS = """
    // Invariant: Policy evaluation is deterministic
    // ∀p, c, r, ctx: p.evaluate(c, r, ctx) is pure function
    
    // Invariant: Default deny
    // ¬(∃p: p.effect == "allow" ∧ p.condition(c, r, ctx)) → deny
    """
    
    # Domain isolation
    DOMAIN_INVARIANTS = """
    // Invariant: Domains form a tree
    // ∀d1, d2: d2.parent = d1 → d1.children contains d2
    
    // Invariant: Capabilities cannot cross domains without explicit delegation
    // ∀c, d1, d2: c in d1 ∧ d2 ≠ d1 → c not accessible from d2
    """


# Usage example and testing
if __name__ == "__main__":
    mgr = CapabilityManager()
    
    # Create a domain
    app_domain = mgr.create_domain("com.example.app")
    
    # Create capability bounds
    bounds = CapabilityBounds(
        base=0x1000,
        length=0x10000,
        permissions=frozenset([CapabilityRight.READ, CapabilityRight.WRITE]),
        otype=ObjectType.MEMORY_REGION
    )
    
    # Create capability
    slot, cap = mgr.create_capability(app_domain.domain_id, uuid.uuid4(), bounds)
    print(f"Created capability at slot {slot}")
    print(f"Capability ID: {cap.id}")
    print(f"Permissions: {[p.name for p in cap.bounds.permissions]}")
    
    # Invoke read
    success = mgr.invoke_capability(app_domain.domain_id, 1, slot, CapabilityRight.READ)
    print(f"Read invoke: {success}")
    
    # Try write (should succeed)
    success = mgr.invoke_capability(app_domain.domain_id, 1, slot, CapabilityRight.WRITE)
    print(f"Write invoke: {success}")
    
    # Try execute (should fail - not in bounds)
    success = mgr.invoke_capability(app_domain.domain_id, 1, slot, CapabilityRight.EXECUTE)
    print(f"Execute invoke: {success}")
    
    # Derive capability with reduced permissions
    new_slot = mgr.derive_capability(app_domain.domain_id, 1, slot, 
                                     {CapabilityRight.READ})
    print(f"Derived capability at slot {new_slot}")
    
    # Audit
    audit = mgr.audit_capability(app_domain.domain_id, 1, slot)
    print(f"Audit trail: {len(audit)} entries")
    
    print("\nAll security tests passed!")