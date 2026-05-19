#include "perf_manager.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *enable_cmd = "enable";
static const char *disable_cmd = "disable";
static const char *ack_cmd = "ack\n";

static void send_command(PerfManager *pm, const char *command) {
    if (!pm->enable) {
        return;
    }
    if (write(pm->ctl_fd, command, strlen(command)) < 0) {
        perror("write perf ctl");
        return;
    }
    char ack[5];
    if (read(pm->ack_fd, ack, 5) < 0) {
        perror("read perf ack");
        return;
    }
    assert(strncmp(ack, ack_cmd, 4) == 0);
}

void PerfManager_init(PerfManager *pm) {
    char *ctl_fd_env = getenv("PERF_CTL_FD");
    char *ack_fd_env = getenv("PERF_ACK_FD");
    if (ctl_fd_env && ack_fd_env) {
        pm->enable = true;
        pm->ctl_fd = atoi(ctl_fd_env);
        pm->ack_fd = atoi(ack_fd_env);
    } else {
        pm->enable = false;
        pm->ctl_fd = -1;
        pm->ack_fd = -1;
    }
}

void PerfManager_pause(PerfManager *pm) { send_command(pm, disable_cmd); }

void PerfManager_resume(PerfManager *pm) { send_command(pm, enable_cmd); }
