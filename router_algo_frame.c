#include "universal_router.h"
#include "parser.h"
#include "simk.h"
#include "fundcl_router.h"


extern htime_t router_get_positive(htime_t a, htime_t b);

extern htime_t router_get_max(htime_t a, htime_t b, htime_t c);
/*
 * Swich port receive packet Description 
 * 1. check if the packet has tick error;
 * 2. if the packet is a FC, send it to credit for increment; 
 * 3. if it is a data packet, send it to receive fifo. 
 */
void router_port_recv_pkt(void *vport, void *pkt, int current_port)
{
   htime_t tick1,tick2;
   struct router_port *port;
   int dst_out,chg_back;
   bool flag_judge;
#ifdef FLOW_CONTROL
   fc_pkth_t *fc_pkth;
#endif
   unicast_pkth_t *pkth; 
   func_enter();
   dst_out = 0;
   chg_back = 0;
   flag_judge = false;

   /* check param */
   port = (struct router_port *)vport;
   bug_on(!port, ERR_NULLP);
   bug_on(!pkt, ERR_NULLP);
   
   tick1 = get_pkt_tick(pkt);
   /* Push packet into its receive fifo */
   dbg_pkt_print("port(%d) update lavt to %"Fu_time"\n", port->local_id, tick1);

   if (pkt_type(pkt)==NET_FLOWCONTROL){
#ifdef FLOW_CONTROL
      auto_get_pkth(pkt, fc_pkth);
             bug_on(port->credit_for_increment[fc_pkth->pkt_vc].tick > tick1, ERR_TICKREVERSE "FC packet with small tick than queue");
      if (Q_state(&port->credit_for_increment[fc_pkth->pkt_vc].queue)==Q_EMPTY)
         port->credit_for_increment[fc_pkth->pkt_vc].tick = tick1;
      Q_enqueue(&port->credit_for_increment[fc_pkth->pkt_vc].queue, pkt);
#endif
   }
   else{
   	 /* check for tick reverse */
	 bug_on(tick1 < port->la_tick, ERR_TICKREVERSE " Case-2");
   	 port->la_tick = tick1;
	 auto_get_netpkth(pkt, pkth);
	
	 if (pkt_type(pkt)==NET_UNICAST){
	     
	     pkth->pkt_delay[1] = pkth->arrive_tick;
	     if (Q_state(&port->recv_fifo[pkth->TC].queue) == Q_EMPTY) {
		 /* 2014.4.3---Can not judge output port when recive!*/
	        /* Use routing stategy, fill the pkt destport  */
	        universal_router_getout_port(port,pkth,current_port);
	        tick2 = router_get_positive(port->router->port[pkth->DestPort].trans_fifo.tick,ROUTER_DELAY*port->router->frequency);
	        port->recv_fifo[pkth->TC].tick = router_get_max(tick2,tick1,port->recv_fifo[pkth->TC].tick);
	     } 
#ifdef TRACE_OPEN
	    port->input_bytes += pkth->pkt_len;
	    port->fifo_used[pkth->TC] += pkth->pkt_len;
	    if((tick1 + port->router->frequency*pkth->pkt_len/port->router->bus_width)>port->router->input_stat_tick)
	       port->router->input_stat_tick = tick1 + port->router->frequency*pkth->pkt_len/port->router->bus_width;
	    port->router->input_stat_time +=1;
#endif
	 }
	 Q_enqueue(&port->recv_fifo[pkth->TC].queue, pkt);
	 if (Q_state(&port->recv_fifo[pkth->TC].queue) == Q_OVERFLOW)
	    bug("routeritch overflow happen!!\n");
   }

   func_return();
}

static inline void fill_fc_packet(void *pkt, num_t vc, htime_t tick, len_t credit)
{
    *(pkt_type_t *) (pkt) = NET_FLOWCONTROL;
    fc_pkth_t *fc_pkth;
    auto_get_pkth(pkt, fc_pkth);
    fc_pkth->pkt_vc = vc;
    fc_pkth->credit = credit;
    fc_pkth->arrive_tick = tick;
}


