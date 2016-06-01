/*
 * File name: sw_algo_user.c
 * Description: This file provide user defined routing algorithms.
 * Date: 2013/11
 * */
#include "universal_sw.h"
#include "sw_config_register.h"
#include "parser.h"
#include<stdbool.h>
#include<math.h>
#include "macro_select.h"

#define MESH_ADAPTIVE_2
#define DEMENSION1
#define DATELINE0
#define VOQ

#ifdef NETWORK_TYPE_FTREE
void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{

}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif

}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    int i;
    netpkth->hop_cnt -= 1;
    netpkth->TC = netpkth->escape;
    netpkth->DestPort = netpkth->src_port[1];
    for(i=1;i<MAX_STAGE-1;i++)
       netpkth->src_port[i] = netpkth->src_port[i+1];

}
#endif

#ifdef NETWORK_TYPE_TORFT
void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif

}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
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
void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif

}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    int i;
    netpkth->hop_cnt -= 1;
    netpkth->TC = netpkth->escape;
    netpkth->DestPort = netpkth->src_port[1];
    for(i=1;i<MAX_STAGE-1;i++)
       netpkth->src_port[i] = netpkth->src_port[i+1];

}
#endif

#ifdef NETWORK_TYPE_FLBFLY

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    struct sw *sw;
    sw_config_register * rgs;
    int k;
    k = dms[0];
    int i = 0;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];

    while(i < dms[1] -1)
    {
	if(rgs->x_id[i] > netpkth->drt[i])
	{
	    netpkth->DestPort = i * (k -1) + netpkth->drt[i];
	   // printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	if(rgs->x_id[i] < netpkth->drt[i])
	{
	    netpkth->DestPort = i * (k -1) + netpkth->drt[i] - 1;
	 //   printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	i++;
    }
   
    if(i == dms[1] -1 )
    {
	netpkth->DestPort = netpkth->to_nic;
    }
    for(i = 0; i < dms[1] -1; i++)
    {
	//printf("rgs->x_id[%d]= %d, netpkth->drt[%d] = %d, netpkth->DestPort = %d \n",i,rgs->x_id[i],i,netpkth->drt[i],netpkth->DestPort);
    }


}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
    pkth->escape = pkth->TC;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_SFLBFLY

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    struct sw *sw;
    sw_config_register * rgs;
    int i = 0;
    int j;
    int index = 0;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];

    while(i < dms[0])
    {

	if(rgs->x_id[i] > netpkth->drt[i])
	{
	    netpkth->DestPort = index + netpkth->drt[i];
	   // printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	if(rgs->x_id[i] < netpkth->drt[i])
	{
	    netpkth->DestPort = index + netpkth->drt[i] - 1;
	 //  printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	i++;
	index +=(dms[i] - 1);
    }

    if(i == dms[0] )
    {
	netpkth->DestPort = netpkth->to_nic;
    }
#ifdef VOQ
    if(i == dms[0] )
    {
	netpkth->escape = rand()%(sw->sw_port_num);
    }
    else
    {
	i++;
	index = 0;
	for(j = 0; j < i; j++)
	{
	    index +=(dms[j+1] - 1);
	}
	while(i < dms[0])
	{

	    if(rgs->x_id[i] > netpkth->drt[i])
	    {
		netpkth->escape = index + netpkth->drt[i];
		break;
	    }
	    if(rgs->x_id[i] < netpkth->drt[i])
	    {
		netpkth->escape = index + netpkth->drt[i] - 1;
		break;
	    }
	    i++;
	    index +=(dms[i] - 1);
	}
	if(i == dms[0] )
	{
	    netpkth->escape = netpkth->to_nic;
	}
     	bug_on(netpkth->TC != netpkth->DestPort,"voq wrong!netpkth->TC=%lu,netpkth->DestPort = %lu \n",netpkth->TC,netpkth->DestPort);
    }

#endif

}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORFB

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    struct sw *sw;
    sw_config_register * rgs;
    int i = 0;
    int j;
    int index = 0;
    int test_i;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];

    while(i < dms[0])
    {

	if(rgs->x_id[i] > netpkth->drt[i])
	{
	    netpkth->DestPort = index + netpkth->drt[i];
	   // printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	if(rgs->x_id[i] < netpkth->drt[i])
	{
	    netpkth->DestPort = index + netpkth->drt[i] - 1;
	 //  printf("netpkth->DestPort:%d\n",netpkth->DestPort);
	    break;
	}
	i++;
	index +=(dms[i] - 1);
    }

    if(i == dms[0] )
    {
	netpkth->DestPort = netpkth->to_nic;
    }
    test_i = i;
#ifdef VOQ
    if(i == dms[0] )
    {
	netpkth->escape = rand()%(sw->sw_port_num);
    }
    else
    {
	i++;
	index = 0;
	for(j = 0; j < i; j++)
	{
	    index +=(dms[j+1] - 1);
	}
	while(i < dms[0])
	{

	    if(rgs->x_id[i] > netpkth->drt[i])
	    {
		netpkth->escape = index + netpkth->drt[i];
		break;
	    }
	    if(rgs->x_id[i] < netpkth->drt[i])
	    {
		netpkth->escape = index + netpkth->drt[i] - 1;
		break;
	    }
	    i++;
	    index +=(dms[i] - 1);
	}
	if(i == dms[0] )
	{
	    netpkth->escape = netpkth->to_nic;
	}
     	bug_on(netpkth->TC != netpkth->DestPort,"voq wrong!netpkth->TC=%lu,netpkth->DestPort = %lu test_i = %d \n",netpkth->TC,netpkth->DestPort,test_i);
    }

#endif

}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_D7K
/* d7k sw user algo design here */
unsigned int d7k_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & d7k_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
    }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    //printf("j = %d\n",j);
    return j;
}

