#include "universal_nic.h"
#include "hoptions.h"
#include "fundcl_nic.h"
#include<stdbool.h>
#include<math.h>
#include"macro_select.h"

pkt_ptr_t universal_nic_make_unicast_pkt(struct nic *nic,int current_vc)
{

    int nic_port_num;
    pkt_ptr_t pkt;
    pkt_type_t *pkt_type;
    universal_unicast_pkth_t *pkth;
    unicast_pkth_t *netpkth;

    pkt = sync_get_buf(sizeof(universal_unicast_pkth_t) + sizeof(pkt_type_t));
    err_exit_on(!pkt, -1, ERR_MALLOC);
    dbg_assert(__FILE__, __LINE__, pkt);

    pkt_type = (pkt_type_t *) pkt;
    *pkt_type = NET_UNICAST;

    auto_get_pkth(pkt,pkth);
    nic_port_num = nic->nic_port_num;
    pkth->clk = nic->port[nic_port_num].la_tick;

    auto_get_netpkth(pkt,netpkth);
    netpkth->add_up_cnt = 0;

    /*the default vc strategy*/
    init_unicast_pkt(nic,netpkth,current_vc);

    /* the universaling memory address */
    unsigned int dst = 0;
    switch (hnet_opt.trace_type) {
	case INNER_UNIFORM:
		universal_addr_uni(&dst,nic);
		break;
	case INNER_RANDOM:
		//        universal_addr_transpose(&dst,nic);
		break;
	case INNER_BIT:
		//      universal_addr_bitcomplement(&dst,nic);
		break;
	case INNER_SHUF:
		//    universal_addr_perfectshuffle(&dst,nic);
		break;
	case INNER_HOT:
		//  universal_addr_hotregion(&dst,nic);
		break;
	case TRACE_FILE:
	default:
		debug("error universal_nic_make_unicast_pkt");
	}

    /*A general route interface for universal nic*/
    universal_nic_calc_route(nic,netpkth,dst,nic->index);

    return pkt;
}


void universal_nic_form_nic_packet(void *vport, void *pkt, int current_vc)
{
    /*Judge whether this vc can send packet*/
    /*varables defined here*/
    htime_t tick = 0;
    struct nic_port *port = (struct nic_port *)vport;
    universal_unicast_pkth_t *pkth;
    unicast_pkth_t *netpkth;
    mem_addr_t dst_addr;

    /* Only the type NET_UNICAST is supported for now */
    if (pkt_type(pkt) == NET_UNICAST){
       auto_get_pkth(pkt, pkth);
       dst_addr = pkth->dst_addr;
       tick = pkth->clk;

       auto_get_netpkth(pkt,netpkth);
       netpkth->arrive_tick = pkth->clk;
       netpkth->pkt_len = g_pkt_length;// SCRIPT_MTU;				//16+rand()%1008;
       netpkth->pkt_delay[0] = pkth->clk;
       netpkth->final = DMAEND; 

       port->la_tick += (netpkth->pkt_len/port->nic->bus_width) * (port->nic->frequency);

       if (Q_state(&port->recv_fifo[current_vc].queue) == Q_EMPTY){
          if(port->nic->port[netpkth->DestPort].trans_fifo.tick > NIC_DELAY * port->nic->frequency) 
	    port->recv_fifo[current_vc].tick = universal_nic_get_max(port->nic->port[netpkth->DestPort].trans_fifo.tick - NIC_DELAY*port->nic->frequency,tick,port->recv_fifo[current_vc].tick);
          else
	    port->recv_fifo[current_vc].tick=universal_nic_get_max(0,tick,port->recv_fifo[current_vc].tick);
       } 
       if (Q_state(&port->recv_fifo[current_vc].queue) == Q_OVERFLOW)
          bug("Overflow happen in nic fifo!!\n");
       Q_enqueue(&port->recv_fifo[current_vc].queue, pkt);
    } else {
       debug("pkt type not supported in nic_form_nic_packet");
    }
}

