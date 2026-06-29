import 'app_item.dart';

class AppData {
  static const List<AppItem> crossApps = [
    AppItem(name: 'Phone', emoji: '📞', category: 'communication', platform: AppPlatform.cross),
    AppItem(name: 'Settings', emoji: '⚙️', category: 'system', platform: AppPlatform.cross),
    AppItem(name: 'Calculator', emoji: '🔢', category: 'tools', platform: AppPlatform.cross),
    AppItem(name: 'Calendar', emoji: '📅', category: 'productivity', platform: AppPlatform.cross),
    AppItem(name: 'Clock', emoji: '⏰', category: 'tools', platform: AppPlatform.cross),
    AppItem(name: 'Weather', emoji: '🌤️', category: 'tools', platform: AppPlatform.cross),
  ];

  static const List<AppItem> androidApps = [
    AppItem(name: 'Chrome', emoji: '🌐', category: 'internet', platform: AppPlatform.android),
    AppItem(name: 'Maps', emoji: '🗺️', category: 'navigation', platform: AppPlatform.android),
    AppItem(name: 'Gmail', emoji: '✉️', category: 'communication', platform: AppPlatform.android),
    AppItem(name: 'YouTube', emoji: '▶️', category: 'media', platform: AppPlatform.android),
    AppItem(name: 'Drive', emoji: '📁', category: 'productivity', platform: AppPlatform.android),
    AppItem(name: 'Photos', emoji: '📸', category: 'media', platform: AppPlatform.android),
  ];

  static const List<AppItem> iosApps = [
    AppItem(name: 'Safari', emoji: '🌐', category: 'internet', platform: AppPlatform.ios),
    AppItem(name: 'Messages', emoji: '💬', category: 'communication', platform: AppPlatform.ios),
    AppItem(name: 'Camera', emoji: '📷', category: 'media', platform: AppPlatform.ios),
    AppItem(name: 'Notes', emoji: '📝', category: 'productivity', platform: AppPlatform.ios),
    AppItem(name: 'Music', emoji: '🎵', category: 'media', platform: AppPlatform.ios),
    AppItem(name: 'Wallet', emoji: '💳', category: 'finance', platform: AppPlatform.ios),
  ];

  static List<AppItem> getAll() => [...crossApps, ...androidApps, ...iosApps];

  static List<AppItem> getByPlatform(AppPlatform platform) {
    if (platform == AppPlatform.cross) return crossApps;
    final base = [...crossApps];
    switch (platform) {
      case AppPlatform.android:
        base.addAll(androidApps);
      case AppPlatform.ios:
        base.addAll(iosApps);
      default:
        break;
    }
    return base;
  }
}
