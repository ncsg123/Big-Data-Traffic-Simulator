#include "universal_nic.h"
#include<stdbool.h>
#include<math.h>
#include"parser.h"
#include"macro_select.h"
#include<time.h>
#include"sw_config_register.h"

#define MESH_ADAPTIVE_2
#define TRANS
#define VOQ
#define LOCALITY
#define DAVI_TOR

#ifdef NETWORK_TYPE_FTREE

/*whether this virtural channel can be used to send packets*/
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
#ifdef LOCALITY
    if(rand()%100 < 80){
	*dst = -1;
    }
    else{
	*dst = rand()%nic_num;
    }

#else
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
    *dst = *dst * nic->nic_port_num + rand()%nic->nic_port_num;
#endif
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
   int i;
   for (i=0;i<MAX_STAGE;i++){
   	  pkth->src_port[i] = 0;
	  pkth->stage_delay[i] = 0;
   }
   int k;
   int n;
   k = (nic->m)/2;
   n = nic->n;
   pkth->DestPort = rand()%nic->nic_port_num;
   src = src * nic->nic_port_num+pkth->DestPort;
   bug_on(k==0,"k equals 0");

   if(dst == -1){
       dst = rand()%(nic->m/2) + src - src%(nic->m/2);
   }

   if(n==1){
      pkth->hop_cnt =2 ;
      pkth->src_port[1] = dst%(2*k);
      pkth->src_port[2] = nic->nic_port_num;
      return;
   }
   if(dst/k == src/k){
      pkth->hop_cnt = 2;
      pkth->src_port[1] = dst%k;
      pkth->src_port[2] = nic->nic_port_num ;
      return;
   }

   if (n==2){
      pkth->hop_cnt = 4;
      pkth->src_port[1] = k + src/(2*k);
      pkth->src_port[2] = dst/k;
      pkth->src_port[3] = dst%k;
      pkth->src_port[4] = nic->nic_port_num;
   }
   if (n==3){
      if (dst/(k*k) == src/(k*k)){
         pkth->hop_cnt = 4;
         pkth->src_port[1] = k + src%k;
         pkth->src_port[2] = (dst%(k*k))/k;
         pkth->src_port[3] = dst%k;
         pkth->src_port[4] = nic->nic_port_num;  
      }
      else{
         pkth->hop_cnt = 6;
         pkth->src_port[1] = k + src%k;
         pkth->src_port[2] = k + (src%(k*k))/k;
         pkth->src_port[3] = dst/(k*k);
         pkth->src_port[4] = (dst%(k*k))/k;
         pkth->src_port[5] = dst%k;
         pkth->src_port[6] = nic->nic_port_num;  
      }
   }
   if (n==4){
      if ((dst/(k*k))==(src/(k*k))){
         pkth->hop_cnt = 4;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = (dst%(k*k))/k;
         pkth->src_port[3] = dst%k;
         pkth->src_port[4] = nic->nic_port_num;  
      }
      else if(dst/(k*k*k)==src/(k*k*k)){
         pkth->hop_cnt = 6;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = k+(src%(k*k))/k;
         pkth->src_port[3] = (dst%(k*k*k))/(k*k);
         pkth->src_port[4] = (dst%(k*k))/k;
         pkth->src_port[5] = dst%k;
         pkth->src_port[6] = nic->nic_port_num;  
      }
      else{
         pkth->hop_cnt = 8;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = k+(src%(k*k))/k;
         pkth->src_port[3] = k+(src%(k*k*k))/(k*k);
         pkth->src_port[4] = dst/(k*k*k);
         pkth->src_port[5] = (dst%(k*k*k))/(k*k);
         pkth->src_port[6] = (dst%(k*k))/k;
         pkth->src_port[7] = dst%k;
         pkth->src_port[8] = nic->nic_port_num;
      }
   }
   if (n==5){
      if ((dst/(k*k))==(src/(k*k))){
         pkth->hop_cnt = 4; 
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = (dst%(k*k))/k;
         pkth->src_port[3] = dst%k;
         pkth->src_port[4] = nic->nic_port_num;  
      }
      else if(dst/(k*k*k)==src/(k*k*k)){
         pkth->hop_cnt = 6;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = k+(src%(k*k))/k;
         pkth->src_port[3] = (dst%(k*k*k))/(k*k);
         pkth->src_port[4] = (dst%(k*k))/k;
         pkth->src_port[5] = dst%k;
         pkth->src_port[6] = nic->nic_port_num;  
      }
      else if(dst/(k*k*k*k)==src/(k*k*k*k)){
         pkth->hop_cnt = 8;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = k+(src%(k*k))/k;
         pkth->src_port[3] = k+(src%(k*k*k))/(k*k);
         pkth->src_port[4] = (dst%(k*k*k*k))/(k*k*k);
         pkth->src_port[5] = (dst%(k*k*k))/(k*k);
         pkth->src_port[6] = (dst%(k*k))/k;
         pkth->src_port[7] = dst%k;
         pkth->src_port[8] = nic->nic_port_num;   
      }
      else{
         pkth->hop_cnt = 10;
         pkth->src_port[1] = k+src%k;
         pkth->src_port[2] = k+(src%(k*k))/k;
         pkth->src_port[3] = k+(src%(k*k*k))/(k*k);
         pkth->src_port[4] = k+(src%(k*k*k*k))/(k*k*k);
         pkth->src_port[5] = dst/(k*k*k*k);
         pkth->src_port[6] = (dst%(k*k*k*k))/(k*k*k);
         pkth->src_port[7] = (dst%(k*k*k))/(k*k);
         pkth->src_port[8] = (dst%(k*k))/k;
         pkth->src_port[9] = dst%k;
         pkth->src_port[10] = nic->nic_port_num;  
      }
   }
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    usi dst;
    htime_t tick; 

    dst  = netpkth->DestPort;
    tick = netpkth->arrive_tick;
    if (netpkth->hop_cnt == 0 ){
       bug_on((dst      != nic->nic_port_num), "Destination address for packets with hop cnt = 0 must be 1");
       bug_on((src_port >= nic->nic_port_num), "source port for packets with hop cnt = 0 must be 0, now source port is %d",src_port);
       return true;
    }

    return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
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

