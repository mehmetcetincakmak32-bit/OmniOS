import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import tkinter as tk
from src.ui.widgets import NotificationBar, NavigationBar
from src.ui.phone_canvas import PhoneCanvas
from src.modes.normal_mode import NormalMode
from src.modes.flow_mode import FlowMode
from src.compatibility.android_layer import AndroidCompatibilityLayer
from src.compatibility.ios_layer import iOSCompatibilityLayer


class OmniOSController(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("OmniOS Prototype")
        self.configure(bg="#0a0a23")
        self.resizable(False, False)

        window_width = 400
        window_height = 720
        screen_w = self.winfo_screenwidth()
        screen_h = self.winfo_screenheight()
        x = (screen_w - window_width) // 2
        y = (screen_h - window_height) // 2
        self.geometry(f"{window_width}x{window_height}+{x}+{y}")

        self.android_layer = AndroidCompatibilityLayer()
        self.ios_layer = iOSCompatibilityLayer()
        self.current_mode = "normal"

        self.notification_bar = NotificationBar(self)
        self.notification_bar.pack(fill=tk.X)

        self.phone = PhoneCanvas(self, self)
        self.phone.pack(fill=tk.BOTH, expand=True)

        self.nav_bar = NavigationBar(self, on_mode_switch=self.toggle_mode, current_mode=self.current_mode)
        self.nav_bar.pack(fill=tk.X)

        self.modes = {}
        self.show_mode("normal")

        self.bind("<Escape>", lambda e: self.destroy())

    def show_mode(self, mode):
        if mode not in self.modes:
            if mode == "normal":
                self.modes[mode] = NormalMode(self.phone.display, self)
            else:
                self.modes[mode] = FlowMode(self.phone.display, self)
        self.current_mode = mode
        self.phone.show_frame(self.modes[mode])

        nav_text = "\u25C9 Normal" if mode == "flow" else "\u25C9 Flow"
        self.nav_bar.mode_btn.config(text=nav_text)

    def toggle_mode(self):
        new_mode = "flow" if self.current_mode == "normal" else "normal"
        self.show_mode(new_mode)

    def show_app_screen(self, app_name, app_icon, platform):
        app_frame = tk.Frame(self.phone.display, bg="#1a1a2e")

        tk.Label(app_frame, text=app_icon, font=("Segoe UI", 48),
                 bg="#1a1a2e", fg="white").pack(pady=(60, 10))

        tk.Label(app_frame, text=app_name, font=("Segoe UI", 22, "bold"),
                 bg="#1a1a2e", fg="white").pack(pady=5)

        tk.Label(app_frame, text=f"Platform: {platform}",
                 font=("Segoe UI", 11), bg="#1a1a2e", fg="#64ffda").pack(pady=2)

        platform_colors = {"android": "#3ddc84", "ios": "#007aff", "OmniOS": "#ff6b6b"}
        color = platform_colors.get(platform, "#64ffda")

        status_frame = tk.Frame(app_frame, bg="#1a1a2e")
        status_frame.pack(pady=20)

        tk.Label(status_frame, text="\u2705", font=("Segoe UI", 14),
                 bg="#1a1a2e", fg="#4caf50").pack(side=tk.LEFT, padx=5)
        tk.Label(status_frame, text=f"{platform} uyumluluk katmanında çalışıyor",
                 font=("Segoe UI", 9), bg="#1a1a2e", fg="#ccc").pack(side=tk.LEFT)

        info_frame = tk.Frame(app_frame, bg="#0a0a23", padx=15, pady=10,
                              highlightthickness=1, highlightbackground="#333355")
        info_frame.pack(pady=10, padx=30, fill=tk.X)

        tk.Label(info_frame, text="Uygulama Bilgisi",
                 font=("Segoe UI", 10, "bold"), bg="#0a0a23", fg="#64ffda").pack()

        details = [
            f"\U0001F4CC Ad: {app_name}",
            f"\U0001F310 Platform: {platform}",
            f"\u26A1 Durum: Aktif",
            f"\U0001F504 API: {35 if platform == 'Android' else 18 if platform == 'iOS' else 'Native'}",
        ]
        for detail in details:
            tk.Label(info_frame, text=detail, font=("Segoe UI", 8),
                     bg="#0a0a23", fg="#aaa", anchor="w").pack(fill=tk.X, pady=1)

        close_btn = tk.Button(
            app_frame, text="\u2716 Ana Ekrana Dön",
            font=("Segoe UI", 10), bg="#16213e", fg="white",
            activebackground="#0f3460", bd=0, padx=15, pady=5,
            cursor="hand2",
            command=lambda: self.phone.show_frame(self.modes[self.current_mode]),
        )
        close_btn.pack(pady=20)

        self.phone.show_frame(app_frame)


if __name__ == "__main__":
    app = OmniOSController()
    app.mainloop()
