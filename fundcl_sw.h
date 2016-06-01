#ifndef _FUNDCL_SW
#define _FUNDCL_SW

#include"macro_select.h"

#ifdef NETWORK_TYPE_FTREE
#endif

#ifdef NETWORK_TYPE_D7K
extern unsigned int d7k_route_teble[8];
#endif
#ifdef NETWORK_TYPE_D7K_DOUBLE_DIMENSION
extern unsigned int d7k_route_teble[8];
#endif

#ifdef NETWORK_TYPE_TORUS
extern unsigned int torus_route_teble[8];
#endif

#ifdef NETWORK_TYPE_MESH
#endif

#ifdef NETWORK_TYPE_ALLTOALL
#endif

extern void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port);
extern void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port);
extern void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth);

#endif
