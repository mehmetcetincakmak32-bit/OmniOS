"""Eklenti Sistemi - OmniOS icin dinamik eklenti yukleme"""
import importlib
import inspect
import os
import sys


class PluginMeta(type):
    plugins = []

    def __new__(mcs, name, bases, attrs):
        cls = super().__new__(mcs, name, bases, attrs)
        if name != "OmniOSPlugin":
            mcs.plugins.append(cls)
        return cls


class OmniOSPlugin(metaclass=PluginMeta):
    name = "unnamed"
    version = "1.0.0"
    description = ""
    author = ""
    dependencies = []

    def __init__(self, engine=None):
        self.engine = engine
        self.enabled = False

    def on_load(self):
        pass

    def on_enable(self):
        self.enabled = True

    def on_disable(self):
        self.enabled = False

    def on_unload(self):
        pass

    def get_info(self) -> dict:
        return {
            "name": self.name,
            "version": self.version,
            "description": self.description,
            "author": self.author,
            "enabled": self.enabled,
        }


class PluginManager:
    def __init__(self, engine=None):
        self.engine = engine
        self._plugins = {}
        self._plugin_dirs = []

    def discover(self, directory: str = None):
        if directory and directory not in self._plugin_dirs:
            self._plugin_dirs.append(directory)
            if directory not in sys.path:
                sys.path.insert(0, directory)

        for dir_path in self._plugin_dirs:
            self._scan_directory(dir_path)

        for plugin_cls in PluginMeta.plugins:
            name = plugin_cls.name
            if name not in self._plugins:
                instance = plugin_cls(engine=self.engine)
                self._plugins[name] = instance
                instance.on_load()

    def _scan_directory(self, directory: str):
        if not os.path.isdir(directory):
            return
        for fname in os.listdir(directory):
            if fname.endswith(".py") and not fname.startswith("_"):
                module_name = fname[:-3]
                try:
                    importlib.import_module(module_name)
                except ImportError as e:
                    print(f"Plugin yukleme hatasi ({fname}): {e}")

    def enable(self, name: str) -> bool:
        if name in self._plugins:
            self._plugins[name].on_enable()
            return True
        return False

    def disable(self, name: str) -> bool:
        if name in self._plugins:
            self._plugins[name].on_disable()
            return True
        return False

    def get_plugin(self, name: str):
        return self._plugins.get(name)

    def get_all_plugins(self) -> list:
        return [p.get_info() for p in self._plugins.values()]

    def get_enabled_plugins(self) -> list:
        return [p.get_info() for p in self._plugins.values() if p.enabled]

    def unload_all(self):
        for plugin in self._plugins.values():
            plugin.on_unload()
        self._plugins.clear()
