import tkinter as tk
from datetime import datetime
from src.apps.app_manager import AppManager


class FlowMode(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent, bg="#050510")
        self.controller = controller
        self.app_manager = AppManager()
        self.gesture_area = None

        self.build_ui()

    def build_ui(self):
        main = tk.Frame(self, bg="#050510")
        main.pack(fill=tk.BOTH, expand=True)

        clock_frame = tk.Frame(main, bg="#050510")
        clock_frame.pack(expand=True)

        self.time_label = tk.Label(
            clock_frame,
            text="",
            font=("Segoe UI", 48, "light"),
            bg="#050510",
            fg="white",
        )
        self.time_label.pack(pady=(0, 0))

        self.date_label = tk.Label(
            clock_frame,
            text="",
            font=("Segoe UI", 12),
            bg="#050510",
            fg="#888899",
        )
        self.date_label.pack()
        self.update_clock()

        hint_frame = tk.Frame(main, bg="#050510")
        hint_frame.pack(pady=20)

        hints = [
            ("\u2B06 Yukarı çek", "Uygulama menüsü", "#ff6b6b"),
            ("\u27A1 Sağa kaydır", "Son uygulamalar", "#4ecdc4"),
            ("\u2B05 Sola kaydır", "Bildirimler", "#45b7d1"),
            ("\u2B07 Aşağı kaydır", "Hızlı ayarlar", "#96ceb4"),
            ("\u2757 Çift dokun", "Ana ekran", "#ffeaa7"),
        ]

        row = tk.Frame(hint_frame, bg="#050510")
        row.pack()
        for i, (gesture, desc, color) in enumerate(hints):
            if i > 0 and i % 3 == 0:
                row = tk.Frame(hint_frame, bg="#050510")
                row.pack()
            card = tk.Frame(row, bg="#0a0a23", bd=0, highlightthickness=1,
                           highlightbackground="#1a1a3e", padx=10, pady=8)
            card.pack(side=tk.LEFT, padx=5, pady=5)

            tk.Label(card, text=gesture, font=("Segoe UI", 11, "bold"),
                     bg="#0a0a23", fg=color).pack()
            tk.Label(card, text=desc, font=("Segoe UI", 8),
                     bg="#0a0a23", fg="#888").pack()

        suggested = self.app_manager.get_suggested_apps(4)
        if suggested:
            suggested_frame = tk.Frame(main, bg="#050510")
            suggested_frame.pack(pady=10)

            tk.Label(suggested_frame, text="\u26A1 Önerilen Uygulamalar",
                     font=("Segoe UI", 9, "bold"), bg="#050510", fg="#64ffda").pack(pady=(0, 6))

            row = tk.Frame(suggested_frame, bg="#050510")
            row.pack()
            for app in suggested:
                colors = {"android": "#3ddc84", "ios": "#007aff", "cross": "#ff6b6b"}
                badge = tk.Frame(row, bg="#0a0a23", padx=12, pady=8,
                                highlightthickness=1, highlightbackground="#1a1a3e")
                badge.pack(side=tk.LEFT, padx=4)

                tk.Label(badge, text=app.icon, font=("Segoe UI", 18),
                         bg="#0a0a23", fg=colors.get(app.platform, "#fff")).pack()
                tk.Label(badge, text=app.name, font=("Segoe UI", 7),
                         bg="#0a0a23", fg="#ccc").pack()
                badge.bind("<Button-1>", lambda e, a=app: self.controller.show_app_screen(
                    a.name, a.icon, {"android": "Android", "ios": "iOS", "cross": "OmniOS"}.get(a.platform, "OmniOS")))

        bottom_hint = tk.Label(
            main, text="\u2191 \u2192 \u2190 \u2193 \u2022 Jestlerle keşfet",
            font=("Segoe UI", 8), bg="#050510", fg="#555",
        )
        bottom_hint.pack(side=tk.BOTTOM, pady=10)

    def update_clock(self):
        now = datetime.now()
        self.time_label.config(text=now.strftime("%H:%M"))
        self.date_label.config(text=now.strftime("%A, %d %B %Y").capitalize())
        self.after(10000, self.update_clock)