void universal_nic_port_recv_pkt(void *vport, void *pkt)
{
   htime_t tick1,tick2;
   struct nic_port *port;
#ifdef FLOW_CONTROL
   fc_pkth_t *fc_pkth;
#endif
   unicast_pkth_t *pkth;

   func_enter();

   /* check param */
   port = (struct nic_port *)vport;
   bug_on(!port, ERR_NULLP);
   bug_on(!pkt, ERR_NULLP);

   /* check for tick reverse */
   tick1 = get_pkt_tick(pkt);

   /* Push packet into its receive fifo */
   dbg_pkt_print("nic port(%d) update lavt to %"Fu_time"\n", port->local_id, tick1);
   if (pkt_type(pkt)==NET_FLOWCONTROL){
#ifdef FLOW_CONTROL
      auto_get_pkth(pkt, fc_pkth);
      bug_on(port->credit_for_increment[fc_pkth->pkt_vc].tick > tick1, ERR_TICKREVERSE "FC packet with small tick than queue");
      if (Q_state(&port->credit_for_increment[fc_pkth->pkt_vc].queue)==Q_EMPTY)
         port->credit_for_increment[fc_pkth->pkt_vc].tick = tick1;
      Q_enqueue(&port->credit_for_increment[fc_pkth->pkt_vc].queue, pkt);
#endif
   }
   else if(pkt_type(pkt)==NET_UNICAST){ 
      bug_on(tick1 < port->la_tick, ERR_TICKREVERSE " Case-2");
      port->la_tick = tick1;
      auto_get_netpkth(pkt, pkth);
      universal_nic_getout_port(port,pkth);
#ifdef TRACE_OPEN
      port->nic->input_bytes += pkth->pkt_len;
      port->nic->input_cnt += 1;
      port->nic->hop_cnt += pkth->add_up_cnt;
      port->nic->input_delay += (pkth->arrive_tick - pkth->pkt_delay[0]);
    //  printf("port->nic->input_delay: %d,pkth->arrive_tick: %d,pkth->delay[0]:%d\n",pkth->arrive_tick - pkth->pkt_delay[0],pkth->arrive_tick,pkth->pkt_delay[0]);
      int i;
      for(i = 0;i < MAX_STAGE-2;i++)
      {
//	  printf("stage_delay[%d] = %f\n",i,pkth->stage_delay[i]);
	  port->nic->stage_delay[i] +=  pkth->stage_delay[i];
      }
#endif
      if (Q_state(&port->recv_fifo[pkth->TC].queue) == Q_EMPTY){ 
         tick2 = universal_nic_get_positive(port->nic->port[pkth->DestPort].trans_fifo.tick,NIC_DELAY*port->nic->frequency); 
         port->recv_fifo[pkth->TC].tick = universal_nic_get_max(tick2,tick1,port->recv_fifo[pkth->TC].tick);
      }
      Q_enqueue(&port->recv_fifo[pkth->TC].queue, pkt);
      if (Q_state(&port->recv_fifo[pkth->TC].queue) == Q_OVERFLOW)
         bug("Overflow happen in nic fifo!\n");
   }

   func_return();
}

static inline void universal_nic_assum_pkt(struct nic *nic, void *pkt)
{
   unicast_pkth_t *pkth;
   auto_get_netpkth(pkt, pkth);
    
#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION
   //used for debuging!
   int src = nic->index/(nic_num / global_swnum);
   int a_src,b_src,c_src,d_src;
   a_src = src % dms[0];
   b_src = src / dms[0] % dms[1];
   c_src = src / (dms[0]*dms[1]) % dms[2];
   d_src = src / (dms[0]*dms[1]*dms[2]) % dms[3] ;

   if(a_src != pkth->a_dest || b_src != pkth->b_dest || c_src != pkth->c_dest || d_src!= pkth->d_dest)
   {
       printf("error!\n");
       bug_on(1,"this is not the packet nic want");
   }
#endif

   bug_on(nic->port[nic->nic_port_num].trans_fifo.tick >(pkth->arrive_tick + (NIC_DELAY+pkth->pkt_len/nic->bus_width)*nic->frequency), ERR_TICKREVERSE "Case_2");
   nic->port[nic->nic_port_num].trans_fifo.tick = pkth->arrive_tick + (NIC_DELAY+pkth->pkt_len/nic->bus_width)*nic->frequency;
   sync_release_buf(pkt);
}

