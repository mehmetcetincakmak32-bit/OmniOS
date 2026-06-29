"""Python-C Koprusu - OmniOS C kutuphanesine baglanti"""
import ctypes
import os
from typing import Optional


class CBridge:
    _instance = None
    _lib = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._load_library()
        return cls._instance

    def _load_library(self):
        lib_paths = [
            os.path.join(os.path.dirname(__file__), "..", "..", "core", "libomnios_core.so"),
            os.path.join(os.path.dirname(__file__), "..", "..", "core", "libomnios_core.dll"),
            os.path.join(os.path.dirname(__file__), "..", "..", "core", "libomnios_core.dylib"),
        ]
        for path in lib_paths:
            if os.path.exists(path):
                try:
                    self._lib = ctypes.CDLL(path)
                    self._loaded = True
                    return
                except Exception:
                    pass
        self._loaded = False

    @property
    def available(self) -> bool:
        return self._loaded

    def get_mode(self) -> str:
        if not self._loaded:
            return "unavailable"
        try:
            self._lib.om_mode_get_current.restype = ctypes.c_int
            mode = self._lib.om_mode_get_current()
            return "flow" if mode == 1 else "normal"
        except Exception:
            return "error"

    def toggle_mode(self) -> str:
        if not self._loaded:
            return "unavailable"
        try:
            self._lib.om_mode_toggle.restype = ctypes.c_int
            mode = self._lib.om_mode_toggle()
            return "flow" if mode == 1 else "normal"
        except Exception:
            return "error"

    def process_count(self) -> int:
        if not self._loaded:
            return -1
        try:
            self._lib.om_process_count.restype = ctypes.c_uint32
            return self._lib.om_process_count()
        except Exception:
            return -1
