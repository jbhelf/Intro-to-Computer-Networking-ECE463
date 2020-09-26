#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>

#define LISTENQ 10
#define BMAX 512
#define PACK 100

#define OK "HTTP/1.0 200 OK\r\n\r\n"
#define NF "HTTP/1.0 404 Not Found\r\n\r\n"
#define FR "HTTP/1.0 403 Forbidden\r\n\r\n"

#define MAX(A, B) (A > B)?A:B

/* THINGS NEEDED TO DO BY TONIGHT:
    1. Run through every single command that sends information and print each argument
        - Make sure to check subarguments/attributes
        - If printing doesnt work, use the command being used (write(), send(), etc.)
    2. What we want to update:
        - We want the Ping_ack_X hostname to be a string, not an IP
        - We want the seq number to increment by one for the ping
    3. Check class notes and see if theres any info hidden there (UDP Stuff)
*/

int open_listenfd(int port){ 
    int listenfd, optval=1; 
    struct sockaddr_in serveraddr; 
   
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        return -1; 
  
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) 
        return -1; 
 
    bzero((char *) &serveraddr, sizeof(serveraddr)); 
    serveraddr.sin_family = AF_INET;  
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    serveraddr.sin_port = htons((unsigned short)port);  
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
        return -1; 
 
    if (listen(listenfd, LISTENQ) < 0) 
        return -1; 
 
    return listenfd; 
}

int udp_open_listenfd(int port){
    int listenfd = -1, optval = 1;
    struct sockaddr_in serveraddr;

    if ((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return -1;
        
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0)
        return -1; 
        
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    
    return listenfd;
}

void encrypt(char * buf, int shift){
    //It is assumed that only letters (uppercase and lowercase) are encrypted, and numbers, special chars, etc. are not
    for(int i = 0;i < BMAX - 1;i++){
        if(buf[i] >= 97 && buf[i] <= 122){
            buf[i] = (char)(buf[i] - shift);
            if(buf[i] < 97){
                buf[i] = (buf[i] - 97) + 122;
            }
        }else if(buf[i] >= 65 && buf[i] <= 90){
            buf[i] = (char)(buf[i] - shift);
            if(buf[i] < 65){
                buf[i] = (buf[i] - 65) + 90;
            }
        }
    }
}

void echo(int connfd){
    char buf[BMAX];
  
    if(read(connfd, buf, BMAX) > 0){
        //Path Generation:
        char * path;
        strtok(buf, " ");
        path = strtok(NULL, " ") + 1; 
        //Find the caesar cypher shift next:
        int shift = atoi(strtok(NULL, " "));

        if(!access(path, R_OK)){
            write(connfd, OK, 23);
            FILE *fhlr = fopen(path, "r");
            while(!feof(fhlr)){
	            bzero(buf,BMAX);
	            fread(buf, sizeof(char), BMAX-1, fhlr);
                encrypt(buf, shift); //encrypt here
	            write(connfd, buf, strlen(buf));
	        }
            fclose(fhlr);
        }else if(!access(path, 13)){ //Check if access forbidden
	        write(connfd, FR, 30);
	    }else{ //Only other option is file not found
	        write(connfd, NF, 30);
	    }
    }
} 

int main(int argc, char **argv) {
    int listenfd, connfd, clientlen, udp_lis;
    struct sockaddr_in clientaddr, udp_client;
    struct hostent *hp;
    char *haddrp, buf[PACK];
    fd_set rfds;
    socklen_t lSock;
    int hostname[BMAX];

    listenfd = open_listenfd(atoi(argv[1])); 
    udp_lis = udp_open_listenfd(atoi(argv[2]));

    while(1){
        FD_ZERO(&rfds);
        FD_SET(listenfd, &rfds);
        FD_SET(udp_lis, &rfds);        
   
        int max = MAX(listenfd, udp_lis) + 1;

        if(select(max, &rfds, NULL, NULL, NULL) < 0){
            return EXIT_FAILURE;
        }
        //printf("%d", max);

        if((FD_ISSET(udp_lis, &rfds))){ //UDP
            bzero(hostname, BMAX);
            int p = recvfrom(udp_lis, buf, PACK, 0, (struct sockaddr *)&udp_client, &lSock);
            *((uint32_t *)buf) = htonl(ntohl(*((uint32_t *)buf)) + 1);
            sendto(udp_lis, buf, p, 0, (struct sockaddr *)&udp_client, lSock);
        }else if (FD_ISSET(listenfd, &rfds)){ //HTTP
            clientlen = sizeof(clientaddr); 
            connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
            hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
            haddrp = inet_ntoa(clientaddr.sin_addr);
            if(fork() == 0){
                close(listenfd); 
                echo(connfd);
                exit(0);
            }

        close(connfd);
        } 
  }

  return EXIT_SUCCESS;
}
