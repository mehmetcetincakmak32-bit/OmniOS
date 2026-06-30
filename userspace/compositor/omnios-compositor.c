/* OmniOS — omnios-compositor.c */
/* Faz 1 Foundation */
/* SPDX-License-Identifier: MIT */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <wlr/xwayland.h>

#define OMNIOS_VERSION "0.1.0"

struct omnios_server {
    struct wl_display *display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_surface;
    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_listener new_layer_surface;

    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *xcursor_manager;
    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_listener request_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;

    struct wl_listener new_output;
};

struct omnios_output {
    struct wl_list link;
    struct omnios_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wl_listener frame;
    struct wl_listener destroy;
    struct wl_listener request_state;
};

struct omnios_view {
    struct wl_list link;
    struct omnios_server *server;
    struct wlr_xdg_surface *xdg_surface;
    struct wlr_scene_tree *scene_tree;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
};

struct omnios_layer_surface {
    struct wl_list link;
    struct omnios_server *server;
    struct wlr_layer_surface_v1 *layer_surface;
    struct wlr_scene_tree *scene_tree;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
    struct wl_listener commit;
};

struct omnios_keyboard {
    struct wl_list link;
    struct omnios_server *server;
    struct wlr_keyboard *wlr_keyboard;
    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

static struct wl_list views;

static struct omnios_view *desktop_view_at(struct omnios_server *server,
        double lx, double ly, struct wlr_surface **surface,
        double *sx, double *sy) {
    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(
        wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy));
    if (scene_buffer == NULL) {
        return NULL;
    }
    struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_buffer(scene_buffer);
    if (scene_surface == NULL) {
        return NULL;
    }
    *surface = scene_surface->surface;
    struct wlr_xdg_surface *xdg_surface =
        wlr_xdg_surface_from_wlr_surface(scene_surface->surface);
    if (xdg_surface == NULL) {
        return NULL;
    }
    struct omnios_view *view;
    wl_list_for_each(view, &views, link) {
        if (view->xdg_surface == xdg_surface) {
            return view;
        }
    }
    return NULL;
}

static void reset_cursor_mode(struct omnios_server *server) {
    wlr_xcursor_manager_set_cursor_image(server->xcursor_manager, "left_ptr",
        server->cursor);
}

static void begin_interactive(struct omnios_view *view,
        enum wlr_seat_pointer_request_move_event::resize_edges edge, bool move) {
    (void)edge;
    (void)move;
    /* TODO: interactive move/resize */
    reset_cursor_mode(view->server);
}

static void xdg_surface_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, map);
    wlr_scene_node_set_enabled(&view->scene_tree->node, true);
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, unmap);
    wlr_scene_node_set_enabled(&view->scene_tree->node, false);
}

static void xdg_surface_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, destroy);
    wl_list_remove(&view->link);
    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);
    free(view);
}

static void begin_move(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, request_move);
    begin_interactive(view, 0, true);
}

static void begin_resize(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *e = data;
    begin_interactive(view, e->edges, false);
}

static void begin_maximize(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, request_maximize);
    struct wlr_xdg_surface *s = view->xdg_surface;
    bool maximized = !s->toplevel->requested.maximized;
    wlr_xdg_toplevel_set_maximized(s, maximized);
}

static void begin_fullscreen(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_view *view = wl_container_of(listener, view, request_fullscreen);
    struct wlr_xdg_surface *s = view->xdg_surface;
    bool fullscreen = !s->toplevel->requested.fullscreen;
    wlr_xdg_toplevel_set_fullscreen(s, fullscreen);
}

static void new_xdg_surface(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server, new_xdg_surface);
    struct wlr_xdg_surface *xdg_surface = data;
    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
        return;
    }
    struct omnios_view *view = calloc(1, sizeof(*view));
    assert(view);
    view->server = server;
    view->xdg_surface = xdg_surface;
    view->scene_tree = wlr_scene_tree_create(server->scene);
    wlr_scene_node_set_enabled(&view->scene_tree->node, false);
    wlr_scene_surface_create(view->scene_tree, xdg_surface->surface);
    view->map.notify = xdg_surface_map;
    wl_signal_add(&xdg_surface->events.map, &view->map);
    view->unmap.notify = xdg_surface_unmap;
    wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
    view->destroy.notify = xdg_surface_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

    struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
    view->request_move.notify = begin_move;
    wl_signal_add(&toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = begin_resize;
    wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    view->request_maximize.notify = begin_maximize;
    wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
    view->request_fullscreen.notify = begin_fullscreen;
    wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);

    wl_list_insert(&views, &view->link);
}

static void layer_surface_map(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_layer_surface *ls = wl_container_of(listener, ls, map);
    wlr_scene_node_set_enabled(&ls->scene_tree->node, true);
}

