/* OmniOS — flutter-embedder/embedder.c */
/* Faz 1 Foundation */
/* SPDX-License-Identifier: MIT */

#define _POSIX_C_SOURCE 200809L
#define FLUTTER_ENGINE_VERSION 1

#include <assert.h>
#include <dlfcn.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <flutter_embedder.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h>

#define WIDTH  1280
#define HEIGHT 720

struct omnios_flutter_state {
    struct wl_display *wl_display;
    struct wl_compositor *wl_compositor;
    struct wl_surface *wl_surface;
    struct wl_egl_window *egl_window;
    struct wl_shell *wl_shell;
    struct wl_shell_surface *wl_shell_surface;
    struct xdg_wm_base *xdg_wm;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;

    FlutterEngine engine;
    FlutterEngineAOTData aot_data;

    bool running;
    bool fullscreen;
    int width;
    int height;
};

static struct omnios_flutter_state state = {0};

/* ── Wayland helpers ─────────────────────────────────────────────── */

static void wl_registry_global(void *data, struct wl_registry *registry,
        uint32_t name, const char *interface, uint32_t version) {
    (void)data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state.wl_compositor = wl_registry_bind(registry, name,
            &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        state.xdg_wm = wl_registry_bind(registry, name,
            &xdg_wm_base_interface, 2);
    } else if (strcmp(interface, wl_shell_interface.name) == 0) {
        state.wl_shell = wl_registry_bind(registry, name,
            &wl_shell_interface, 1);
    }
}

static void wl_registry_global_remove(void *data,
        struct wl_registry *registry, uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    wl_registry_global,
    wl_registry_global_remove
};

static void xdg_surface_configure(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_configure
};

static void xdg_toplevel_configure(void *data,
        struct xdg_toplevel *toplevel, int32_t width, int32_t height,
        struct wl_array *states) {
    (void)data;
    (void)toplevel;
    enum xdg_toplevel_state *s;
    state.fullscreen = false;
    wl_array_for_each(s, states) {
        if (*s == XDG_TOPLEVEL_STATE_FULLSCREEN) {
            state.fullscreen = true;
        }
    }
    if (width > 0) state.width = width;
    if (height > 0) state.height = height;
}

static void xdg_toplevel_close(void *data,
        struct xdg_toplevel *toplevel) {
    (void)data;
    (void)toplevel;
    state.running = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    xdg_toplevel_configure,
    xdg_toplevel_close
};

static bool init_wayland(void) {
    state.wl_display = wl_display_connect(NULL);
    if (!state.wl_display) {
        fprintf(stderr, "Failed to connect to Wayland display\n");
        goto err;
    }

    struct wl_registry *registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(state.wl_display);

    if (!state.wl_compositor) {
        fprintf(stderr, "No wl_compositor available\n");
        goto err;
    }
    if (!state.xdg_wm) {
        fprintf(stderr, "No xdg_wm_base available\n");
        goto err;
    }

    state.wl_surface = wl_compositor_create_surface(state.wl_compositor);
    if (!state.wl_surface) {
        fprintf(stderr, "Failed to create wl_surface\n");
        goto err;
    }

    state.xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_wm,
        state.wl_surface);
    xdg_surface_add_listener(state.xdg_surface, &xdg_surface_listener, NULL);

    state.xdg_toplevel = xdg_surface_get_toplevel(state.xdg_surface);
    xdg_toplevel_add_listener(state.xdg_toplevel,
        &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(state.xdg_toplevel, "OmniOS");
    xdg_toplevel_set_app_id(state.xdg_toplevel, "com.omnios.systemui");

    wl_surface_commit(state.wl_surface);
    wl_display_roundtrip(state.wl_display);

    state.egl_window = wl_egl_window_create(state.wl_surface,
        state.width ? state.width : WIDTH,
        state.height ? state.height : HEIGHT);
    if (!state.egl_window) {
        fprintf(stderr, "Failed to create wl_egl_window\n");
        goto err;
    }
    return true;

err:
    return false;
}

/* ── EGL / GLES ──────────────────────────────────────────────────── */

