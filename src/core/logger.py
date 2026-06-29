"""OmniOS Logging System"""
from enum import Enum
from datetime import datetime
from typing import Callable, Optional
import sys


class LogLevel(Enum):
    DEBUG = 0
    INFO = 1
    WARN = 2
    ERROR = 3
    CRITICAL = 4


class Logger:
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
        self.level = LogLevel.INFO
        self.handlers = []
        self.format = "[{time}] {level}: {msg}"

    def set_level(self, level: LogLevel):
        self.level = level

    def add_handler(self, handler: Callable[[str], None]):
        self.handlers.append(handler)

    def _log(self, level: LogLevel, msg: str):
        if level.value < self.level.value:
            return
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        formatted = self.format.format(time=timestamp, level=level.name, msg=msg)
        for handler in self.handlers:
            handler(formatted)

    def debug(self, msg: str): self._log(LogLevel.DEBUG, msg)
    def info(self, msg: str): self._log(LogLevel.INFO, msg)
    def warn(self, msg: str): self._log(LogLevel.WARN, msg)
    def error(self, msg: str): self._log(LogLevel.ERROR, msg)
    def critical(self, msg: str): self._log(LogLevel.CRITICAL, msg)


def get_logger() -> Logger:
    return Logger()


def setup_logging(level: LogLevel = LogLevel.INFO):
    logger = get_logger()
    logger.set_level(level)
    logger.add_handler(lambda msg: print(msg, file=sys.stderr))
    return logger