static void layer_surface_unmap(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_layer_surface *ls = wl_container_of(listener, ls, unmap);
    wlr_scene_node_set_enabled(&ls->scene_tree->node, false);
}

static void layer_surface_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_layer_surface *ls = wl_container_of(listener, ls, destroy);
    wl_list_remove(&ls->link);
    wl_list_remove(&ls->map.link);
    wl_list_remove(&ls->unmap.link);
    wl_list_remove(&ls->destroy.link);
    wl_list_remove(&ls->commit.link);
    free(ls);
}

static void layer_surface_commit(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_layer_surface *ls = wl_container_of(listener, ls, commit);
    struct wlr_layer_surface_v1 *wlr_ls = ls->layer_surface;
    struct wlr_scene_node *node = &ls->scene_tree->node;
    struct wlr_output *output = wlr_ls->output;
    if (output == NULL) {
        return;
    }
    int32_t width = output->width;
    int32_t height = output->height;

    enum zwlr_layer_shell_v1_anchor anchor = wlr_ls->current.anchor;
    int x = 0, y = 0;
    if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_CENTER) {
        x = (width - wlr_ls->current.desired_width) / 2;
    } else {
        if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_RIGHT) {
            x = width - wlr_ls->current.desired_width;
        }
        if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_LEFT) {
            x = 0;
        }
    }
    if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_BOTTOM) {
        y = height - wlr_ls->current.desired_height;
    } else if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_TOP) {
        y = 0;
    } else if (anchor & ZWLR_LAYER_SHELL_V1_ANCHOR_CENTER) {
        y = (height - wlr_ls->current.desired_height) / 2;
    }
    wlr_scene_node_set_position(node, x, y);
}

static void new_layer_surface(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server, new_layer_surface);
    struct wlr_layer_surface_v1 *wlr_ls = data;

    if (!wlr_ls->output) {
        if (!wl_list_empty(&server->outputs)) {
            struct omnios_output *output = wl_container_of(
                server->outputs.next, output, link);
            wlr_ls->output = output->wlr_output;
        }
    }

    struct omnios_layer_surface *ls = calloc(1, sizeof(*ls));
    assert(ls);
    ls->server = server;
    ls->layer_surface = wlr_ls;
    ls->scene_tree = wlr_scene_tree_create(server->scene);
    wlr_scene_node_set_enabled(&ls->scene_tree->node, false);
    wlr_scene_surface_create(ls->scene_tree, wlr_ls->surface);

    ls->map.notify = layer_surface_map;
    wl_signal_add(&wlr_ls->surface->events.map, &ls->map);
    ls->unmap.notify = layer_surface_unmap;
    wl_signal_add(&wlr_ls->surface->events.unmap, &ls->unmap);
    ls->destroy.notify = layer_surface_destroy;
    wl_signal_add(&wlr_ls->events.destroy, &ls->destroy);
    ls->commit.notify = layer_surface_commit;
    wl_signal_add(&wlr_ls->surface->events.commit, &ls->commit);

    wl_list_insert(&server->outputs, &ls->link);
    wlr_layer_surface_v1_configure(wlr_ls,
        wlr_ls->output ? wlr_ls->output->width : 1280,
        wlr_ls->output ? wlr_ls->output->height : 720);
}

static void output_frame(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene_output *scene_output = output->scene_output;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_commit(scene_output, NULL);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

static void output_request_state(struct wl_listener *listener, void *data) {
    struct omnios_output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = data;
    wlr_output_set_state(output->wlr_output, event->state);
}

static void output_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_output *output = wl_container_of(listener, output, destroy);
    wl_list_remove(&output->link);
    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->request_state.link);
    free(output);
}

static void new_output(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = data;

    wlr_output_init_empty(wlr_output);
    wlr_output_set_custom_mode(wlr_output, 1280, 720, 60000);
    wlr_output_enable(wlr_output, true);

    struct omnios_output *output = calloc(1, sizeof(*output));
    assert(output);
    output->server = server;
    output->wlr_output = wlr_output;
    output->scene_output = wlr_scene_get_scene_output(server->scene, wlr_output);
    wlr_output_layout_add_auto(server->output_layout, wlr_output);

    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);
    output->request_state.notify = output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    wl_list_insert(&server->outputs, &output->link);
    wlr_output_commit(wlr_output);

    wlr_xcursor_manager_load(server->xcursor_manager, wlr_output->scale);
}