void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    int destport;
    unsigned int gbits;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    guide_bits_compute(rgs, netpkth);
    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
	destport = direction / 2;
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
//	printf("destport = %d\n",destport);
	if(direction % 2 == 0) //A+B+C+D+
	{
	    netpkth->DestPort = rgs->xp_pd[destport];
	    if(netpkth->DestPort>7)
	    {
		printf("sw_al_user1 destport %d,xp_dp[destport]%d",destport,rgs->xp_pd[destport]);
		printf("sw->index-nic_num%lu",sw->index - nic_num);
	    }
	    bug_on(netpkth->DestPort>7,">7");
	}
	else
	{
	    netpkth->DestPort = rgs->xn_pd[destport];
	    bug_on(netpkth->DestPort>7,"sw_al_user2");
	}
	
	if((gbits != netpkth->guide_bits) && (netpkth->escape ==0))
	{
	    netpkth->escape = 2 + rand()%(sw->sw_vc_num - 2);
	}
	if(rgs->x_id[0] == 0 && rgs->inner_id == 0 && ((current_port == 4 && direction == 0) || (current_port == 3 && direction == 1)))//A
	{ 
	    netpkth->escape = 0;
	//    printf("sadf1\n");
	}
	if(rgs->x_id[1] == 0 && rgs->inner_id == 1 && ((current_port == 4 && direction == 2) || (current_port == 3 && direction == 3)))//B
	{
	    netpkth->escape = 0;
	}
	if(rgs->x_id[2] == 0 && rgs->inner_id == 2 && ((current_port == 4 && direction == 4) || (current_port == 3 && direction == 5)))//C
	{
	    netpkth->escape = 0;
	}
	if(rgs->x_id[3] == 0 && rgs->inner_id == 3 && ((current_port == 4 && direction == 6) || (current_port == 3 && direction == 7)))//D
	{
	    netpkth->escape = 0;
	}
    }
    else
    {
	if(netpkth->e_dest == rgs->x_id[4])
	{
	    netpkth->DestPort = netpkth->to_nic;
	    bug_on(netpkth->DestPort>7,"sw_al_user ");
	}
	else if(netpkth->e_dest > rgs->x_id[4])
	{
	    netpkth->escape = 1;
	    netpkth->DestPort = netpkth->e_dest -1;
	    bug_on(netpkth->DestPort>7,"sw_al_user4");
	}
	else
	{
	    netpkth->escape = 1;
	    netpkth->DestPort = netpkth->e_dest;
	    bug_on(netpkth->DestPort>7,"sw_al_user5");
	}

    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif

#ifdef NETWORK_TYPE_TORUS 
/* torus sw user algo design here */
unsigned int torus_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & torus_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
  }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    return j;
}
void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    
    guide_bits_compute(rgs, netpkth);

    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
        netpkth->DestPort = direction;
	
	if(gbits != netpkth->guide_bits && netpkth->escape ==0)
	{
	    netpkth->escape = 1;
	}

	if(rgs->dataline[0] && ((current_port == 0 && direction  ==1)||(current_port == 1 && direction  == 0)))//A
	{ 
	    netpkth->escape = 0; 
	}
	if(rgs->dataline[1] && ((current_port == 2 && direction  ==3)||(current_port == 3 && direction  == 2 ) ))//B
	{
	    netpkth->escape = 0;
	}
	if(rgs->dataline[2] && ((current_port == 4 && direction  ==5)||(current_port == 5 && direction  == 4 ) ))//C
	{
	    netpkth->escape = 0;
	}
	if(rgs->dataline[3] && ((current_port == 6 && direction  ==7)||(current_port == 7 && direction  == 6 ) ))//D
	{
	    netpkth->escape = 0;
	}
    }
    else
    {
	netpkth->DestPort = 8;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif

#ifdef NETWORK_TYPE_MESH
/* mesh sw user algo design here */

void mesh_guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x7;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xB;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xD;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xE;
    }
}

