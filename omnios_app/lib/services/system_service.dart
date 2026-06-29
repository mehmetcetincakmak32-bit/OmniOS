import '../models/app_item.dart';
import '../models/app_data.dart';
import 'runtime/runtime_manager.dart';

class SystemInfo {
  final String state;
  final String mode;
  final int activeProcesses;
  final int totalApps;
  final bool androidReady;
  final bool iosReady;
  final int uptime;

  const SystemInfo({
    required this.state,
    required this.mode,
    required this.activeProcesses,
    required this.totalApps,
    required this.androidReady,
    required this.iosReady,
    required this.uptime,
  });

  Map<String, dynamic> toJson() => {
    'state': state,
    'mode': mode,
    'activeProcesses': activeProcesses,
    'totalApps': totalApps,
    'androidReady': androidReady,
    'iosReady': iosReady,
    'uptime': uptime,
  };
}

class SystemService {
  static int _startTime = DateTime.now().millisecondsSinceEpoch;
  static final List<AppItem> _runningApps = [];

  static SystemInfo getInfo() {
    return SystemInfo(
      state: _runningApps.isEmpty ? 'IDLE' : 'RUNNING',
      mode: 'normal',
      activeProcesses: _runningApps.length,
      totalApps: AppData.getAll().length,
      androidReady: true,
      iosReady: true,
      uptime: (DateTime.now().millisecondsSinceEpoch - _startTime) ~/ 1000,
    );
  }

  static bool launch(AppItem app) {
    if (_runningApps.length >= 50) return false;
    _runningApps.add(app);
    RuntimeManager.launch(app);
    return true;
  }

  static bool kill(AppItem app) {
    return _runningApps.remove(app);
  }

  static List<AppItem> get runningApps => List.unmodifiable(_runningApps);

  static int get processCount => _runningApps.length;

  static String get uptime {
    final secs = (DateTime.now().millisecondsSinceEpoch - _startTime) ~/ 1000;
    final h = secs ~/ 3600;
    final m = (secs % 3600) ~/ 60;
    final s = secs % 60;
    return '${h}h ${m}m ${s}s';
  }
}
