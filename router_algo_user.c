#include "universal_router.h"
#include "parser.h"
#include<stdbool.h>
#include<math.h>
#include "macro_select.h"

#define DEMENSION1
#define DATELINE0
#define VOQ


#ifndef NETWORK_TYPE_TORFT
#ifndef NETWORK_TYPE_VTORFT
void universal_router_getout_port(struct router_port *port,unicast_pkth_t *netpkth, int current_port)
{
}
void router_get_nexthop_portvc(struct router *router, unicast_pkth_t *pkth, id_t src_port)
{
}
void universal_router_send_pkt_dispose(struct router *router, unicast_pkth_t *netpkth)
{
}
#endif
#endif

#ifdef NETWORK_TYPE_TORFT
void universal_router_getout_port(struct router_port *port,unicast_pkth_t *netpkth, int current_port)
{
}

void router_get_nexthop_portvc(struct router *router, unicast_pkth_t *pkth, id_t src_port)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif

}

void universal_router_send_pkt_dispose(struct router *router, unicast_pkth_t *netpkth)
{
    int i;
    netpkth->hop_cnt -= 1;
    netpkth->TC = netpkth->escape;
    netpkth->DestPort = netpkth->src_port[1];
    for(i=1;i<MAX_STAGE-1;i++)
       netpkth->src_port[i] = netpkth->src_port[i+1];

}
#endif

#ifdef NETWORK_TYPE_VTORFT

void universal_router_getout_port(struct router_port *port,unicast_pkth_t *netpkth, int current_port)
{
}

void router_get_nexthop_portvc(struct router *router, unicast_pkth_t *pkth, id_t src_port)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif

}

void universal_router_send_pkt_dispose(struct router *router, unicast_pkth_t *netpkth)
{
    int i;
    netpkth->hop_cnt -= 1;
    netpkth->TC = netpkth->escape;
    netpkth->DestPort = netpkth->src_port[1];
    for(i=1;i<MAX_STAGE-1;i++)
       netpkth->src_port[i] = netpkth->src_port[i+1];

}
#endif