static inline htime_t router_send_packet_out(struct router *router, void *pkt, num_t src_id, num_t dst_id, htime_t mini_tick, bool fc_en, bool stat_en)
{
   unicast_pkth_t *pkth;
   htime_t tick;
   usi origin = 0;
   
   auto_get_netpkth(pkt, pkth);

   bug_on(((pkth->arrive_tick+(ROUTER_DELAY + pkth->pkt_len/router->bus_width)*router->frequency) < router->port[dst_id].trans_fifo.tick), ERR_TICKREVERSE "Case-2");
   router->port[dst_id].trans_fifo.tick = pkth->arrive_tick + (ROUTER_DELAY + pkth->pkt_len/router->bus_width)*router->frequency; /* record latest packet tick in output side */
   tick = pkth->arrive_tick  + (ROUTER_DELAY*router->frequency)+LINE_DELAY;//paket new arrive tick
   set_pkt_tick(pkt, tick);

   router->port[dst_id].credit[pkth->escape] -= pkth->pkt_len;
   origin = pkth->TC; 

   pkth->stage_delay[pkth->add_up_cnt] = tick - pkth->pkt_delay[1] - LINE_DELAY;
   pkth->add_up_cnt++;


   if(pkt_type(pkt)==NET_UNICAST){
       universal_router_send_pkt_dispose(router, pkth);
   }
   bug_on((pkth->add_up_cnt > 50),"to many count!"); 
#ifdef TRACE_OPEN
   router->port[dst_id].pkt_cnt += 1;
   router->port[dst_id].output_bytes += pkth->pkt_len;
   if(stat_en){
      router->port[dst_id].pkt_delay += tick - mini_tick-LINE_DELAY;
      router->port[dst_id].avg_delay  = router->port[dst_id].pkt_delay/router->port[dst_id].pkt_cnt;
      router->port[src_id].fifo_used[pkth->TC] -= pkth->pkt_len;
   }
#endif

   sync_send(netpkth_to_netpkt(pkth), router->port[dst_id].local_id, router->port[dst_id].dst_id, tick); 
   if(fc_en){
#ifdef FLOW_CONTROL
      void *fc_pkt;
      /* Send a Flow control packet from corresponding port*/        
      fc_pkt = sync_get_buf(sizeof(pkt_type_t) + sizeof(fc_pkth_t));
      fill_fc_packet(fc_pkt, origin,router->port[dst_id].trans_fifo.tick+140*router->frequency, pkth->pkt_len);
      sync_send(fc_pkt, router->port[src_id].local_id, router->port[src_id].dst_id, router->port[src_id].trans_fifo.tick+LINE_DELAY);  
#endif    
   }

   return (router->port[dst_id].trans_fifo.tick-ROUTER_DELAY*router->frequency);

}

/*
 * Adjust receive fifo's tick Description Update receive fifo's tick  according to packet just scheduled. Mainly for contention simulation.
 * 1. search all ports, if the port number equals to source id, jump to 2;else jump to 3; 
 * 2. update receive fifo's tick with max(header packet's tick, tick+len); 3. if the port is the same as dst port, then  update its tick; 
 */