static const EGLint config_attribs[] = {
    EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
    EGL_RED_SIZE,        8,
    EGL_GREEN_SIZE,      8,
    EGL_BLUE_SIZE,       8,
    EGL_ALPHA_SIZE,      8,
    EGL_DEPTH_SIZE,      24,
    EGL_STENCIL_SIZE,    8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_NONE
};

static const EGLint context_attribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE
};

static bool init_egl(void) {
    state.egl_display = eglGetDisplay(state.wl_display);
    if (state.egl_display == EGL_NO_DISPLAY) {
        state.egl_display = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    }
    if (state.egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return false;
    }

    if (!eglInitialize(state.egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return false;
    }

    EGLint count;
    if (!eglChooseConfig(state.egl_display, config_attribs,
            &state.egl_config, 1, &count) || count == 0) {
        fprintf(stderr, "Failed to choose EGL config\n");
        return false;
    }

    state.egl_context = eglCreateContext(state.egl_display,
        state.egl_config, EGL_NO_CONTEXT, context_attribs);
    if (state.egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return false;
    }

    state.egl_surface = eglCreateWindowSurface(state.egl_display,
        state.egl_config, state.egl_window, NULL);
    if (state.egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return false;
    }

    if (!eglMakeCurrent(state.egl_display, state.egl_surface,
            state.egl_surface, state.egl_context)) {
        fprintf(stderr, "Failed to make EGL context current\n");
        return false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(state.egl_display, state.egl_surface);
    return true;
}

/* ── Flutter callbacks ───────────────────────────────────────────── */

static void *task_runner_thread(void *data) {
    (void)data;
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 8000000};
    while (state.running) {
        FlutterEngineRunExpiredTasks(state.engine);
        nanosleep(&ts, NULL);
    }
    return NULL;
}

static bool platform_task_runner_init(size_t identifier, void *user_data) {
    (void)identifier;
    (void)user_data;
    return true;
}

static void platform_task_runner_post_task(
        FlutterTask task, size_t target_time_nanos,
        void *user_data, const FlutterEngineResult *result) {
    (void)target_time_nanos;
    (void)user_data;
    FlutterEnginePostDartTask(state.engine, &task);
    (void)result;
}

static FlutterTaskRunnerDescription platform_task_runner = {
    .struct_size = sizeof(FlutterTaskRunnerDescription),
    .identifier = 0,
    .user_data = NULL,
    .runs_task_currently = NULL,
    .post_task = platform_task_runner_post_task,
    .init = platform_task_runner_init,
    .deinitialize = NULL,
};

static bool software_renderer(void *user_data, const void *allocation,
        size_t row_bytes, size_t height) {
    (void)user_data;
    (void)allocation;
    (void)row_bytes;
    (void)height;
    return false;
}

static FlutterEngineResult fbo_callback(void *user_data,
        struct FlutterOpenGLFrameBufferInfo *fbo_info) {
    (void)user_data;
    fbo_info->target = GL_TEXTURE_2D;
    fbo_info->name = 0;
    fbo_info->existing_fbo = 0;
    fbo_info->destruction_callback = NULL;
    fbo_info->user_data = NULL;
    return kSuccess;
}

static FlutterEngineResult make_current_callback(void *user_data) {
    (void)user_data;
    if (!eglMakeCurrent(state.egl_display, state.egl_surface,
            state.egl_surface, state.egl_context)) {
        return kInternalInconsistency;
    }
    return kSuccess;
}

static FlutterEngineResult clear_current_callback(void *user_data) {
    (void)user_data;
    if (!eglMakeCurrent(state.egl_display, EGL_NO_SURFACE,
            EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        return kInternalInconsistency;
    }
    return kSuccess;
}

static FlutterEngineResult present_callback(void *user_data) {
    (void)user_data;
    if (!eglSwapBuffers(state.egl_display, state.egl_surface)) {
        return kInternalInconsistency;
    }
    return kSuccess;
}

static FlutterEngineResult platform_message_callback(
        const FlutterPlatformMessage *message, void *user_data) {
    (void)user_data;
    if (strcmp(message->channel, "flutter/system") == 0) {
        fprintf(stdout, "[system] %.*s\n",
            (int)message->message_size,
            (const char *)message->message);
    }
    FlutterEngineSendPlatformMessageResponse(state.engine,
        message->response_handle, NULL, 0);
    return kSuccess;
}

/* ── Main ────────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    state.running = true;
    state.width = WIDTH;
    state.height = HEIGHT;

    if (!init_wayland()) {
        fprintf(stderr, "OmniOS Flutter Embedder: Wayland init failed\n");
        return 1;
    }

    if (!init_egl()) {
        fprintf(stderr, "OmniOS Flutter Embedder: EGL init failed\n");
        return 1;
    }

    /* Locate AOT / kernel blob */
    FlutterEngineAOTDataSource source;
    memset(&source, 0, sizeof(source));
    source.type = kFlutterEngineAOTDataSourceTypeNone;

    FILE *aot_fp = fopen("/usr/lib/omnios/flutter_app.aot", "rb");
    if (aot_fp) {
        fseek(aot_fp, 0, SEEK_END);
        size_t aot_size = ftell(aot_fp);
        fseek(aot_fp, 0, SEEK_SET);
        void *aot_data = malloc(aot_size);
        fread(aot_data, 1, aot_size, aot_fp);
        fclose(aot_fp);
        source.type = kFlutterEngineAOTDataSourceTypeElfPath;
        source.elf_path = "/usr/lib/omnios/flutter_app.aot";
    }

    FlutterOpenGLRendererConfig gl_config = {
        .struct_size = sizeof(FlutterOpenGLRendererConfig),
        .type = kOpenGL,
        .make_current = make_current_callback,
        .clear_current = clear_current_callback,
        .present = present_callback,
        .fbo_callback = fbo_callback,
        .make_resource_current = make_current_callback,
        .gl_external_texture_frame_callback = NULL,
    };

    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = "/usr/lib/omnios/flutter_assets",
        .icu_data_path = "/usr/lib/omnios/icudtl.dat",
        .command_line_argc = argc,
        .command_line_argv = argv,
        .platform_message_callback = platform_message_callback,
        .vm_snapshot_data_path = NULL,
        .vm_snapshot_instructions_path = NULL,
        .custom_task_runners = &platform_task_runner,
        .aot_data = NULL,
        .shutdown_dart_vm_when_done = true,
    };

    FlutterEngineResult res = FlutterEngineRun(FLUTTER_ENGINE_VERSION,
        &gl_config, &args, &state, &state.engine);
    if (res != kSuccess) {
        fprintf(stderr, "FlutterEngineRun failed: %d\n", res);
        return 1;
    }

    pthread_t task_thread;
    pthread_create(&task_thread, NULL, task_runner_thread, NULL);
    pthread_detach(task_thread);

    fprintf(stdout, "OmniOS Flutter Embedder started\n");

    /* Event loop */
    struct pollfd fds[1];
    fds[0].fd = wl_display_get_fd(state.wl_display);
    fds[0].events = POLLIN;

    while (state.running) {
        if (wl_display_prepare_read(state.wl_display) == 0) {
            wl_display_flush(state.wl_display);
            if (poll(fds, 1, 16) > 0 && fds[0].revents & POLLIN) {
                wl_display_read_events(state.wl_display);
            } else {
                wl_display_cancel_read(state.wl_display);
            }
        } else {
            wl_display_dispatch(state.wl_display);
        }
    }

    FlutterEngineDeinitialize(state.engine);
    FlutterEngineDestroy(state.engine);
    eglDestroySurface(state.egl_display, state.egl_surface);
    eglDestroyContext(state.egl_display, state.egl_context);
    eglTerminate(state.egl_display);
    wl_egl_window_destroy(state.egl_window);
    xdg_toplevel_destroy(state.xdg_toplevel);
    xdg_surface_destroy(state.xdg_surface);
    wl_surface_destroy(state.wl_surface);
    wl_display_disconnect(state.wl_display);

    fprintf(stdout, "OmniOS Flutter Embedder stopped\n");
    return 0;
}
