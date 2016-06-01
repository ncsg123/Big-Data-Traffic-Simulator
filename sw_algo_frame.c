/*
 * File name: sw_algo_frame.c
 * Description: This file provide the detail micro-architecture of switch.
 * Date: 2013/11
 * */

#include "universal_sw.h"
#include "sw_config_register.h"
#include "parser.h"
#include "simk.h"
#include "fundcl_sw.h"


extern htime_t sw_get_positive(htime_t a, htime_t b);

extern htime_t sw_get_max(htime_t a, htime_t b, htime_t c);
/*
 * Swich port receive packet Description 
 * 1. check if the packet has tick error;
 * 2. if the packet is a FC, send it to credit for increment; 
 * 3. if it is a data packet, send it to receive fifo. 
 */
void sw_port_recv_pkt(void *vport, void *pkt, int current_port)
{
   htime_t tick1,tick2;
   struct sw_port *port;
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
   port = (struct sw_port *)vport;
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
		//  2014.4.3---Can not judge all output port when recive!
	          /* Use routing stategy, fill the pkt destport,move it here*/
	          universal_sw_getout_port(port,pkth,current_port);
	          tick2 = sw_get_positive(port->sw->port[pkth->DestPort].trans_fifo.tick,SW_DELAY*port->sw->frequency);
	          port->recv_fifo[pkth->TC].tick = sw_get_max(tick2,tick1,port->recv_fifo[pkth->TC].tick);
	     }
#ifdef TRACE_OPEN
	    port->input_bytes += pkth->pkt_len;
	    port->fifo_used[pkth->TC] += pkth->pkt_len;
	    if((tick1 + port->sw->frequency*pkth->pkt_len/port->sw->bus_width)>port->sw->input_stat_tick)
	       port->sw->input_stat_tick = tick1 + port->sw->frequency*pkth->pkt_len/port->sw->bus_width;
	    port->sw->input_stat_time +=1;
#endif
	 }
	 Q_enqueue(&port->recv_fifo[pkth->TC].queue, pkt);
	 struct sw * sw; 
	 sw = port->sw;
	 if (Q_state(&port->recv_fifo[pkth->TC].queue) == Q_OVERFLOW)
	     bug("switch overflow happen!! sw->index %d, pkth->TC=%d, current_port %d, pkth->hop_cnt:%d \n",sw->index-nic_num,pkth->TC,current_port,pkth->add_up_cnt);
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


static inline htime_t sw_send_packet_out(struct sw *sw, void *pkt, num_t src_id, num_t dst_id, htime_t mini_tick, bool fc_en, bool stat_en)
{
   unicast_pkth_t *pkth;
   htime_t tick;
   usi origin = 0;
   
   auto_get_netpkth(pkt, pkth);

   bug_on(((pkth->arrive_tick+(SW_DELAY + pkth->pkt_len/sw->bus_width)*sw->frequency) < sw->port[dst_id].trans_fifo.tick), ERR_TICKREVERSE "Case-2");
   sw->port[dst_id].trans_fifo.tick = pkth->arrive_tick + (SW_DELAY + pkth->pkt_len/sw->bus_width)*sw->frequency; /* record latest packet tick in output side */
   tick = pkth->arrive_tick  + (SW_DELAY*sw->frequency)+LINE_DELAY;//paket new arrive tick
   set_pkt_tick(pkt, tick);

   sw->port[dst_id].credit[pkth->escape] -= pkth->pkt_len;
   origin = pkth->TC; 

   pkth->stage_delay[pkth->add_up_cnt] = tick - pkth->pkt_delay[1] - LINE_DELAY;
   //printf("sw_stage_delay:%lu\n",tick - pkth->pkt_delay[1]-LINE_DELAY);
   pkth->add_up_cnt++;


   if(pkt_type(pkt)==NET_UNICAST){
       universal_sw_send_pkt_dispose(sw, pkth);
   }
  // bug_on((pkth->add_up_cnt > 50),"to many count!"); 
#ifdef TRACE_OPEN
   sw->port[dst_id].pkt_cnt += 1;
   sw->port[dst_id].output_bytes += pkth->pkt_len;
   if(stat_en){
      sw->port[dst_id].pkt_delay += tick - mini_tick-LINE_DELAY;
      sw->port[dst_id].avg_delay  = sw->port[dst_id].pkt_delay/sw->port[dst_id].pkt_cnt;
      sw->port[src_id].fifo_used[pkth->TC] -= pkth->pkt_len;
   }
#endif

   sync_send(netpkth_to_netpkt(pkth), sw->port[dst_id].local_id, sw->port[dst_id].dst_id, tick); 
   if(fc_en){
#ifdef FLOW_CONTROL
      void *fc_pkt;
      /* Send a Flow control packet from corresponding port*/        
      fc_pkt = sync_get_buf(sizeof(pkt_type_t) + sizeof(fc_pkth_t));
      fill_fc_packet(fc_pkt, origin,sw->port[dst_id].trans_fifo.tick+140*sw->frequency, pkth->pkt_len);
      sync_send(fc_pkt, sw->port[src_id].local_id, sw->port[src_id].dst_id, sw->port[src_id].trans_fifo.tick+LINE_DELAY);  
#endif    
   }

   return (sw->port[dst_id].trans_fifo.tick-SW_DELAY*sw->frequency);

}

