/*
 * OmniOS Core C Test Suite
 */

#include "include/omnios_core.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  Testing %s... ", name);
#define END_TEST(result) do { \
    if (result) { printf("OK\n"); tests_passed++; } \
    else { printf("FAIL\n"); tests_failed++; } \
} while(0)

void test_state_machine(void) {
    printf("\n[State Machine]\n");

    SystemConfig config = {
        .max_processes = 50,
        .total_memory_mb = 4096,
        .screen_width = 390,
        .screen_height = 844,
        .dark_mode = true,
        .gestures_enabled = true,
    };

    om_system_init(&config);
    TEST("Baslangic durumu IDLE");
    END_TEST(om_system_get_state() == STATE_IDLE);

    TEST("Normal mod baslangic");
    END_TEST(om_mode_get_current() == MODE_NORMAL);

    TEST("Mod gecisi NORMAL -> FLOW");
    om_mode_set(MODE_FLOW);
    END_TEST(om_mode_get_current() == MODE_FLOW);

    TEST("Mod gecisi FLOW -> NORMAL");
    om_mode_set(MODE_NORMAL);
    END_TEST(om_mode_get_current() == MODE_NORMAL);

    TEST("Mod toggle");
    om_mode_toggle();
    END_TEST(om_mode_get_current() == MODE_FLOW);

    TEST("Gecersiz mod gecisi");
    bool result = om_mode_set((UIMode)99);
    END_TEST(result == false);

    om_system_shutdown();
    TEST("Shutdown durumu");
    END_TEST(om_system_get_state() == STATE_SHUTDOWN);
}

void test_process_manager(void) {
    printf("\n[Process Manager]\n");

    Process* p1 = om_process_create("Chrome", PLATFORM_ANDROID);
    TEST("Process olusturma");
    END_TEST(p1 != NULL && p1->pid > 0);

    TEST("Process adi dogru");
    END_TEST(strcmp(p1->name, "Chrome") == 0);

    TEST("Process platformu Android");
    END_TEST(p1->platform == PLATFORM_ANDROID);

    Process* p2 = om_process_create("Safari", PLATFORM_IOS);
    TEST("iOS process olusturma");
    END_TEST(p2 != NULL && p2->platform == PLATFORM_IOS);

    TEST("Process sayisi");
    END_TEST(om_process_count() == 2);

    TEST("Process sonlandirma");
    END_TEST(om_process_kill(p1->pid) == true);

    TEST("Process sayisi azaldi");
    END_TEST(om_process_count() == 1);

    TEST("Olmayan PID sonlandirma");
    END_TEST(om_process_kill(9999) == false);

    TEST("Process suspend");
    END_TEST(om_process_suspend(p2->pid) == true);

    TEST("Process resume");
    END_TEST(om_process_resume(p2->pid) == true);
}

void test_gesture_engine(void) {
    printf("\n[Gesture Engine]\n");

    TEST("Swipe up tanima");
    END_TEST(om_gesture_recognize("swipe_up") == GESTURE_SWIPE_UP);

    TEST("Double tap tanima");
    END_TEST(om_gesture_recognize("double_tap") == GESTURE_DOUBLE_TAP);

    TEST("Gecersiz jest");
    END_TEST(om_gesture_recognize("invalid_gesture") == GESTURE_NONE);

    TEST("Normal mod aksiyonu");
    END_TEST(strcmp(om_gesture_get_action(GESTURE_SWIPE_UP, MODE_NORMAL), "open_app_menu") == 0);

    TEST("Flow mod aksiyonu");
    END_TEST(strcmp(om_gesture_get_action(GESTURE_SWIPE_UP, MODE_FLOW), "open_app_menu") == 0);

    TEST("Pinch out flow mod");
    END_TEST(strcmp(om_gesture_get_action(GESTURE_PINCH_OUT, MODE_FLOW), "app_selector_flow") == 0);

    GestureType supported[16];
    uint32_t count = om_gesture_get_supported(supported, 16);
    TEST("Desteklenen jest sayisi");
    END_TEST(count >= 7);
}

void test_memory(void) {
    printf("\n[Memory Manager]\n");

    om_memory_init(4096);

    TEST("Baslangic bellegi dogru");
    END_TEST(om_memory_get_available() == 4096);

    TEST("Bellek ayirma");
    END_TEST(om_memory_allocate(1, 512) == true);

    TEST("Kullanilan bellek");
    END_TEST(om_memory_get_used() == 512);

    TEST("Fazla bellek ayirma");
    END_TEST(om_memory_allocate(2, 5000) == false);

    TEST("Bellek serbest birakma");
    END_TEST(om_memory_free(1) == true);

    TEST("Bellek sifirlandi");
    END_TEST(om_memory_get_used() == 0);

    TEST("Kullanim yuzdesi");
    END_TEST(om_memory_get_usage_percent() == 0.0f);
}

void test_runtime_loader(void) {
    printf("\n[Runtime Loader]\n");

    TEST("APK tespiti");
    END_TEST(om_runtime_detect("app.apk") == PLATFORM_ANDROID);

    TEST("IPA tespiti");
    END_TEST(om_runtime_detect("app.ipa") == PLATFORM_IOS);

    TEST("OmniPKG tespiti");
    END_TEST(om_runtime_detect("app.omnipkg") == PLATFORM_CROSS);

    TEST("Android runtime adi");
    const char* name = om_runtime_get_name(PLATFORM_ANDROID);
    END_TEST(strstr(name, "Android") != NULL);

    TEST("iOS API seviyesi");
    END_TEST(om_runtime_get_api_level(PLATFORM_IOS) == 18);

    TEST("Android API seviyesi");
    END_TEST(om_runtime_get_api_level(PLATFORM_ANDROID) == 35);

    TEST("Runtime kullanilabilirlik");
    END_TEST(om_runtime_is_available(PLATFORM_ANDROID) == true);

    TEST("Cross-platform calisabilirlik");
    END_TEST(om_runtime_can_run(PLATFORM_ANDROID, PLATFORM_CROSS) == true);
}

int main(void) {
    printf("=== OmniOS Core C Test Suite ===\n");

    test_state_machine();
    test_process_manager();
    test_gesture_engine();
    test_memory();
    test_runtime_loader();

    printf("\n================================\n");
    printf("Sonuc: %d passed, %d failed\n", tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
