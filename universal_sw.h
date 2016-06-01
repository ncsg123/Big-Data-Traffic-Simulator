/*
 *sw.h desrcibe the structure of switch, it is seems like nic. 
 */

#ifndef _SW_H
#define _SW_H 

#include "common.h"
#include "parser.h"

#define SW_XPORT_NUM            64	/* ports connect out NB */
#define SW_FUNC_NUM             40     /* total function components */
#define SW_XVC_NUM              2000 
#define FC_INTERLEAVE           2
#define OK                      0 
#define ERROR                   1
#define FALSE                   0
#define TRUE                    1
#define GROUP_NUM               16

#define TRACE_OPEN
#define FLOW_CONTROL

#define SW_DELAY                20
//ROUT_DELAY+RECV_DELAY+ARBI_DELAY+TRANS_DELAY

struct sw_mlist {
        htime_t tick;
        Q_t queue;
};

struct sw_port {
       int sw_vc_num;
       id_t port_id;                                          /* sw port index */
       id_t local_id;                                         /* hcommu lib local_id */
       id_t dst_id;                                           /* hcommu lib dst_id */
       htime_t tick;                                          /* tick for output */
       htime_t la_tick;                                       /* out put tick will not less than this tick, it records most recently received packet's arrival tick. */
       num_t   credit[SW_XVC_NUM];                            /* remote credit for each VC */
       struct sw_mlist credit_for_increment[SW_XVC_NUM];      /* credit from remote to update local credit */
       struct sw_mlist recv_fifo[SW_XVC_NUM];                 /* Input fifo, its tick is the schedule tick, neither head nor tail */
       struct sw_mlist trans_fifo;                            /* Output fifo, its tick is the tail's tick */
       struct sw *sw;
       float input_bytes;
       float output_bytes;
       float fifo_used[SW_XVC_NUM];                           //fifo runtime capacity
       int   priority[SW_XVC_NUM];
       float  pkt_cnt; 
       float avg_delay;
       float pkt_delay;                                       //packet transmit delay
};

struct sw {
	int n;
	num_t  sw_port_num;                                   /* Number of interfaces with sw/SW */
	num_t  sw_vc_num;
	num_t  swnum;
	int frequency;
	int bus_width;
	int fifo_threshold;
	bool  barrier_priority_en;
	compt_id_t id;
	struct sw_port port [SW_XPORT_NUM];                   /* sw port declaration */
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
	unsigned int cmb_cnt[SW_XPORT_NUM];
};

#endif /* _sw_H */

extern sw_desc_t *sw_desc;
extern int counter_sw;
extern bool sw_handle_unicast(struct sw *sw, void *mini_pkt, id_t src_id, htime_t mini_tick);
extern void sw_port_recv_pkt(void *vport, void *pkt, int current_port);
