"""OmniOS App Recommender Plugin - Yapay zeka oneri sistemi"""
import random
from collections import Counter
from src.core.plugin_system import OmniOSPlugin


class AppRecommenderPlugin(OmniOSPlugin):
    name = "app_recommender"
    version = "1.0.0"
    description = "Kullanim aliskanligina gore uygulama onerir"
    author = "OmniOS Team"

    def __init__(self, engine=None):
        super().__init__(engine)
        self._usage_log = []
        self._categories = {}

    def on_load(self):
        if self.engine:
            self.engine.on("launch", lambda d: self._log_launch(d))

    def _log_launch(self, app_name: str):
        self._usage_log.append(app_name)
        if len(self._usage_log) > 1000:
            self._usage_log.pop(0)

    def get_recommendations(self, count: int = 4) -> list:
        if not self._usage_log:
            return self._default_recommendations(count)

        counter = Counter(self._usage_log)
        most_used = [app for app, _ in counter.most_common(count)]
        return most_used

    def get_frequent_apps(self, limit: int = 10) -> list:
        if not self._usage_log:
            return []
        counter = Counter(self._usage_log)
        return [{"name": name, "count": count} for name, count in counter.most_common(limit)]

    def get_category_stats(self) -> dict:
        if not self._usage_log:
            return {}

        if self.engine:
            all_apps = self.engine.app_manager.get_all_apps()
            for app in all_apps:
                launch_count = self._usage_log.count(app.name)
                if launch_count > 0:
                    cat = app.category
                    if cat not in self._categories:
                        self._categories[cat] = 0
                    self._categories[cat] += launch_count

        return dict(sorted(self._categories.items(), key=lambda x: x[1], reverse=True))

    def predict_next_launch(self) -> str:
        if not self._usage_log:
            return "Phone"

        last = self._usage_log[-3:]
        transitions = []
        for i in range(len(self._usage_log) - 1):
            if self._usage_log[i] in last:
                transitions.append(self._usage_log[i + 1])

        if transitions:
            return Counter(transitions).most_common(1)[0][0]
        return self._usage_log[-1]

    def _default_recommendations(self, count: int) -> list:
        defaults = ["Phone", "Messages", "Weather", "Chrome", "Settings",
                     "Camera", "Music", "Calendar"]
        random.shuffle(defaults)
        return defaults[:count]
