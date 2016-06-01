/*
 * File name: universal_sw.c
 * Description: This file provide basic function of switch.
 * Date:2013/10
 * */

#include "universal_sw.h"
#include "parser.h"
#include "simk.h"
#include "fundcl_sw.h"
#include "sw_config_register.h"


htime_t sw_get_positive(htime_t a, htime_t b)
{
    if (a > b)
	return (a - b);
    else
	return 0;
}

htime_t sw_get_max(htime_t a, htime_t b, htime_t c)
{
    htime_t max_time = 0;
    if (max_time < a)
	max_time = a;
    if (max_time < b)
	max_time = b;
    if (max_time < c)
	max_time = c;
    return max_time;
}

static void sw_data_init(sw_desc_t * sw_desc, struct sw *sw)
{
    int i, j;
    func_enter();
    /* set port number */
#if (VC_SHEME==0)		// Input buffer
    // sw->sw_vc_num = sw_desc->sw_param.sw_vc_num;
    sw->sw_vc_num = sw_desc->vcnum;
#endif
    sw->input_stat_time = 0;
    sw->sw_port_num = sw_desc->m;
    sw->n = sw_desc->n;
    sw->frequency = sw_desc->frequency;
    sw->bus_width = sw_desc->bus_width;
    sw->fifo_threshold = sw_desc->fifo_depth;
    sw->priority_port = 0;
    sw->priority_vc = 0;
    sw->input_stat_tick = 0;
    // sw->input_bytes = 0.000;
    // sw->output_bytes = 0.000;
    sw->last_trace_time = 0;
    sw->index = sw_desc->index;
    bug_on(sw->sw_port_num > SW_XPORT_NUM,
	   "Too many ports (%d ports) for sw", sw->sw_port_num);
    bug_on((sw->fifo_threshold == 0), "Error fifo capacity");

    sw_confg_register_init(&sw_crgs[sw->index - nic_num],sw->index - nic_num,dms);

    /* init all port data */
    for (i = 0; i < sw->sw_port_num; i++) {
	sw->port[i].port_id = i;/*in the sw the id of port*/
	sw->port[i].local_id = 0;
	sw->port[i].dst_id = 0;
	sw->port[i].tick = 0;
	sw->port[i].la_tick = 0;
	sw->port[i].input_bytes = 0.000;
	sw->port[i].output_bytes = 0.000;
	sw->port[i].sw = sw;
	sw->port[i].trans_fifo.tick = 0;
	sw->port[i].sw_vc_num = sw->sw_vc_num;
	sw->port[i].pkt_delay = 0.000;
	sw->port[i].avg_delay = 0.000;
	sw->port[i].pkt_cnt = 0.000;
	sw->cmb_cnt[i] = 0;
	Q_init(&sw->port[i].trans_fifo.queue, NET_Q_LEN);
	for (j = 0; j < sw->sw_vc_num; j++) {
	    sw->port[i].recv_fifo[j].tick = 0;
	    sw->port[i].credit[j] = sw->fifo_threshold;
	    sw->port[i].credit_for_increment[j].tick = 0;
	    Q_init(&sw->port[i].credit_for_increment[j].queue, NET_Q_LEN);
	    Q_init(&sw->port[i].recv_fifo[j].queue, NET_Q_LEN);
	    sw->port[i].fifo_used[j] = 0;
	    sw->port[i].priority[j] = 0;
	}
    }
    func_return();
}

/*
 * Update Flow Control Description This function is used to update all VC's Credit
 *  1. get POINT points to switches' first port; 
 *  2. for each VC do 3-5;
 *  3. if current credit_for_increment fifo is not empty, means FC packet needs to be deal with, jump to 4, otherwise jump 7; 
 *  4. get transmit fifo's tick, either from its head element, or from last packet sent out. jump to 5; 
 *  5. deal with FC packets, whose tick is not bigger than transmit fifo's tick, update credit of corresponding VC, jump to 6; 
 *  6. release FC packets' buffer; 
 *  7. return 
 */
