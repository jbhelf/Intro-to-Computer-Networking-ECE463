#include "ne.h"
#include "router.h"

/* ----- GLOBAL VARIABLES ----- */
struct route_entry routingTable[MAX_ROUTERS];
int NumRoutes;

/*
Remember that route_entry contains:
	- dest_id = (final) destination router id
	- next_hop = next hop in shortest path
	- cost = cost to (final) router
*/


////////////////////////////////////////////////////////////////
void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID){
	/* ----- YOUR CODE HERE ----- */
	/*
	Remember that pck_INIT_RESPONSE contains:
		- no_nbr = the # of neighbors
		- nbrcost[] = array with cost and corresponding neighbor
			- nbr = neighbor id
			- cost = cost to neighbor
	*/

	//1st: Initialize the current router
	routingTable[0] = (struct route_entry){.cost = 0, .path_len = 1, .path[0] = myID, .dest_id = myID, .next_hop = myID};

	//2nd: Go through all other routers
	int i;
	for (i = 0; i < InitResponse->no_nbr; i++){
		routingTable[i+1] = (struct route_entry){.path_len = 2, .path[0] = myID, .path[1] = InitResponse->nbrcost[i].nbr, .dest_id = InitResponse->nbrcost[i].nbr, .next_hop = InitResponse->nbrcost[i].nbr, .cost = InitResponse->nbrcost[i].cost};
	}

	//3rd: Update the number of routes
	NumRoutes = InitResponse->no_nbr + 1;
}


////////////////////////////////////////////////////////////////
int UpdateRoutes(struct pkt_RT_UPDATE *RecvdUpdatePacket, int costToNbr, int myID){
	/* ----- YOUR CODE HERE ----- */
	/*
	Remember that pkt_RT_UPDATE contains:
		- sender_id
		- dest_id
		- no_routes = number of routes in table
		- route[] = contains rows of:
			- dest_id
			- next_hop
			- cost
	*/
  	int i, j, k, flag_f = 0, flag_c = 0, found_path = 0, flag_rep = 1, trial_path;
	struct route_entry pot_ent; //Potential entry

  	for(i = 0; i < RecvdUpdatePacket->no_routes; i++){ //For each packet recieved
    	pot_ent = RecvdUpdatePacket->route[i]; //Update potential entry
    	trial_path = (costToNbr + pot_ent.cost) <= INFINITY ?  (costToNbr + pot_ent.cost): INFINITY;
    	flag_f = 0;

    	for(j = 0; j < NumRoutes; j++){ //For each route
      		if(pot_ent.dest_id == routingTable[j].dest_id){ //If entry contained in routing table 
				found_path = 0; //Preemptively lower path flag
				flag_f = 1; //Raise found flag

				////////////////////////////////////////////////Path vector protocol
				for(k = 0; k < pot_ent.path_len;k++){ 
	  				if(myID == pot_ent.path[k]){ //ID is found in path
	    				found_path = 1;
						break;
	  				}
				}
				//////////////////////////////////////////////

				////////////////////////////////////////////////Split horizon rule
				if((trial_path < routingTable[j].cost) && (pot_ent.next_hop != myID) && !found_path){ 
	  				flag_c = 1;

					if(pot_ent.path_len < MAX_PATH_LEN){
			  			routingTable[j].path[0] = myID;
	    				routingTable[j].next_hop = RecvdUpdatePacket->sender_id;
	    				for(k = 0; k < routingTable[j].path_len; k++){
	      					routingTable[j].path[k + 1] = pot_ent.path[k];
	    				}
	    				routingTable[j].path_len = pot_ent.path_len + 1;
	    				routingTable[j].cost = trial_path;
					}else{
						routingTable[j].cost = INFINITY;
	  				}
				}
				//////////////////////////////////////////////

				////////////////////////////////////////////////Forced update rule
				if((routingTable[j].next_hop == RecvdUpdatePacket->sender_id) && (trial_path == INFINITY || !found_path)){ 
	  				for(k = 1; k < routingTable[j].path_len; k++){
	    				if(RecvdUpdatePacket->route[i].path[k-1] != routingTable[j].path[k]){
	      					flag_rep = 0;
							break;
	    				}
	  				}

	  				if(routingTable[j].cost != trial_path || (trial_path != INFINITY && flag_rep == 0)){
	    				flag_c = 1;
	    			
						if (pot_ent.path_len < MAX_PATH_LEN) {
							routingTable[j].path[0] = myID;
	      					for(k = 0; k < routingTable[j].path_len; k++){
								routingTable[j].path[k + 1] = pot_ent.path[k];
	     					}
	      					routingTable[j].path_len = pot_ent.path_len + 1;
	      					routingTable[j].cost = trial_path;	      
						}else{
							routingTable[j].cost = INFINITY;
	      				}
	  				}
        		}
				//////////////////////////////////////////////
      		}
    	}

		////////////////////////////////////////////////If entry was not present
    	if(!flag_f){ 
			flag_c = 1;
      		NumRoutes += 1;
      		
			routingTable[j].dest_id = pot_ent.dest_id;
      		routingTable[j].next_hop = RecvdUpdatePacket->sender_id;

			routingTable[j].path[0] = myID;
      		for(k = 0; k < pot_ent.path_len; k++){
        		routingTable[j].path[k + 1] = pot_ent.path[k];
      		}

      		routingTable[j].path_len = pot_ent.path_len + 1;     		
      		routingTable[j].cost = trial_path;
    	}
  	}
  	return flag_c;
}


