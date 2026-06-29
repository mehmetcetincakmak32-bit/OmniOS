"""OmniOS Notification System"""
from enum import Enum, auto
from dataclasses import dataclass, field
from typing import Callable, List, Optional, Dict
from datetime import datetime
from .logger import get_logger


class NotificationPriority(Enum):
    LOW = 0
    NORMAL = 1
    HIGH = 2
    CRITICAL = 3


class NotificationCategory(Enum):
    SYSTEM = "system"
    APP = "app"
    MESSAGE = "message"
    EMAIL = "email"
    SOCIAL = "social"
    REMINDER = "reminder"
    UPDATE = "update"
    SECURITY = "security"


@dataclass
class Notification:
    id: str
    title: str
    body: str
    category: NotificationCategory
    priority: NotificationPriority = NotificationPriority.NORMAL
    app_name: str = "System"
    app_icon: Optional[str] = None
    timestamp: datetime = field(default_factory=datetime.now)
    actions: List[Dict[str, str]] = field(default_factory=list)
    is_read: bool = False
    is_dismissed: bool = False
    group_id: Optional[str] = None
    sound: Optional[str] = None
    vibration: bool = True
    led_color: Optional[str] = None


class NotificationCenter:
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
        self.notifications: List[Notification] = []
        self.listeners: List[Callable] = []
        self.max_notifications = 500
        self.grouped_notifications: Dict[str, List[Notification]] = {}

    def add_listener(self, callback: Callable[[Notification], None]):
        self.listeners.append(callback)

    def remove_listener(self, callback: Callable):
        if callback in self.listeners:
            self.listeners.remove(callback)

    def notify(self, notification: Notification) -> str:
        if len(self.notifications) >= self.max_notifications:
            self.notifications.pop(0)

        self.notifications.insert(0, notification)

        if notification.group_id:
            if notification.group_id not in self.grouped_notifications:
                self.grouped_notifications[notification.group_id] = []
            self.grouped_notifications[notification.group_id].insert(0, notification)

        for listener in self.listeners:
            try:
                listener(notification)
            except Exception as e:
                self.logger.error(f"Notification listener error: {e}")

        self.logger.info(f"Notification: {notification.title} - {notification.body}")
        return notification.id

    def create_notification(self, title: str, body: str, category: NotificationCategory = NotificationCategory.SYSTEM,
                          priority: NotificationPriority = NotificationPriority.NORMAL,
                          app_name: str = "System", actions: List[Dict[str, str]] = None,
                          group_id: Optional[str] = None) -> Notification:
        notification = Notification(
            id=f"notif_{datetime.now().strftime('%Y%m%d%H%M%S%f')}",
            title=title,
            body=body,
            category=category,
            priority=priority,
            app_name=app_name,
            actions=actions or [],
            group_id=group_id
        )
        self.notify(notification)
        return notification

    def mark_read(self, notification_id: str) -> bool:
        for n in self.notifications:
            if n.id == notification_id:
                n.is_read = True
                return True
        return False

    def dismiss(self, notification_id: str) -> bool:
        for i, n in enumerate(self.notifications):
            if n.id == notification_id:
                n.is_dismissed = True
                self.notifications.pop(i)
                return True
        return False

    def dismiss_all(self):
        self.notifications.clear()
        self.grouped_notifications.clear()
        self.logger.info("All notifications dismissed")

    def get_notifications(self, unread_only: bool = False, category: Optional[NotificationCategory] = None,
                         limit: Optional[int] = None) -> List[Notification]:
        result = []
        for n in self.notifications:
            if n.is_dismissed:
                continue
            if unread_only and n.is_read:
                continue
            if category and n.category != category:
                continue
            result.append(n)
            if limit and len(result) >= limit:
                break
        return result

    def get_unread_count(self) -> int:
        return sum(1 for n in self.notifications if not n.is_read and not n.is_dismissed)

    def get_grouped(self, group_id: str) -> List[Notification]:
        return self.grouped_notifications.get(group_id, [])

    def clear_group(self, group_id: str):
        if group_id in self.grouped_notifications:
            for n in self.grouped_notifications[group_id]:
                n.is_dismissed = True
                if n in self.notifications:
                    self.notifications.remove(n)
            del self.grouped_notifications[group_id]


def get_notification_center() -> NotificationCenter:
    return NotificationCenter()