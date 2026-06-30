/* OmniOS — compositor/meson.build */
/* Faz 1 Foundation */
/* SPDX-License-Identifier: MIT */

project('omnios-compositor', 'c',
    version: '0.1.0',
    license: 'MIT',
    meson_version: '>=1.0.0')

wlroots        = dependency('wlroots',         version: '>=0.17')
wayland_server = dependency('wayland-server',  version: '>=1.22')
wayland_proto  = dependency('wayland-protocols', version: '>=1.34')
xkbcommon      = dependency('xkbcommon',       version: '>=1.6.0')
pixman         = dependency('pixman-1',        version: '>=0.42')
libinput       = dependency('libinput',        version: '>=1.25')
libdrm         = dependency('libdrm',          version: '>=2.4.120')

executable('omnios-compositor',
    'omnios-compositor.c',
    dependencies: [wlroots, wayland_server, wayland_proto, xkbcommon, pixman, libinput, libdrm],
    install: true)