static inline void sw_update_flowcontrol(struct sw *sw)
{
    struct sw_port *base_port = &sw->port[0];
    void *head_pkt;
    fc_pkth_t *head_pkth;
    htime_t head_tick;
    int i, j;
    for (i = 0; i < sw->sw_port_num; i++, base_port++) {
	for (j = 0; j < sw->sw_vc_num; j++) {
	    if (Q_state(&base_port->credit_for_increment[j].queue) != Q_EMPTY) {
		head_tick = base_port->recv_fifo[j].tick + LINE_DELAY;
		while (base_port->credit_for_increment[j].tick <= head_tick) {
		    head_pkt = Q_dequeue_ele(&base_port->credit_for_increment[j].queue);
		    auto_get_pkth(head_pkt, head_pkth);
		    base_port->credit[j] = base_port->credit[j] + head_pkth->credit;
		    sync_release_buf(head_pkt);
		    if (Q_state(&base_port->credit_for_increment[j].queue) != Q_EMPTY) {
			head_pkt = Q_front_ele(&(base_port->credit_for_increment[j].queue));
			base_port->credit_for_increment[j].tick = get_pkt_tick(head_pkt);
		    } else
			break;
		}
	    }
	}
    }
}

static inline void *sw_output_arbiter(struct sw *sw, id_t * port_id, htime_t * mini_tick, bool priority_change)	// this  is  a  vc_num*port_num  x  vc_num*port_num  crossbar
{
    id_t tmp_id = 0;
    struct sw_port *tmp_port = sw->port;
    void *tmp_pkt = NULL;
    void *mini_pkt = NULL;
    htime_t tmp_tick = MAX_TIME;
    int i = 0, j = 0;
#if (VC_SHEME==2)
    bool mini_is_barrier = FALSE;
#endif

#if (ARBITER_ALGORITHM==0)	// loop priority

    int tmp_vc = 0;
    for (i = 0; i < sw->sw_port_num; i++, tmp_port++) {
	for (j = 0; j < sw->sw_vc_num; j++) {
	    tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
	    if (tmp_tick > tmp_port->recv_fifo[j].tick) {
		tmp_tick = tmp_port->recv_fifo[j].tick;
		mini_pkt = tmp_pkt;
		tmp_id = i;
		tmp_vc = j;
	    } else if ((tmp_tick == tmp_port->recv_fifo[j].tick)
		       && tmp_pkt) {
		if ((tmp_port->priority[j] == 1) || (mini_pkt == NULL)) {
		    mini_pkt = tmp_pkt;
		    tmp_id = i;
		    tmp_vc = j;
		}
	    }
	}
    }
    if ((priority_change == true) && (mini_pkt != NULL)) {
	sw->port[sw->priority_port].priority[sw->priority_vc] = 0;
	sw->port[tmp_id].priority[tmp_vc] = 0;
	if (tmp_vc == (sw->sw_vc_num - 1)) {
	    if (tmp_id == (sw->sw_port_num - 1)) {
		sw->port[0].priority[0] = 1;
		sw->priority_port = 0;
		sw->priority_vc = 0;
	    } else {
		sw->priority_port = tmp_id + 1;
		sw->priority_vc = 0;
		sw->port[sw->priority_port].priority[0] = 1;
	    }
	} else {
	    sw->priority_vc = tmp_vc + 1;
	    sw->port[sw->priority_port].priority[sw->priority_vc] = 1;
	}
    }
#elif  (ARBITER_ALGORITHM==1)	// round robin

    int tmp_vc = 0;
    for (i = 0; i < sw->sw_port_num; i++) {
	tmp_tick = get_port_min_pkt_tick(sw->port[i].local_id);
	for (j = 0; j < sw->sw_vc_num; j++)
	    if ((Q_state(&sw->port[i].recv_fifo[j].queue) ==
		 Q_EMPTY) && (sw->port[i].recv_fifo[j].tick < tmp_tick))
		sw->port[i].recv_fifo[j].tick = tmp_tick;
    }
    tmp_tick = MAX_TIME;
    for (i = 0; i < sw->sw_port_num; i++, tmp_port++) {
	for (j = 0; j < sw->sw_vc_num; j++) {
	    tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
	    if (tmp_tick > tmp_port->recv_fifo[j].tick) {
		tmp_tick = tmp_port->recv_fifo[j].tick;
		mini_pkt = tmp_pkt;
		tmp_id = i;
		tmp_vc = j;
	    } else if ((tmp_tick == tmp_port->recv_fifo[j].tick)
		       && tmp_pkt) {
		if ((tmp_port->priority[j] == 1) || (mini_pkt == NULL)) {
		    mini_pkt = tmp_pkt;
		    tmp_id = i;
		    tmp_vc = j;
		}
	    }
	}
    }
    if ((priority_change == true) && (mini_pkt != NULL)) {
	sw->port[sw->priority_port].priority[sw->priority_vc] = 0;
	sw->port[tmp_id].priority[tmp_vc] = 0;
	if (tmp_vc == (sw->sw_vc_num - 1)) {
	    if (tmp_id == (sw->sw_port_num - 1)) {
		sw->port[0].priority[0] = 1;
		sw->priority_port = 0;
		sw->priority_vc = 0;
	    } else {
		sw->priority_port = tmp_id + 1;
		sw->priority_vc = 0;
		sw->port[sw->priority_port].priority[0] = 1;
	    }
	} else {
	    sw->priority_port = tmp_id;
	    sw->priority_vc = tmp_vc + 1;
	    sw->port[tmp_id].priority[sw->priority_vc] = 1;
	}
    }
#elif  (ARBITER_ALGORITHM==2)	// matrix Arbiter

    int tmp_vc = 0;
    for (i = 0; i < sw->sw_port_num; i++) {
	tmp_tick = get_port_min_pkt_tick(sw->port[i].local_id);
	for (j = 0; j < sw->sw_vc_num; j++)
	    if ((Q_state(&sw->port[i].recv_fifo[j].queue) == Q_EMPTY) && (sw->port[i].recv_fifo[j].tick < tmp_tick))
		sw->port[i].recv_fifo[j].tick = tmp_tick;
    }
    tmp_tick = MAX_TIME;
    int priority = 0;
    for (i = 0; i < sw->sw_port_num; i++, tmp_port++) {
	for (j = 0; j < sw->sw_vc_num; j++) {
	    tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
	    if (tmp_tick > tmp_port->recv_fifo[j].tick) {
		tmp_tick = tmp_port->recv_fifo[j].tick;
		mini_pkt = tmp_pkt;
		tmp_id = i;
		tmp_vc = j;
		priority = tmp_port->priority[j];
	    } else if ((tmp_tick == tmp_port->recv_fifo[j].tick)&& tmp_pkt) {
		if ((tmp_port->priority[j] >= priority) || (!mini_pkt)) {
		    mini_pkt = tmp_pkt;
		    tmp_id = i;
		    tmp_vc = j;
		    priority = tmp_port->priority[j];
		}
	    }
	}
    }
    if ((priority_change == true) && (mini_pkt != NULL)) {
	for (i = 0; i < sw->sw_port_num; i++) {
	    for (j = 0; j < sw->sw_vc_num; j++) {
		sw->port[i].priority[j] = sw->port[i].priority[j] + 1;
	    }
	}
        if (sw->port[tmp_id].priority[tmp_vc]>=2)
           sw->port[tmp_id].priority[tmp_vc] = sw->port[tmp_id].priority[tmp_vc] - 2; 
    }
#endif

    for (i = 0; i < sw->sw_port_num; i++) {
	if (sw->port[i].trans_fifo.tick < tmp_tick + SW_DELAY * sw->frequency)
	    sw->port[i].trans_fifo.tick = tmp_tick + SW_DELAY * sw->frequency;
    }

    if (mini_pkt != NULL) {
 	unicast_pkth_t *pkth;
        auto_get_netpkth(mini_pkt, pkth);
	if (get_pkt_tick(mini_pkt) > tmp_tick)
	{
	    printf("error_sw");
//	    printf("pkth->TC = %d , pkth->escape = %d ,pkth->DestPort %d, pkth->a_dest = %d,pkth->b_dest =%d, pkth->c_dest=%d,pkth->d_dest=%d\n",pkth->TC,pkth->escape,pkth->DestPort,pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest);
	}
	bug_on((get_pkt_tick(mini_pkt) > tmp_tick),"revers_sw_abi");
//	printf("sw src port id: %d,sw dst port  id: %d\n",sw->port[tmp_id].local_id,sw->port[pkth->DestPort].dst_id);
	*mini_tick = get_pkt_tick(mini_pkt);
	if (priority_change == true)
	    set_pkt_tick(mini_pkt, tmp_tick);	/* update packet's tick
						 * with latest schedule
						 * tick */
	*port_id = tmp_id;
    }
    sw->tick = tmp_tick + LINE_DELAY;
    set_compt_tick(sw->id, sw->tick);

    return mini_pkt;
}