int mesh_find_direction(unicast_pkth_t *netpkth)
{
    int i;
    unsigned int gbits;
    gbits = netpkth->guide_bits;
    for(i = 0; i < 4; i++)
    {
	if(gbits&0x8)
	{
	    break;
	}
	else
	{
	    gbits = gbits<<1;
	}
    }
    return i;
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    
    mesh_guide_bits_compute(rgs, netpkth);

    netpkth->escape = netpkth->TC;
    //netpkth->escape = (netpkth->TC+1)%2;
    //netpkth->escape = rand()%5;

    if(netpkth->guide_bits != 0)
    {
	direction = mesh_find_direction(netpkth);
	bug_on(direction>=4,"sw,direciton error!");
	if(direction == 0)
	{
	    if(netpkth->a_dest > rgs->x_id[0])
	    {
		netpkth->DestPort = 0;
	    }
	    else
	    {
		netpkth->DestPort = 1;
	    }
	}
	if(direction == 1)
	{
	    if(netpkth->b_dest > rgs->x_id[1])
	    {
		netpkth->DestPort = 2;
	    }
	    else
	    {
		netpkth->DestPort = 3;
	    }
	}
	if(direction == 2)
	{
	    if(netpkth->c_dest > rgs->x_id[2])
	    {
		netpkth->DestPort = 4;
	    }
	    else
	    {
		netpkth->DestPort = 5;
	    }
	}
	if(direction == 3)
	{
	    if(netpkth->d_dest > rgs->x_id[3])
	    {
		netpkth->DestPort = 6;
	    }
	    else
	    {
		netpkth->DestPort = 7;
	    }
	}
	
    }
    else
    {
	netpkth->DestPort = 8;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif

#ifdef NETWORK_TYPE_ALLTOALL
void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    struct sw *sw;
    unsigned int src,dst;
    sw = port->sw;
    src = sw->index - nic_num;
    dst = netpkth->strategy_id;
    if(src == dst)
    {
	netpkth->DestPort = 9;
    }
    if(src < dst)
    {
	netpkth->DestPort = dst - 1;
    }
    if(src > dst)
    {
	netpkth->DestPort = dst;
    }
   // printf("sw->index %d,src %d ,current_port %d,dst%d, netpkth->DestPort %d\n",sw->index,netpkth->guide_bits,current_port,dst, netpkth->DestPort);
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
    pkth->escape = pkth->TC;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS_NEW
/* torus sw user algo design here */
unsigned int torus_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

int get_drt_num(unsigned num)
{
    num = num << 24;
    int i;
    for(i =0; i < 8; i++)
    {
	if(num && 0x80000000)
	    return i;
	else
	    num <<= 1;
    }
    return i;
}

int find_direction(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    int i,j,k;
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    else
    {
	if(netpkth->guide_bits && 0xC0 == 0xC0 )
	{
	    i = rgs->x_id[0] - netpkth->a_dest;
	    j = abs(i);
	    if((j <=dms[0]/2 && i < 0) || (j > dms[0]/2 && i > 0 ))
	    {
		netpkth->guide_bits &= 0x7F;
	    }
	    else
	    {
		netpkth->guide_bits &= 0xBF;
	    }
	}
	return get_drt_num(netpkth->guide_bits);
    }

    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    else
    {
	if(netpkth->guide_bits && 0xC0 == 0xC0 )
	{
	    i = rgs->x_id[0] - netpkth->a_dest;
	    j = abs(i);
	    if((j <=dms[0]/2 && i < 0) || (j > dms[0]/2 && i > 0 ))
	    {
		netpkth->guide_bits &= 0x7F;
	    }
	    else
	    {
		netpkth->guide_bits &= 0xBF;
	    }
	}
	return get_drt_num(netpkth->guide_bits);


    }
    
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    else
    {
	if(netpkth->guide_bits && 0xC0 == 0xC0 )
	{
	    i = rgs->x_id[0] - netpkth->a_dest;
	    j = abs(i);
	    if((j <=dms[0]/2 && i < 0) || (j > dms[0]/2 && i > 0 ))
	    {
		netpkth->guide_bits &= 0x7F;
	    }
	    else
	    {
		netpkth->guide_bits &= 0xBF;
	    }
	}
	return get_drt_num(netpkth->guide_bits);

    }

    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
    else
    {
	if(netpkth->guide_bits && 0xC0 == 0xC0 )
	{
	    i = rgs->x_id[0] - netpkth->a_dest;
	    j = abs(i);
	    if((j <=dms[0]/2 && i < 0) || (j > dms[0]/2 && i > 0 ))
	    {
		netpkth->guide_bits &= 0x7F;
	    }
	    else
	    {
		netpkth->guide_bits &= 0xBF;
	    }
	}
	return get_drt_num(netpkth->guide_bits);

    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    
    guide_bits_compute(rgs, netpkth);

    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
        netpkth->DestPort = direction;
	
	if(rgs->dataline[0] && ((current_port == 0 && direction  ==1)||(current_port == 1 && direction  == 0)))//A
	{ 
	    netpkth->escape = rand()%2;
	//    printf("sadf1\n");
	}
	if(rgs->dataline[1] && ((current_port == 2 && direction  ==3)||(current_port == 3 && direction  == 2 ) ))//B
	{
	    netpkth->escape = rand()%2;
	  //  printf("sadf2\n");
	}
	if(rgs->dataline[2] && ((current_port == 4 && direction  ==5)||(current_port == 5 && direction  == 4 ) ))//C
	{
	    netpkth->escape = rand()%2;
	   // printf("sadf3\n");
	}
	if(rgs->dataline[3] && ((current_port == 6 && direction  ==7)||(current_port == 7 && direction  == 6 ) ))//D
	{
	    netpkth->escape = rand()%2;
	   // printf("sadf4\n");
	}
	if(gbits != netpkth->guide_bits && ((netpkth->escape ==0)||(netpkth->escape ==1)))
	{
	    netpkth->escape = rand()%4 + 2;
	}
    }
    else
    {
	netpkth->DestPort = 8;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif

#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION 
/* torus sw user algo design here */
unsigned int torus_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

#ifdef DEMENSION0
int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & torus_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
  }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    //printf("pkth->guide_bits: %x return direction :%d \n",pkth->guide_bits,j);
    return j;
}
#endif
#ifdef DEMENSION1
int find_direction(unicast_pkth_t *pkth)
{
    unsigned int tmp_bits = pkth->guide_bits;
    if(tmp_bits & 0xC0)
    {
	tmp_bits >>= 6;
	if(tmp_bits == 2)
	{
	    return 0;
	}
	else if( tmp_bits == 1)
	{
	    return 1;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction,a+a-");
	}

    }
    else if(tmp_bits & 0x30)
    {
	tmp_bits >>= 4;
	if(tmp_bits == 2)
	{
	    return 2;
	}
	else if( tmp_bits == 1)
	{
	    return 3;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0xC)
    {
	tmp_bits >>= 2;
	if(tmp_bits == 2)
	{
	    return 4;
	}
	else if( tmp_bits == 1)
	{
	    return 5;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0x3)
    {
	if(tmp_bits == 2)
	{
	    return 6;
	}
	else if( tmp_bits == 1)
	{
	    return 7;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else
    {
	return -1;
	bug_on(1,"in find_direction!");
    }
}
#endif

void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index - nic_num];
    gbits = netpkth->guide_bits;
    
    guide_bits_compute(rgs, netpkth);

    netpkth->escape = netpkth->TC;

//    printf("in sw_algo, current_vc %d\n",netpkth->TC);
    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
        netpkth->DestPort = direction;
	
/*	if(direction == 1)
	{
	    printf("current_port: %d \n",current_port);
	}
*/
	if(gbits != netpkth->guide_bits && (netpkth->escape <1))
	{
	    netpkth->escape = 1 + rand()%(sw->sw_vc_num - 1);
	}
#ifdef DATELINE0
	if(rgs->dataline[0] && ((current_port == 0 && direction  ==1)||(current_port == 1 && direction  == 0)))//A
	{ 
	    netpkth->escape = 0;
	}
	if(rgs->dataline[1] && ((current_port == 2 && direction  ==3)||(current_port == 3 && direction  == 2 ) ))//B
	{
	    netpkth->escape = 0;
	}
	if(rgs->dataline[2] && ((current_port == 4 && direction  ==5)||(current_port == 5 && direction  == 4 ) ))//C
	{
	    netpkth->escape = 0;
	}
	if(rgs->dataline[3] && ((current_port == 6 && direction  ==7)||(current_port == 7 && direction  == 6 ) ))//D
	{
	    netpkth->escape = 0;
	}
#endif
#ifdef DATELINE1
	if((rgs->x_id[0] == 0 && current_port == 0 && direction  ==1)||(rgs->x_id[0] == dms[0]-1 && current_port == 1 && direction  == 0))//A
	{ 
	    netpkth->escape = 0;
	}
	if((rgs->x_id[1] == 0 && current_port == 2 && direction  ==3)||(rgs->x_id[1] == dms[1]-1 && current_port == 3 && direction  == 2 ))//B
	{
	    netpkth->escape = 0;
	}
	if((rgs->x_id[2] == 0 && current_port == 4 && direction  ==5)||(rgs->x_id[2] == dms[2]-1 && current_port == 5 && direction  == 4 ))//C
	{
	    netpkth->escape = 0;
	}
	if((rgs->x_id[3] == 0 && current_port == 6 && direction  ==7)||(rgs->x_id[3] == dms[3]-1 && current_port == 7 && direction  == 6 ))//D
	{
	    netpkth->escape = 0;
	}
#endif 
    }
    else
    {
	netpkth->DestPort = netpkth->to_nic;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif
#ifdef NETWORK_TYPE_TORUS_DOUBLE_HIGH_LOW 
/* torus sw user algo design here */
unsigned int torus_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & torus_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
  }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    return j;
}
void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    
    guide_bits_compute(rgs, netpkth);

    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
        netpkth->DestPort = direction;
	
	if((current_port == 1 && direction  == 0 && rgs->x_id[0] != dms[0] -1)||(current_port == 0 && direction == 1 && rgs->x_id[0] == 0)\
		||(current_port == 2 && direction  == 3 && rgs->x_id[1] != dms[1] -1)||(current_port == 3 && direction == 2 && rgs->x_id[1] == 0)\
		||(current_port == 4 && direction  == 5 && rgs->x_id[2] != dms[2] -1)||(current_port == 5 && direction == 4 && rgs->x_id[2] == 0)\
		||(current_port == 6 && direction  == 7 && rgs->x_id[3] != dms[3] -1)||(current_port == 7 && direction == 6 && rgs->x_id[3] == 0))
	{
	    netpkth->escape = 1;
	}
	else
	{
	    netpkth->escape = 0;
	}
    }
    else
    {
	netpkth->DestPort = 8;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}

#endif

#ifdef NETWORK_TYPE_D7K_DOUBLE_DEMENSION
/* d7k sw user algo design here */
unsigned int d7k_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

#ifdef DEMENSION0
int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & d7k_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
    }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    //printf("j = %d\n",j);
    return j;
}
#endif