////////////////////////////////////////////////////////////////
void ConvertTabletoPkt(struct pkt_RT_UPDATE *UpdatePacketToSend, int myID){
	/* ----- YOUR CODE HERE ----- */
	/*
	Remember that pkt_RT_UPDATE contains:
		- sender_id
		- dest_id
		- no_routes = number of routes in table
		- route[] = contains rows of:
			- dest_id
			- next_hop
			- cost
	*/
	int i;

	//Establish Packet Sender ID and number of routes:
	UpdatePacketToSend->sender_id = myID;
	UpdatePacketToSend->no_routes = NumRoutes;

	//Establish packet route->destination path:
	UpdatePacketToSend->dest_id = routingTable[myID].next_hop;

	for (i = 0; i < NumRoutes; i++){
	  UpdatePacketToSend->route[i] = routingTable[i];
	}
}


////////////////////////////////////////////////////////////////
//It is highly recommended that you do not change this function!
void PrintRoutes (FILE* Logfile, int myID){
	/* ----- PRINT ALL ROUTES TO LOG FILE ----- */
	int i = myID;
	int j = 0;
		
	for(i = 0; i < NumRoutes; i++){
		fprintf(Logfile, "<R%d -> R%d> Path: R%d", myID, routingTable[i].dest_id, myID);

		/*----- PRINT PATH VECTOR ----- */
		for(j = 1; j < routingTable[i].path_len; j++){
			fprintf(Logfile, " -> R%d", routingTable[i].path[j]);	
		}
		fprintf(Logfile, ", Cost: %d\n", routingTable[i].cost);
	}
	fprintf(Logfile, "\n");
	fflush(Logfile);
}


////////////////////////////////////////////////////////////////
void UninstallRoutesOnNbrDeath(int DeadNbr){
	/* ----- YOUR CODE HERE ----- */
	int i;
	for(i = 0;i < MAX_ROUTERS;i++){
		if(DeadNbr == routingTable[i].dest_id){ //For any router whose final hop is DeadNbr...
			routingTable[i].cost = INFINITY; //...Make cost infinitely large after death
		}else{ //Check for the dead router in path
			int j;
			for(j = 0; j < routingTable[i].path_len; j++){ //For each in current path...
				if(DeadNbr == routingTable[i].path[j]){
					routingTable[i].cost = INFINITY;
				}
			}
		}
	}
}