static inline void adjust_recv_fifo_tick(struct router *router, num_t vc, id_t src_id, id_t dst, htime_t tick, len_t len, bool successful)
{
    int i, j;
    struct router_port *base_port = router->port;
    void *tmp_pkt;
    unicast_pkth_t *pkth;
    
    id_t dst_addr = 0;
    htime_t tick1 = 0;
    for (i = 0; i < router->router_port_num; i++, base_port++) {
	for (j = 0; j < router->router_vc_num; j++) {
	    tmp_pkt = Q_front_ele(&base_port->recv_fifo[j].queue);
	    if (tmp_pkt) {
   		auto_get_netpkth(tmp_pkt, pkth);
		dst_addr = pkth->DestPort;
		bug_on((dst_addr >= router->router_port_num),"in th router.cdst_addr is bigger than max router port num:%d",dst_addr);
	    }
	    if ((i == src_id) & (j == vc)) {
		if (tmp_pkt) {
		    tick1 = router_get_positive(router->port[dst_addr].trans_fifo.tick,ROUTER_DELAY * router->frequency);
		    base_port->recv_fifo[j].tick =router_get_max(get_pkt_tick(tmp_pkt),(tick + len * router->frequency), tick1);
		    //printf("in adjust fifo,tmp_pkt tick: %d\n",get_pkt_tick(tmp_pkt));
		} 
		else
		{
		    base_port->recv_fifo[j].tick = tick + len * router->frequency;
		    //printf("base j %d,tick: %d\n",j,tick+len*router->frequency);
		}
	    } else {
		if (tmp_pkt) {
		    if ((base_port->recv_fifo[j].tick + ROUTER_DELAY *router->frequency) < router->port[dst_addr].trans_fifo.tick)
			base_port->recv_fifo[j].tick = router->port[dst_addr].trans_fifo.tick -ROUTER_DELAY * router->frequency;
		}
	    }
	}
    }
}


/*
 * Switch handle unicast Description This function is used to handle unicast packet to be sent out. 
 * 1. first check if there is enough credit for packet to send out. If can't, update all receive fifo's tick with FC Interval, else jump to 2; 
 * 2. update its route information, then put the packet into transmit fifo, jump to 3; 
 * 3. send a FC packet to src_id, update all receive fifo's tick with packet length 
 */
bool router_handle_unicast(struct router *router, void *mini_pkt, id_t src_id, htime_t mini_tick)
{
    unicast_pkth_t *pkth_head;
    void *tmp_pkt;
    unicast_pkth_t *pkth;
    route_t dst;
    htime_t tick = 0;
    auto_get_netpkth(mini_pkt, pkth);

    dst = pkth->DestPort;
    tick = pkth->arrive_tick;
    bug_on((dst >= router->router_port_num), "dst is bigger than max port num");
    int origin = pkth->TC;
  

    router_get_nexthop_portvc(router, pkth,src_id);
    if (router->port[dst].credit[pkth->escape] >= pkth->pkt_len) {	/* This packet can be sent out */
	if (get_port_lavt(router->port[src_id].local_id) < pkth->arrive_tick) {
	    // router->tick = pkth->arrive_tick;
	    // set_compt_tick(router->id, pkth->arrive_tick);
	    return false;
	}
	Q_dequeue_ele(&router->port[src_id].recv_fifo[pkth->TC].queue);

         /* 2014/4/3 change for adaptive routing*/
	tmp_pkt = Q_front_ele(&router->port[src_id].recv_fifo[pkth->TC].queue);
	if(tmp_pkt)
	{
	    auto_get_netpkth(tmp_pkt, pkth_head);
	    universal_router_getout_port(&router->port[src_id],pkth_head,router->port[src_id].port_id);
	}
        
	router_send_packet_out(router, mini_pkt, src_id, dst, mini_tick, true, true);
         
	adjust_recv_fifo_tick(router, origin, src_id, dst, tick, (pkth->pkt_len / router->bus_width), 1);/* contention  simulation */
    } 
    else {
//	printf("A = %d B = %d C = %d D = %d inner_id = %d port_src_dst %d  port_src_local %d port_local %d port_dest %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4],router->port[src_id].dst_id,router->port[src_id].local_id,router->port[dst].local_id,router->port[dst].dst_id);

	router->port[src_id].recv_fifo[pkth->TC].tick = router_get_max(router->port[src_id].recv_fifo[pkth->TC].tick,
			router->port[src_id].credit_for_increment[pkth->TC].tick, router->port[dst].recv_fifo[pkth->TC].tick + 140);
	adjust_recv_fifo_tick(router, pkth->TC, src_id, dst, router->port[src_id].recv_fifo[pkth->TC].tick, 0, 0);/* contention simulation  */
	return false;
    }
    return true;
}
