"""OmniOS Settings Manager"""
from enum import Enum
from dataclasses import dataclass, field, asdict
from typing import Any, Dict, Optional, Callable, List
import json
import os
from pathlib import Path
from .logger import get_logger


class SettingType(Enum):
    BOOLEAN = "boolean"
    INTEGER = "integer"
    FLOAT = "float"
    STRING = "string"
    ENUM = "enum"
    COLOR = "color"
    FILE_PATH = "file_path"
    DIR_PATH = "dir_path"


@dataclass
class SettingDefinition:
    key: str
    name: str
    description: str
    setting_type: SettingType
    default_value: Any
    options: Optional[List[Any]] = None
    min_value: Optional[float] = None
    max_value: Optional[float] = None
    category: str = "general"
    restart_required: bool = False
    validator: Optional[Callable[[Any], bool]] = None


class SettingsManager:
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
        self._definitions: Dict[str, SettingDefinition] = {}
        self._values: Dict[str, Any] = {}
        self._listeners: Dict[str, List[Callable]] = {}
        self._settings_file = Path.home() / ".omnios" / "settings.json"
        self._load_defaults()
        self._load_settings()

    def _load_defaults(self):
        defaults = [
            SettingDefinition(
                key="system.dark_mode",
                name="Karanlık Mod",
                description="Sistemi karanlık temayla başlat",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="appearance",
                restart_required=True
            ),
            SettingDefinition(
                key="system.animations",
                name="Animasyonlar",
                description="UI animasyonlarını etkinleştir",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="appearance"
            ),
            SettingDefinition(
                key="system.language",
                name="Dil",
                description="Sistem dili",
                setting_type=SettingType.ENUM,
                default_value="tr",
                options=["tr", "en", "de", "fr", "es"],
                category="general",
                restart_required=True
            ),
            SettingDefinition(
                key="system.theme_color",
                name="Tema Rengi",
                description="Sistem vurgulama rengi",
                setting_type=SettingType.COLOR,
                default_value="#64FFDA",
                category="appearance"
            ),
            SettingDefinition(
                key="notifications.enabled",
                name="Bildirimler",
                description="Tüm bildirimleri etkinleştir",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="notifications"
            ),
            SettingDefinition(
                key="notifications.lock_screen",
                name="Kilit Ekranı Bildirimleri",
                description="Kilit ekranında bildirimleri göster",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="notifications"
            ),
            SettingDefinition(
                key="notifications.sound",
                name="Bildirim Sesi",
                description="Bildirim seslerini çal",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="notifications"
            ),
            SettingDefinition(
                key="gestures.enabled",
                name="Jestler",
                description="Jest tabanlı navigasyon",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="gestures"
            ),
            SettingDefinition(
                key="gestures.sensitivity",
                name="Jest Hassasiyeti",
                description="Jest algılama hassasiyeti",
                setting_type=SettingType.INTEGER,
                default_value=5,
                min_value=1,
                max_value=10,
                category="gestures"
            ),
            SettingDefinition(
                key="display.screen_timeout",
                name="Ekran Kapanma Süresi",
                description="Ekranın kapanma süresi (saniye)",
                setting_type=SettingType.INTEGER,
                default_value=30,
                min_value=15,
                max_value=300,
                category="display"
            ),
            SettingDefinition(
                key="display.always_on",
                name="Her Zaman Açık Ekran",
                description="Saat ve bildirimleri her zaman göster",
                setting_type=SettingType.BOOLEAN,
                default_value=False,
                category="display"
            ),
            SettingDefinition(
                key="power.battery_saver",
                name="Pil Tasarrufu",
                description="Pil tasarrufu modunu etkinleştir",
                setting_type=SettingType.BOOLEAN,
                default_value=False,
                category="power"
            ),
            SettingDefinition(
                key="power.auto_suspend",
                name="Otomatik Uykuda",
                description="Belirli süre inaktiflikten sonra uyku moduna geç",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="power"
            ),
            SettingDefinition(
                key="security.lock_screen",
                name="Kilit Ekranı",
                description="PIN/parola ile kilit ekranı",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="security"
            ),
            SettingDefinition(
                key="security.biometric",
                name="Biyometrik Doğrulama",
                description="Parmak izi/üz tanıma ile açma",
                setting_type=SettingType.BOOLEAN,
                default_value=False,
                category="security"
            ),
            SettingDefinition(
                key="apps.auto_update",
                name="Otomatik Güncelleme",
                description="Uygulamaları otomatik güncelle",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="apps"
            ),
            SettingDefinition(
                key="apps.unknown_sources",
                name="Bilinmeyen Kaynaklar",
                description="Resmi store dışı uygulamalara izin ver",
                setting_type=SettingType.BOOLEAN,
                default_value=False,
                category="apps"
            ),
            SettingDefinition(
                key="network.mobile_data",
                name="Mobil Veri",
                description="Mobil veri bağlantısını kullan",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="network"
            ),
            SettingDefinition(
                key="network.wifi_scan",
                name="Wi-Fi Tarama",
                description="Periyodik Wi-Fi ağı taraması",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="network"
            ),
            SettingDefinition(
                key="system.haptics",
                name="Titresim Geribildirimi",
                description="Dokunmatik titreşim geri bildirimi",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="system"
            ),
            SettingDefinition(
                key="system.auto_rotate",
                name="Otomatik Döndürme",
                description="Ekran yönünü otomatik değiştir",
                setting_type=SettingType.BOOLEAN,
                default_value=True,
                category="system"
            ),
        ]

        for d in defaults:
            self._definitions[d.key] = d
            self._values[d.key] = d.default_value

    def _load_settings(self):
        if self._settings_file.exists():
            try:
                with open(self._settings_file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                for key, value in data.items():
                    if key in self._definitions:
                        if self._validate(key, value):
                            self._values[key] = value
                self.logger.info(f"Settings loaded from {self._settings_file}")
            except Exception as e:
                self.logger.error(f"Failed to load settings: {e}")

    def _save_settings(self):
        try:
            self._settings_file.parent.mkdir(parents=True, exist_ok=True)
            with open(self._settings_file, 'w', encoding='utf-8') as f:
                json.dump(self._values, f, indent=2, ensure_ascii=False)
            self.logger.debug("Settings saved")
        except Exception as e:
            self.logger.error(f"Failed to save settings: {e}")

    def _validate(self, key: str, value: Any) -> bool:
        if key not in self._definitions:
            return False
        d = self._definitions[key]
        if d.validator and not d.validator(value):
            return False
        if d.setting_type == SettingType.INTEGER and d.min_value is not None:
            if value < d.min_value or (d.max_value is not None and value > d.max_value):
                return False
        if d.setting_type == SettingType.FLOAT and d.min_value is not None:
            if value < d.min_value or (d.max_value is not None and value > d.max_value):
                return False
        if d.setting_type == SettingType.ENUM and d.options is not None:
            if value not in d.options:
                return False
        return True

    def register(self, definition: SettingDefinition):
        self._definitions[definition.key] = definition
        if definition.key not in self._values:
            self._values[definition.key] = definition.default_value

    def get(self, key: str, default: Any = None) -> Any:
        return self._values.get(key, self._definitions.get(key, SettingDefinition(key, "", "", SettingType.STRING, default)).default_value)

    def set(self, key: str, value: Any) -> bool:
        if not self._validate(key, value):
            self.logger.warning(f"Invalid value for {key}: {value}")
            return False

        old_value = self._values.get(key)
        if old_value == value:
            return True

        self._values[key] = value
        self._save_settings()

        if key in self._listeners:
            for callback in self._listeners[key]:
                try:
                    callback(value, old_value)
                except Exception as e:
                    self.logger.error(f"Settings listener error for {key}: {e}")

        self.logger.info(f"Setting changed: {key} = {value}")
        return True

    def get_definition(self, key: str) -> Optional[SettingDefinition]:
        return self._definitions.get(key)

    def get_category(self, category: str) -> Dict[str, Any]:
        return {k: self._values[k] for k, d in self._definitions.items() if d.category == category and k in self._values}

    def get_all(self) -> Dict[str, Any]:
        return dict(self._values)

    def get_definitions(self) -> Dict[str, SettingDefinition]:
        return dict(self._definitions)

    def add_listener(self, key: str, callback: Callable[[Any, Any], None]):
        if key not in self._listeners:
            self._listeners[key] = []
        self._listeners[key].append(callback)

    def remove_listener(self, key: str, callback: Callable):
        if key in self._listeners and callback in self._listeners[key]:
            self._listeners[key].remove(callback)

    def reset(self, key: Optional[str] = None):
        if key:
            if key in self._definitions:
                self._values[key] = self._definitions[key].default_value
                self._save_settings()
        else:
            for key, d in self._definitions.items():
                self._values[key] = d.default_value
            self._save_settings()
        self.logger.info(f"Settings reset: {key or 'all'}")


def get_settings_manager() -> 'SettingsManager':
    return SettingsManager()