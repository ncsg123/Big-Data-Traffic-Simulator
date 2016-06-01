#ifndef _FUNDCL_ROUTER
#define _FUNDCL_ROUTER

#include"macro_select.h"

#ifdef NETWORK_TYPE_FTREE
#endif

extern void universal_router_getout_port(struct router_port *port,unicast_pkth_t *netpkth, int current_port);
extern void router_get_nexthop_portvc(struct router *router, unicast_pkth_t *pkth, id_t src_port);
extern void universal_router_send_pkt_dispose(struct router *router, unicast_pkth_t *netpkth);

#endif
