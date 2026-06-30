/* OmniOS — userspace/security/Makefile */
/* Faz 2 - Capability Token IPC */
/* SPDX-License-Identifier: MIT */

CC       = gcc
CFLAGS   = -Wall -Wextra -O2 -I.
LDFLAGS  =
LIBS     =

DAEMON_SRC   = capability_daemon.c
CLIENT_SRC   = capability_client.c
TEST_SRC     = test_capability.c
DAEMON_BIN   = omnios-capd
CLIENT_LIB   = libomnios_cap.a
TEST_BIN     = test_capability

OBJS     = capability_client.o

.PHONY: all daemon lib test clean

all: daemon lib test

daemon: $(DAEMON_BIN)

$(DAEMON_BIN): $(DAEMON_SRC) capability_protocol.h
	$(CC) $(CFLAGS) -o $@ $(DAEMON_SRC) $(LDFLAGS) $(LIBS)

lib: $(CLIENT_LIB)

$(CLIENT_LIB): $(OBJS)
	$(AR) rcs $@ $^

$(OBJS): capability_client.c capability_client.h capability_protocol.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) $(CLIENT_LIB)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) -L. -lomnios_cap

clean:
	rm -f $(DAEMON_BIN) $(CLIENT_LIB) $(OBJS) $(TEST_BIN)
