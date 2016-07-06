#include "backups.h"

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "global.h"
#include "rsync_args.h"
#include "string_manipulation.h"

#define MAX_DATE_SIZE 40

/******************************************************************************
 *                            FORWARD DECLARATIONS                            *
 ******************************************************************************/
// Make backups for a single configuration
static int make_backups_sub(const backup_config *conf, FILE *output);

// Get folder name for a backup
static char *get_folder_name(const backup_config *conf,
                             const time_t curr_unix_time);

/******************************************************************************
 *                                DEFINITIONS                                 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                                               make_backups
// -----------------------------------------------------------------------------
// Check and make new backups if required
int make_backups(const backup_config *conf, FILE *output) {
  // Loop through list of backup configuratons
  while (conf != NULL) {
    if (make_backups_sub(conf, output) != EXIT_SUCCESS) {
      fprintf(stderr, "Error creating backup %s. Exiting...\n", conf->name);
      return EXIT_FAILURE;
    }
    conf = conf->next_backup;
  }
  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                                                           make_backups_sub
// -----------------------------------------------------------------------------
// Make backups for a single configuration
static int make_backups_sub(const backup_config *conf, FILE *output) {
  fprintf(output, "Making backup %s...\n", conf->name);

  // Get current time
  time_t curr_unix_time;
  tzset();
  curr_unix_time = time(NULL);

  // Get folder name
  char *folder_name = NULL;
  if ((folder_name = get_folder_name(conf, curr_unix_time)) == NULL) {
    return EXIT_FAILURE;
  }

  rsync_args args_struct;
  DO_OR_DIE(get_rsync_args_from_config(&args_struct, conf, folder_name));

  fprintf(output, "Command to run: ");
  print_rsync_args(&args_struct, output);

  printf("Running some shit.\n");
  char *args_array[args_struct.n_arguments + 1];
  convert_rsync_args_to_array(args_array, &args_struct);

  // Fork new process and run rsync
  pid_t pid;
  int statval;
  if ((pid = fork()) == -1) {
    free_rsync_args(&args_struct);
    fprintf(stderr, "Error forking rsync process using function fork.\n");
  }
  if (pid == 0) {
    execvp("rsync", args_array);
    free_rsync_args(&args_struct);
    fprintf(stderr, "Error running rsync using function execvp.\n");
    return EXIT_FAILURE;
  } else {
    // wait(NULL);
    waitpid(pid, &statval, WUNTRACED
#ifdef WCONTINUED /* Not all implementations support this */
                               | WCONTINUED
#endif
            );
    if (WIFEXITED(statval))
      printf("Child's exit code %d\n", WEXITSTATUS(statval));
    else
      printf("Child did not terminate with exit\n");
  }

  free_rsync_args(&args_struct);
  free(folder_name);

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                                                            get_folder_name
// -----------------------------------------------------------------------------
// Return the folder name for a given backup
static char *get_folder_name(const backup_config *conf,
                             const time_t curr_unix_time) {
  char *folder_name;
  struct tm curr_time = *localtime(&curr_unix_time);
  char date_string[MAX_DATE_SIZE + 1];
  date_string[0] = '\0';

  // Write date
  size_t date_size =
      strftime(date_string, MAX_DATE_SIZE, conf->date_format, &curr_time);
  if (date_size == MAX_DATE_SIZE) {
    return NULL;
  }

  // Allocate and copy folder prefix
  const char *strings[] = {conf->folder_prefix, date_string};
  if ((folder_name = concat_and_copy_strings(strings, 2)) == NULL) {
    return NULL;
  }

  printf("%s\n", folder_name);

  return folder_name;
}
