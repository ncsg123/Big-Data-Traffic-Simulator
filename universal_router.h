/*
 *router.h desrcibe the structure of routeritch, it is seems like nic. 
 */

#ifndef _ROUTER_H
#define _ROUTER_H 

#include "common.h"
#include "parser.h"

#define ROUTER_XPORT_NUM            64	/* ports connect out NB */
#define ROUTER_FUNC_NUM             64     /* total function components */
#define ROUTER_XVC_NUM              2000 
#define FC_INTERLEAVE           2
#define OK                      0 
#define ERROR                   1
#define FALSE                   0
#define TRUE                    1
#define GROUP_NUM               16

#define TRACE_OPEN
#define FLOW_CONTROL

#define ROUTER_DELAY                15
//ROUT_DELAY+RECV_DELAY+ARBI_DELAY+TRANS_DELAY

struct router_mlist {
        htime_t tick;
        Q_t queue;
};

struct router_port {
       int router_vc_num;
       id_t port_id;                                          /* router port index */
       id_t local_id;                                         /* hcommu lib local_id */
       id_t dst_id;                                           /* hcommu lib dst_id */
       htime_t tick;                                          /* tick for output */
       htime_t la_tick;                                       /* out put tick will not less than this tick, it records most recently received packet's arrival tick. */
       num_t   credit[ROUTER_XVC_NUM];                            /* remote credit for each VC */
       struct router_mlist credit_for_increment[ROUTER_XVC_NUM];      /* credit from remote to update local credit */
       struct router_mlist recv_fifo[ROUTER_XVC_NUM];                 /* Input fifo, its tick is the schedule tick, neither head nor tail */
       struct router_mlist trans_fifo;                            /* Output fifo, its tick is the tail's tick */
       struct router *router;
       float input_bytes;
       float output_bytes;
       float fifo_used[ROUTER_XVC_NUM];                           //fifo runtime capacity
       int   priority[ROUTER_XVC_NUM];
       float  pkt_cnt; 
       float avg_delay;
       float pkt_delay;                                       //packet transmit delay
};

struct router {
	int n;
	num_t  router_port_num;                                   /* Number of interfaces with router/SW */
	num_t  router_vc_num;
	num_t  routernum;
	int frequency;
	int bus_width;
	int fifo_threshold;
	bool  barrier_priority_en;
	compt_id_t id;
	struct router_port port [ROUTER_XPORT_NUM];                   /* router port declaration */
	FILE *fp;
	htime_t tick;
	unsigned long index;
	int   priority_port;
	int   priority_vc;
	bool  barrier_enable;
	bool  multicast_enable;
	int   multicast_routing;
	float input_stat_tick;
	/*statistics information of universal_nic*/
	float input_bytes; 				      //bytes received 
	float output_bytes; 			              //bytes transmitted
	float input_stat_time;
	htime_t last_trace_time;
	unsigned int cmb_cnt[ROUTER_XPORT_NUM];
};

#endif /* _router_H */

extern router_desc_t *router_desc;
extern int counter_router;
extern bool router_handle_unicast(struct router *router, void *mini_pkt, id_t src_id, htime_t mini_tick);
extern void router_port_recv_pkt(void *vport, void *pkt, int current_port);
