"""API Cevirici - iOS ve Android API'leri arasinda kopru"""

class APITranslator:
    IOS_TO_ANDROID = {
        "UIView": "android.view.View",
        "UIViewController": "android.app.Activity",
        "UITableView": "androidx.recyclerview.widget.RecyclerView",
        "UICollectionView": "androidx.recyclerview.widget.RecyclerView",
        "UINavigationController": "androidx.navigation.NavController",
        "UITabBarController": "com.google.android.material.bottomnavigation.BottomNavigationView",
        "UIAlertController": "android.app.AlertDialog",
        "UIScrollView": "android.widget.ScrollView",
        "WKWebView": "android.webkit.WebView",
        "UILabel": "android.widget.TextView",
        "UIButton": "android.widget.Button",
        "UIImageView": "android.widget.ImageView",
        "UITextField": "android.widget.EditText",
        "UISwitch": "android.widget.Switch",
        "UISlider": "android.widget.SeekBar",
        "UIProgressView": "android.widget.ProgressBar",
        "UIActivityIndicator": "android.widget.ProgressBar",
        "UIStackView": "android.widget.LinearLayout",
        "CGRect": "android.graphics.Rect",
        "CGPoint": "android.graphics.Point",
        "CGSize": "android.util.Size",
        "UIColor": "android.graphics.Color",
        "UIFont": "android.graphics.Typeface",
        "UIImage": "android.graphics.Bitmap",
        "NSData": "byte[]",
        "NSString": "java.lang.String",
        "NSArray": "java.util.ArrayList",
        "NSDictionary": "java.util.HashMap",
        "NSDate": "java.util.Date",
        "NSURL": "android.net.Uri",
        "NSBundle": "android.content.res.Resources",
        "NSUserDefaults": "android.content.SharedPreferences",
        "NSNotificationCenter": "androidx.localbroadcastmanager.content.LocalBroadcastManager",
        "UIApplication": "android.app.Application",
        "UIScreen": "android.util.DisplayMetrics",
        "UIDevice": "android.os.Build",
    }

    ANDROID_TO_IOS = {v: k for k, v in IOS_TO_ANDROID.items()}

    def ios_to_android(self, ios_class: str) -> str:
        return self.IOS_TO_ANDROID.get(ios_class, f"/* {ios_class} -> esdegeri bulunamadi */")

    def android_to_ios(self, android_class: str) -> str:
        return self.ANDROID_TO_IOS.get(android_class, f"/* {android_class} -> esdegeri bulunamadi */")

    def translate_method(self, platform: str, class_name: str, method: str, args: list = None) -> str:
        if platform == "ios":
            return self._translate_ios_method(class_name, method, args)
        return self._translate_android_method(class_name, method, args)

    def _translate_ios_method(self, class_name: str, method: str, args: list = None) -> str:
        ios_methods = {
            "UIView.addSubview:": ("addView", ["android.view.View"]),
            "UIViewController.presentViewController:animated:completion:": ("startActivity", ["android.content.Intent"]),
            "UITableView.reloadData": ("notifyDataSetChanged", []),
            "UIScrollView.setContentOffset:animated:": ("scrollTo", ["int", "int"]),
            "UIApplication.sharedApplication.openURL:": ("startActivity", ["android.content.Intent"]),
        }
        key = f"{class_name}.{method}"
        if key in ios_methods:
            android_method, android_args = ios_methods[key]
            return f"{android_method}({', '.join(android_args)})"
        return f"/* {method} -> cevirilemedi */"

    def _translate_android_method(self, class_name: str, method: str, args: list = None) -> str:
        android_methods = {
            "android.view.View.setOnClickListener": ("addTarget:action:forControlEvents:", ["id", "SEL", "UIControlEvents"]),
            "android.app.Activity.startActivity": ("presentViewController:animated:completion:", ["UIViewController", "BOOL", "void"]),
        }
        key = f"{class_name}.{method}"
        if key in android_methods:
            ios_method, ios_args = android_methods[key]
            return f"{ios_method}({', '.join(ios_args)})"
        return f"/* {method} -> cevirilemedi */"

    def get_translation_table(self) -> dict:
        return {
            "ios_to_android_count": len(self.IOS_TO_ANDROID),
            "android_to_ios_count": len(self.ANDROID_TO_IOS),
            "table": self.IOS_TO_ANDROID,
        }
