#ifndef _PACKET_H
#define _PACKET_H

#include<stdio.h>
#include"simk.h"
#include"macro_select.h"

#define MAX_STAGE	        100	
#define mem_addr_t 		uint64_t
#define X			0
#define Y			1
#define Z			2

typedef unsigned long int usi;

typedef struct src_addr_packet{
    usi     	src_port[MAX_STAGE];
    usi     	src_vc[MAX_STAGE];
    usi     	hop_cnt;
    /*the univeral information for all packet*/
    usi     	DestPort;                            /* In sw or nic destination port number */
    usi     	TC;				     /* Virtural Channel number */
    usi		escape;	
    usi		add_up_cnt;			     /* Accumlation hop count for the packet */
    float     	stage_delay[MAX_STAGE];		     /* Statistic information for delay */
    usi		pkt_delay[2];			     /* input delay information! */
    len_t     	pkt_len;                             /* Payload Length */
    htime_t  	arrive_tick;                         /* Tick for this packet just arrived */
    num_t    	final;
    mem_addr_t 	src_addr;
    /****************************************/
}src_addr_packet_t;

typedef struct hld_dms_packet{
    usi		guide_bits;
    usi		strategy_id;
    usi		to_nic;
    usi		a_dest;
    usi		b_dest;
    usi		c_dest;
    usi		d_dest;
    usi		e_dest;
    /*the univeral information for all packet*/
    usi     	DestPort;                            /* In sw or nic destination port number */
    usi     	TC;				     /* Virtural Channel number */
    usi		escape;	
    usi		add_up_cnt;			     /* Accumlation hop count for the packet */
    float     	stage_delay[MAX_STAGE];		     /* Statistic information for delay */
    route_t	pkt_delay[2];			     /* input delay information! */
    len_t     	pkt_len;                             /* Payload Length */
    htime_t  	arrive_tick;                         /* Tick for this packet just arrived */
    num_t    	final;
    mem_addr_t 	src_addr;
    /****************************************/
}hld_dms_packet_t;

typedef struct flbfly_packet{
      usi		to_nic;
      usi		drt[10];
      /*the univeral information for all packet*/
      usi	     	DestPort;                            /* In sw or nic destination port number */
      usi    	 	TC;				     /* Virtural Channel number */
      usi		escape;	
      usi		add_up_cnt;			     /* Accumlation hop count for the packet */
      float     	stage_delay[MAX_STAGE];		     /* Statistic information for delay */
      route_t 		pkt_delay[2];			     /* input delay information! */
      len_t     	pkt_len;                             /* Payload Length */
      htime_t   	arrive_tick;                         /* Tick for this packet just arrived */
      num_t     	final;
      mem_addr_t 	src_addr;
    /****************************************/
}flbfly_packet_t;

#ifdef NETWORK_TYPE_FTREE
typedef src_addr_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORFT
typedef src_addr_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_VTORFT
typedef src_addr_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_FLBFLY
typedef flbfly_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_SFLBFLY
typedef flbfly_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORFB
typedef flbfly_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_D7K
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_D7K_DOUBLE_DEMENSION
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_D7K_VERSION1
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORUS
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_MESH
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_ALLTOALL
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORUS_NEW
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION 
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_TORUS_DOUBLE_HIGH_LOW 
typedef hld_dms_packet_t unicast_pkth_t;
#endif
#ifdef NETWORK_TYPE_MESH_ADAPTIVE
typedef hld_dms_packet_t unicast_pkth_t;
#endif

#endif