static inline void universal_fill_fc_packet(void *pkt, num_t vc, htime_t tick, len_t credit)
{
   *(pkt_type_t*)(pkt) = NET_FLOWCONTROL;
   fc_pkth_t *fc_pkth;
   auto_get_pkth(pkt, fc_pkth);
   fc_pkth->pkt_vc = vc;
   fc_pkth->credit = credit;
   fc_pkth->arrive_tick = tick;        
}

static inline void universal_nic_send_packet_out(struct nic *nic, void *pkt, num_t src_id, num_t dst_id, bool fc_en)
{
   unicast_pkth_t *pkth;
   htime_t tick;
   usi origin = 0;
   auto_get_netpkth(pkt, pkth);
   pkth->add_up_cnt++;
   bug_on(((pkth->arrive_tick+(NIC_DELAY + pkth->pkt_len/nic->bus_width)*nic->frequency) < nic->port[dst_id].trans_fifo.tick), ERR_TICKREVERSE "Case-2");
   nic->port[dst_id].trans_fifo.tick = pkth->arrive_tick + (NIC_DELAY + pkth->pkt_len/nic->bus_width)*nic->frequency; /* record latest packet tick in output side */
   /* Now the tick is the time arrive at next componets */
   tick = pkth->arrive_tick  + (NIC_DELAY*nic->frequency)+LINE_DELAY;
   set_pkt_tick(pkt, tick);
   pkth->stage_delay[pkth->add_up_cnt - 1] = tick - LINE_DELAY - pkth->pkt_delay[0];
   //printf("tick:%lu,pkth->pkt_delay[0]:%lu, pkth->stage_delay[]:%f\n",tick,pkth->pkt_delay[0],pkth->stage_delay[pkth->add_up_cnt -1]);

   nic->port[dst_id].credit[pkth->escape] -= pkth->pkt_len;
   origin = pkth->TC; 
   if(pkt_type(pkt)== NET_UNICAST){
       universal_nic_send_pkt_dispose(nic, pkth);
   }
   sync_send(netpkth_to_netpkt(pkth), nic->port[dst_id].local_id, nic->port[dst_id].dst_id, tick); 
   /* accordance with the vitrual channel judged in hanle unicast */
#ifdef TRACE_OPEN 
   nic->output_bytes += pkth->pkt_len;
   nic->output_cnt += 1;
#endif
   if(fc_en){
#ifdef FLOW_CONTROL
      void *fc_pkt;
      /* Send a Flow control packet from corresponding port*/        
      fc_pkt = sync_get_buf(sizeof(pkt_type_t) + sizeof(fc_pkth_t));
      universal_fill_fc_packet(fc_pkt, origin,nic->port[dst_id].trans_fifo.tick+140*nic->frequency, pkth->pkt_len);
      sync_send(fc_pkt, nic->port[src_id].local_id, nic->port[src_id].dst_id, nic->port[src_id].trans_fifo.tick+LINE_DELAY);  
#endif   
   } 
}

