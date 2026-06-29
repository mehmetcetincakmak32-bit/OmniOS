/*
 * OmniOS API Translator
 * iOS <-> Android API translation for the compatibility layer
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
    const char* ios_class;
    const char* android_class;
} ClassMapping;

static const ClassMapping _ios_to_android[] = {
    {"UIView",              "android.view.View"},
    {"UIViewController",    "android.app.Activity"},
    {"UITableView",         "androidx.recyclerview.widget.RecyclerView"},
    {"UICollectionView",    "androidx.recyclerview.widget.RecyclerView"},
    {"UINavigationController", "androidx.navigation.NavController"},
    {"UITabBarController",  "com.google.android.material.bottomnavigation.BottomNavigationView"},
    {"UIAlertController",   "android.app.AlertDialog"},
    {"UIScrollView",        "android.widget.ScrollView"},
    {"WKWebView",           "android.webkit.WebView"},
    {"UILabel",             "android.widget.TextView"},
    {"UIButton",            "android.widget.Button"},
    {"UIImageView",         "android.widget.ImageView"},
    {"UITextField",         "android.widget.EditText"},
    {"UISwitch",            "android.widget.Switch"},
    {"UISlider",            "android.widget.SeekBar"},
    {"UIProgressView",      "android.widget.ProgressBar"},
    {"UIActivityIndicatorView", "android.widget.ProgressBar"},
    {"UIStackView",         "android.widget.LinearLayout"},
    {"UITableViewCell",     "android.widget.FrameLayout"},
    {"UIViewController",    "androidx.fragment.app.Fragment"},
    {"CGRect",              "android.graphics.Rect"},
    {"CGPoint",             "android.graphics.Point"},
    {"CGSize",              "android.util.Size"},
    {"UIColor",             "android.graphics.Color"},
    {"UIFont",              "android.graphics.Typeface"},
    {"UIImage",             "android.graphics.Bitmap"},
    {"NSData",              "byte[]"},
    {"NSString",            "java.lang.String"},
    {"NSArray",             "java.util.ArrayList"},
    {"NSDictionary",        "java.util.HashMap"},
    {"NSDate",              "java.util.Date"},
    {"NSURL",               "android.net.Uri"},
    {"NSBundle",            "android.content.res.Resources"},
    {"NSUserDefaults",      "android.content.SharedPreferences"},
    {"NSNotificationCenter", "androidx.localbroadcastmanager.content.LocalBroadcastManager"},
    {"UIApplication",       "android.app.Application"},
    {"UIScreen",            "android.util.DisplayMetrics"},
    {"UIDevice",            "android.os.Build"},
    {NULL, NULL}
};

static const ClassMapping _android_to_ios[100];
static int _reverse_count = 0;

static void _build_reverse_map(void) {
    if (_reverse_count > 0) return;
    for (int i = 0; _ios_to_android[i].ios_class != NULL; i++) {
        _android_to_ios[_reverse_count].ios_class = _ios_to_android[i].android_class;
        _android_to_ios[_reverse_count].android_class = _ios_to_android[i].ios_class;
        _reverse_count++;
    }
}

const char* om_api_ios_to_android(const char* ios_class) {
    if (!ios_class) return NULL;
    for (int i = 0; _ios_to_android[i].ios_class != NULL; i++) {
        if (strcmp(_ios_to_android[i].ios_class, ios_class) == 0)
            return _ios_to_android[i].android_class;
    }
    return NULL;
}

const char* om_api_android_to_ios(const char* android_class) {
    if (!android_class) return NULL;
    _build_reverse_map();
    for (int i = 0; i < _reverse_count; i++) {
        if (strcmp(_android_to_ios[i].ios_class, android_class) == 0)
            return _android_to_ios[i].android_class;
    }
    return NULL;
}

uint32_t om_api_get_translation_count(void) {
    uint32_t count = 0;
    for (int i = 0; _ios_to_android[i].ios_class != NULL; i++)
        count++;
    return count;
}

int om_api_translate_method(const char* platform, const char* class_name,
                            const char* method, char* out, uint32_t out_size) {
    if (!platform || !class_name || !method || !out || out_size < 64)
        return -1;

    if (strcmp(platform, "ios") == 0) {
        if (strcmp(class_name, "UIView") == 0 && strcmp(method, "addSubview:") == 0) {
            snprintf(out, out_size, "addView(android.view.View)");
            return 1;
        }
        if (strcmp(class_name, "UIViewController") == 0 &&
            strcmp(method, "presentViewController:animated:completion:") == 0) {
            snprintf(out, out_size, "startActivity(android.content.Intent)");
            return 1;
        }
        if (strcmp(class_name, "UITableView") == 0 && strcmp(method, "reloadData") == 0) {
            snprintf(out, out_size, "notifyDataSetChanged()");
            return 1;
        }
        snprintf(out, out_size, "/* %s::%s -> not translated */", class_name, method);
        return 0;
    }

    if (strcmp(platform, "android") == 0) {
        if (strcmp(class_name, "android.view.View") == 0 &&
            strcmp(method, "setOnClickListener") == 0) {
            snprintf(out, out_size, "addTarget:action:forControlEvents:");
            return 1;
        }
        snprintf(out, out_size, "/* %s::%s -> not translated */", class_name, method);
        return 0;
    }

    return -1;
}

const char* om_api_get_runtime(PlatformType platform) {
    switch (platform) {
        case PLATFORM_ANDROID:
            return "Android Runtime (ART) v4.0, API 35";
        case PLATFORM_IOS:
            return "iOS Runtime (UIKit Bridge) v3.2, API 18";
        case PLATFORM_CROSS:
            return "OmniOS Native Runtime v1.0";
        default:
            return "Unknown Runtime";
    }
}

int om_api_get_api_level(PlatformType platform) {
    switch (platform) {
        case PLATFORM_ANDROID: return 35;
        case PLATFORM_IOS:     return 18;
        case PLATFORM_CROSS:   return 1;
        default:               return 0;
    }
}

bool om_api_is_compatible(PlatformType app, PlatformType runtime) {
    if (app == PLATFORM_CROSS) return true;
    if (app == runtime) return true;
    return false;
}
