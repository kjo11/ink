#include "backups.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "global.h"
#include "string_manipulation.h"

#define MAX_DATE_SIZE 40
#define RSYNC_MAX_ARGS 10

#define RSYNC_PROGRAM_NAME "rsync"
#define RSYNC_SHORT_OPTS "-a"
#define RSYNC_INFO_OPTS "--info=progress2"
#define RSYNC_EXCLUDE_PREFIX "--exclude-from="
#define RSYNC_LINK_PREFIX "--link-dest="
#define RSYNC_BACKUPDIR_PREFIX "--backup-dir="
#define RSYNC_LOGFILE_PREFIX "--log-file="

#define DO_OR_DIE(x)                              \
  do {                                            \
    if ((x) != EXIT_SUCCESS) return EXIT_FAILURE; \
  } while (0);

#define NNULL_OR_DIE(x)                   \
  do {                                    \
    if ((x) == NULL) return EXIT_FAILURE; \
  } while (0);

/******************************************************************************
 *                            FORWARD DECLARATIONS                            *
 ******************************************************************************/
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

// Make backups for a single configuration
static int make_backups_sub(const backup_config *conf);

// Get folder name for a backup
static int get_folder_name(char *folder_name, const backup_config *conf,
                           const time_t curr_unix_time);

// Get rsync args for a backup based on configuration
static int get_rsync_args_from_config(rsync_args *args,
                                      const backup_config *conf,
                                      const char *folder_name);

// Free memory malloc'd by rsync_args
static void free_rsync_args(rsync_args *args);

/******************************************************************************
 *                                DEFINITIONS                                 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                               make_backups
// -----------------------------------------------------------------------------
// Check and make new backups if required
int make_backups(const backup_config *conf, FILE *output) {
  // Loop through list of backup configuratons
  while (conf != NULL) {
    fprintf(output, "Making backup %s...\n", conf->name);
    if (make_backups_sub(conf) != EXIT_SUCCESS) {
      fprintf(stderr, "Error creating backup %s. Exiting...\n", conf->name);
      return EXIT_FAILURE;
    }
    conf = conf->next_backup;
  }
  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                             make_backups_sub
// -----------------------------------------------------------------------------
// Make backups for a single configuration
static int make_backups_sub(const backup_config *conf) {
  char *folder_name = NULL;

  // Get current time
  time_t curr_unix_time;
  tzset();
  curr_unix_time = time(NULL);

  // Get folder name
  if (get_folder_name(folder_name, conf, curr_unix_time) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  rsync_args args_struct;
  get_rsync_args_from_config(&args_struct, conf, folder_name);

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                              get_folder_name
// -----------------------------------------------------------------------------
// Return the folder name for a given backup
static int get_folder_name(char *folder_name, const backup_config *conf,
                           const time_t curr_unix_time) {
  struct tm curr_time = *localtime(&curr_unix_time);
  char date_string[MAX_DATE_SIZE + 1];
  date_string[0] = '\0';

  // Write date
  size_t date_size =
      strftime(date_string, MAX_DATE_SIZE, conf->date_format, &curr_time);
  if (date_size == MAX_DATE_SIZE) {
    return EXIT_FAILURE;
  }

  // Allocate and copy folder prefix
  const char *strings[] = {conf->folder_prefix, date_string};
  if ((folder_name = concat_and_copy_strings(strings, 2)) == NULL) {
    return EXIT_FAILURE;
  }

  printf("%s\n", folder_name);

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                          get_rsync_args_from_config
// -----------------------------------------------------------------------------
static int get_rsync_args_from_config(rsync_args *arg_struct,
                                      const backup_config *conf,
                                      const char *folder_name) {
  arg_struct->n_arguments = 0;  // Number of arguments added

  // Program name
  NNULL_OR_DIE(arg_struct->program_name =
                   allocate_and_copy_string(RSYNC_PROGRAM_NAME));
  arg_struct->n_arguments++;

  // Short options
  NNULL_OR_DIE(arg_struct->short_opts =
                   allocate_and_copy_string(RSYNC_SHORT_OPTS));
  arg_struct->n_arguments++;

  // Info options
  NNULL_OR_DIE(arg_struct->info_opts =
                   allocate_and_copy_string(RSYNC_INFO_OPTS));
  arg_struct->n_arguments++;

  // Exclude file -- null if not given
  if (conf->exclude_file != NULL) {
    NNULL_OR_DIE(arg_struct->excludes = concat_and_copy_strings2(
                     RSYNC_EXCLUDE_PREFIX, conf->exclude_file));
    arg_struct->n_arguments++;
  }

  // Link or backup destination
  if ((conf->type == INCREMENTAL) && (conf->link_name != NULL)) {
    NNULL_OR_DIE(
        arg_struct->link_backup_dest = concat_and_copy_strings4(
            RSYNC_LINK_PREFIX, conf->backup_folder, "/", conf->link_name));
    arg_struct->n_arguments++;
  }

  // Backup directory
  else if ((conf->type == INCREMENTAL_NOLINKS) && (conf->link_name != NULL)) {
    // Read location of last backup
    char last_backup_name[MAX_PATHNAME_LENGTH];
    char *link_full_path =
        concat_and_copy_strings3(conf->backup_folder, "/", conf->link_name);
    const int n_chars =
        readlink(link_full_path, last_backup_name, MAX_PATHNAME_LENGTH);
    free(link_full_path);

    // append trailing null char
    if (n_chars == -1) {
      arg_struct->link_backup_dest = NULL;
    } else {
      last_backup_name[n_chars] = '\0';

      NNULL_OR_DIE(arg_struct->link_backup_dest = concat_and_copy_strings3(
                       RSYNC_BACKUPDIR_PREFIX, "/", last_backup_name));
      arg_struct->n_arguments++;
    }
  }

  // Log file
  if (conf->log_file != NULL) {
    NNULL_OR_DIE(arg_struct->log_file = concat_and_copy_strings2(
                     RSYNC_LOGFILE_PREFIX, conf->log_file));
    arg_struct->n_arguments++;
  }

  // What to backup
  NNULL_OR_DIE(arg_struct->source_dir =
                   allocate_and_copy_string(conf->to_backup));
  arg_struct->n_arguments++;

  // Where to put it
  if (conf->type == SNAPSHOT) {
    NNULL_OR_DIE((arg_struct->source_dir =
                      allocate_and_copy_string(conf->backup_folder)));
    arg_struct->n_arguments++;
  } else {
    NNULL_OR_DIE((arg_struct->destination_dir = concat_and_copy_strings2(
                      conf->backup_folder, folder_name)));
    arg_struct->n_arguments++;
  }

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                           free_rsync_args
// -----------------------------------------------------------------------------
static void free_rsync_args(rsync_args *args) {
  free(args->program_name);
  free(args->short_opts);
  free(args->info_opts);
  free(args->excludes);
  free(args->link_backup_dest);
  free(args->log_file);
  free(args->destination_dir);
  free(args->source_dir);
}
