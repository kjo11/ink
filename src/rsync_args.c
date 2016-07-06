#include "rsync_args.h"

#include <stdio.h>
#include <unistd.h>

#include "global.h"
#include "string_manipulation.h"

#define RSYNC_MAX_ARGS 10

#define RSYNC_PROGRAM_NAME "rsync"
#define RSYNC_SHORT_OPTS "-a"
#define RSYNC_INFO_OPTS "--info=progress2"
#define RSYNC_EXCLUDE_PREFIX "--exclude-from="
#define RSYNC_LINK_PREFIX "--link-dest="
#define RSYNC_BACKUPDIR_PREFIX "--backup-dir="
#define RSYNC_LOGFILE_PREFIX "--log-file="

/******************************************************************************
 *                            FORWARD DECLARATIONS                            *
 ******************************************************************************/
static void init_rsync_args_to_null(rsync_args *args);

/******************************************************************************
 *                                DEFINITIONS                                 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                                convert_rsync_args_to_array
// -----------------------------------------------------------------------------
void convert_rsync_args_to_array(char **array_out, const rsync_args *args) {
  int n = 0;
  array_out[n] = args->program_name;
  n++;
  array_out[n] = args->short_opts;
  n++;
  array_out[n] = args->info_opts;
  n++;
  if (args->excludes != NULL) {
    array_out[n] = args->excludes;
    n++;
  }
  if (args->log_file != NULL) {
    array_out[n] = args->log_file;
    n++;
  }
  if (args->link_backup_dest != NULL) {
    array_out[n] = args->link_backup_dest;
    n++;
  }
  array_out[n] = args->source_dir;
  n++;
  array_out[n] = args->destination_dir;
  n++;
  array_out[n] = NULL;
}

// -----------------------------------------------------------------------------
//                                                 get_rsync_args_from_config
// -----------------------------------------------------------------------------
int get_rsync_args_from_config(rsync_args *arg_struct,
                               const backup_config *conf,
                               const char *folder_name) {
  // Initialize everything to NULL
  init_rsync_args_to_null(arg_struct);

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
  } else {
    arg_struct->excludes = NULL;
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
  } else {
    arg_struct->link_backup_dest = NULL;
  }

  // Log file
  if (conf->log_file != NULL) {
    NNULL_OR_DIE(arg_struct->log_file = concat_and_copy_strings2(
                     RSYNC_LOGFILE_PREFIX, conf->log_file));
    arg_struct->n_arguments++;
  } else {
    arg_struct->log_file = NULL;
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
//                                                            free_rsync_args
// -----------------------------------------------------------------------------
void free_rsync_args(rsync_args *args) {
  FREE_NOT_NULL(args->program_name);
  FREE_NOT_NULL(args->short_opts);
  FREE_NOT_NULL(args->info_opts);
  FREE_NOT_NULL(args->excludes);
  FREE_NOT_NULL(args->link_backup_dest);
  FREE_NOT_NULL(args->log_file);
  FREE_NOT_NULL(args->destination_dir);
  FREE_NOT_NULL(args->source_dir);
}

// -----------------------------------------------------------------------------
//                                                    init_rsync_args_to_null
// -----------------------------------------------------------------------------
static void init_rsync_args_to_null(rsync_args *args) {
  args->program_name = NULL;
  args->short_opts = NULL;
  args->info_opts = NULL;
  args->excludes = NULL;
  args->link_backup_dest = NULL;
  args->log_file = NULL;
  args->destination_dir = NULL;
  args->source_dir = NULL;
}

// -----------------------------------------------------------------------------
//                                                           print_rsync_args
// -----------------------------------------------------------------------------
void print_rsync_args(const rsync_args *args) {
  char *args_array[args->n_arguments + 1];
  convert_rsync_args_to_array(args_array, args);
  for (int i = 0; i < args->n_arguments; i++) {
    printf("%s ", args_array[i]);
  }
  printf("\n\n");
}