/*
 * Adjust receive fifo's tick Description Update receive fifo's tick  according to packet just scheduled. Mainly for contention simulation.
 * 1. search all ports, if the port number equals to source id, jump to 2;else jump to 3; 
 * 2. update receive fifo's tick with max(header packet's tick, tick+len); 3. if the port is the same as dst port, then  update its tick; 
 */
static inline void adjust_recv_fifo_tick(struct sw *sw, num_t vc, id_t src_id, id_t dst, htime_t tick, len_t len, bool successful)
{
    int i, j;
    struct sw_port *base_port = sw->port;
    void *tmp_pkt;
    unicast_pkth_t *pkth;
    
    id_t dst_addr = 0;
    htime_t tick1 = 0;
    for (i = 0; i < sw->sw_port_num; i++, base_port++) {
	for (j = 0; j < sw->sw_vc_num; j++) {
	    tmp_pkt = Q_front_ele(&base_port->recv_fifo[j].queue);
	    if (tmp_pkt) {
   		auto_get_netpkth(tmp_pkt, pkth);
		dst_addr = pkth->DestPort;
		bug_on((dst_addr >= sw->sw_port_num),"in th sw.cdst_addr is bigger than max sw port num:%d",dst_addr);
	    }
	    if ((i == src_id) & (j == vc)) {
		if (tmp_pkt) {
		    tick1 = sw_get_positive(sw->port[dst_addr].trans_fifo.tick,SW_DELAY * sw->frequency);
		    base_port->recv_fifo[j].tick =sw_get_max(get_pkt_tick(tmp_pkt),(tick + len * sw->frequency), tick1);
		    //printf("in adjust fifo,tmp_pkt tick: %d\n",get_pkt_tick(tmp_pkt));
		} 
		else
		{
		    base_port->recv_fifo[j].tick = tick + len * sw->frequency;
		    //printf("base j %d,tick: %d\n",j,tick+len*sw->frequency);
		}
	    } else {
		if (tmp_pkt) {
		    if ((base_port->recv_fifo[j].tick + SW_DELAY *sw->frequency) < sw->port[dst_addr].trans_fifo.tick)
			base_port->recv_fifo[j].tick = sw->port[dst_addr].trans_fifo.tick -SW_DELAY * sw->frequency;
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
bool sw_handle_unicast(struct sw *sw, void *mini_pkt, id_t src_id, htime_t mini_tick)
{
    unicast_pkth_t *pkth;
    unicast_pkth_t *pkth_head;
    void *tmp_pkt;
    route_t dst;
    htime_t tick = 0;
    sw_config_register * rgs;
    rgs = &sw_crgs[sw->index -nic_num];
    auto_get_netpkth(mini_pkt, pkth);

    dst = pkth->DestPort;
    tick = pkth->arrive_tick;
    bug_on((dst >= sw->sw_port_num), "dst is bigger than max port num");
    int origin = pkth->TC;
  

    sw_get_nexthop_portvc(sw, pkth,src_id);
    if (sw->port[dst].credit[pkth->escape] >= pkth->pkt_len) {	/* This packet can be sent out */
	if (get_port_lavt(sw->port[src_id].local_id) < pkth->arrive_tick) {
	    // sw->tick = pkth->arrive_tick;
	    // set_compt_tick(sw->id, pkth->arrive_tick);
	    return false;
	}
	Q_dequeue_ele(&sw->port[src_id].recv_fifo[pkth->TC].queue);
         /* 2014/4/3 change for adaptive routing*/
	tmp_pkt = Q_front_ele(&sw->port[src_id].recv_fifo[pkth->TC].queue);

        
	sw_send_packet_out(sw, mini_pkt, src_id, dst, mini_tick, true, true);
         
	adjust_recv_fifo_tick(sw, origin, src_id, dst, tick, (pkth->pkt_len / sw->bus_width), 1);/* contention  simulation */

	if(tmp_pkt)
	{
	    auto_get_netpkth(tmp_pkt, pkth_head);
	    universal_sw_getout_port(&sw->port[src_id],pkth_head,sw->port[src_id].port_id);
	}
    } 
    else {
//	printf("A = %d B = %d C = %d D = %d inner_id = %d port_src_dst %d  port_src_local %d port_local %d port_dest %d\n",rgs->x_id[0],rgs->x_id[1],rgs->x_id[2],rgs->x_id[3],rgs->x_id[4],sw->port[src_id].dst_id,sw->port[src_id].local_id,sw->port[dst].local_id,sw->port[dst].dst_id);

	sw->port[src_id].recv_fifo[pkth->TC].tick = sw_get_max(sw->port[src_id].recv_fifo[pkth->TC].tick,
			sw->port[src_id].credit_for_increment[pkth->TC].tick, sw->port[dst].recv_fifo[pkth->TC].tick + 140);
	adjust_recv_fifo_tick(sw, pkth->TC, src_id, dst, sw->port[src_id].recv_fifo[pkth->TC].tick, 0, 0);/* contention simulation  */
	return false;
    }
    return true;
}
