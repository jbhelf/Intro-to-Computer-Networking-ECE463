  /*
   *  ECE 463 Introduction to Computer Networks LAB 2 SOURCE FILE
   *
   *  File Name: unit-test.c
   *
   *  Purpose: Contains Unit test cases to test the functions in routingtable.c 
   *
   */


#include "ne.h"
#include "router.h"
#define WrongPath -2
#define Pass 1
#define MyAssert(X,Y) ASSERT(__FILE__,__FUNCTION__,__LINE__,X,Y)

void ASSERT(char *file,const char *func,int line,int x,char *y) {
    if(x == 0) {
       printf("Assert failed:file %s, function %s, line %i\n",file,func,line);
       printf("**%s**\n",y);
       exit(1);
    }
}
int checkPath(struct pkt_RT_UPDATE* updpkt,int dest, int *correctPath){
	int i,nbr;
	nbr=999;
	MyAssert(updpkt->no_routes <= MAX_ROUTERS,"wrong no_routes");
    for(i=0; i < updpkt->no_routes; i++) {
        if(updpkt->route[i].dest_id == dest) {
           nbr = i;
        }
    }
	MyAssert(updpkt->route[nbr].path_len <= MAX_PATH_LEN,"wrong path_len");
    for (i=0; i < updpkt->route[nbr].path_len;i++){		

    	if(correctPath[i]!=updpkt->route[nbr].path[i])
    		return WrongPath;
    }
    return Pass;
}
struct pkt_INIT_RESPONSE nbrs;
int MyRouterId = 0;
void printPacket( struct pkt_RT_UPDATE* updpkt){

	printf("******************************\n");
	printf("sender id: %d \nno. routers: %d\n",updpkt->sender_id,updpkt->no_routes);



	int i,j;
	MyAssert(updpkt->no_routes <= MAX_ROUTERS,"wrong no_routes");
	for(i = 0; i < updpkt->no_routes; i++){
		printf("Route[%d]:\n",i);
		printf("dest id: %d\nnext hop: %d\ncost: %d\npath len: %d\n",updpkt->route[i].dest_id, updpkt->route[i].next_hop,updpkt->route[i].cost,updpkt->route[i].path_len);
		//memcpy(UpdatePacketToSend->route[i].path, routingTable[i].path, MAX_ROUTERS);
		printf("path: ");
		MyAssert(updpkt->route[i].path_len <= MAX_PATH_LEN,"wrong path_len");
		for (j=0;j<updpkt->route[i].path_len-1;j++){

			printf("R%d ->",updpkt->route[i].path[j]);

		}
		printf("R%d\n",updpkt->route[i].path[updpkt->route[i].path_len-1]);
		printf("--------------------\n");
	}
	printf("******************************\n");
}
int TestInitRT() {

    int i;
    int nbr1 = 999;
    int nbr2 = 999;
    struct pkt_RT_UPDATE resultpkt;
    nbrs.no_nbr = 2;
    nbrs.nbrcost[0].nbr = 1;
    nbrs.nbrcost[0].cost = 4;
    nbrs.nbrcost[1].nbr = 2;
    nbrs.nbrcost[1].cost = 3;
    
    InitRoutingTbl (&nbrs, MyRouterId);
    ConvertTabletoPkt(&resultpkt, MyRouterId);
    printPacket(&resultpkt);
    MyAssert(resultpkt.no_routes==3,"Incorrect number of routes after initializing the routing table");
    for(i=0; i<resultpkt.no_routes; i++) {
        if(resultpkt.route[i].dest_id == 1) {
           nbr1 = i;
        }
        if(resultpkt.route[i].dest_id == 2) {
           nbr2 = i;
        }
    }
    MyAssert(nbr1!=999,"A Neighbor not found after initializing the routing table");
    MyAssert((resultpkt.route[nbr1].next_hop==1 && resultpkt.route[nbr1].cost==4),"Incorrect next hop or cost to a neighbor after initializing the routing table");
    MyAssert(nbr2!=999,"A Neighbor not found after initializing the routing table");
    MyAssert((resultpkt.route[nbr2].next_hop==2 && resultpkt.route[nbr2].cost==3),"Incorrect next hop or cost to a neighbor after initializing the routing table");
    return 0;
}


