import '../../models/app_item.dart';

class iOSRuntimeService {
  static const String name = 'iOS Runtime (UIKit Bridge)';
  static const String version = '3.2';
  static const int apiLevel = 18;
  static const bool available = true;

  static bool canRun(AppItem app) {
    return app.platform == AppPlatform.ios || app.platform == AppPlatform.cross;
  }

  static String launch(AppItem app) {
    return '[iOS UIKit] ${app.name} baslatiliyor... (API $apiLevel)';
  }

  static Map<String, dynamic> getInfo() {
    return {
      'name': name,
      'version': version,
      'apiLevel': apiLevel,
      'available': available,
    };
  }

  static List<String> getSupportedApis() {
    return [
      'UIKit',
      'SwiftUI',
      'Foundation',
      'CoreLocation',
      'NotificationCenter',
      'WebKit',
    ];
  }
}
