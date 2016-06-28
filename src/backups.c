#include "backups.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#define MAX_DATE_SIZE 40

// Make backups for a single configuration
static int make_backups_sub(const backup_config *conf);

// Get folder name for a backup
static int get_folder_name(char *folder_name, const backup_config *conf, const time_t curr_unix_time);

// Check and make new backups if required
int make_backups(const backup_config *conf, FILE *output) {
  // Loop through list of backup configuratons
  while (conf != NULL) {
    fprintf(output, "Making backup %s...\n", conf->name);
    if (make_backups_sub(conf)!=EXIT_SUCCESS) {
      fprintf(stderr, "Error creating backup %s. Exiting...\n",conf->name);
      return EXIT_FAILURE;
    }
    conf = conf->next_backup;
  }
  return EXIT_SUCCESS;
}

static int get_folder_name(char* folder_name, const backup_config *conf, const time_t curr_unix_time) {
  struct tm curr_time = *localtime(&curr_unix_time);
  char date_string[MAX_DATE_SIZE+1];
  date_string[MAX_DATE_SIZE] = '\0';

  // Make folder name
  size_t date_size =
      strftime(date_string, MAX_DATE_SIZE, conf->date_format, &curr_time);
  if (date_size == MAX_DATE_SIZE) {
    return EXIT_FAILURE;
  }


  const size_t n = strlen(conf->folder_prefix) + date_size + 1;
  folder_name = calloc(n, sizeof(char));

  snprintf(folder_name, n, "%s%s", conf->folder_prefix, date_string);

  return EXIT_SUCCESS;
}

static int make_backups_sub(const backup_config *conf) {
  char *folder_name = NULL;

  // Get current time
  time_t curr_unix_time;
    tzset();
    curr_unix_time = time(NULL);

  if (get_folder_name(folder_name, conf, curr_unix_time)!=EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