/*whether this virtural channel can be used to send packets*/
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
#ifdef DAVI_TOR
    int i;
    if(rand()%100 < 80)
    {
	i = router_port_num - nic->m/2;
	*dst = nic->index - nic->index%i + rand()%i;
    }
    else
    {
	*dst = rand()%nic_num;
    }
#else
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
    *dst = *dst * nic->nic_port_num + rand()%nic->nic_port_num;
#endif
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst_real, unsigned int src_real)
{
   int i;
   for (i=0;i<MAX_STAGE;i++){
   	  pkth->src_port[i] = 0;
	  pkth->stage_delay[i] = 0;
   }
   int k;
   int n;
   int r_d;
   int radix; 
   int dst;
   int src;
   k = (nic->m)/2;
   n = nic->n;
   r_d = router_port_num - k;
   radix = r_d / k;
 //  printf("radix:%d\n",radix);
   src = src_real / radix;
   dst = dst_real /radix;
   pkth->DestPort = rand()%nic->nic_port_num;
   bug_on(k==0,"k equals 0");

   if(dst_real/r_d == src_real/r_d){
      pkth->hop_cnt = 2;
      pkth->src_port[1] = dst_real%r_d;
      pkth->src_port[2] = nic->nic_port_num;
      return;
   }

   if (n==2){
      pkth->hop_cnt = 4;
      pkth->src_port[1] = r_d + src%k;
      pkth->src_port[2] = dst/k;
      pkth->src_port[3] = dst_real%r_d;
      pkth->src_port[4] = nic->nic_port_num;
   }
   if (n==3){
      if (dst/(k*k) == src/(k*k)){
         pkth->hop_cnt = 4;
         pkth->src_port[1] = r_d + src%k;
         pkth->src_port[2] = (dst%(k*k))/k;
         pkth->src_port[3] = dst_real%r_d;
         pkth->src_port[4] = nic->nic_port_num;  
      }
      else{
         pkth->hop_cnt = 6;
         pkth->src_port[1] = r_d + src%k;
         pkth->src_port[2] = k + (src%(k*k))/k;
         pkth->src_port[3] = dst/(k*k);
         pkth->src_port[4] = (dst%(k*k))/k;
         pkth->src_port[5] = dst_real%r_d;
         pkth->src_port[6] = nic->nic_port_num;  
      }
   }
   /*
   int i;
   for (i=0;i<MAX_STAGE;i++){
   	  pkth->src_port[i] = 0;
	  pkth->stage_delay[i] = 0;
   }
   int k;
   int n;
   int r_d;
   int radix; 
   int dst;
   int src;
   k = (nic->m)/2;
   n = nic->n;
   r_d = router_port_num - k;
   //printf("r_d = %d\n",r_d);
   radix = r_d / k;
   pkth->DestPort = rand()%nic->nic_port_num;
   src_real = src_real * nic->nic_port_num+pkth->DestPort;
   src = src_real / radix;
   dst = dst_real /radix;
   bug_on(k==0,"k equals 0");

   if(dst_real/r_d == src_real/r_d)
   {
       pkth->hop_cnt = 2;
       pkth->src_port[1] = dst_real%r_d;
       pkth->src_port[2] = nic->nic_port_num;
       return;
   }

   if(n==1){
       
      pkth->hop_cnt =4 ;
      pkth->src_port[1] = r_d + src_real%k;
      pkth->src_port[2] = dst%(2*k);
      pkth->src_port[3] = dst_real%r_d;
      pkth->src_port[4] = nic->nic_port_num;
//    printf("dst_real= %d, src_real = %d\n",dst_real, src_real);
//      for(i = 0; i< 4;i ++)
//      {
//	  printf("pkth->src_port[%d] = %d\n",i+1,pkth->src_port[i+1]);
//      }
//
      return;
   }

   if (n==2){
      pkth->hop_cnt = 6;
      pkth->src_port[1] = r_d + src_real%k;
      pkth->src_port[2] = k + src%k;
      pkth->src_port[3] = dst/k;
      pkth->src_port[4] = dst%k;
      pkth->src_port[5] = dst_real%r_d;
      pkth->src_port[6] = nic->nic_port_num;
   }
   if (n==3){
      if (dst/(k*k) == src/(k*k)){
         pkth->hop_cnt = 6;
         pkth->src_port[1] = r_d + src_real%k;
         pkth->src_port[2] = k + src%k;
         pkth->src_port[3] = (dst%(k*k))/k;
         pkth->src_port[4] = dst%k;
         pkth->src_port[5] = dst_real%r_d;
         pkth->src_port[6] = nic->nic_port_num;  
      }
      else{
         pkth->hop_cnt = 8;
         pkth->src_port[1] = r_d + src_real%k;
         pkth->src_port[2] = k + src%k;
         pkth->src_port[3] = k + (src%(k*k))/k;
         pkth->src_port[4] = dst/(k*k);
         pkth->src_port[5] = (dst%(k*k))/k;
         pkth->src_port[6] = dst%k;
         pkth->src_port[7] = dst_real%r_d;
         pkth->src_port[8] = nic->nic_port_num;  
      }
   }
   */
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    usi dst;
    htime_t tick; 

    dst  = netpkth->DestPort;
    tick = netpkth->arrive_tick;
    if (netpkth->hop_cnt == 0 ){
       bug_on((dst      != nic->nic_port_num), "Destination address for packets with hop cnt = 0 must be 1");
       bug_on((src_port >= nic->nic_port_num), "source port for packets with hop cnt = 0 must be 0, now source port is %d",src_port);
       return true;
    }

    return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
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

/*whether this virtural channel can be used to send packets*/
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    int i;
    if(rand()%100 < 80)
    {
	i = router_port_num - nic->m/2/nic->nic_port_num;
	*dst = nic->index - nic->index%i + rand()%i;
    }
    else
    {
	*dst = rand()%nic_num;
    }
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst_real, unsigned int src_real)
{
   int i;
   for (i=0;i<MAX_STAGE;i++){
   	  pkth->src_port[i] = 0;
	  pkth->stage_delay[i] = 0;
   }
   int k;
   int n;
   int r_pn;
   int r_d;
   int radix; 
   int dst;
   int src;
   k = (nic->m)/2;
   n = nic->n;
   r_pn = k/nic->nic_port_num; 
   r_d = router_port_num - k/nic->nic_port_num;
   //printf("r_d = %d\n",r_d);
   radix = r_d / k;
   pkth->DestPort = rand()%nic->nic_port_num;
   src = src_real / radix;
   dst = dst_real /radix;
   bug_on(k==0,"k equals 0");

   if(dst_real/r_d == src_real/r_d)
   {
       pkth->hop_cnt = 2;
       pkth->src_port[1] = dst_real%r_d;
       pkth->src_port[2] = nic->nic_port_num;
       return;
   }

   if(n==1){
       
      pkth->hop_cnt =4 ;
      pkth->src_port[1] = r_d + src_real%r_pn;
      pkth->src_port[2] = dst%(2*k);
      pkth->src_port[3] = dst_real%r_d;
      pkth->src_port[4] = nic->nic_port_num;
/*    printf("dst_real= %d, src_real = %d\n",dst_real, src_real);
      for(i = 0; i< 4;i ++)
      {
	  printf("pkth->src_port[%d] = %d\n",i+1,pkth->src_port[i+1]);
      }
*/
      return;
   }

   if (n==2){
      pkth->hop_cnt = 6;
      pkth->src_port[1] = r_d + src_real%r_pn;
      pkth->src_port[2] = k + src%k;
      pkth->src_port[3] = dst/k;
      pkth->src_port[4] = dst%k;
      pkth->src_port[5] = dst_real%r_d;
      pkth->src_port[6] = nic->nic_port_num;
   }
   if (n==3){
      if (dst/(k*k) == src/(k*k)){
         pkth->hop_cnt = 6;
         pkth->src_port[1] = r_d + src_real%r_pn;
         pkth->src_port[2] = k + src%k;
         pkth->src_port[3] = (dst%(k*k))/k;
         pkth->src_port[4] = dst%k;
         pkth->src_port[5] = dst_real%r_d;
         pkth->src_port[6] = nic->nic_port_num;  
      }
      else{
         pkth->hop_cnt = 8;
         pkth->src_port[1] = r_d + src_real%r_pn;
         pkth->src_port[2] = k + src%k;
         pkth->src_port[3] = k + (src%(k*k))/k;
         pkth->src_port[4] = dst/(k*k);
         pkth->src_port[5] = (dst%(k*k))/k;
         pkth->src_port[6] = dst%k;
         pkth->src_port[7] = dst_real%r_d;
         pkth->src_port[8] = nic->nic_port_num;  
      }
   }
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    usi dst;
    htime_t tick; 

    dst  = netpkth->DestPort;
    tick = netpkth->arrive_tick;
    if (netpkth->hop_cnt == 0 ){
       bug_on((dst      != nic->nic_port_num), "Destination address for packets with hop cnt = 0 must be 1");
       bug_on((src_port >= nic->nic_port_num), "source port for packets with hop cnt = 0 must be 0, now source port is %d",src_port);
       return true;
    }

    return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
#ifdef VOQ
    pkth->escape = pkth->src_port[1];
#else
    pkth->escape = pkth->TC;
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
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
/*d7k nic init */
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int i;
    dst_dms = dst / dms[0];

    for(i = 0; i < dms[1] - 1; i++)
    {
	pkth->drt[i] = dst_dms /(int)pow(dms[0],i) % dms[0];
//	printf("pkth->drt[%d] = %d\n",i,pkth->drt[i]);
    }
    pkth->to_nic = (dms[0]-1) * (dms[1] -1) + dst % dms[0];
    //printf("pkth->to_nic = %d",pkth->to_nic);

    pkth->DestPort = 0;
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->DestPort == 1)
    {
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_SFLBFLY
/*d7k nic init */
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int i;
    int id = 1;
    dst_dms = dst / nic->n;
    int pnic=0;
    for(i = 0; i < nic->m; i++)
    {
	pnic+=(dms[i+1]-1);
    }

    for(i = 0; i < nic->m ; i++)
    {
	pkth->drt[i] = dst_dms /id % dms[i+1];
	id *= dms[i+1];
    }
 
    pkth->to_nic = pnic + (dst%nic->n);
    pkth->DestPort = 0;
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->DestPort == 1)
    {
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
#ifdef VOQ
    int i = 0;
    int index = 0;
    sw_config_register * rgs;
    pkth->escape = pkth->TC;
    rgs = &sw_crgs[(nic->index)/(nic->n)];
//    printf("nic->index%d\n",nic->index);

    while(i < dms[0])
    {
	if(rgs->x_id[i] > pkth->drt[i])
	{
	    pkth->escape = index + pkth->drt[i];
	    break;
	}
	if(rgs->x_id[i] < pkth->drt[i])
	{
	    pkth->escape = index + pkth->drt[i] - 1;
	    break;
	}
	i++;
	index += (dms[i] - 1);
    }
    if(i == dms[0])
    {
	pkth->escape= pkth->to_nic;
    }
//    printf("pkth->escape = %lu\n",pkth->escape);
#else
    pkth->escape = pkth->TC;
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORFB
/*d7k nic init */
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int i;
    int id = 1;
    dst_dms = dst / (nic->n/nic->nic_port_num);
    //printf("nic->n/nic->nic_port_num=%d\n",nic->n/nic->nic_port_num);
    int pnic=0;
    for(i = 0; i < nic->m; i++)
    {
	pnic+=(dms[i+1]-1);
    }

    for(i = 0; i < nic->m ; i++)
    {
	pkth->drt[i] = dst_dms /id % dms[i+1];
	id *= dms[i+1];
    }
 
    pkth->to_nic = pnic + (dst%(nic->n/nic->nic_port_num)*nic->nic_port_num+rand()%nic->nic_port_num);
    pkth->DestPort = rand()%nic->nic_port_num;
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    struct nic *nic;
    nic = port->nic;
    pkth->DestPort = nic->nic_port_num;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->DestPort == nic->nic_port_num)
    {
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
#ifdef VOQ
    int i = 0;
    int index = 0;
    sw_config_register * rgs;
    pkth->escape = pkth->TC;
    rgs = &sw_crgs[(nic->index)/(nic->n/nic->nic_port_num)];
//    printf("nic->index%d\n",nic->index);

    while(i < dms[0])
    {
	if(rgs->x_id[i] > pkth->drt[i])
	{
	    pkth->escape = index + pkth->drt[i];
	    break;
	}
	if(rgs->x_id[i] < pkth->drt[i])
	{
	    pkth->escape = index + pkth->drt[i] - 1;
	    break;
	}
	i++;
	index += (dms[i] - 1);
    }
    if(i == dms[0])
    {
	pkth->escape= pkth->to_nic;
    }
//    printf("pkth->escape = %lu\n",pkth->escape);
#else
    pkth->escape = pkth->TC;
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_D7K
/*d7k nic init */
bool judge_vc(int vc)
{
    if(vc > 1)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int nnic;
    nnic = nic_num / global_swnum;
    dst_dms = dst / (4 * nnic);

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    pkth->e_dest = dst % (4 * nnic) / nnic; 
    pkth->to_nic = 5 + dst % nnic;

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS 
/*torus nic init */
bool judge_vc(int vc)
{
    if(vc > 0)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
    //*dst = (nic->index + 3)%dms[0];
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    dst_dms = dst;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    pkth->e_dest = dst % 4; 

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_MESH 
/*mesh nic init */
bool judge_vc(int vc)
{
    if(vc < 1)
    return true;
    else 
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    dst_dms = dst;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = rand()%nic->nic_vc_num;
    //pkth->escape = rand()%2;
    //pkth->escape = rand()%5;
   // pkth->escape = 0;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_ALLTOALL
/*alltoall nic init */
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    pkth->strategy_id  = dst;
    pkth->guide_bits   = src;
    pkth->DestPort     = 0;
    //printf("dst is %d, src is %d \n",dst,src);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(nic->index == netpkth->strategy_id)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS_NEW
/*torus nic init */
bool judge_vc(int vc)
{
    return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
//    netpkth->to_nic = 3;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    dst_dms = dst;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    pkth->e_dest = dst % 4; 

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION 
/*torus nic init */
bool judge_vc(int vc)
{
    if(vc >= 1)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;

//    printf("current_vc :%d\n",current_vc);

    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
//   printf("nic->index is %d dst is %d \n",nic->index,*dst);
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int nnic;
    int dst_dms;
    int src_dms;
    nnic = nic_num / global_swnum;
    dst_dms = dst / nnic;
    int a_src,b_src,c_src,d_src;
    int p_a,p_b,p_c,p_d;
    int x_a,x_b,x_c,x_d;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    pkth->to_nic = 8 + dst % nnic;

    src_dms = src / nnic;
    a_src = src_dms % dms[0];
    b_src = src_dms / dms[0] % dms[1];
    c_src = src_dms / (dms[0]*dms[1]) % dms[2];
    d_src = src_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    
    x_a = a_src - pkth->a_dest;
    p_a = abs(x_a);
    if(((p_a <= dms[0]/2)&&(x_a < 0))||(( p_a > dms[0]/2 )&&( x_a > 0)))
    {
    //    printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("+a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
	pkth->guide_bits &= 0xBF;
    }
    else
    {
	pkth->guide_bits &= 0x7F;
//        printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("-a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
    }

    if(dms[0]%2 == 0 && p_a == dms[0]/2)
    {
	pkth->guide_bits |= 0xC0;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xBF;
	}
	else
	{
	    pkth->guide_bits &= 0x7F;
	}
    }

    x_b = b_src - pkth->b_dest;
    p_b = abs(x_b);
   
    if(((p_b <= dms[1]/2)&&(x_b < 0))||(( p_b > dms[1]/2 )&&( x_b > 0)))
    {
	pkth->guide_bits &= 0xEF;
    }
    else
    {
	pkth->guide_bits &= 0xDF;
    }

    if(dms[1]%2 == 0 && p_b == dms[1]/2)
    {
	pkth->guide_bits |= 0x30;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xEF;
	}
	else
	{
	    pkth->guide_bits &= 0xDF;
	}
    }
    x_c = c_src - pkth->c_dest;
    p_c = abs(x_c);
    if(((p_c <= dms[2]/2)&&(x_c < 0))||(( p_c > dms[2]/2 )&&( x_c > 0)))
    {
	pkth->guide_bits &= 0xFB;
    }
    else
    {
	pkth->guide_bits &= 0xF7;
    }
    if(dms[2]%2 == 0 && p_c == dms[2]/2)
    {
	pkth->guide_bits |= 0xC;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFB;
	}
	else
	{
	    pkth->guide_bits &= 0xF7;
	}
    }
    
    x_d = d_src - pkth->d_dest;
    p_d = abs(x_d);
    if(((p_d <= dms[3]/2)&&(x_d < 0))||(( p_d > dms[3]/2 )&&( x_d > 0)))
    {
	pkth->guide_bits &= 0xFE;
    }
    else
    {
	pkth->guide_bits &= 0xFD;
    }
    if(dms[3]%2 == 0 && p_d == dms[3]/2)
    {
	pkth->guide_bits |= 0x3;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFE;
	}
	else
	{
	    pkth->guide_bits &= 0xFD;
	}
    }

    pkth->DestPort = 0;
//    printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
 //   printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS_DOUBLE_HIGH_LOW 
/*torus nic init */
bool judge_vc(int vc)
{
	return true;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    dst_dms = dst;
    int a_src,b_src,c_src,d_src;
    int p_a,p_b,p_c,p_d;
    int x_a,x_b,x_c,x_d;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    a_src = src % dms[0];
    b_src = src / dms[0] % dms[1];
    c_src = src / (dms[0]*dms[1]) % dms[2];
    d_src = src / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    
    x_a = a_src - pkth->a_dest;
    p_a = abs(x_a);
    if(((p_a <= dms[0]/2)&&(x_a < 0))||(( p_a > dms[0]/2 )&&( x_a > 0)))
    {
	pkth->guide_bits &= 0xBF;
    }
    else
    {
	pkth->guide_bits &= 0x7F;
    }
   
    x_b = b_src - pkth->b_dest;
    p_b = abs(x_b);
   
    if(((p_b <= dms[1]/2)&&(x_b < 0))||(( p_b > dms[1]/2 )&&( x_b > 0)))
    {
	pkth->guide_bits &= 0xEF;
    }
    else
    {
	pkth->guide_bits &= 0xDF;
    }

    x_c = c_src - pkth->c_dest;
    p_c = abs(x_c);
    if(((p_c <= dms[2]/2)&&(x_c < 0))||(( p_c > dms[2]/2 )&&( x_c > 0)))
    {
	pkth->guide_bits &= 0xFB;
    }
    else
    {
	pkth->guide_bits &= 0xF7;
    }
    
    x_d = d_src - pkth->d_dest;
    p_d = abs(x_d);
    if(((p_d <= dms[3]/2)&&(x_d < 0))||(( p_d > dms[3]/2 )&&( x_d > 0)))
    {
	pkth->guide_bits &= 0xFE;
    }
    else
    {
	pkth->guide_bits &= 0xFD;
    }

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}
void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_TORUS_ADAPTIVE_DATELINE
/*torus nic init */
bool judge_vc(int vc)
{
    if(vc > 0)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = nic->index;
    while(*dst == nic->index)
    {
	*dst = rand()%nic_num;
    }
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    dst_dms = dst;
    int a_src,b_src,c_src,d_src;
    int p_a,p_b,p_c,p_d;
    int x_a,x_b,x_c,x_d;

    pkth->a_dest = dst_dms % dms[0];
    pkth->b_dest = dst_dms / dms[0] % dms[1];
    pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
    pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    a_src = src % dms[0];
    b_src = src / dms[0] % dms[1];
    c_src = src / (dms[0]*dms[1]) % dms[2];
    d_src = src / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    
    x_a = a_src - pkth->a_dest;
    p_a = abs(x_a);
    if(((p_a <= dms[0]/2)&&(x_a < 0))||(( p_a > dms[0]/2 )&&( x_a > 0)))
    {
	pkth->guide_bits &= 0xBF;
    }
    else
    {
	pkth->guide_bits &= 0x7F;
    }
 
    x_b = b_src - pkth->b_dest;
    p_b = abs(x_b);
   
    if(((p_b <= dms[1]/2)&&(x_b < 0))||(( p_b > dms[1]/2 )&&( x_b > 0)))
    {
	pkth->guide_bits &= 0xEF;
    }
    else
    {
	pkth->guide_bits &= 0xDF;
    }

    x_c = c_src - pkth->c_dest;
    p_c = abs(x_c);
    if(((p_c <= dms[2]/2)&&(x_c < 0))||(( p_c > dms[2]/2 )&&( x_c > 0)))
    {
	pkth->guide_bits &= 0xFB;
    }
    else
    {
	pkth->guide_bits &= 0xF7;
    }
    
    x_d = d_src - pkth->d_dest;
    p_d = abs(x_d);
    if(((p_d <= dms[3]/2)&&(x_d < 0))||(( p_d > dms[3]/2 )&&( x_d > 0)))
    {
	pkth->guide_bits &= 0xFE;
    }
    else
    {
	pkth->guide_bits &= 0xFD;
    }

    pkth->DestPort = 0;
//    printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
 //   printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_D7K_DOUBLE_DEMENSION
/*d7k nic init */
bool judge_vc(int vc)
{
    if(vc > 0)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int a_src,b_src,c_src,d_src;
    int p_a,p_b,p_c,p_d;
    int x_a,x_b,x_c,x_d;
    int nnic;
    int rand_neighbor;

    nnic = nic_num / global_swnum;
    int src_dms = src /(nnic*4);
    a_src = src_dms % dms[0];
    b_src = src_dms / dms[0] % dms[1];
    c_src = src_dms / (dms[0]*dms[1]) % dms[2];
    d_src = src_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    int rand_num = rand()%100;
    if(rand_num >= 0)
    {
	dst_dms = dst /(4 * nnic);

	pkth->a_dest = dst_dms % dms[0];
	pkth->b_dest = dst_dms / dms[0] % dms[1];
	pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
	pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
	pkth->e_dest = dst % (4 * nnic) / nnic; 
        pkth->to_nic = 5 + dst % nnic;
    }
/*    else if( rand_num > 70)
    {
	pkth->a_dest = a_src;
	pkth->b_dest = b_src;
	pkth->c_dest = c_src;
	pkth->d_dest = d_src;
	pkth->e_dest = src % 4;
	while(pkth->e_dest == src % 4)
	{
	    pkth->e_dest = rand()%4;
	}
        pkth->to_nic = 5 + rand()%nnic;
	rand_neighbor = rand()%8 - 4;
	switch (rand_neighbor)
	{
	    case -4: pkth->a_dest =(dms[0] + pkth->a_dest - 1)%dms[0];
		     break;
	    case -3: pkth->b_dest =(dms[1] + pkth->b_dest - 1)%dms[1];
		     break;
	    case -2: pkth->c_dest =(dms[2] + pkth->c_dest - 1)%dms[2];
		     break;
	    case -1: pkth->d_dest =(dms[3] + pkth->d_dest - 1)%dms[3];
		     break;
	    case  0: pkth->a_dest =(pkth->a_dest + 1) % dms[0];
		     break;
	    case  1: pkth->b_dest =(pkth->b_dest + 1) % dms[1];
		     break;
	    case  2: pkth->c_dest =(pkth->c_dest + 1) % dms[2];
		     break;
	    case  3: pkth->d_dest =(pkth->d_dest + 1) % dms[3];
		     break;
	    default: printf("wrong!\n");
	}
    }
    else
    {
	pkth->a_dest = a_src;
	pkth->b_dest = b_src;
	pkth->c_dest = c_src;
	pkth->d_dest = d_src;
	pkth->e_dest = src % 4;
	while(pkth->e_dest == src % 4)
	{
	    pkth->e_dest = rand()%4;
	}
        pkth->to_nic = 5 + rand()%nnic;
    }
*/    
    x_a = a_src - pkth->a_dest;
    p_a = abs(x_a);
    if(((p_a <= dms[0]/2)&&(x_a < 0))||(( p_a > dms[0]/2 )&&( x_a > 0)))
    {
    //    printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("+a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
	pkth->guide_bits &= 0xBF;
    }
    else
    {
	pkth->guide_bits &= 0x7F;
//        printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("-a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
    }

    if(dms[0]%2 == 0 && p_a == dms[0]/2)
    {
	pkth->guide_bits |= 0xC0;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xBF;
	}
	else
	{
	    pkth->guide_bits &= 0x7F;
	}
    }

    x_b = b_src - pkth->b_dest;
    p_b = abs(x_b);
   
    if(((p_b <= dms[1]/2)&&(x_b < 0))||(( p_b > dms[1]/2 )&&( x_b > 0)))
    {
	pkth->guide_bits &= 0xEF;
    }
    else
    {
	pkth->guide_bits &= 0xDF;
    }

    if(dms[1]%2 == 0 && p_b == dms[1]/2)
    {
	pkth->guide_bits |= 0x30;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xEF;
	}
	else
	{
	    pkth->guide_bits &= 0xDF;
	}
    }
    x_c = c_src - pkth->c_dest;
    p_c = abs(x_c);
    if(((p_c <= dms[2]/2)&&(x_c < 0))||(( p_c > dms[2]/2 )&&( x_c > 0)))
    {
	pkth->guide_bits &= 0xFB;
    }
    else
    {
	pkth->guide_bits &= 0xF7;
    }
    if(dms[2]%2 == 0 && p_c == dms[2]/2)
    {
	pkth->guide_bits |= 0xC;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFB;
	}
	else
	{
	    pkth->guide_bits &= 0xF7;
	}
    }
    
    x_d = d_src - pkth->d_dest;
    p_d = abs(x_d);
    if(((p_d <= dms[3]/2)&&(x_d < 0))||(( p_d > dms[3]/2 )&&( x_d > 0)))
    {
	pkth->guide_bits &= 0xFE;
    }
    else
    {
	pkth->guide_bits &= 0xFD;
    }
    if(dms[3]%2 == 0 && p_d == dms[3]/2)
    {
	pkth->guide_bits |= 0x3;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFE;
	}
	else
	{
	    pkth->guide_bits &= 0xFD;
	}
    }

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif

#ifdef NETWORK_TYPE_D7K_VERSION1
/*d7k nic init */
bool judge_vc(int vc)
{
    if(vc > 1)
	return true;
    else
	return false;
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xFF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
    int dst_dms;
    int a_src,b_src,c_src,d_src;
    int p_a,p_b,p_c,p_d;
    int x_a,x_b,x_c,x_d;
    int nnic;
    int rand_neighbor;

    nnic = nic_num / global_swnum;
    int src_dms = src /(nnic*4);
    a_src = src_dms % dms[0];
    b_src = src_dms / dms[0] % dms[1];
    c_src = src_dms / (dms[0]*dms[1]) % dms[2];
    d_src = src_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;

    int rand_num = rand()%100;
    if(rand_num >= 90)
    {
	dst_dms = dst /(4 * nnic);

	pkth->a_dest = dst_dms % dms[0];
	pkth->b_dest = dst_dms / dms[0] % dms[1];
	pkth->c_dest = dst_dms / (dms[0]*dms[1]) % dms[2];
	pkth->d_dest = dst_dms / (dms[0]*dms[1]*dms[2]) % dms[3] ;
	pkth->e_dest = dst % (4 * nnic) / nnic; 
        pkth->to_nic = 5 + dst % nnic;
    }
    else if( rand_num > 70)
    {
	pkth->a_dest = a_src;
	pkth->b_dest = b_src;
	pkth->c_dest = c_src;
	pkth->d_dest = d_src;
	pkth->e_dest = src % 4;
	while(pkth->e_dest == src % 4)
	{
	    pkth->e_dest = rand()%4;
	}
        pkth->to_nic = 5 + rand()%nnic;
	rand_neighbor = rand()%8 - 4;
	switch (rand_neighbor)
	{
	    case -4: pkth->a_dest =(dms[0] + pkth->a_dest - 1)%dms[0];
		     break;
	    case -3: pkth->b_dest =(dms[1] + pkth->b_dest - 1)%dms[1];
		     break;
	    case -2: pkth->c_dest =(dms[2] + pkth->c_dest - 1)%dms[2];
		     break;
	    case -1: pkth->d_dest =(dms[3] + pkth->d_dest - 1)%dms[3];
		     break;
	    case  0: pkth->a_dest =(pkth->a_dest + 1) % dms[0];
		     break;
	    case  1: pkth->b_dest =(pkth->b_dest + 1) % dms[1];
		     break;
	    case  2: pkth->c_dest =(pkth->c_dest + 1) % dms[2];
		     break;
	    case  3: pkth->d_dest =(pkth->d_dest + 1) % dms[3];
		     break;
	    default: printf("wrong!\n");
	}
    }
    else
    {
	pkth->a_dest = a_src;
	pkth->b_dest = b_src;
	pkth->c_dest = c_src;
	pkth->d_dest = d_src;
	pkth->e_dest = src % 4;
	while(pkth->e_dest == src % 4)
	{
	    pkth->e_dest = rand()%4;
	}
        pkth->to_nic = 5 + rand()%nnic;
    }
  
    x_a = a_src - pkth->a_dest;
    p_a = abs(x_a);
    if(((p_a <= dms[0]/2)&&(x_a < 0))||(( p_a > dms[0]/2 )&&( x_a > 0)))
    {
    //    printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("+a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
	pkth->guide_bits &= 0xBF;
    }
    else
    {
	pkth->guide_bits &= 0x7F;
//        printf("x_a = %d,p_a = %d\n",x_a,p_a);
//	printf("-a_src = %d , pkth->a_dest %d \n",a_src, pkth->a_dest);
    }

    if(dms[0]%2 == 0 && p_a == dms[0]/2)
    {
	pkth->guide_bits |= 0xC0;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xBF;
	}
	else
	{
	    pkth->guide_bits &= 0x7F;
	}
    }

    x_b = b_src - pkth->b_dest;
    p_b = abs(x_b);
   
    if(((p_b <= dms[1]/2)&&(x_b < 0))||(( p_b > dms[1]/2 )&&( x_b > 0)))
    {
	pkth->guide_bits &= 0xEF;
    }
    else
    {
	pkth->guide_bits &= 0xDF;
    }

    if(dms[1]%2 == 0 && p_b == dms[1]/2)
    {
	pkth->guide_bits |= 0x30;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xEF;
	}
	else
	{
	    pkth->guide_bits &= 0xDF;
	}
    }
    x_c = c_src - pkth->c_dest;
    p_c = abs(x_c);
    if(((p_c <= dms[2]/2)&&(x_c < 0))||(( p_c > dms[2]/2 )&&( x_c > 0)))
    {
	pkth->guide_bits &= 0xFB;
    }
    else
    {
	pkth->guide_bits &= 0xF7;
    }
    if(dms[2]%2 == 0 && p_c == dms[2]/2)
    {
	pkth->guide_bits |= 0xC;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFB;
	}
	else
	{
	    pkth->guide_bits &= 0xF7;
	}
    }
    
    x_d = d_src - pkth->d_dest;
    p_d = abs(x_d);
    if(((p_d <= dms[3]/2)&&(x_d < 0))||(( p_d > dms[3]/2 )&&( x_d > 0)))
    {
	pkth->guide_bits &= 0xFE;
    }
    else
    {
	pkth->guide_bits &= 0xFD;
    }
    if(dms[3]%2 == 0 && p_d == dms[3]/2)
    {
	pkth->guide_bits |= 0x3;
	if(rand()%2 == 0)
	{
	    pkth->guide_bits &= 0xFE;
	}
	else
	{
	    pkth->guide_bits &= 0xFD;
	}
    }

    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
    pkth->escape = pkth->TC;
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}
#endif
#ifdef NETWORK_TYPE_MESH_ADAPTIVE
/*mesh nic init */
bool judge_vc(int vc)
{
   if(vc < 1)
      return true;
   else 
      return false; 
}