#ifdef DEMENSION1
int find_direction(unicast_pkth_t *pkth)
{
    unsigned int tmp_bits = pkth->guide_bits;
    if(tmp_bits & 0xC0)
    {
	tmp_bits >>= 6;
	if(tmp_bits == 2)
	{
	    return 0;
	}
	else if( tmp_bits == 1)
	{
	    return 1;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction,a+a-");
	}

    }
    else if(tmp_bits & 0x30)
    {
	tmp_bits >>= 4;
	if(tmp_bits == 2)
	{
	    return 2;
	}
	else if( tmp_bits == 1)
	{
	    return 3;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0xC)
    {
	tmp_bits >>= 2;
	if(tmp_bits == 2)
	{
	    return 4;
	}
	else if( tmp_bits == 1)
	{
	    return 5;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0x3)
    {
	if(tmp_bits == 2)
	{
	    return 6;
	}
	else if( tmp_bits == 1)
	{
	    return 7;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else
    {
	return -1;
	bug_on(1,"in find_direction!");
    }
}
#endif

void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    int destport;
    unsigned int gbits;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    guide_bits_compute(rgs, netpkth);
    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
	destport = direction / 2;
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
//	printf("destport = %d\n",destport);
	if(direction % 2 == 0) //A+B+C+D+
	{
	    netpkth->DestPort = rgs->xp_pd[destport];
	    if(netpkth->DestPort>5)
	    {
		printf("sw_al_user1 destport %d,xp_dp[destport]%d",destport,rgs->xp_pd[destport]);
		printf("sw->index-nic_num%lu",sw->index - nic_num);
	    }
	    bug_on(netpkth->DestPort>5,">5");
	}
	else
	{
	    netpkth->DestPort = rgs->xn_pd[destport];
	    bug_on(netpkth->DestPort>5,"sw_al_user2");
	}
	
	if((gbits != netpkth->guide_bits) && (netpkth->escape ==0))
	{
	    netpkth->escape = 1 + rand()%(sw->sw_vc_num - 1);
	}
	if(rgs->x_id[0] == 0 && rgs->inner_id == 0 && ((current_port == 4 && direction == 0) || (current_port == 3 && direction == 1)))//A
	{ 
	    netpkth->escape = 0;
	//    printf("sadf1\n");
	}
	if(rgs->x_id[1] == 0 && rgs->inner_id == 1 && ((current_port == 4 && direction == 2) || (current_port == 3 && direction == 3)))//B
	{
	    netpkth->escape = 0;
	}
	if(rgs->x_id[2] == 0 && rgs->inner_id == 2 && ((current_port == 4 && direction == 4) || (current_port == 3 && direction == 5)))//C
	{
	    netpkth->escape = 0;
	}
	if(rgs->x_id[3] == 0 && rgs->inner_id == 3 && ((current_port == 4 && direction == 6) || (current_port == 3 && direction == 7)))//D
	{
	    netpkth->escape = 0;
	}
    }
    else
    {
	if(netpkth->e_dest == rgs->x_id[4])
	{
	    netpkth->DestPort = netpkth->to_nic;
	    bug_on(netpkth->DestPort>7,"sw_al_user3");
	}
	else if(netpkth->e_dest > rgs->x_id[4])
	{
	    netpkth->escape = 0;
	    netpkth->DestPort = netpkth->e_dest -1;
	    bug_on(netpkth->DestPort>7,"sw_al_user4");
	}
	else
	{
	    netpkth->escape = 0;
	    netpkth->DestPort = netpkth->e_dest;
	    bug_on(netpkth->DestPort>7,"sw_al_user5");
	}

    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}
#endif
#ifdef NETWORK_TYPE_D7K_VERSION1
/* d7k sw user algo design here */
unsigned int d7k_route_table[8]=
{
    0x80,
    0xA0,
    0xA8,
    0xAA,
    0xEA,
    0xFA,
    0xFE,
    0xFF
};

#ifdef DEMENSION0
int find_direction(unicast_pkth_t *pkth)
{
   unsigned int tmp_bits   = 0;
   unsigned int bits_least = 0;
   unsigned int tmp_bits_cal;
   int i,j = 0;
   int num,numleast = 8;
   for(i = 0; i < 8 ; i++)
   {
	   tmp_bits = pkth->guide_bits & d7k_route_table[i] & 0xFF;
	   tmp_bits_cal = tmp_bits;
	   num = 0;
	   for(j = 0 ;j <8 ; j++)
	   {
		    if(1 == (tmp_bits_cal&0x1))
		    {
			    num++;
			    tmp_bits_cal = tmp_bits_cal>>1;
		    }
		    else
			    tmp_bits_cal = tmp_bits_cal>>1;
	   }
	   
	  if(num < numleast && num !=0)
	  {
		numleast = num;
		bits_least = tmp_bits;
	  }
	    
    }
    bits_least = bits_least<<24;
    bits_least &= 0xFF000000;
    j = 0;
    for(i = 0; i < 8 ; i++)
    {
	  if(!(bits_least&0x80000000))
	  {
		j++;
		bits_least<<=1;			
	  }
	  else
		  break;
    }
    bug_on(j==8||j<0,"route_error%d",j);
    //printf("j = %d\n",j);
    return j;
}
#endif

#ifdef DEMENSION1
int find_direction(unicast_pkth_t *pkth)
{
    unsigned int tmp_bits = pkth->guide_bits;
    if(tmp_bits & 0xC0)
    {
	tmp_bits >>= 6;
	if(tmp_bits == 2)
	{
	    return 0;
	}
	else if( tmp_bits == 1)
	{
	    return 1;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction,a+a-");
	}

    }
    else if(tmp_bits & 0x30)
    {
	tmp_bits >>= 4;
	if(tmp_bits == 2)
	{
	    return 2;
	}
	else if( tmp_bits == 1)
	{
	    return 3;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0xC)
    {
	tmp_bits >>= 2;
	if(tmp_bits == 2)
	{
	    return 4;
	}
	else if( tmp_bits == 1)
	{
	    return 5;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else if(tmp_bits & 0x3)
    {
	if(tmp_bits == 2)
	{
	    return 6;
	}
	else if( tmp_bits == 1)
	{
	    return 7;
	}
	else
	{
	    return -1;
	    bug_on(1,"in find_direction!");
	}
    }
    else
    {
	return -1;
	bug_on(1,"in find_direction!");
    }
}
#endif

void guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x3F;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xCF;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xF3;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xFC;
    }
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction;
    int destport;
    unsigned int gbits;

    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
    guide_bits_compute(rgs, netpkth);
    netpkth->escape = netpkth->TC;

    if(netpkth->guide_bits != 0)
    {
	direction = find_direction(netpkth);
	destport = direction / 2;
//	printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
//	printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
//	printf("destport = %d\n",destport);
	if(direction % 2 == 0) //A+B+C+D+
	{
	    netpkth->DestPort = rgs->xp_pd[destport];
	    if(netpkth->DestPort>5)
	    {
		printf("sw_al_user1 destport %d,xp_dp[destport]%d",destport,rgs->xp_pd[destport]);
		printf("sw->index-nic_num%lu",sw->index - nic_num);
	    }
	    bug_on(netpkth->DestPort>5,">5");
	}
	else
	{
	    netpkth->DestPort = rgs->xn_pd[destport];
	    bug_on(netpkth->DestPort>5,"sw_al_user2");
	}
/********************************************/	
/*change back vc*/
	if((gbits != netpkth->guide_bits) && (netpkth->escape ==0))
	{
	    netpkth->escape = 2 + rand()%2;
	}
	if((gbits != netpkth->guide_bits) && (netpkth->escape ==1))
	{
	    netpkth->escape = 4;
	}
/********************************************/

	if(rgs->x_id[0] == 0 && rgs->inner_id == 0 && ((current_port == 4 && direction == 0) || (current_port == 3 && direction == 1)))//A
	{ 
	    if(netpkth->escape ==2 || netpkth->escape ==3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4)
		netpkth->escape = 1;
	//    printf("sadf1\n");
	}
	if(rgs->x_id[1] == 0 && rgs->inner_id == 1 && ((current_port == 4 && direction == 2) || (current_port == 3 && direction == 3)))//B
	{
	    if(netpkth->escape ==2 || netpkth->escape ==3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4)
		netpkth->escape = 1;
	}
	if(rgs->x_id[2] == 0 && rgs->inner_id == 2 && ((current_port == 4 && direction == 4) || (current_port == 3 && direction == 5)))//C
	{
	    if(netpkth->escape ==2 || netpkth->escape ==3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4)
		netpkth->escape = 1;
	}
	if(rgs->x_id[3] == 0 && rgs->inner_id == 3 && ((current_port == 4 && direction == 6) || (current_port == 3 && direction == 7)))//D
	{
	    if(netpkth->escape == 2 || netpkth->escape ==3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4)
		netpkth->escape = 1;
	}
    }
    else
    {
	if(netpkth->e_dest == rgs->x_id[4])
	{
	    netpkth->DestPort = netpkth->to_nic;
	    bug_on(netpkth->DestPort>7,"sw_al_user3");
	}
	else if(netpkth->e_dest > rgs->x_id[4])
	{
	    if(netpkth->escape == 2 || netpkth->escape == 3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4) 
		netpkth->escape = 1;
	    netpkth->DestPort = netpkth->e_dest -1;
	    bug_on(netpkth->DestPort>7,"sw_al_user4");
	}
	else
	{
	    if(netpkth->escape == 2 || netpkth->escape == 3)
		netpkth->escape = 0;
	    if(netpkth->escape == 4) 
		netpkth->escape = 1;
	    netpkth->DestPort = netpkth->e_dest;
	    bug_on(netpkth->DestPort>7,"sw_al_user5");
	}

    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
//    pkth->escape = pkth->TC;
//    sw_config_register * rgs;
}

void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    //printf("sw      a %d, b %d, c %d, d %d,e %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4]);
    //printf("netpkth a %d ,b %d, c %d, d %d,e %d\n",netpkth->a_dest,netpkth->b_dest,netpkth->c_dest,netpkth->d_dest,netpkth->e_dest);
    //printf("destport = %d hopcnt %d\n",netpkth->DestPort,netpkth->add_up_cnt);
    netpkth->TC = netpkth->escape;
}
#endif
#ifdef NETWORK_TYPE_MESH_ADAPTIVE
void mesh_guide_bits_compute(sw_config_register *rgs, unicast_pkth_t *netpkth)
{
    if(rgs->x_id[0] == netpkth->a_dest)
    {
	netpkth->guide_bits &= 0x7;
    }
    if(rgs->x_id[1] == netpkth->b_dest)
    {
	netpkth->guide_bits &= 0xB;
    }
    if(rgs->x_id[2] == netpkth->c_dest)
    {
	netpkth->guide_bits &= 0xD;
    }
    if(rgs->x_id[3] == netpkth->d_dest)
    {
	netpkth->guide_bits &= 0xE;
    }
}

int mesh_find_direction(unicast_pkth_t *netpkth)
{
    int i;
    unsigned int gbits;
    gbits = netpkth->guide_bits;
    for(i = 0; i < 4; i++)
    {
	if(gbits&0x8)
	{
	    break;
	}
	else
	{
	    gbits = gbits<<1;
	}
    }
    return i;
}
void dms_drt_find(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    struct sw *sw;
    sw_config_register * rgs;
    int direction,i;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    gbits = netpkth->guide_bits;
	    direction = mesh_find_direction(netpkth);
	    bug_on(direction>=4,"sw,direciton error!");
	    if(direction == 0)
	    {
		if(netpkth->a_dest > rgs->x_id[0])
		{
		    netpkth->DestPort = 0;
		}
		else
		{
		    netpkth->DestPort = 1;
		}
	    }
	    if(direction == 1)
	    {
		if(netpkth->b_dest > rgs->x_id[1])
		{
		    netpkth->DestPort = 2;
		}
		else
		{
		    netpkth->DestPort = 3;
		}
	    }
	    if(direction == 2)
	    {
		if(netpkth->c_dest > rgs->x_id[2])
		{
		    netpkth->DestPort = 4;
		}
		else
		{
		    netpkth->DestPort = 5;
		}
	    }
	    if(direction == 3)
	    {
		if(netpkth->d_dest > rgs->x_id[3])
		{
		    netpkth->DestPort = 6;
		}
		else
		{
		    netpkth->DestPort = 7;
		}
	    }
	    

}
void mesh_adp_routing(struct sw_port * port, unicast_pkth_t* netpkth, sw_config_register *rgs,int current_port)
{
    struct sw *sw;
    sw = port->sw;
    unsigned int tbit = 1;
    int buflen = 0;
    int drt_adp = 0;
    int gbits = netpkth->guide_bits;
    /*
    if(netpkth->a_dest > rgs->x_id[0] && netpkth->b_dest > rgs->x_id[1])
    {
	if(rand()%2)
	{
	    netpkth->DestPort = 0;
	}
	else
	{
	    netpkth->DestPort = 2;
	}
    }
    if(netpkth->a_dest < rgs->x_id[0] && netpkth->b_dest > rgs->x_id[1])
    {
	if(rand()%2)
	{
	    netpkth->DestPort = 1;
	}
	else
	{
	    netpkth->DestPort = 2;
	}
    }
    if(netpkth->a_dest < rgs->x_id[0] && netpkth->b_dest < rgs->x_id[1])
    {
	if(rand()%2)
	{
	    netpkth->DestPort = 1;
	}
	else
	{
	    netpkth->DestPort = 3;
	}
    }
    if(netpkth->a_dest > rgs->x_id[0] && netpkth->b_dest < rgs->x_id[1])
    {
	if(rand()%2)
	{
	    netpkth->DestPort = 0;
	}
	else
	{
	    netpkth->DestPort = 3;
	}
    }
    if(netpkth->a_dest == rgs->x_id[0])
    {
	if(netpkth->b_dest > rgs->x_id[1])
	    netpkth->DestPort = 2;
	else
	    netpkth->DestPort = 3;
    }
    if(netpkth->b_dest == rgs->x_id[1])
    {
	if(netpkth->a_dest > rgs->x_id[0])
	    netpkth->DestPort = 0;
	else
	    netpkth->DestPort = 1;
    }

    if(sw->port[netpkth->DestPort].credit[netpkth->TC] < netpkth->pkt_len)
    {
	dms_drt_find(port, netpkth,current_port);
	netpkth->escape = 2;
    }
    */

    if(0x8 & gbits)
    {
	if(netpkth->a_dest > rgs->x_id[0])
	{
	    if (buflen <= sw->port[0].credit[netpkth->TC])
	    {
		buflen = sw->port[0].credit[netpkth->TC];
		drt_adp = 0;
	    }
	}
	else
	{

	     if(buflen <= sw->port[1].credit[netpkth->TC])
	     {
		 buflen = sw->port[1].credit[netpkth->TC];
		 drt_adp = 1;
	     }
	 }
    }
    if(0x4 & gbits)
    {
	if(netpkth->b_dest > rgs->x_id[1])
	{
	    if (buflen <= sw->port[2].credit[netpkth->TC])
	    {
		buflen = sw->port[2].credit[netpkth->TC];
		drt_adp = 2;
	    }
	}
	else
	{

	     if(buflen <= sw->port[3].credit[netpkth->TC])
	     {
		 buflen = sw->port[3].credit[netpkth->TC];
		 drt_adp = 3;
	     }
	 }
    }
    if(0x2 & gbits)
    {
	if(netpkth->c_dest > rgs->x_id[2])
	{
	    if (buflen <= sw->port[4].credit[netpkth->TC])
	    {
		buflen = sw->port[4].credit[netpkth->TC];
		drt_adp = 4;
	    }
	}
	else
	{

	     if(buflen <= sw->port[5].credit[netpkth->TC])
	     {
		 buflen = sw->port[5].credit[netpkth->TC];
		 drt_adp = 5;
	     }
	 }
    }
    if(0x1 & gbits)
    {
	if(netpkth->d_dest > rgs->x_id[3])
	{
	    if (buflen <= sw->port[6].credit[netpkth->TC])
	    {
		buflen = sw->port[6].credit[netpkth->TC];
		drt_adp = 6;
	    }
	}
	else
	{

	     if(buflen <= sw->port[7].credit[netpkth->TC])
	     {
		 buflen = sw->port[7].credit[netpkth->TC];
		 drt_adp = 7;
	     }
	 }
    }

    if(buflen >= netpkth->pkt_len)
    {
	netpkth->DestPort = drt_adp;
    }
    else
    {
	dms_drt_find(port, netpkth,current_port);
	netpkth->escape = 1;
    }
    
}

void mesh_adp2_routing(struct sw_port * port, unicast_pkth_t* netpkth, sw_config_register *rgs,int current_port)
{
    struct sw *sw;
    sw = port->sw;
    unsigned int tbit = 1;
    int buflen = 0;
    int esp;
    int drt_adp = 0;
    int gbits = netpkth->guide_bits;
/*    if(0x8 & gbits)
    {
	if(netpkth->a_dest > rgs->x_id[0])
	{
	    if (buflen <= sw->port[0].credit[0])
	    {
		buflen = sw->port[0].credit[0];
    		esp = 0;
		drt_adp = 0;
	    }
	}
	else
	{

	     if(buflen <= sw->port[1].credit[1])
	     {
		 buflen = sw->port[1].credit[1];
    		 esp = 1;
		 drt_adp = 1;
	     }
	 }
    }
    if(0x4 & gbits)
    {
	if(netpkth->b_dest > rgs->x_id[1])
	{
	    if (buflen <= sw->port[2].credit[0])
	    {
		buflen = sw->port[2].credit[0];
		esp  = 0;
		drt_adp = 2;
	    }
	    if (buflen <= sw->port[2].credit[1])
	    {
		buflen = sw->port[2].credit[1];
		esp  = 1;
		drt_adp = 2;
	    }
	}
	else
	{

	     if(buflen <= sw->port[3].credit[0])
	     {
		 buflen = sw->port[3].credit[0];
		 esp = 0;
		 drt_adp = 3;
	     }
	     if(buflen <= sw->port[3].credit[1])
	     {
		 buflen = sw->port[3].credit[1];
		 esp = 1;
		 drt_adp = 3;
	     }
	 }
    }

    netpkth->escape = esp;
    */
    if(0x8 & gbits)
    {
	if(netpkth->a_dest > rgs->x_id[0])
	{
	    if (buflen <= sw->port[0].credit[netpkth->TC])
	    {
		buflen = sw->port[0].credit[netpkth->TC];
		drt_adp = 0;
	    }
	}
	else
	{

	     if(buflen <= sw->port[1].credit[netpkth->TC])
	     {
		 buflen = sw->port[1].credit[netpkth->TC];
		 drt_adp = 1;
	     }
	 }
    }
    if(0x4 & gbits)
    {
	if(netpkth->b_dest > rgs->x_id[1])
	{
	    if (buflen <= sw->port[2].credit[netpkth->TC])
	    {
		buflen = sw->port[2].credit[netpkth->TC];
		drt_adp = 2;
	    }
	}
	else
	{

	     if(buflen <= sw->port[3].credit[netpkth->TC])
	     {
		 buflen = sw->port[3].credit[netpkth->TC];
		 drt_adp = 3;
	     }
	}
    }
    netpkth->DestPort = drt_adp;
}

void universal_sw_getout_port(struct sw_port *port,unicast_pkth_t *netpkth, int current_port)
{
    /* guide bit A+A-B+B-C+C-D+D- */
    struct sw *sw;
    sw_config_register * rgs;
    int direction,i;
    unsigned int gbits;
    sw = port->sw;
    rgs = &sw_crgs[sw->index -nic_num];
    netpkth->escape = netpkth->TC;
    mesh_guide_bits_compute(rgs, netpkth);

    if(netpkth->guide_bits != 0)
    {
#ifdef MESH_ADAPTIVE_1
	if( netpkth->TC > 0)
    	    {
	    	    dms_drt_find(port, netpkth,current_port);

	    }
	    else
	    {
		    mesh_adp_routing(port,netpkth, rgs,current_port);
	    }
#endif
#ifdef MESH_ADAPTIVE_2
	    mesh_adp2_routing(port,netpkth,rgs,current_port);
#endif
	    
    }
    else
    {
	netpkth->DestPort = 8;
    }
}

void sw_get_nexthop_portvc(struct sw *sw, unicast_pkth_t *pkth, id_t src_port)
{
}
void universal_sw_send_pkt_dispose(struct sw *sw, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif
