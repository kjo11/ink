#ifndef BACKUPS_HEADER_DEF_
#define BACKUPS_HEADER_DEF_

#define _GNU_SOURCE 1

#include "user-config.h"
#include <stdio.h>

/*
 * Make backups defined in conf.
 * Exit 0 on success, EXIT_FAILURE on failure.
 */
int make_backups(const backup_config* conf, FILE* output);

#endif
