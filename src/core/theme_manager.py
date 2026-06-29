"""OmniOS Theme Manager"""
from enum import Enum
from dataclasses import dataclass, field
from typing import Dict, Optional, List
from .logger import get_logger
from .settings_manager import get_settings_manager


class ThemeMode(Enum):
    LIGHT = "light"
    DARK = "dark"
    SYSTEM = "system"


@dataclass
class ColorPalette:
    primary: str = "#64FFDA"
    primary_variant: str = "#00BFA5"
    secondary: str = "#FF6B6B"
    secondary_variant: str = "#FF3B3B"
    background: str = "#0A0A23"
    surface: str = "#1A1A2E"
    surface_variant: str = "#16213E"
    on_primary: str = "#000000"
    on_secondary: str = "#FFFFFF"
    on_background: str = "#FFFFFF"
    on_surface: str = "#FFFFFF"
    error: str = "#CF6679"
    on_error: str = "#FFFFFF"
    outline: str = "#333355"
    shadow: str = "#000000"
    scrim: str = "#000000"
    inverse_surface: str = "#F5F5F5"
    inverse_on_surface: str = "#0A0A23"
    inverse_primary: str = "#006D5B"


@dataclass
class LightColorPalette:
    primary: str = "#006D5B"
    primary_variant: str = "#004D3A"
    secondary: str = "#C41E3A"
    secondary_variant: str = "#931024"
    background: str = "#FEF7FF"
    surface: str = "#FFFFFF"
    surface_variant: str = "#E7E0EC"
    on_primary: str = "#FFFFFF"
    on_secondary: str = "#FFFFFF"
    on_background: str = "#1D1B20"
    on_surface: str = "#1D1B20"
    error: str = "#BA1A1A"
    on_error: str = "#FFFFFF"
    outline: str = "#79747E"
    shadow: str = "#000000"
    scrim: str = "#000000"
    inverse_surface: str = "#313033"
    inverse_on_surface: str = "#F5F5F5"
    inverse_primary: str = "#64FFDA"


@dataclass
class Theme:
    name: str
    mode: ThemeMode
    colors: ColorPalette
    font_family: str = "Roboto"
    font_scale: float = 1.0
    corner_radius: float = 8.0
    elevation_levels: List[float] = field(default_factory=lambda: [0, 1, 3, 6, 12, 24])
    animation_duration: int = 200
    icon_set: str = "material"


class ThemeManager:
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
        self.settings = get_settings_manager()

        self._dark_theme = Theme(
            name="OmniOS Dark",
            mode=ThemeMode.DARK,
            colors=ColorPalette()
        )
        self._light_theme = Theme(
            name="OmniOS Light",
            mode=ThemeMode.LIGHT,
            colors=LightColorPalette()
        )
        self._custom_themes: Dict[str, Theme] = {}
        self._current_mode = ThemeMode.DARK

        self._register_settings()

    def _register_settings(self):
        self.settings.add_listener("system.dark_mode", self._on_dark_mode_changed)
        self.settings.add_listener("system.theme_color", self._on_theme_color_changed)

    def _on_dark_mode_changed(self, new_value: bool, old_value: bool):
        self._current_mode = ThemeMode.DARK if new_value else ThemeMode.LIGHT
        self.logger.info(f"Theme mode changed: {'Dark' if new_value else 'Light'}")

    def _on_theme_color_changed(self, new_value: str, old_value: str):
        if self._current_mode == ThemeMode.DARK:
            self._dark_theme.colors.primary = new_value
            self._dark_theme.colors.primary_variant = self._darken_color(new_value)
        else:
            self._light_theme.colors.primary = new_value
            self._light_theme.colors.primary_variant = self._darken_color(new_value)
        self.logger.info(f"Theme color changed: {new_value}")

    def _darken_color(self, hex_color: str) -> str:
        if not hex_color.startswith('#'):
            return hex_color
        r = int(hex_color[1:3], 16)
        g = int(hex_color[3:5], 16)
        b = int(hex_color[5:7], 16)
        r = max(0, int(r * 0.7))
        g = max(0, int(g * 0.7))
        b = max(0, int(b * 0.7))
        return f"#{r:02x}{g:02x}{b:02x}"

    def get_current_theme(self) -> Theme:
        if self._current_mode == ThemeMode.LIGHT:
            return self._light_theme
        return self._dark_theme

    def get_mode(self) -> ThemeMode:
        return self._current_mode

    def set_mode(self, mode: ThemeMode):
        self._current_mode = mode
        self.logger.info(f"Theme mode set to: {mode.value}")

    def toggle_mode(self) -> ThemeMode:
        self._current_mode = ThemeMode.LIGHT if self._current_mode == ThemeMode.DARK else ThemeMode.DARK
        self.settings.set("system.dark_mode", self._current_mode == ThemeMode.DARK)
        return self._current_mode

    def get_color(self, color_name: str) -> str:
        theme = self.get_current_theme()
        return getattr(theme.colors, color_name, theme.colors.primary)

    def register_theme(self, name: str, theme: Theme):
        self._custom_themes[name] = theme
        self.logger.info(f"Custom theme registered: {name}")

    def get_custom_theme(self, name: str) -> Optional[Theme]:
        return self._custom_themes.get(name)

    def list_themes(self) -> List[str]:
        return list(self._custom_themes.keys()) + ["OmniOS Dark", "OmniOS Light"]

    def apply_theme_to_widget(self, widget, theme: Optional[Theme] = None):
        t = theme or self.get_current_theme()
        if hasattr(widget, 'configure'):
            widget.configure(
                bg=t.colors.background,
                fg=t.colors.on_background,
                font=(t.font_family, int(10 * t.font_scale))
            )


def get_theme_manager() -> 'ThemeManager':
    return ThemeManager()