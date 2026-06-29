enum AppPlatform { android, ios, cross }

class AppItem {
  final String name;
  final String emoji;
  final String category;
  final AppPlatform platform;

  const AppItem({
    required this.name,
    required this.emoji,
    required this.category,
    required this.platform,
  });

  String get platformLabel {
    switch (platform) {
      case AppPlatform.android:
        return 'Android (ART)';
      case AppPlatform.ios:
        return 'iOS (UIKit)';
      case AppPlatform.cross:
        return 'OmniOS Native';
    }
  }

  String get platformColor {
    switch (platform) {
      case AppPlatform.android:
        return '#3DDC84';
      case AppPlatform.ios:
        return '#007AFF';
      case AppPlatform.cross:
        return '#FF6B6B';
    }
  }

  String get apiLevel {
    switch (platform) {
      case AppPlatform.android:
        return 'API 35';
      case AppPlatform.ios:
        return 'API 18';
      case AppPlatform.cross:
        return 'Native';
    }
  }
}
