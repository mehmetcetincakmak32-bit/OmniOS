/* OmniOS — userspace/security/capability_client.h */
/* Faz 2 - Capability Token IPC Client Library */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_CAP_CLIENT_H
#define OMNIOS_CAP_CLIENT_H

#include "capability_protocol.h"

/* Opaque handle */
typedef struct omnios_cap_ctx omnios_cap_ctx_t;

/* ── Lifecycle ───────────────────────────────────────────────────── */

omnios_cap_ctx_t *omnios_cap_connect(void);
void               omnios_cap_disconnect(omnios_cap_ctx_t *ctx);

/* ── Token operations ────────────────────────────────────────────── */

int omnios_cap_request(omnios_cap_ctx_t *ctx, omnios_cap_rights_t rights,
        const char *label);

int omnios_cap_revoke(omnios_cap_ctx_t *ctx, int token_id);

int omnios_cap_verify(omnios_cap_ctx_t *ctx, int token_id,
        omnios_cap_token_t *out);

int omnios_cap_list(omnios_cap_ctx_t *ctx, uint32_t *count_out);

/* ── Utility ─────────────────────────────────────────────────────── */

const char *omnios_cap_strerror(int err);

#endif /* OMNIOS_CAP_CLIENT_H */
