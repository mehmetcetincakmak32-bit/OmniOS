import json
import os


class App:
    def __init__(self, name, icon, category, platform="cross"):
        self.name = name
        self.icon = icon
        self.category = category
        self.platform = platform

    def to_dict(self):
        return {
            "name": self.name,
            "icon": self.icon,
            "category": self.category,
            "platform": self.platform,
        }


class AppManager:
    ANDROID_APPS = [
        App("Chrome", "\U0001F310", "internet", "android"),
        App("Maps", "\U0001F5FA", "navigation", "android"),
        App("Gmail", "\u2709", "communication", "android"),
        App("YouTube", "\u25B6", "media", "android"),
        App("Drive", "\U0001F4C1", "productivity", "android"),
        App("Photos", "\U0001F4F7", "media", "android"),
    ]

    IOS_APPS = [
        App("Safari", "\U0001F310", "internet", "ios"),
        App("Messages", "\U0001F4AC", "communication", "ios"),
        App("Camera", "\U0001F4F7", "media", "ios"),
        App("Notes", "\U0001F4DD", "productivity", "ios"),
        App("Music", "\U0001F3B5", "media", "ios"),
        App("Wallet", "\U0001F4B3", "finance", "ios"),
    ]

    CROSS_APPS = [
        App("Settings", "\u2699", "system", "cross"),
        App("Phone", "\U0001F4DE", "communication", "cross"),
        App("Calculator", "\U0001F522", "tools", "cross"),
        App("Calendar", "\U0001F4C5", "productivity", "cross"),
        App("Clock", "\u23F0", "tools", "cross"),
        App("Weather", "\u2601", "tools", "cross"),
    ]

    def get_all_apps(self):
        return self.CROSS_APPS + self.ANDROID_APPS + self.IOS_APPS

    def get_apps_by_platform(self, platform):
        if platform == "all":
            return self.get_all_apps()
        apps = self.CROSS_APPS[:]
        if platform == "android":
            apps += self.ANDROID_APPS
        elif platform == "ios":
            apps += self.IOS_APPS
        return apps

    def get_suggested_apps(self, count=4):
        import random

        all_apps = self.get_all_apps()
        random.shuffle(all_apps)
        return all_apps[:count]