int TestNewRoute() {

    int i;
    int nbrnew = 999, nbrold=999;

    struct pkt_RT_UPDATE updpkt, resultpkt; 
 
    updpkt.sender_id = 1;   
    updpkt.dest_id = 0;
    updpkt.no_routes = 2;
    updpkt.route[0].dest_id = 2;        // Shouldn't be updated on this route
    updpkt.route[0].next_hop = 2;
    updpkt.route[0].cost = 40;
    updpkt.route[0].path_len=2;
    updpkt.route[0].path[0]=1;
    updpkt.route[0].path[1]=2; 

    updpkt.route[1].dest_id = 4;        // New dest not known previously
    updpkt.route[1].next_hop = 4;
    updpkt.route[1].cost = 5;
    updpkt.route[1].path_len=2;
    updpkt.route[1].path[0]=1;
    updpkt.route[1].path[1]=4;
    MyAssert(UpdateRoutes(&updpkt, nbrs.nbrcost[0].cost, MyRouterId) == 1, "Router doesn't return the correct Update Flag after adding a new destination");

    ConvertTabletoPkt(&resultpkt, MyRouterId);
    printPacket(&resultpkt);
    MyAssert(resultpkt.no_routes==4,"Incorrect number of routes after adding a new destination");
    for(i=0; i<resultpkt.no_routes; i++) {
        if(resultpkt.route[i].dest_id == 2) {   
           nbrold = i;
        } else if(resultpkt.route[i].dest_id == 4) {   
           nbrnew = i;
        }
    }
    MyAssert(nbrnew!=999,"Router didn't add route to a new destination that was previously unknown");
    
    MyAssert((resultpkt.route[nbrnew].next_hop==1 && resultpkt.route[nbrnew].cost==9),"Incorrect next hop or cost to a newly added destination");
    MyAssert((resultpkt.route[nbrold].next_hop==2 && resultpkt.route[nbrold].cost==3),"Incorrect next hop or cost to a previously existing destination");

    int path[]={0,1,4};
    MyAssert(checkPath(&resultpkt, 4, path)==Pass,"wrong path");

    return 0;
}

int TestDVUpdate() {

    int i;
    int nbr = 999;
    struct pkt_RT_UPDATE updpkt, resultpkt;   

    updpkt.sender_id = 2;
    updpkt.dest_id = 0;
    updpkt.no_routes = 1;
    updpkt.route[0].dest_id = 4;
    updpkt.route[0].next_hop = 8;
    updpkt.route[0].cost = 2;
    updpkt.route[0].path_len=3;
    updpkt.route[0].path[0]=2;
    updpkt.route[0].path[1]=8;
    updpkt.route[0].path[2]=4;
    MyAssert(UpdateRoutes(&updpkt, nbrs.nbrcost[1].cost, MyRouterId)==1, "Router doesn't return the correct Update Flag on finding another shortest path");
    ConvertTabletoPkt(&resultpkt, MyRouterId);
    printPacket(&resultpkt);
    MyAssert(resultpkt.no_routes==4,"Incorrect number of routes while testing shortest path calculation");
    for(i=0; i<resultpkt.no_routes; i++) {
        if(resultpkt.route[i].dest_id ==4) {
           nbr = i;
        }
    }
    MyAssert(nbr!=999,"A destination got removed on finding another shortest path to that destination");

    MyAssert((resultpkt.route[nbr].next_hop==2 && resultpkt.route[nbr].cost==5),"Incorrect next hop or cost to a destination on finding another shortest path");

    int path[]={0,2 ,8,4};
    MyAssert(checkPath(&resultpkt, 4, path)==Pass,"wrong path");
    return 0;
}

