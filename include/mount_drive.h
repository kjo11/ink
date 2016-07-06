#ifndef MOUNT_DRIVE_H
#define MOUNT_DRIVE_H

#include "user-config.h"

/// Check if drive is mounted at conf->mount_point
int is_drive_mounted(const backup_config *conf);

/// Mount drive given by conf->UUID at conf->mount_point
int mount_drive(const backup_config *conf);

/// Unmount drive mounted at conf->mount_point
int unmount_drive(backup_config conf);

#endif
