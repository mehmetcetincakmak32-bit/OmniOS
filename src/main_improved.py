"""OmniOS - Gelismis CLI Arayuzu"""
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from src.core.engine import OmniOSEngine
from src.core.gesture_engine import Gesture


class OmniOSCLI:
    def __init__(self):
        self.engine = OmniOSEngine()
        self.running = True

    def run(self):
        print("\n" + "="*50)
        print("  O m n i O S   v 1 . 0")
        print("  Tek OS, Tum Platformlar")
        print("="*50)

        while self.running:
            try:
                self._show_prompt()
            except KeyboardInterrupt:
                print("\n")
                self._cmd_shutdown()
                break

    def _show_prompt(self):
        mode = self.engine.mode_manager.current_mode.value.upper()
        state = self.engine.state.name
        proc_count = len(self.engine.process_manager.active_processes)
        prompt = f"omnios[{mode}][{state}]({proc_count}ps)> "

        cmd = input(prompt).strip().lower()
        if not cmd:
            return

        parts = cmd.split(maxsplit=1)
        command = parts[0]
        args = parts[1] if len(parts) > 1 else ""

        self._dispatch(command, args)

    def _dispatch(self, cmd: str, args: str):
        commands = {
            "help": self._cmd_help,
            "h": self._cmd_help,
            "launch": self._cmd_launch,
            "l": self._cmd_launch,
            "kill": self._cmd_kill,
            "k": self._cmd_kill,
            "mode": self._cmd_mode,
            "m": self._cmd_mode,
            "gesture": self._cmd_gesture,
            "g": self._cmd_gesture,
            "ps": self._cmd_ps,
            "processes": self._cmd_ps,
            "info": self._cmd_info,
            "i": self._cmd_info,
            "apps": self._cmd_apps,
            "a": self._cmd_apps,
            "clear": self._cmd_clear,
            "cls": self._cmd_clear,
            "shutdown": self._cmd_shutdown,
            "exit": self._cmd_shutdown,
            "quit": self._cmd_shutdown,
        }

        handler = commands.get(cmd)
        if handler:
            handler(args)
        else:
            print(f"  Bilinmeyen komut: {cmd}. 'help' yazin.")

    def _cmd_help(self, args=""):
        print("""
  KOMUTLAR:
    launch <ad>     - Uygulama baslat (l)
    kill <ad/pid>   - Uygulama durdur (k)
    mode            - Mod degistir (m)
    gesture <jest>  - Jest gonder (g)
    ps              - Process listesi
    info            - Sistem bilgisi (i)
    apps            - Uygulama listesi (a)
    clear/cls       - Ekrani temizle
    shutdown/exit   - Cikis

  Flow Mod Jestleri:
    swipe_up, swipe_down, swipe_left, swipe_right
    double_tap, long_press, pinch_out

  ORNEK:
    launch Chrome
    mode
    gesture swipe_up
    ps
""")

    def _cmd_launch(self, args=""):
        if not args:
            print("  Kullanim: launch <uygulama_adi>")
            return
        if self.engine.launch_app(args):
            print(f"  ✓ {args} baslatildi")
        else:
            print(f"  ✗ {args} baslatilamadi")

    def _cmd_kill(self, args=""):
        if not args:
            print("  Kullanim: kill <uygulama_adi>")
            return
        if self.engine.kill_app(args):
            print(f"  ✓ {args} durduruldu")
        else:
            print(f"  ✗ {args} bulunamadi")

    def _cmd_mode(self, args=""):
        new_mode = self.engine.toggle_mode()
        print(f"  ✓ Mod: {new_mode}")

    def _cmd_gesture(self, args=""):
        if not args:
            print("  Kullanim: gesture <jest_adi>")
            return
        action = self.engine.process_gesture(args)
        print(f"  ✓ Jest '{args}' -> {action}")

    def _cmd_ps(self, args=""):
        procs = self.engine.process_manager.active_processes
        if not procs:
            print("  Aktif process yok")
            return
        print(f"  Aktif process: {len(procs)}")
        print(f"  {'PID':<10} {'Isim':<20} {'Platform':<15} {'Durum':<12} {'Calisma':<8}")
        print(f"  {'-'*65}")
        for p in procs:
            info = p.info()
            print(f"  {info['pid']:<10} {info['name']:<20} {info['platform']:<15} "
                  f"{info['state']:<12} {info['uptime']:<8}s")

    def _cmd_info(self, args=""):
        info = self.engine.get_system_info()
        print(f"""
  SISTEM BILGISI:
    Durum:    {info['state']}
    Mod:      {info['mode']}
    Process:  {info['active_processes']} aktif
    Uygulama: {info['total_apps']} toplam
    Android:  {info['android_runtime']}
    iOS:      {info['ios_runtime']}
    Jestler:  {info['gestures_supported']} desteklenen
""")

    def _cmd_apps(self, args=""):
        apps = self.engine.app_manager.get_all_apps()
        print(f"\n  Platformlar Arasi ({len([a for a in apps if a.platform=='cross'])})")
        for a in apps:
            if a.platform == "cross":
                print(f"    {a.icon} {a.name}")
        print(f"\n  Android ({len([a for a in apps if a.platform=='android'])})")
        for a in apps:
            if a.platform == "android":
                print(f"    {a.icon} {a.name}")
        print(f"\n  iOS ({len([a for a in apps if a.platform=='ios'])})")
        for a in apps:
            if a.platform == "ios":
                print(f"    {a.icon} {a.name}")
        print(f"\n  Toplam: {len(apps)} uygulama")

    def _cmd_clear(self, args=""):
        os.system("cls" if os.name == "nt" else "clear")

    def _cmd_shutdown(self, args=""):
        self.engine.shutdown()
        print("\n  OmniOS kapatildi. Gorusmek uzere!")
        self.running = False


if __name__ == "__main__":
    cli = OmniOSCLI()
    cli.run()
