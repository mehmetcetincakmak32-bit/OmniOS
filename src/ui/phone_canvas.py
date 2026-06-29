import tkinter as tk


class PhoneCanvas(tk.Frame):
    def __init__(self, parent, controller, **kwargs):
        super().__init__(parent, bg="#0a0a23", **kwargs)
        self.controller = controller

        outer = tk.Frame(self, bg="#1a1a2e", bd=0, highlightthickness=2,
                         highlightbackground="#333355")
        outer.pack(padx=8, pady=8, fill=tk.BOTH, expand=True)

        self.display = tk.Frame(outer, bg="#1a1a2e")
        self.display.pack(fill=tk.BOTH, expand=True, padx=2, pady=2)

        self.current_content = None

    def show_frame(self, frame):
        if self.current_content:
            self.current_content.pack_forget()
        self.current_content = frame
        frame.pack(fill=tk.BOTH, expand=True)
