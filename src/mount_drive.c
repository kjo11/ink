#include "mount_drive.h"

#include <blkid/blkid.h>
#include <errno.h>
#include <mntent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>

#include "global.h"
#include "user-config.h"

#define MTAB_FILENAME "/etc/mtab"
#define FSTAB_FILENAME "/etc/fstab"

#define MAX_DRIVE_TYPE_STRING_LENGTH 100

#define MOUNT_ERROR "Error mounting drive. Try mounting it manually. Exiting.\n"

/******************************************************************************
 *                            FORWARD DECLARATIONS                            *
 ******************************************************************************/
// Read UUID of a drive mounted at conf->mount_point.
// Memory is allocated and needs to be freed.
static char *read_UUID_from_fstab(const backup_config *conf);

// Mount a drive mounted at conf->mount_point based on fstab.
static int mount_drive_from_fstab(const backup_config *conf);

/******************************************************************************
 *                                DEFINITIONS                                 *
 ******************************************************************************/

// -----------------------------------------------------------------------------
//                                                           is_drive_mounted
// -----------------------------------------------------------------------------
int is_drive_mounted(const backup_config *conf) {
  FILE *mtab = NULL;
  struct mntent *part = NULL;

  int is_mounted = 0;

// Make sure mount point is defined (no segfault with strcmp)
#ifndef NDEBUG
  if (conf->mount_point == NULL) {
    fprintf(stderr, "Mount point is not defined (NULL). Exiting.\n");
    return EXIT_FAILURE;
  }
#endif

  // Read through each entry in mtab and compare the mount points
  if ((mtab = setmntent(MTAB_FILENAME, "r"))) {
    while ((part = getmntent(mtab))) {
      if ((part->mnt_dir) && (strcmp(part->mnt_dir, conf->mount_point)) == 0) {
        is_mounted = 1;
        break;
      }
    }
    endmntent(mtab);
  }

  return is_mounted;
}

// -----------------------------------------------------------------------------
//                                                       read_UUID_from_fstab
// -----------------------------------------------------------------------------
static char *read_UUID_from_fstab(const backup_config *conf) {
  // If no UUID provided, read through fstab
  FILE *fstab = NULL;
  struct mntent *part = NULL;
  char *p_UUID = NULL;

  if ((fstab = setmntent(FSTAB_FILENAME, "r"))) {
    while ((part = getmntent(fstab))) {
      // Check if mount point is the correct one
      if (part->mnt_dir) {
        if ((strncmp(part->mnt_dir, conf->mount_point,
                     strlen(conf->mount_point))) == 0) {
          // If drive is given by UUID in fstab
          if (strncmp(part->mnt_fsname, "UUID=", strlen("UUID=")) == 0) {
            p_UUID = malloc(strlen(part->mnt_fsname) - strlen("UUID=") + 1);

            strcpy(p_UUID, part->mnt_fsname + strlen("UUID="));
            break;
          }
        }
      }
    }
  }
  endmntent(fstab);
  return p_UUID;
}

// -----------------------------------------------------------------------------
//                                                     mount_drive_from_fstab
// -----------------------------------------------------------------------------
static int mount_drive_from_fstab(const backup_config *conf) {
  // If no UUID provided, read through fstab
  FILE *fstab = NULL;
  FILE *mtab = NULL;
  struct mntent *part = NULL;

  if ((fstab = setmntent(FSTAB_FILENAME, "r"))) {
    while ((part = getmntent(fstab))) {
      // Check if mount point is the correct one
      if (part->mnt_dir) {
        if ((strncmp(part->mnt_dir, conf->mount_point,
                     strlen(conf->mount_point))) == 0) {
          // Try to mount drive
          if (mount(part->mnt_fsname, part->mnt_dir, part->mnt_type, 0, NULL) ==
              0) {
            if ((mtab = setmntent(MTAB_FILENAME, "a"))) {
              addmntent(mtab, part);
              endmntent(mtab);
            }
            endmntent(fstab);
            return EXIT_SUCCESS;
          }
        }
      }
    }
  }
  endmntent(fstab);
  return EXIT_FAILURE;
}

// -----------------------------------------------------------------------------
//                                                                mount_drive
// -----------------------------------------------------------------------------
int mount_drive(const backup_config *conf) {
  char *p_UUID = conf->UUID;

  // If no UUID given, attempt mounting from fstab
  if (conf->UUID == NULL) {
    return mount_drive_from_fstab(conf);
  }

  // Get name of device
  char *dev_name = blkid_evaluate_tag("UUID", p_UUID, NULL);
  if (!dev_name) return EXIT_FAILURE;

  blkid_probe pr = blkid_new_probe_from_filename(dev_name);
  if (!pr) return EXIT_FAILURE;

  // Get type of filesystem
  blkid_do_probe(pr);
  const char *type = malloc(MAX_DRIVE_TYPE_STRING_LENGTH);
  blkid_probe_lookup_value(pr, "TYPE", &type, NULL);

  // Attempt to mount
  DO_OR_DIE(mount(dev_name, conf->mount_point, type, 0, NULL));

  // Add entry in mtab if mounting successful
  FILE *mtab = NULL;
  struct mntent part;
  if ((mtab = setmntent(MTAB_FILENAME, "a"))) {
    part.mnt_fsname = dev_name;
    part.mnt_opts = "defaults";
    part.mnt_dir = conf->mount_point;
    part.mnt_type = (char *)type;
    part.mnt_freq = 0;
    part.mnt_passno = 0;
    addmntent(mtab, &part);
    endmntent(mtab);
  }

  // Free everything
  blkid_free_probe(pr);
  free((void *)type);

  FREE_NOT_NULL(p_UUID);

  return EXIT_SUCCESS;
}

// -----------------------------------------------------------------------------
//                                                              unmount_drive
// -----------------------------------------------------------------------------
int unmount_drive(backup_config conf) { return umount(conf.mount_point); }
