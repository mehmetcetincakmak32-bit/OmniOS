import tkinter as tk
from tkinter import ttk
from src.ui.widgets import AppIcon
from src.apps.app_manager import AppManager


class NormalMode(tk.Frame):
    def __init__(self, parent, controller):
        super().__init__(parent, bg="#1a1a2e")
        self.controller = controller
        self.app_manager = AppManager()
        self.current_platform = "all"

        self.header = tk.Frame(self, bg="#1a1a2e", height=40)
        self.header.pack(fill=tk.X, padx=10, pady=(8, 4))
        self.header.pack_propagate(False)

        tk.Label(self.header, text="Uygulamalar", font=("Segoe UI", 14, "bold"),
                 bg="#1a1a2e", fg="white").pack(side=tk.LEFT)

        self.platform_var = tk.StringVar(value="all")

        platform_menu = ttk.Combobox(
            self.header, textvariable=self.platform_var,
            values=["all", "android", "ios"], state="readonly",
            width=8, font=("Segoe UI", 8)
        )
        platform_menu.pack(side=tk.RIGHT)
        platform_menu.bind("<<ComboboxSelected>>", self.on_platform_change)

        canvas = tk.Canvas(self, bg="#1a1a2e", highlightthickness=0)
        scrollbar = tk.Scrollbar(self, orient="vertical", command=canvas.yview)
        self.scroll_frame = tk.Frame(canvas, bg="#1a1a2e")

        self.scroll_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=self.scroll_frame, anchor="nw", tags="inner")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True, padx=5)
        scrollbar.pack(side="right", fill="y")

        self.bind_mousewheel(canvas)

        self.render_apps()

    def bind_mousewheel(self, canvas):
        def on_mousewheel(event):
            canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
        canvas.bind_all("<MouseWheel>", on_mousewheel)

    def on_platform_change(self, event=None):
        self.current_platform = self.platform_var.get()
        self.render_apps()

    def render_apps(self):
        for w in self.scroll_frame.winfo_children():
            w.destroy()

        apps = self.app_manager.get_apps_by_platform(self.current_platform)

        if self.current_platform == "all":
            sections = [
                ("\U0001F310" + " Platformlar Arası", self.app_manager.CROSS_APPS),
                ("\U0001F4F1" + " Android", self.app_manager.ANDROID_APPS),
                ("\U0001F4F1" + " iOS", self.app_manager.IOS_APPS),
            ]
            for title, section_apps in sections:
                if section_apps:
                    section_label = tk.Label(
                        self.scroll_frame, text=title,
                        font=("Segoe UI", 9, "bold"), bg="#1a1a2e",
                        fg="#64ffda", anchor="w"
                    )
                    section_label.pack(fill=tk.X, padx=10, pady=(10, 2))

                    row_frame = tk.Frame(self.scroll_frame, bg="#1a1a2e")
                    row_frame.pack(fill=tk.X, padx=5)

                    for i, app in enumerate(section_apps):
                        if i > 0 and i % 4 == 0:
                            row_frame = tk.Frame(self.scroll_frame, bg="#1a1a2e")
                            row_frame.pack(fill=tk.X, padx=5)
                        icon = AppIcon(row_frame, app, on_launch=self.launch_app)
                        icon.pack(side=tk.LEFT, padx=4, pady=4)
        else:
            row_frame = tk.Frame(self.scroll_frame, bg="#1a1a2e")
            row_frame.pack(fill=tk.X, padx=5, pady=10)

            for i, app in enumerate(apps):
                if i > 0 and i % 4 == 0:
                    row_frame = tk.Frame(self.scroll_frame, bg="#1a1a2e")
                    row_frame.pack(fill=tk.X, padx=5)
                icon = AppIcon(row_frame, app, on_launch=self.launch_app)
                icon.pack(side=tk.LEFT, padx=4, pady=4)

    def launch_app(self, app):
        platform_names = {"android": "Android (ART)", "ios": "iOS (UIKit)", "cross": "OmniOS"}
        platform = platform_names.get(app.platform, "OmniOS")
        self.controller.show_app_screen(app.name, app.icon, platform)