void init_unicast_pkt(struct nic * nic, unicast_pkth_t *netpkth, int current_vc)
{
    netpkth->TC = current_vc;
    netpkth->guide_bits = 0xF;
    netpkth->strategy_id = 0;
}

void universal_addr_uni(unsigned int *dst,struct nic * nic)
{
//    *dst = rand()%nic_num;
}

void universal_nic_calc_route(struct nic*nic, unicast_pkth_t*pkth, unsigned int dst, unsigned int src)
{
#ifdef UNI
    pkth->a_dest = rand()%dms[0];
    pkth->b_dest = rand()%dms[1];
    pkth->c_dest = rand()%dms[2];
    pkth->d_dest = rand()%dms[3];
#endif
#ifdef TRANS
    sw_config_register * rgs;
    rgs = &sw_crgs[nic->index];
    pkth->a_dest = rgs->x_id[1];
    pkth->b_dest = rgs->x_id[0];
    pkth->c_dest = 0;
    pkth->d_dest = 0;
#endif
    pkth->DestPort = 0;
    //printf("pkth a %d, b %d, c %d, d %d, e %d\n ",pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest,pkth->e_dest);
    //printf("nic->index %d\n",nic->index);
}

void universal_nic_getout_port(struct nic_port *port, unicast_pkth_t* pkth)
{
    pkth->DestPort = 1;
}

bool judge_reach_destination(struct nic *nic, unicast_pkth_t* netpkth, id_t src_port)
{
    if(netpkth->guide_bits == 0)
    {
	bug_on((netpkth->DestPort != 1 ),"Destination address for packets with last must be 1");
	return true;
    }
    else return false;
}

void nic_get_nexthop_portvc(struct nic *nic, unicast_pkth_t *pkth)
{
 //   pkth->escape = rand()%4;
#ifdef MESH_ADAPTIVE_1
    pkth->escape = 0;
#endif
#ifdef MESH_ADAPTIVE_2
//    pkth->escape = rand()%2;
    sw_config_register * rgs;
    rgs = &sw_crgs[nic->index];
    if(pkth->a_dest > rgs->x_id[0])
	pkth->escape = 0;
    if(pkth->a_dest < rgs->x_id[0])
	pkth->escape = 1;
    if(pkth->a_dest == rgs->x_id[0])
	pkth->escape = rand()%2;
	
#endif
}

void universal_nic_send_pkt_dispose(struct nic *nic, unicast_pkth_t *netpkth)
{
    netpkth->TC = netpkth->escape;
}

#endif
