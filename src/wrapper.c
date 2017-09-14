/* 
 * Copyright (c) 2017, SingularityWare, LLC. All rights reserved.
 * 
 * This software is licensed under a 3-clause BSD license.  Please
 * consult LICENSE file distributed with the sources of this project regarding
 * your rights to use or distribute this software.
 * 
 */


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mount.h>

#include "config.h"
#include "util/file.h"
#include "util/util.h"
#include "util/registry.h"
#include "util/config_parser.h"
#include "util/capability.h"
#include "util/privilege.h"
#include "util/suid.h"
#include "lib/image/image.h"
#include "lib/runtime/runtime.h"

#ifndef SYSCONFDIR
#error SYSCONFDIR not defined
#endif

#define MOUNT_BINARY    "mount"
#define START_BINARY    "start"
#define ACTION_BINARY   "action"

struct cmd_wrapper {
    char *command;
    char *binary;
};

struct cmd_wrapper cmd_wrapper[] = {
    { .command = "shell",           .binary = ACTION_BINARY },
    { .command = "exec",            .binary = ACTION_BINARY },
    { .command = "run",             .binary = ACTION_BINARY },
    { .command = "test",            .binary = ACTION_BINARY },
    { .command = "mount",           .binary = MOUNT_BINARY },
    { .command = "help",            .binary = MOUNT_BINARY },
    { .command = "apps",            .binary = MOUNT_BINARY },
    { .command = "inspect",         .binary = MOUNT_BINARY },
    { .command = "check",           .binary = MOUNT_BINARY },
    { .command = "image.import",    .binary = MOUNT_BINARY },
    { .command = "image.export",    .binary = MOUNT_BINARY },
    { .command = "instance.start",  .binary = START_BINARY },
    { .command = NULL,              .binary = NULL }
};

int main(int argc, char **argv) {
    int index;
    char *command;
    char *libexec_bin = joinpath(LIBEXECDIR, "/singularity/bin/");

    singularity_registry_init();

    singularity_config_init(joinpath(SYSCONFDIR, "/singularity/singularity.conf"));

    singularity_capability_init();

    singularity_suid_init(argv);

    command = singularity_registry_get("COMMAND");

    if ( command == NULL ) {
        singularity_message(ERROR, "no command passed\n");
        ABORT(255);
    }

    for ( index = 0; cmd_wrapper[index].command != NULL; index++) {
        if ( strcmp(command, cmd_wrapper[index].command) == 0 ) {
            envar_set("SINGULARITY_SUID_WRAPPER", "1", 1);

            argv[0] = strjoin(libexec_bin, cmd_wrapper[index].binary);
            execve(argv[0], argv, environ);

            singularity_message(ERROR, "Failed to execute %s binary\n", cmd_wrapper[index].binary);
            ABORT(255);
        }
    }

    singularity_message(ERROR, "unknown command %s\n", command);
    ABORT(255);

    return(0);
}