/*
 * Select minimum packet description This function is used to select
 * packet with minimum tick from recv_fifos 1. search all receive
 * fifos, to get a minimum tick and minimum packet (regardless of real 
 * or NULL); 2. if NULL packet is got, that means no real packet can
 * be sent out, then return; 3. if a real packet is got, deal the mini 
 * packet with proper function according to its type; 4. loop until
 * gets NULL packet. 
 */
static inline bool select_minimum_pkt(struct sw *sw)
{
    id_t tmp_id = 0;
    void *mini_pkt = NULL;
    htime_t mini_tick = 0;
    bool real_pkt_done = FALSE;
    /*
     * select packet with minimum tick from recv_fifos
     * tmp_id is port_id 
     */
    mini_pkt = sw_output_arbiter(sw, &tmp_id, &mini_tick, true);

    if (mini_pkt) {
	if (pkt_type(mini_pkt) == NET_UNICAST) {
	    real_pkt_done =(sw_handle_unicast(sw, mini_pkt, tmp_id, mini_tick));
	}
	sw_output_arbiter(sw, &tmp_id, &mini_tick, false);
	return real_pkt_done;
    }
    return false;
}

static inline void universal_sw_recv_pkt(struct sw *sw)
{
    int i = 0, j = 0;
    void *pkt;
    for (i = 0; i < sw->sw_port_num; i++) {
	pkt = sync_recv(sw->port[i].local_id);
	if (pkt == NULL) {
	    sw->port[i].la_tick = get_port_min_pkt_tick(sw->port[i].local_id);
	    for (j = 0; j < sw->sw_vc_num; j++)
		if ((Q_state(&sw->port[i].recv_fifo[j].queue) == Q_EMPTY)&& (sw->port[i].recv_fifo[j].tick < sw->port[i].la_tick))
		    sw->port[i].recv_fifo[j].tick = sw->port[i].la_tick;
	}
	while (pkt) {
	    sw_port_recv_pkt(&(sw->port[i]), pkt,i);
	    pkt = sync_recv(sw->port[i].local_id);
	}
    }

}

