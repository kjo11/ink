#ifndef USER_CONFIG_HEADER_DEF_
#define USER_CONFIG_HEADER_DEF_

#define _POSIX_SOURCE 1

#include <time.h>

/*
 * Type of backup to be made
 */
typedef enum {
  INCREMENTAL,
  INCREMENTAL_NOLINKS,
  SNAPSHOT,
  FULL
} backup_type;

/*
 * Structure containing config for one backup.
 */
typedef struct backup_config {
  char *name;
  char *mount_point;
  char *backup_folder;
  char *exclude_file;
  char *log_file;
  char *err_file;
  char *to_backup;
  char *link_name;
  char *folder_prefix;
  char *date_format;
  int freq_seconds;
  time_t last_backup;
  char *UUID;
  backup_type type;
  const struct backup_config* next_backup;
} backup_config;

#endif
