/*
 * File name: common.h
 * Description: This file provide global variables.
 * Date:2014/3
 * */

#ifndef _COMMEN_H
#define _COMMEN_H

#include "simk.h"
#include "packet.h"

#define mem_addr_t		uint64_t
#define DMAEND                  3
#define LINE_DELAY	       	100
#define RECV_DELAY              4
#define ROUT_DELAY              2
#define ARBI_DELAY              4
#define TRANS_DELAY             4
#define ARBITER_ALGORITHM       2
#define VC_SHEME                0
#define NET_Q_LEN               80ULL           /*number of packet in the queue, will not overflow*/
#define BUSWIDTH                16

#define MAX_TIME (1ULL << (sizeof(htime_t) * 8 -1))

typedef struct fc_pkth_desc{
	num_t     pkt_vc;                  /* Virtual Channel ID*/
	num_t     credit;                  /* Credit value for arrive tick*/
	htime_t  arrive_tick;              /* Tick for this packet just arrived */
} fc_pkth_t;

typedef enum pkt_type_desc{
	NET_UNICAST,
	NET_BARRIER,
	NET_MULTICAST,
	NET_FLOWCONTROL,
}pkt_type_t;

struct pkt_list{
	struct list_head list;
};

typedef struct universal_unicast_pkth_desc{
	struct pkt_list pkt_list;
	unicast_pkth_t netpkth;
	mem_addr_t dst_addr;
	htime_t	clk;
	usi stage_delay[2];
	}universal_unicast_pkth_t;

/*packet opreations*/
#define pkt_type(pkt)	(*(pkt_type_t *)pkt)

/*auto get pkth is can just get script_unicast_t and so an*/
#define auto_get_pkth(pkt,pkth) do{\
	pkth = (typeof(pkth))((void*)pkt + sizeof(pkt_type_t));\
}while(0)

/*auto get net_paket which is define above such as unicast_path_de*/
#define auto_get_netpkth(pkt,pkth) do{\
	pkth = (typeof(pkth))((void *)pkt + sizeof(pkt_type_t) + sizeof(struct pkt_list)); \
}while(0)

#define pkth_to_pkt(pkth) (pkt_ptr_t)((void *)pkth - sizeof(pkt_ptr_t))

#define netpkth_to_netpkt(pkth) (pkt_ptr_t)((void *)pkth - sizeof(pkt_type_t) - sizeof (struct pkt_list))

/*Get pkt tick,the pkt is origin pkt,contains pkt_type,pkt_list and so on*/
static inline htime_t get_pkt_tick(void *pkt)
{
	switch (pkt_type(pkt)) {
		case NET_UNICAST:
			return ((unicast_pkth_t *)(pkt + sizeof(pkt_type_t) + sizeof(struct pkt_list)))->arrive_tick;
		case NET_BARRIER:
			return ((unicast_pkth_t *)(pkt + sizeof(pkt_type_t) + sizeof(struct pkt_list)))->arrive_tick;
		case NET_FLOWCONTROL:
			return ((fc_pkth_t *)(pkt + sizeof(pkt_type_t)))->arrive_tick;
		default:
			bug(ERR_UNHANDLED);
			return 0;
	}
}


/*Set pkt tick,same as Get pkt tick*/
static inline void set_pkt_tick(void *pkt, htime_t tick)
{
	switch(pkt_type(pkt)){
		case NET_UNICAST:
			((unicast_pkth_t *)(pkt + sizeof(pkt_type_t) + sizeof(struct pkt_list)))->arrive_tick = tick;
			break;
		case NET_MULTICAST:
		case NET_BARRIER:
		case NET_FLOWCONTROL:

		default:
			printf("ptr = %p",pkt);
			bug(ERR_UNHANDLED);
	}
}

static inline pkt_ptr_t head2pkt(struct pkt_list *list)
{
	return (pkt_ptr_t)((char *)list - sizeof(pkt_type_t));
}


static inline len_t get_pkt_buflen(void *pkt)
{
	void *pkth;
	switch(pkt_type(pkt)){
		case NET_UNICAST:
			pkth = pkt + sizeof(pkt_type_t) + sizeof(struct pkt_list);
			return ((unicast_pkth_t *)pkth)->pkt_len + sizeof(pkt_type_t);
		case NET_BARRIER:
		case NET_MULTICAST:
		case NET_FLOWCONTROL: break;
	}
}

#endif
