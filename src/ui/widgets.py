import tkinter as tk


class AppIcon(tk.Frame):
    def __init__(self, parent, app, on_launch=None):
        super().__init__(parent, bg="#1a1a2e", width=70, height=90)
        self.app = app
        self.on_launch = on_launch
        self.pack_propagate(False)

        icon_frame = tk.Frame(self, bg="#16213e", width=56, height=56)
        icon_frame.pack(pady=(4, 2))
        icon_frame.pack_propagate(False)

        platform_colors = {"android": "#3ddc84", "ios": "#007aff", "cross": "#ff6b6b"}
        color = platform_colors.get(app.platform, "#ff6b6b")

        icon_label = tk.Label(
            icon_frame, text=app.icon, font=("Segoe UI", 20), bg="#16213e", fg=color
        )
        icon_label.pack(expand=True)

        color_dot = tk.Label(self, text="●", font=("Segoe UI", 6), bg="#1a1a2e", fg=color)
        color_dot.pack()

        name_label = tk.Label(
            self,
            text=app.name,
            font=("Segoe UI", 7),
            bg="#1a1a2e",
            fg="#e0e0e0",
            wraplength=68,
        )
        name_label.pack()

        if on_launch:
            for w in (icon_frame, icon_label, name_label, color_dot, self):
                w.bind("<Button-1>", lambda e, a=app: on_launch(a))


class NotificationBar(tk.Frame):
    def __init__(self, parent, **kwargs):
        super().__init__(parent, height=30, bg="#0f3460", **kwargs)
        self.pack_propagate(False)

        self.time_label = tk.Label(
            self, text="12:00", font=("Segoe UI", 10, "bold"), bg="#0f3460", fg="white"
        )
        self.time_label.pack(side=tk.LEFT, padx=12)

        platform_label = tk.Label(
            self, text="OmniOS", font=("Segoe UI", 8), bg="#0f3460", fg="#64ffda"
        )
        platform_label.pack(side=tk.LEFT, padx=4)

        status_frame = tk.Frame(self, bg="#0f3460")
        status_frame.pack(side=tk.RIGHT, padx=8)

        tk.Label(status_frame, text="\u25CF", fg="#4caf50", bg="#0f3460",
                 font=("Segoe UI", 8)).pack(side=tk.LEFT, padx=2)
        tk.Label(status_frame, text="\u2601", fg="white", bg="#0f3460",
                 font=("Segoe UI", 8)).pack(side=tk.LEFT, padx=2)
        tk.Label(status_frame, text="\u26A1", fg="#ffeb3b", bg="#0f3460",
                 font=("Segoe UI", 8)).pack(side=tk.LEFT, padx=2)

        self.update_time()

    def update_time(self):
        from datetime import datetime
        now = datetime.now()
        self.time_label.config(text=now.strftime("%H:%M"))
        self.after(30000, self.update_time)


class NavigationBar(tk.Frame):
    def __init__(self, parent, on_mode_switch=None, current_mode="normal", **kwargs):
        super().__init__(parent, height=50, bg="#0a0a23", **kwargs)
        self.pack_propagate(False)
        self.current_mode = current_mode

        self.home_btn = tk.Label(
            self, text="○", font=("Segoe UI", 18), bg="#0a0a23", fg="white", cursor="hand2"
        )
        self.home_btn.pack(side=tk.LEFT, padx=30)

        tk.Label(self, text="●", font=("Segoe UI", 8), bg="#0a0a23", fg="#333").pack(side=tk.LEFT, padx=10)

        mode_text = "◉ Flow" if current_mode == "normal" else "◉ Normal"
        self.mode_btn = tk.Label(
            self, text=mode_text, font=("Segoe UI", 9), bg="#0a0a23", fg="#64ffda", cursor="hand2"
        )
        self.mode_btn.pack(side=tk.LEFT, padx=10)

        tk.Label(self, text="●", font=("Segoe UI", 8), bg="#0a0a23", fg="#333").pack(side=tk.LEFT, padx=10)

        self.back_btn = tk.Label(
            self, text="◁", font=("Segoe UI", 16), bg="#0a0a23", fg="white", cursor="hand2"
        )
        self.back_btn.pack(side=tk.LEFT, padx=30)

        if on_mode_switch:
            self.mode_btn.bind("<Button-1>", lambda e: on_mode_switch())
