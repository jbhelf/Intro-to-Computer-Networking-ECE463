#include "ne.h"
#include "router.h"
#include <time.h>
#include <pthread.h>

struct NBR {
	int n_id, cost, dead;
	long last_update;
} NBR;

struct ARGS {
  	int fd, host, r_id;
	FILE * fhlr;
  	struct sockaddr_in ne_addr, r_addr;
}ARGS;

struct NBR neighbors[MAX_ROUTERS];

//Timekeepers
int n_count, conv_flag = 0;
unsigned int time_u = 0, time_c, time_s;
pthread_mutex_t lock;

FILE * logHlr;

void * receiver(void *arg){
  	int i, retval;
  	struct ARGS * args = arg;
  	struct pkt_RT_UPDATE p_update;

  	while (1) {
		retval = recvfrom(args->fd, &p_update, sizeof(p_update),0,NULL,NULL);
   		if(retval < 0){
      		printf("recvfrom failed\n");
    	}

    	ntoh_pkt_RT_UPDATE(&p_update);

    	pthread_mutex_lock(&lock); //Preemptively lock

    	for (i = 0; i < n_count; i++){
     		if(neighbors[i].n_id == p_update.sender_id){
				neighbors[i].dead = 0;
				neighbors[i].last_update = time(NULL);
				break;
      		}
    	}

    	if(UpdateRoutes(&p_update, neighbors[i].cost, args->r_id)){
      		time_c = time(NULL);
      		conv_flag = 0;
			PrintRoutes(logHlr, args->r_id);
    	}

    	pthread_mutex_unlock(&lock); //Unlock
  	}

  	return NULL;
}

void * sender(void *arg){
	int i, t_diff, retval;
  	struct pkt_RT_UPDATE p_update;
  	struct ARGS * args = arg;
  	socklen_t temp_adlen = sizeof(args->ne_addr);
	  
  	while (1){
    	pthread_mutex_lock(&lock); //Preemptively lock thread

		///////////////////Converge Section///////////////////
		t_diff = time(NULL) - time_c;
    	if(CONVERGE_TIMEOUT <= t_diff){
			t_diff = time(NULL) - time_s;
      		if(!conv_flag){
				time_c = time(NULL);
				conv_flag = 1;

				fprintf(logHlr, "%d:Converged\n", t_diff);
				fflush(logHlr);
      		}
    	}
		//////////////////////////////////////////////////////

		///////////////////Failure Section///////////////////
		for(i = 0; i < n_count; i++){
			t_diff = time(NULL) - neighbors[i].last_update;
      		if(FAILURE_DETECTION < t_diff){
				if(!neighbors[i].dead){
					time_c = time(NULL);
					conv_flag = 0;
					neighbors[i].dead = 1;

					UninstallRoutesOnNbrDeath(neighbors[i].n_id);
        			PrintRoutes(logHlr, args->r_id);
				}	
      		}
    	}
		/////////////////////////////////////////////////////

		///////////////////Update Section///////////////////
		t_diff = time(NULL) - time_u;
    	if(UPDATE_INTERVAL <= t_diff){
      		for(i = 0; i < n_count; i++){
				bzero(&p_update, sizeof(p_update));    
				ConvertTabletoPkt(&p_update, args->r_id);
				p_update.dest_id = neighbors[i].n_id;
				hton_pkt_RT_UPDATE(&p_update);
				
				retval = sendto(args->fd, &p_update, sizeof(p_update), 0, (struct sockaddr *)&(args->ne_addr), temp_adlen);
				if(retval < 0){
	  				printf("sendto failed\n");
				}

				time_u = time(NULL);
      		}
    	}
		////////////////////////////////////////////////////

    	pthread_mutex_unlock(&lock);
  	}

	return NULL;
}

int main(int argc, char **argv) {
	if (argc != 5){
		printf("invalid usage\n");
    	return EXIT_FAILURE;
  	}

  	int i, r_id = atoi(argv[1]), r_port = atoi(argv[4]), listenfd, ne_port = atoi(argv[3]), receive_len;
  	char * ne_host = argv[2], filename[11];
	struct hostent * ne = gethostbyname(ne_host);
  	struct pkt_INIT_REQUEST request_pck;
  	struct pkt_INIT_RESPONSE response_pck;
	struct sockaddr_in ne_addr, r_addr;
  	struct ARGS args;
  	socklen_t ne_addrlen = sizeof(ne_addr);
	pthread_t thread_r, thread_s;

  	if((MAX_ROUTERS - 1) < r_id || 0 > r_id){
    	printf("router ID bad\n");
    	return EXIT_FAILURE;
  	}

  	request_pck.router_id = htonl(r_id);

  	sprintf(filename, "router%d.log",r_id);
  	logHlr = fopen(filename,"w+");

	listenfd = socket(AF_INET, SOCK_DGRAM,0);
  	if(listenfd < 0){
    	printf("socket failed\n");
    	return EXIT_FAILURE;
  	}

  	if(ne == NULL){
    	printf("bad host\n");
		return EXIT_FAILURE;
  	}
	
	//Zero out addresses
  	bzero((char *) &r_addr, sizeof(r_addr));
  	bzero((char *) &ne_addr, sizeof(ne_addr));

	//Set router address properties
	r_addr.sin_family = AF_INET;
  	r_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
  	r_addr.sin_port = htons((unsigned short)r_port);

	//Set network address properties
  	ne_addr.sin_family = AF_INET;
  	memcpy((void *)&ne_addr.sin_addr, ne->h_addr_list[0], ne->h_length);
  	ne_addr.sin_port = htons((unsigned short)ne_port);

  	if(bind(listenfd, (struct sockaddr *)&r_addr, sizeof(r_addr))<0){
    	printf("bind failed\n");
    	close(listenfd);
    	return EXIT_FAILURE;
 	}
	 
  	if(sendto(listenfd, &request_pck, sizeof(request_pck),0,(struct sockaddr *)&ne_addr,ne_addrlen) < 0){
    	printf("sendto failed\n");
    	return EXIT_FAILURE;
  	}

  	receive_len = recvfrom(listenfd, &response_pck, sizeof(response_pck), 0, (struct sockaddr *)&r_addr, &ne_addrlen);
  	if(receive_len < 0){
    	printf("recvfrom failed\n");
    	return EXIT_FAILURE;
  	}

  	//Table initialization
  	ntoh_pkt_INIT_RESPONSE(&response_pck);
  	InitRoutingTbl(&response_pck, r_id);
  	n_count = response_pck.no_nbr;
  	PrintRoutes(logHlr, r_id); //Initialization printing step
  		
	time_s = time(NULL);
  	for (i = 0; i < response_pck.no_nbr; i++){
    	neighbors[i] = (struct NBR){.last_update = time(NULL), .n_id = response_pck.nbrcost[i].nbr, .cost = response_pck.nbrcost[i].cost, .dead = 0};
  	}
  	time_c = time(NULL);

  	args = (struct ARGS){.fd = listenfd, .ne_addr = ne_addr, .r_id = r_id, .fhlr = logHlr};

  	if(pthread_create(&thread_s, NULL, sender, (void *)&args)){
    	printf("Thread 1 failed to create\n");
    	return EXIT_FAILURE;
  	}

  	if(pthread_create(&thread_r, NULL, receiver, (void *)&args)){
    	printf("Thread 2 failed to create\n");
    	return EXIT_FAILURE;
  	}

  	pthread_join(thread_r, NULL);
  	pthread_join(thread_s, NULL);

  	return EXIT_SUCCESS;
}
