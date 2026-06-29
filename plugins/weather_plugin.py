"""OmniOS Weather Plugin - Ornek eklenti"""
import random
from src.core.plugin_system import OmniOSPlugin


class WeatherPlugin(OmniOSPlugin):
    name = "weather"
    version = "1.0.0"
    description = "Hava durumu bilgisi saglar"
    author = "OmniOS Team"

    def __init__(self, engine=None):
        super().__init__(engine)
        self._cache = {}
        self._locations = ["Istanbul", "Ankara", "Izmir", "London", "Tokyo", "New York"]

    def on_load(self):
        if self.engine:
            self.engine.on("boot", lambda d: self._on_boot(d))

    def on_enable(self):
        super().on_enable()
        print(f"[WeatherPlugin] Aktif - {len(self._locations)} konum hazir")

    def _on_boot(self, data):
        pass

    def get_weather(self, location: str = None) -> dict:
        if not location:
            location = random.choice(self._locations)

        if location in self._cache:
            return self._cache[location]

        weather = {
            "location": location,
            "temperature": round(random.uniform(-5, 40), 1),
            "condition": random.choice(["Gunesli", "Bulutlu", "Yagmurlu", "Karlı", "Ruzgarli"]),
            "humidity": random.randint(30, 95),
            "wind": round(random.uniform(0, 30), 1),
            "icon": random.choice(["☀️", "☁️", "🌧️", "❄️", "💨"]),
        }

        self._cache[location] = weather
        if len(self._cache) > 10:
            self._cache.pop(next(iter(self._cache)))

        return weather

    def get_forecast(self, location: str = None, days: int = 5) -> list:
        if not location:
            location = random.choice(self._locations)
        forecast = []
        for i in range(days):
            forecast.append({
                "day": i + 1,
                "location": location,
                "temperature": round(random.uniform(-5, 40), 1),
                "condition": random.choice(["Gunesli", "Bulutlu", "Yagmurlu", "Karlı", "Ruzgarli"]),
            })
        return forecast

    def get_supported_locations(self) -> list:
        return self._locations