/* This is a call-back function, do param check */
static run_ret_t sw_port_run(void *vsw)
{
    struct sw *sw;
    bool real_pkt_get;

    func_enter();

    real_pkt_get = FALSE;
    sw = (struct sw *) vsw;
    bug_on(!sw, ERR_NULLP);

    /* Receive Packets */
    universal_sw_recv_pkt(sw);

    /* Update flow control information */
#ifdef FLOW_CONTROL
    sw_update_flowcontrol(sw);
#endif

    /* Select packets with minimum tick from all recv_fifo, besides it can be sent out from this port */
    real_pkt_get = select_minimum_pkt(sw);

    /* Print Statistic Information */
#ifdef TRACE_OPEN
    int i = 0;
    float inband = 0.0000;
    float outband = 0.0000;
    float avg_delay = 0.0000;
    if (sw->tick - sw->last_trace_time > 100000) {
	sw->last_trace_time = sw->tick;
	fprintf(sw->fp, "at time %" Fu_time ":\n", sw->tick);
	for (i = 0; i < sw->sw_port_num; i++) {
	    inband    += 8 * sw->port[i].input_bytes / sw->input_stat_tick;
	    outband   += 8 * sw->port[i].output_bytes / sw->port[i].trans_fifo.tick;
	    avg_delay += sw->port[i].avg_delay;
	    fprintf(sw->fp,
		    "port %d:input bandwidth is %0.3f Gbps ",
		    sw->port[i].port_id,
		    8 * sw->port[i].input_bytes / sw->input_stat_tick);
	    fprintf(sw->fp, "port %d:output bandwidth is %0.3f Gbps;mean delay is %0.3f ;packet number is %0.3f\n", sw->port[i].port_id, 8 * sw->port[i].output_bytes / sw->port[i].trans_fifo.tick, sw->port[i].avg_delay, sw->port[i].pkt_cnt);	// realtime 
	    // recording 
	    // input 
	    // bandwidth
	}
	fprintf(sw->fp,
		"The through put is %0.3f input is %0.3f output is %0.3f mean delay is %0.3f input pkt number is %f:\n",
		outband / inband, inband / (sw->sw_port_num - 1),
		outband / (sw->sw_port_num - 1),
		avg_delay / sw->sw_port_num, sw->input_stat_time);
    }
#endif

    return (run_ret_t){real_pkt_get};
}

