import '../../models/app_item.dart';

class AndroidRuntimeService {
  static const String name = 'Android Runtime (ART)';
  static const String version = '4.0';
  static const int apiLevel = 35;
  static const bool available = true;

  static bool canRun(AppItem app) {
    return app.platform == AppPlatform.android || app.platform == AppPlatform.cross;
  }

  static String launch(AppItem app) {
    return '[Android ART] ${app.name} baslatiliyor... (API $apiLevel)';
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
      'android.app.Activity',
      'android.content.Intent',
      'android.view.View',
      'android.widget.*',
      'androidx.*',
      'com.google.android.*',
    ];
  }
}
