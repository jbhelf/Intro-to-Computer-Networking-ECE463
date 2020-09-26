#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#define BMAX 512


int open_clientfd(char * sName, int sPort){
    int clientfd; 
  struct hostent *hp; 
  struct sockaddr_in serveraddr; 
 
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    return -1; /* check errno for cause of error */ 
 
  /* Fill in the server's IP address and port */ 
  if ((hp = gethostbyname(sName)) == NULL) 
    return -2; /* check h_errno for cause of error */ 
  bzero((char *) &serveraddr, sizeof(serveraddr)); 
  serveraddr.sin_family = AF_INET; 
  bcopy((char *)hp->h_addr,  
        (char *)&serveraddr.sin_addr.s_addr, hp->h_length); 
  serveraddr.sin_port = htons(sPort); 
 
  /* Establish a connection with the server */ 
  if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
    return -1; 
  return clientfd;
}

int checkEcode(char * buf){
    char eCode[3];
    char * hlr = strstr(buf, " "); //error code appears after 1st space
    for(int i = 0;i < 3;i++){
        eCode[i] = hlr[i + 1];
    }
    
    if(atoi(eCode) != 200){
        int i = 0;
        printf("Error: ");
        while(hlr[i] != '\n'){
            printf("%c", hlr[i++]);
        }
        return 1;
    }
    return 0;
}

int conCliSer(int * clientfd, char * buf, int sPort, char * sName){
    *clientfd = open_clientfd(sName, sPort);
    
    if(*clientfd < 0){ //Ensure that connection is successful
        printf("Error: connection failed!");
        return 1;
    }

    write(*clientfd, buf, strlen(buf)); //write buffer to feed   

    return 0;
}

int main(int argc, char ** argv){
    /*Arguments:
        1. Server Name (dtunes.ecn.purdue.edu)
        2. Server Port (80)
        3. Path Name (/ece463/lab1/path_short.txt)
    */

    if(argc != 5){
        printf("%d is not the correct number of arguments!", argc);
        return EXIT_FAILURE;
    }
    char * sName = argv[1], * pName = argv[3], buf[BMAX];
    int sPort = atoi(argv[2]), clientfd;
    int shift = atoi(argv[3]);
    bzero(buf, BMAX); //initialize buf values
    sprintf(buf, "GET %s %d HTTP/1.0\r\n\r\n", pName, shift); //get request 1

    //connect to client server
    if(conCliSer(&clientfd, buf, sPort, sName)){
        return EXIT_FAILURE;
    }

    if(!read(clientfd, buf, BMAX)){ //read feed into buffer
        printf("Error: Failed to read the client feed to the buffer!");
        return EXIT_FAILURE;
    }
    
    //Get error code, and check if 200:
    if(checkEcode(buf)){
        return EXIT_FAILURE;
    }

    //Print out server response:
    printf("%s", buf);

    //Get new path:
    char * pNew;
    if((pNew = strstr(buf, "\r\n\r\n")) != NULL){
        pNew += sizeof(char) * 4;
    }

    int p = 0;
    while (pNew[p] != '\n'){
      p++;
    }
    pNew[p] = '\0'; //terminate end of path

    sprintf(buf, "GET %s %d HTTP/1.0\r\n\r\n", pName, shift); //get request 2

    if(conCliSer(&clientfd, buf, sPort, sName)){
        return EXIT_FAILURE;
    }
    bzero(buf, BMAX);

    while (read(clientfd, buf, BMAX-1)) {
        printf("%s", buf);
        bzero(buf, BMAX);
    }
    
    return EXIT_SUCCESS;
}