static void keyboard_modifiers(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_keyboard *kb = wl_container_of(listener, kb, modifiers);
    wlr_seat_set_keyboard(kb->server->seat, kb->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(kb->server->seat,
        &kb->wlr_keyboard->modifiers);
}

static bool handle_keybinding(struct omnios_server *server, xkb_keysym_t sym) {
    switch (sym) {
    case XKB_KEY_Escape:
        wl_display_terminate(server->display);
        return true;
    case XKB_KEY_F1:
        if (fork() == 0) {
            execl("/usr/bin/omnios-flutter-embedder",
                "omnios-flutter-embedder", NULL);
            _exit(EXIT_FAILURE);
        }
        return true;
    default:
        return false;
    }
}

static void keyboard_key(struct wl_listener *listener, void *data) {
    struct omnios_keyboard *kb = wl_container_of(listener, kb, key);
    struct wlr_keyboard_key_event *event = data;
    struct wlr_seat *seat = kb->server->seat;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(kb->wlr_keyboard->xkb_state,
        keycode, &syms);

    bool handled = false;
    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED && nsyms > 0) {
        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(kb->server, syms[i]);
            if (handled) break;
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, kb->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec,
            event->keycode, event->state);
    }
}

static void keyboard_destroy(struct wl_listener *listener, void *data) {
    (void)data;
    struct omnios_keyboard *kb = wl_container_of(listener, kb, destroy);
    wl_list_remove(&kb->link);
    wl_list_remove(&kb->modifiers.link);
    wl_list_remove(&kb->key.link);
    wl_list_remove(&kb->destroy.link);
    free(kb);
}

static void server_new_keyboard(struct omnios_server *server,
        struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    struct omnios_keyboard *kb = calloc(1, sizeof(*kb));
    assert(kb);
    kb->server = server;
    kb->wlr_keyboard = wlr_keyboard;

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    kb->modifiers.notify = keyboard_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &kb->modifiers);
    kb->key.notify = keyboard_key;
    wl_signal_add(&wlr_keyboard->events.key, &kb->key);
    kb->destroy.notify = keyboard_destroy;
    wl_signal_add(&device->events.destroy, &kb->destroy);

    wlr_seat_set_keyboard(server->seat, device);
    wl_list_insert(&server->keyboards, &kb->link);
}

static void server_new_pointer(struct omnios_server *server,
        struct wlr_input_device *device) {
    (void)device;
    wlr_cursor_attach_input_device(server->cursor, device);
}

static void server_new_input(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        server_new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        server_new_pointer(server, device);
        break;
    default:
        break;
    }
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused_client =
        server->seat->pointer_state.focused_client;
    if (event->seat_client == focused_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

static void seat_request_set_selection(struct wl_listener *listener, void *data) {
    struct omnios_server *server = wl_container_of(listener, server,
        request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

static const char *startup_cmds[] = {
    "/usr/bin/omnios-flutter-embedder",
    NULL
};

int main(int argc, char *argv[]) {
    wlr_log_init(WLR_DEBUG, NULL);

    struct omnios_server server = {0};
    wl_list_init(&views);
    wl_list_init(&server.outputs);
    wl_list_init(&server.keyboards);

    server.display = wl_display_create();
    server.backend = wlr_backend_autocreate(server.display, NULL);

    server.renderer = wlr_renderer_autocreate(server.backend);
    wlr_renderer_init_wl_shm(server.renderer);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    server.scene = wlr_scene_create();
    wlr_scene_attach_output_layout(server.scene, server.output_layout);

    wlr_compositor_create(server.display, 5, server.renderer);
    wlr_subcompositor_create(server.display);
    wlr_data_device_manager_create(server.display);

    server.output_layout = wlr_output_layout_create(server.display);
    server.new_output.notify = new_output;
    wl_signal_add(&server.backend->events.new_output, &server.new_output);

    server.xdg_shell = wlr_xdg_shell_create(server.display, 3);
    server.new_xdg_surface.notify = new_xdg_surface;
    wl_signal_add(&server.xdg_shell->events.new_surface,
        &server.new_xdg_surface);

    server.layer_shell = wlr_layer_shell_v1_create(server.display, 4);
    server.new_layer_surface.notify = new_layer_surface;
    wl_signal_add(&server.layer_shell->events.new_surface,
        &server.new_layer_surface);

    server.cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server.cursor, server.output_layout);

    server.xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    wlr_xcursor_manager_load(server.xcursor_manager, 1);

    server.seat = wlr_seat_create(server.display, "seat0");
    server.new_input.notify = server_new_input;
    wl_signal_add(&server.backend->events.new_input, &server.new_input);
    server.request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server.seat->events.request_set_cursor,
        &server.request_cursor);
    server.request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server.seat->events.request_set_selection,
        &server.request_set_selection);

    if (!wlr_backend_start(server.backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return 1;
    }

    /* Launch Flutter System UI */
    for (int i = 0; startup_cmds[i] != NULL; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            execl(startup_cmds[i], startup_cmds[i], NULL);
            _exit(EXIT_FAILURE);
        }
    }

    wlr_log(WLR_INFO, "OmniOS Compositor v%s started", OMNIOS_VERSION);
    wl_display_run(server.display);

    wl_display_destroy(server.display);
    return 0;
}
