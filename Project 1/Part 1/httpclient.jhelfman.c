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

/* ALL THAT IS LEFT:
    
*/

int open_clientfd(char * sName, int sPort){
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    if((hp = gethostbyname(sName)) == NULL)
        return -2;

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr, (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(sPort);

    //to establish a connection
    if(connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}


int main(int argc, char ** argv){
    /*Arguments:
        1. Server Name (dtunes.ecn.purdue.edu)
        2. Server Port (80)
        3. Path Name (/ece463/lab1/path_short.txt)
    */

    if(argc != 4){
        printf("%d is not the correct number of arguments!", argc);
        return EXIT_FAILURE;
    }
    char * sName = argv[1]; //server name
    int sPort = atoi(argv[2]); //server port
    char * pName = argv[3]; //path name
    char buf[BMAX]; //buffer
    int clientfd; 

    bzero(buf, BMAX);

    //connect to the correct server port:

    clientfd = open_clientfd(sName, sPort);

    if(clientfd < 0){
        printf("Could not open connection!");
        return EXIT_FAILURE;
    }
    
    write(clientfd, buf, strlen(buf)); //write buffer to feed

    //send 1st GET request to server:
    sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", pName);
    int bRead = read(clientfd, buf, BMAX);

    //Get error code and check if 200:
    char eCode[3];
    for(int i = 0;i < 3;i++){
        eCode[i] += buf[9+i];
    } 
    if(atoi(eCode) != 200){
        printf("Error: %d\n\n", atoi(eCode));
        return EXIT_FAILURE;
    }

    //print server response:
    //fprintf(stdout, "%s", buf);
    return EXIT_SUCCESS;

    //---------------------------------ERRORS ARISE FROM THIS SECTION---------------------------------
    
    /*bzero(buf, BMAX);
    sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", pName);

    //connect to the correct server port:

    clientfd = open_clientfd(sName, sPort);

    if(clientfd < 0){
        printf("Could not open connection!");
        return EXIT_FAILURE;
    }
    
    write(clientfd, buf, strlen(buf)); //write buffer to feed

    //send 1st GET request to server:

    bRead = read(clientfd, buf, BMAX);

    //Get error code and check if 200:
    eCode[3];
    for(int i = 0;i < 3;i++){
        eCode[i] += buf[9+i];
    }

    if(atoi(eCode) != 200){
        printf("Error: %d\n\n", atoi(eCode));
        return EXIT_FAILURE;
    }

    //print server response:
    fprintf(stdout, "%s", buf); 

    
    //Reopen Connection With server:
    clientfd = open_clientfd(sName, sPort);

    if(clientfd < 0){
        printf("Could not open connection!");
        return EXIT_FAILURE;
    }
    
    write(clientfd, buf, strlen(buf)); //write buffer to feed

    //send 2nd GET request to server:
    sprintf(buf, "GET %s HTTP/1.0\r\n\r\n", pName);
    bRead = read(clientfd, buf, BMAX);

    //print complete server response
    fprintf(stdout, "%s", buf);
    
    close(clientfd);

    return EXIT_SUCCESS;*/
}