int TestForcedUpd() {

    int i;
    int nbr = 999;
    struct pkt_RT_UPDATE updpkt, resultpkt;   

    updpkt.sender_id = 2;
    updpkt.dest_id = 0;
    updpkt.no_routes = 1;
    updpkt.route[0].dest_id = 4;
    updpkt.route[0].next_hop = 10;
    updpkt.route[0].cost = 3;
    updpkt.route[0].path_len=3;
    updpkt.route[0].path[0]=2;
    updpkt.route[0].path[1]=10;
    updpkt.route[0].path[2]=4;
    MyAssert(UpdateRoutes(&updpkt, nbrs.nbrcost[1].cost, MyRouterId)==1, "Router doesn't return the correct Update Flag while testing forced update rule");
    ConvertTabletoPkt(&resultpkt, MyRouterId);
    printPacket(&resultpkt);
    MyAssert(resultpkt.no_routes==4,"Incorrect number of routes while testing forced update");
    for(i=0; i<resultpkt.no_routes; i++) {
        if(resultpkt.route[i].dest_id ==4) {
           nbr = i;
        }
    }
    MyAssert(nbr!=999,"A destination got removed while testing forced update");
    MyAssert((resultpkt.route[nbr].next_hop==2 && resultpkt.route[nbr].cost==6),"Incorrect next hop or cost to a destination while testing forced update rule");

    int path[]={0,2 ,10,4};
    MyAssert(checkPath(&resultpkt, 4, path)==Pass,"wrong path");
    return 0;
}

//NOTE: we are using path vector, which is a generalization of of split horizon.
int TestSplitHorizon() {

    int i;
    int nbr = 999;
    struct pkt_RT_UPDATE updpkt, resultpkt;   

    updpkt.sender_id = 2;
    updpkt.dest_id = 0;
    updpkt.no_routes = 1;
    updpkt.route[0].dest_id = 1;
    updpkt.route[0].next_hop = 20;
    updpkt.route[0].cost = 0;
    updpkt.route[0].path_len=4;
    updpkt.route[0].path[0]=2;
    updpkt.route[0].path[1]=20;
    updpkt.route[0].path[2]=0;
    updpkt.route[0].path[3]=1;
    
    MyAssert(UpdateRoutes(&updpkt, nbrs.nbrcost[1].cost, MyRouterId)==0, "Router doesn't return the correct Update Flag while testing split horizon rule");
    ConvertTabletoPkt(&resultpkt, MyRouterId);
    printPacket(&resultpkt);
    MyAssert(resultpkt.no_routes==4,"Incorrect number of routes while testing split horizon");
    for(i=0; i<resultpkt.no_routes; i++) {
        if(resultpkt.route[i].dest_id ==1) {
           nbr = i;
        }
    }
    MyAssert(nbr!=999,"A neighbor got removed while testing split horizon");
    MyAssert((resultpkt.route[nbr].next_hop==1 && resultpkt.route[nbr].cost==4),"Incorrect next hop or cost to a neighbor while testing split horizon rule");

    int path[]={0,1};
    MyAssert(checkPath(&resultpkt, 1, path)==Pass,"wrong path");
    return 0;
}



int main (int argc, char *argv[])
{

//Testing Initialization of Routing Table

    TestInitRT();
    printf("Test Case 1: PASS Initialized routing table\n");

//Testing New Route Update
    
    TestNewRoute();
    printf("Test Case 2: PASS Added new route to routing table\n");
   
//Testing Distance Vector Calculation

    TestDVUpdate();
    printf("Test Case 3: PASS Updated new shortest path\n");

//Testing Forced Update Rule

    TestForcedUpd();
    printf("Test Case 4: PASS Forced update rule taken care\n");

//Testing Split Horizon Rule

    TestSplitHorizon();
    printf("Test Case 5: PASS Split horizon rule taken care\n");

return 0;

}
