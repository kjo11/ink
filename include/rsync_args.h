#ifndef RSYNC_ARGS_H
#define RSYNC_ARGS_H

#define _GNU_SOURCE 1

#include "user-config.h"

/// Structure containing all available arguments to rsync
typedef struct rsync_args {
  char *program_name;
  char *short_opts;
  char *info_opts;
  char *excludes;
  char *link_backup_dest;
  char *log_file;
  char *destination_dir;
  char *source_dir;

  int n_arguments;

} rsync_args;

/**
 * Get rsync args for a backup based on configuration.
 * All non-assigned entries are set to NULL.
 */
int get_rsync_args_from_config(rsync_args *args,
                                      const backup_config *conf,
                                      const char *folder_name);

/// Convert rsync args into NULL-terminated char* array
void convert_rsync_args_to_array(char **array_out,
                                       const rsync_args *args);

/// Free memory malloc'd by rsync_args
void free_rsync_args(rsync_args *args);

/// Print rsync command to be run to stdout
void print_rsync_args(const rsync_args* args);


#endif
