import '../models/app_item.dart';

class CompatibilityService {
  static const Map<String, String> _androidApiNames = {
    'Chrome': 'org.chromium.chrome',
    'Maps': 'com.google.android.apps.maps',
    'Gmail': 'com.google.android.gm',
    'YouTube': 'com.google.android.youtube',
    'Drive': 'com.google.android.apps.docs',
    'Photos': 'com.google.android.apps.photos',
  };

  static const Map<String, String> _iosApiNames = {
    'Safari': 'com.apple.mobilesafari',
    'Messages': 'com.apple.MobileSMS',
    'Camera': 'com.apple.camera',
    'Notes': 'com.apple.mobilenotes',
    'Music': 'com.apple.mobilemusic',
    'Wallet': 'com.apple.passbook',
  };

  static String getBundleId(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return _androidApiNames[app.name] ?? 'unknown.android';
      case AppPlatform.ios:
        return _iosApiNames[app.name] ?? 'unknown.ios';
      case AppPlatform.cross:
        return 'com.omnios.${app.name.toLowerCase()}';
    }
  }

  static bool isFullyCompatible(AppItem app) {
    switch (app.platform) {
      case AppPlatform.cross:
        return true;
      case AppPlatform.android:
        return _androidApiNames.containsKey(app.name);
      case AppPlatform.ios:
        return _iosApiNames.containsKey(app.name);
    }
  }

  static String getRuntimeInfo(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return 'ART Emulator v4.0 • API 35';
      case AppPlatform.ios:
        return 'UIKit Bridge v3.2 • API 18';
      case AppPlatform.cross:
        return 'OmniOS Runtime • Native';
    }
  }
}
