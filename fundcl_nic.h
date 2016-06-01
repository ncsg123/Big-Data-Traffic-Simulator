#ifndef _FUNDCL_NIC
#define _FUNDCL_NIC

#include<stdio.h>
#include "common.h"

extern bool judge_vc(int vc);
extern void universal_addr_uni(unsigned int *dst, struct nic * nic);
extern void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src);
extern void init_unicast_pkt(struct nic *nic, unicast_pkth_t *netpkth, int current_vc);
extern void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth);
extern bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port);
extern void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *netpkth);
extern void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth);

#endif 