static inline void universal_adjust_recv_fifo_tick(struct nic *nic, num_t vc, id_t src_id, id_t dst, htime_t tick, len_t len)
{
   int i,j;
   struct nic_port *base_port = nic->port;
   void *tmp_pkt;
   unicast_pkth_t *netpkth;
   htime_t tmp_tick=0;
   id_t dst_addr=0;

   for (i = 0; i< (nic->nb_port_num+nic->nic_port_num); i++, base_port++){
      for (j = 0; j< nic->nic_vc_num; j++){
         tmp_pkt =  Q_front_ele(&base_port->recv_fifo[j].queue);
         if(tmp_pkt) {
		 auto_get_netpkth(tmp_pkt,netpkth);
		 dst_addr = netpkth->DestPort;
		 bug_on(dst_addr > nic->nic_port_num,"in universal pkt's dst port id is wrong%d\n",dst_addr);
	 }
         if ((i==src_id) & (j==vc)){
            if (tmp_pkt){
               tmp_tick=universal_nic_get_positive(nic->port[dst_addr].trans_fifo.tick,NIC_DELAY*nic->frequency);
               base_port->recv_fifo[j].tick = universal_nic_get_max(get_pkt_tick(tmp_pkt),(tick+len*nic->frequency), tmp_tick);
            } 
         } 
         else{ 
            if (tmp_pkt) {
               if((base_port->recv_fifo[j].tick+NIC_DELAY*nic->frequency)<nic->port[dst_addr].trans_fifo.tick)
                  base_port->recv_fifo[j].tick = nic->port[dst_addr].trans_fifo.tick-NIC_DELAY*nic->frequency;
            }
         }
      }
   }
}

bool universal_nic_handle_unicast(struct nic *nic, void *mini_pkt, id_t src_id)
{
   unicast_pkth_t *pkth; 
   route_t dst;
   htime_t tick;
   len_t len;
   id_t vc;
   bool judge_dst;
#ifdef FLOW_CONTROL
   void *fc_pkt;
#endif
   auto_get_netpkth(mini_pkt, pkth);

   /* Univeral interface for judge whether reach destination! */
   judge_dst = judge_reach_destination(nic, pkth, src_id);
   dst = pkth->DestPort;
   vc = pkth->TC;
   tick = pkth->arrive_tick;
   len = pkth->pkt_len;

   if(judge_dst){
      mini_pkt = Q_dequeue_ele(&nic->port[src_id].recv_fifo[pkth->TC].queue);
      universal_nic_assum_pkt(nic, mini_pkt);//send to nb port   
      
      /* Send a Flow control packet from corresponding port*/  
#ifdef FLOW_CONTROL
      fc_pkt = sync_get_buf(sizeof(pkt_type_t) + sizeof(fc_pkth_t));
      universal_fill_fc_packet(fc_pkt, vc,nic->port[src_id].trans_fifo.tick+LINE_DELAY,len);
      sync_send(fc_pkt, nic->port[src_id].local_id, nic->port[src_id].dst_id, nic->port[src_id].trans_fifo.tick+LINE_DELAY);
#endif

      universal_adjust_recv_fifo_tick(nic, vc, src_id, dst, tick, (len/nic->bus_width)); /* contention simulation */ 
      return true;
   }
   else{
      /*check if port->tick is not smaller than mini_pkt, if it does, then modify mini_pkt and receive queue's tick, don't send it out*/
      nic_get_nexthop_portvc(nic, pkth);
      if (nic->port[dst].credit[pkth->escape] >= pkth->pkt_len){		/* This packet can be sent out */    
	   Q_dequeue_ele(&nic->port[src_id].recv_fifo[pkth->TC].queue);
	   universal_nic_send_packet_out(nic, mini_pkt, src_id, dst, false);
	   universal_adjust_recv_fifo_tick(nic, vc, src_id, dst, tick, (pkth->pkt_len/nic->bus_width)); /* contention simulation */
	   return true;
      }
      else{
	   nic->port[src_id].recv_fifo[pkth->TC].tick = universal_nic_get_max(nic->port[src_id].recv_fifo[pkth->TC].tick, nic->port[src_id].credit_for_increment[pkth->TC].tick,nic->port[dst].recv_fifo[pkth->TC].tick+140);    
	   universal_adjust_recv_fifo_tick(nic, pkth->TC, src_id, dst, nic->port[src_id].recv_fifo[pkth->TC].tick, 0); /* contention simulation */
	   return false;
      }
   }
}
