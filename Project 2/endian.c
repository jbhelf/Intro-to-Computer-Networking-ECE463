  /*
   *  ECE 463 Introduction to Computer Networks LAB 2 SOURCE FILE
   *
   *  File Name: endian.c
   *
   *  Purpose: Defines routines that are useful in endian conversions 
   *
   */


#include "ne.h"

/*
 *  This function converts struct pkt_INIT_RESPONSE
 *  from network to host byte order.
 */
void ntoh_pkt_INIT_RESPONSE (struct pkt_INIT_RESPONSE *ne_resp) {

	int i;

	ne_resp->no_nbr = ntohl(ne_resp->no_nbr);

	for(i=0;i<(ne_resp->no_nbr);i++) {
		(ne_resp->nbrcost[i]).nbr = ntohl((ne_resp->nbrcost[i]).nbr);
		(ne_resp->nbrcost[i]).cost = ntohl((ne_resp->nbrcost[i]).cost);
	}
}

/*
 *  This function converts struct pkt_RT_UPDATE
 *  from host to network byte order.
 */
void hton_pkt_RT_UPDATE (struct pkt_RT_UPDATE *send_upd) {

	  int i;
	  int j;

	  send_upd->sender_id = htonl (send_upd->sender_id);
	  send_upd->dest_id = htonl (send_upd->dest_id);

	  for (i = 0; i < send_upd->no_routes; i++)
	  {
	    (send_upd->route[i]).dest_id = htonl ((send_upd->route[i]).dest_id);
	    (send_upd->route[i]).next_hop = htonl ((send_upd->route[i]).next_hop);
	    (send_upd->route[i]).cost = htonl ((send_upd->route[i]).cost);

	    for(j = 0; j < send_upd->route[i].path_len; j++){
	    	send_upd->route[i].path[j] = htonl(send_upd->route[i].path[j]);
	    }

	    send_upd->route[i].path_len = htonl(send_upd->route[i].path_len);
	  }

	  send_upd->no_routes = htonl (send_upd->no_routes);
}

/*
 *  This function converts struct pkt_RT_UPDATE
 *  from network to host byte order.
 */
void ntoh_pkt_RT_UPDATE (struct pkt_RT_UPDATE *recv_upd) {

	  int i;
	  int j;

	  recv_upd->sender_id = ntohl (recv_upd->sender_id);
	  recv_upd->dest_id = ntohl (recv_upd->dest_id);
	  recv_upd->no_routes = ntohl (recv_upd->no_routes);

	  for (i = 0; i < recv_upd->no_routes; i++)
	  {
	    (recv_upd->route[i]).dest_id = ntohl ((recv_upd->route[i]).dest_id);
	    (recv_upd->route[i]).next_hop = ntohl ((recv_upd->route[i]).next_hop);
	    (recv_upd->route[i]).cost = ntohl ((recv_upd->route[i]).cost);

	    recv_upd->route[i].path_len = ntohl(recv_upd->route[i].path_len);

	    for(j = 0; j < recv_upd->route[i].path_len; j++){
	    	recv_upd->route[i].path[j] = ntohl(recv_upd->route[i].path[j]);
	    }
	  }

}

