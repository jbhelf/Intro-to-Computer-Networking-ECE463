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

#define OK "HTTP/1.0 200 OK\r\n\r\n"
#define NF "HTTP/1.0 404 Not Found\r\n\r\n"
#define FR "HTTP/1.0 403 Forbidden\r\n\r\n"


int open_listenfd(int port){ 
    int listenfd, optval=1; 
    struct sockaddr_in serveraddr; 
   
    /* Create a socket descriptor */ 
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        return -1; 
  
    /* Eliminates "Address already in use" error from bind. */ 
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) 
        return -1; 
 
    /* Listenfd will be an endpoint for all requests to port on any IP address for this host */ 
    bzero((char *) &serveraddr, sizeof(serveraddr)); 
    serveraddr.sin_family = AF_INET;  
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    serveraddr.sin_port = htons((unsigned short)port);  
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
        return -1; 
 
    /* Make it a listening socket ready to accept connection requests */ 
    if (listen(listenfd, LISTENQ) < 0) 
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
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;

    port = atoi(argv[1]); /* the server listens on a port passed on the command line */
    listenfd = open_listenfd(port); 

    while (1) {
        clientlen = sizeof(clientaddr); 
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        hp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        haddrp = inet_ntoa(clientaddr.sin_addr);

        if(!fork()){
            close(listenfd);
            echo(connfd);
            exit(0);
        }
        close(connfd);
  }
}

