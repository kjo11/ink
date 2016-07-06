#include "backups.h"
#include "user-config.h"

int main(void) {
  printf("Ink running.\n");
  backup_config conf = get_default_conf();
  conf.name = "katie";

  make_backups(&conf, stdout);
  return 0;
}
