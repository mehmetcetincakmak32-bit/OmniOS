import '../../models/app_item.dart';
import 'android_runtime.dart';
import 'ios_runtime.dart';

class RuntimeManager {
  static String launch(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return AndroidRuntimeService.launch(app);
      case AppPlatform.ios:
        return iOSRuntimeService.launch(app);
      case AppPlatform.cross:
        return '[OmniOS Native] ${app.name} baslatiliyor...';
    }
  }

  static bool canRun(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return AndroidRuntimeService.canRun(app);
      case AppPlatform.ios:
        return iOSRuntimeService.canRun(app);
      case AppPlatform.cross:
        return true;
    }
  }

  static Map<String, dynamic> getRuntimeInfo(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return AndroidRuntimeService.getInfo();
      case AppPlatform.ios:
        return iOSRuntimeService.getInfo();
      case AppPlatform.cross:
        return {
          'name': 'OmniOS Native Runtime',
          'version': '1.0',
          'apiLevel': 1,
          'available': true,
        };
    }
  }

  static String getPlatformApiLevel(AppItem app) {
    switch (app.platform) {
      case AppPlatform.android:
        return 'API ${AndroidRuntimeService.apiLevel}';
      case AppPlatform.ios:
        return 'API ${iOSRuntimeService.apiLevel}';
      case AppPlatform.cross:
        return 'Native';
    }
  }
}
