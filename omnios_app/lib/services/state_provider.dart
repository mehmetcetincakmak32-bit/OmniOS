import 'package:flutter/foundation.dart';
import '../models/app_item.dart';
import '../models/app_data.dart';
import 'runtime/runtime_manager.dart';
import 'gesture_service.dart';
import 'system_service.dart';

enum UIMode { normal, flow }

enum SystemState { idle, running, sleeping, shutdown }

class OmniOSState extends ChangeNotifier {
  UIMode _currentMode = UIMode.normal;
  SystemState _systemState = SystemState.idle;
  final List<AppItem> _runningApps = [];
  final List<String> _eventLog = [];
  AppItem? _currentApp;
  String _currentScreen = 'home';

  UIMode get currentMode => _currentMode;
  SystemState get systemState => _systemState;
  List<AppItem> get runningApps => List.unmodifiable(_runningApps);
  AppItem? get currentApp => _currentApp;
  String get currentScreen => _currentScreen;
  bool get isFlowMode => _currentMode == UIMode.flow;
  int get processCount => _runningApps.length;

  void toggleMode() {
    _currentMode = _currentMode == UIMode.normal ? UIMode.flow : UIMode.normal;
    _logEvent('Mod degisti: ${_currentMode.name}');
    notifyListeners();
  }

  void setMode(UIMode mode) {
    if (_currentMode != mode) {
      _currentMode = mode;
      _logEvent('Mod: ${mode.name}');
      notifyListeners();
    }
  }

  bool launchApp(AppItem app) {
    if (_runningApps.length >= 50) return false;
    _runningApps.add(app);
    _systemState = SystemState.running;
    _currentApp = app;
    _currentScreen = 'detail';
    _logEvent('${RuntimeManager.launch(app)}');
    notifyListeners();
    return true;
  }

  void killApp(AppItem app) {
    _runningApps.remove(app);
    if (_runningApps.isEmpty) {
      _systemState = SystemState.idle;
    }
    notifyListeners();
  }

  void goHome() {
    _currentApp = null;
    _currentScreen = 'home';
    notifyListeners();
  }

  String processGesture(String gesture) {
    final action = GestureService.getAction(gesture, isFlow: isFlowMode);
    if (action == null) return 'unknown';
    _logEvent('Jest: $gesture -> ${action.action}');
    return action.action;
  }

  List<AppItem> getAppsByPlatform(String platform) {
    switch (platform) {
      case 'android':
        return AppData.getByPlatform(AppPlatform.android);
      case 'ios':
        return AppData.getByPlatform(AppPlatform.ios);
      default:
        return AppData.getAll();
    }
  }

  SystemInfo get systemInfo => SystemService.getInfo();

  void _logEvent(String event) {
    _eventLog.add(event);
    if (_eventLog.length > 100) _eventLog.removeAt(0);
  }

  List<String> getRecentEvents(int count) {
    return _eventLog.length <= count
        ? List.from(_eventLog)
        : _eventLog.sublist(_eventLog.length - count);
  }
}
