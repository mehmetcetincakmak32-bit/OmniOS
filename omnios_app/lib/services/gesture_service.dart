class GestureAction {
  final String gesture;
  final String action;
  final String description;

  const GestureAction({
    required this.gesture,
    required this.action,
    required this.description,
  });
}

class GestureService {
  static const Map<String, Map<String, String>> _normalActions = {
    'swipe_up':    {'action': 'open_app_menu',       'desc': 'Uygulama menusu'},
    'swipe_right': {'action': 'open_recent_apps',    'desc': 'Son uygulamalar'},
    'swipe_left':  {'action': 'open_notifications',  'desc': 'Bildirimler'},
    'swipe_down':  {'action': 'open_quick_settings', 'desc': 'Hizli ayarlar'},
    'double_tap':  {'action': 'go_home',             'desc': 'Ana ekran'},
    'long_press':  {'action': 'voice_command',       'desc': 'Sesli komut'},
    'pinch_out':   {'action': 'app_selector',        'desc': 'Uygulama secici'},
  };

  static const Map<String, Map<String, String>> _flowActions = {
    'swipe_up':    {'action': 'open_app_menu',       'desc': 'Uygulama menusu'},
    'swipe_right': {'action': 'open_recent_apps',    'desc': 'Son uygulamalar'},
    'swipe_left':  {'action': 'open_notifications',  'desc': 'Bildirimler'},
    'swipe_down':  {'action': 'open_quick_settings', 'desc': 'Hizli ayarlar'},
    'double_tap':  {'action': 'go_home',             'desc': 'Ana ekran'},
    'long_press':  {'action': 'voice_command',       'desc': 'Sesli komut'},
    'pinch_out':   {'action': 'app_selector_flow',   'desc': 'Coverflow secici'},
  };

  static GestureAction? getAction(String gesture, {bool isFlow = false}) {
    final map = isFlow ? _flowActions : _normalActions;
    final entry = map[gesture];
    if (entry == null) return null;
    return GestureAction(
      gesture: gesture,
      action: entry['action']!,
      description: entry['desc']!,
    );
  }

  static List<GestureAction> getHints({bool isFlow = false}) {
    final map = isFlow ? _flowActions : _normalActions;
    return map.entries.map((e) => GestureAction(
      gesture: e.key,
      action: e.value['action']!,
      description: e.value['desc']!,
    )).toList();
  }

  static List<String> get supportedGestures =>
      _normalActions.keys.toList();
}