static htime_t sw_get_lookahead(void *vsw, cid_t cid)
{
    struct sw *p_sw;
    htime_t ahead_tick = 0;
    int i;
    func_enter();
    p_sw = (struct sw *) vsw;
    bug_on(!p_sw, ERR_NULLP);
    for (i = 0; i < p_sw->sw_port_num; i++)
	if (p_sw->port[i].local_id == cid) {
	    ahead_tick = p_sw->port[i].trans_fifo.tick + LINE_DELAY;
	    break;
	}
    func_return(ahead_tick);
}

void sw_init(void *sw_desc_ptr)
{
    int sw_port;
    struct sw *sw;
    id_t local_id, dst_id;
    port_t tmp_port;
    sw_desc_t *sw_desc_uni = (sw_desc_t *) sw_desc_ptr;
    pmalloc(1, sw);
    err_exit_on(!sw, -1, ERR_MALLOC);
    sw_data_init(sw_desc_uni, sw);
    entity_t entity;
    // compt_t *compt;
    entity.id = get_compt_id_new();
    sprintf(entity.name, "sw%" Fu_compt_id, entity.id);
    entity.owner = sw;
    entity.run = sw_port_run;
    entity.get_lookahead = sw_get_lookahead;
    entity.flag = MSG_WAKE_UP | LAVT_WAKE_UP;
    entity.ch_num = sw->sw_port_num;
    sw->id = entity.id;
    compt_register_new(&entity);
   //init port 
   // printf("sw->port_num in int %d \n",sw->sw_port_num);
    for (sw_port = 0; sw_port < sw->sw_port_num; sw_port++) {
	local_id = sw_desc_uni->port[sw_port].local;
	dst_id = sw_desc_uni->port[sw_port].remote;
	sw->port[sw_port].local_id = local_id;
	sw->port[sw_port].dst_id = dst_id;
	debug("Init SW sw(%" Fu_compt_id ") port[%d]: local_id = %d, dst_id = %d", entity.id, sw_port, local_id, dst_id);
	tmp_port.src_cid = local_id;
	tmp_port.dst_cid = dst_id;
	//printf("sw++tem_port.src_id=%d  tem_port.dst_cid=%d\n",tmp_port.src_cid,tmp_port.dst_cid);
	sw->port[sw_port].port_id = sw_port;
	port_register(entity.id, tmp_port);
    }
    counter_sw--;
    if(counter_sw == 0)
    {
	free(sw_desc);
    }

#ifdef TRACE_OPEN
    int i, j = 0, k;
    char filename[30];
    char tmp_title[] = "res1//sw_";
    char tmp_id[8];
    sprintf(tmp_id, "%lu", sw->index);
    for (i = 0; i < sizeof(tmp_title) / sizeof(char) - 1; ++i) {
	filename[i] = tmp_title[i];
    }
    for (j = (sizeof(tmp_title) / sizeof(char) - 1), k = 0;
	 j < (sizeof(tmp_title) / sizeof(char) + sizeof(tmp_id) / sizeof(char));) {
	filename[j++] = tmp_id[k++];
    }
    sw->fp = fopen(filename, "w");
#endif
    func_return();
}

