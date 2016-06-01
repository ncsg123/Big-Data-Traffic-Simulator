#ifndef _UNIVERSAL_NIC_H
#define _UNIVERSAL_NIC_H

#include "simk.h"
#include "common.h"
#include "parser.h"
#include <stdbool.h>

/* parallel nic structures */
#define NIC_XPORT_NUM 	   8   /*ports connect out NB*/
#define NIC_FUNC_NUM       40   /* total function components */
#define NIC_XVC_NUM        2000   /* vc counters per port, minimum value is 64 */
#define TRUE               1
#define FALSE              0
#define OK                 0
#define ERROR              1
#define MTU                2048

#define NIC_DELAY          12     // RECV_DELAY+ARBI_DELAY+TRANS_DELAY

#define SCRIPT_MTU         256 
#define THRESHOLD          39 
#define TRACE_OPEN
#define FLOW_CONTROL

struct nic_mlist {
   htime_t tick;
   Q_t queue;
};

struct nic_port {
   int nic_vc_num;
   id_t port_id;                                         /* nic port index */
   id_t local_id;                                        /* hcommu lib local_id */
   id_t dst_id;                                          /* hcommu lib dst_id */
   htime_t tick;                                         /* tick for output */
   htime_t la_tick;                                      /* out put tick will not less than this tick, it records most recently received packet's arrival tick. */
   num_t   credit[NIC_XVC_NUM];                          /* remote credit for each VC */
   struct nic_mlist credit_for_increment[NIC_XVC_NUM];   /* credit from remote to update local credit */
   struct nic_mlist recv_fifo[NIC_XVC_NUM];              /* Input fifo, its tick is the schedule tick, neither head nor tail */
   struct nic_mlist trans_fifo;                          /* Output fifo, its tick is the tail's tick */
   struct nic *nic;
   int   priority[NIC_XVC_NUM];
};

struct nic {
   int m;
   int n;
   num_t  nb_port_num;                                   /*Port connected to net bridge*/
   num_t  nic_port_num;                                  /*Number of interfaces with NIC/SW */
   num_t  nic_vc_num;                                    /*vc number*/
   struct nic_port port[NIC_XPORT_NUM];                  /* NIC port declaration */
   num_t frequency;					 /*one cycle time, unit (ns) */
   num_t bus_width;
   num_t fifo_depth;
   htime_t tick;					 /*time stamp of nic*/
   num_t vc_allocated;
   int   priority_port;
   int   priority_vc;
   id_t  id;
   unsigned int index;                                            /* index of the NIC among all NICs; starts at 0 */
   htime_t last_stat_tick;
   bool do_stat;
   /*statistics information of universal_nic*/
   FILE *fp;
   float output_bytes;
   float output_cnt;
   float input_bytes;
   float input_cnt;
   float input_delay;
   float stage_delay[MAX_STAGE];				/*every hop delay*/
   float hop_cnt;
   /****************************************/
};

extern int nic_num;
extern int inject_rate;
extern int g_pkt_length;
extern pkt_ptr_t universal_nic_make_unicast_pkt(struct nic *nic,int current_vc);
extern void universal_nic_form_packet(void *vport, void *pkt, int current_vc);
extern bool universal_nic_handle_unicast(struct nic *nic, void *mini_pkt, id_t src_id);
extern htime_t universal_nic_get_positive(htime_t a, htime_t b);
extern htime_t universal_nic_get_max(htime_t a, htime_t b, htime_t c);
extern void universal_nic_form_nic_packet(void *vport, void *pkt, int current_vc);
extern void universal_nic_port_recv_pkt(void *vport, void *pkt);
extern int counter_nic;
extern nic_desc_t *nic_desc;


#endif /* _UNIVERSAL_NIC_H */
