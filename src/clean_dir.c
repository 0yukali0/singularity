/* 
 * Copyright (c) 2016, Brian Bockelman. All rights reserved.
 * 
 * "Singularity” Copyright (c) 2016, The Regents of the University of California,
 * through Lawrence Berkeley National Laboratory (subject to receipt of any
 * required approvals from the U.S. Dept. of Energy).  All rights reserved.
 * 
 * This software is licensed under a customized 3-clause BSD license.  Please
 * consult LICENSE file distributed with the sources of this project regarding
 * your rights to use or distribute this software.
 * 
 * NOTICE.  This Software was developed under funding from the U.S. Department of
 * Energy and the U.S. Government consequently retains certain rights. As such,
 * the U.S. Government has been granted for itself and others acting on its
 * behalf a paid-up, nonexclusive, irrevocable, worldwide license in the Software
 * to reproduce, distribute copies to the public, prepare derivative works, and
 * perform publicly and display publicly, and to permit other to do so. 
 * 
 */

#include "clean_dir.h"

#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <errno.h>

#include "message.h"

// Code adopted from http://stackoverflow.com/questions/2256945

int clean_dir(const char *dirname) {

    int ret = 0;
    FTS *ftsp = NULL;
    FTSENT *curr;

    char *files[] = { (char *) dirname, NULL };

    ftsp = fts_open(files, FTS_NOCHDIR | FTS_PHYSICAL | FTS_XDEV, NULL);
    if ( !ftsp ) {
        message(VERBOSE, "%s: failed to open a directory: %s\n", dirname, strerror(errno));
        ret = -1;
        goto finish;
    }

    while ( ( curr = fts_read(ftsp) ) ) {
        switch (curr->fts_info) {
        case FTS_NS:
        case FTS_DNR:
        case FTS_ERR:
            message(VERBOSE, "%s: fts_read error: %s\n",
                    curr->fts_accpath, strerror(curr->fts_errno));
            break;

        case FTS_DC:
        case FTS_DOT:
        case FTS_NSOK:
            // Not reached unless FTS_LOGICAL, FTS_SEEDOT, or FTS_NOSTAT were
            // passed to fts_open()
            break;

        case FTS_D:
            // Do nothing. Need depth-first search, so directories are deleted
            // in FTS_DP
            break;

        case FTS_DP:
        case FTS_F:
        case FTS_SL:
        case FTS_SLNONE:
        case FTS_DEFAULT:
            if (remove(curr->fts_accpath) < 0) {
                message(VERBOSE, "%s: Failed to remove: %s\n",
                        curr->fts_path, strerror(errno));
                ret = -1;
            }
            break;
        }
    }

finish:
    if ( ftsp ) {
        fts_close(ftsp);
    }

    return ret;
}
