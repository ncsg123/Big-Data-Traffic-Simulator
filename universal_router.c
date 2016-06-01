#include "universal_router.h"
#include "parser.h"
#include "simk.h"
#include "fundcl_router.h"


htime_t router_get_positive(htime_t a, htime_t b)
{
    if (a > b)
	return (a - b);
    else
	return 0;
}

htime_t router_get_max(htime_t a, htime_t b, htime_t c)
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

static void router_data_init(router_desc_t * router_desc, struct router *router)
{
    int i, j;
    func_enter();
    /* set port number */
#if (VC_SHEME==0)		// Input buffer
    // router->router_vc_num = router_desc->router_param.router_vc_num;
    router->router_vc_num = router_desc->vcnum;
#endif
    router->input_stat_time = 0;
    router->router_port_num = router_desc->m;
    // printf("router->route_port_num = %d\n",router->router_port_num);
    router->n = router_desc->n;
    router->frequency = router_desc->frequency;
    router->bus_width = router_desc->bus_width;
    router->fifo_threshold = router_desc->fifo_depth;
    router->priority_port = 0;
    router->priority_vc = 0;
    router->input_stat_tick = 0;
    // router->input_bytes = 0.000;
    // router->output_bytes = 0.000;
    router->last_trace_time = 0;
    router->index = router_desc->index;
    bug_on(router->router_port_num > ROUTER_XPORT_NUM,
	   "Too many ports (%d ports) for router", router->router_port_num);
    bug_on((router->fifo_threshold == 0), "Error fifo capacity");


    /* init all port data */
    for (i = 0; i < router->router_port_num; i++) {
	router->port[i].port_id = i;/*in the router the id of port*/
	router->port[i].local_id = 0;
	router->port[i].dst_id = 0;
	router->port[i].tick = 0;
	router->port[i].la_tick = 0;
	router->port[i].input_bytes = 0.000;
	router->port[i].output_bytes = 0.000;
	router->port[i].router = router;
	router->port[i].trans_fifo.tick = 0;
	router->port[i].router_vc_num = router->router_vc_num;
	router->port[i].pkt_delay = 0.000;
	router->port[i].avg_delay = 0.000;
	router->port[i].pkt_cnt = 0.000;
	router->cmb_cnt[i] = 0;
	Q_init(&router->port[i].trans_fifo.queue, NET_Q_LEN);
	for (j = 0; j < router->router_vc_num; j++) {
	    router->port[i].recv_fifo[j].tick = 0;
	    router->port[i].credit[j] = router->fifo_threshold;
	    router->port[i].credit_for_increment[j].tick = 0;
	    Q_init(&router->port[i].credit_for_increment[j].queue, NET_Q_LEN);
	    Q_init(&router->port[i].recv_fifo[j].queue, NET_Q_LEN);
	    router->port[i].fifo_used[j] = 0;
	    router->port[i].priority[j] = 0;
	}
    }
    func_return();
}

/*
 * Update Flow Control Description This function is used to update all VC's Credit
 *  1. get POINT points to routeritches' first port; 
 *  2. for each VC do 3-5;
 *  3. if current credit_for_increment fifo is not empty, means FC packet needs to be deal with, jump to 4, otherwise jump 7; 
 *  4. get transmit fifo's tick, either from its head element, or from last packet sent out. jump to 5; 
 *  5. deal with FC packets, whose tick is not bigger than transmit fifo's tick, update credit of corresponding VC, jump to 6; 
 *  6. release FC packets' buffer; 
 *  7. return 
 */
static inline void router_update_flowcontrol(struct router *router)
{
    struct router_port *base_port = &router->port[0];
    void *head_pkt;
    fc_pkth_t *head_pkth;
    htime_t head_tick;
    int i, j;
    for (i = 0; i < router->router_port_num; i++, base_port++) {
	for (j = 0; j < router->router_vc_num; j++) {
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

static inline void *router_output_arbiter(struct router *router, id_t * port_id, htime_t * mini_tick, bool priority_change)	// this  is  a  vc_num*port_num  x  vc_num*port_num  crossbar
{
    id_t tmp_id = 0;
    struct router_port *tmp_port = router->port;
    void *tmp_pkt = NULL;
    void *mini_pkt = NULL;
    htime_t tmp_tick = MAX_TIME;
    int i = 0, j = 0;
#if (VC_SHEME==2)
    bool mini_is_barrier = FALSE;
#endif

#if (ARBITER_ALGORITHM==0)	// loop priority

    int tmp_vc = 0;
    for (i = 0; i < router->router_port_num; i++, tmp_port++) {
	for (j = 0; j < router->router_vc_num; j++) {
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
	router->port[router->priority_port].priority[router->priority_vc] = 0;
	router->port[tmp_id].priority[tmp_vc] = 0;
	if (tmp_vc == (router->router_vc_num - 1)) {
	    if (tmp_id == (router->router_port_num - 1)) {
		router->port[0].priority[0] = 1;
		router->priority_port = 0;
		router->priority_vc = 0;
	    } else {
		router->priority_port = tmp_id + 1;
		router->priority_vc = 0;
		router->port[router->priority_port].priority[0] = 1;
	    }
	} else {
	    router->priority_vc = tmp_vc + 1;
	    router->port[router->priority_port].priority[router->priority_vc] = 1;
	}
    }
#elif  (ARBITER_ALGORITHM==1)	// round robin

    int tmp_vc = 0;
    for (i = 0; i < router->router_port_num; i++) {
	tmp_tick = get_port_min_pkt_tick(router->port[i].local_id);
	for (j = 0; j < router->router_vc_num; j++)
	    if ((Q_state(&router->port[i].recv_fifo[j].queue) ==
		 Q_EMPTY) && (router->port[i].recv_fifo[j].tick < tmp_tick))
		router->port[i].recv_fifo[j].tick = tmp_tick;
    }
    tmp_tick = MAX_TIME;
    for (i = 0; i < router->router_port_num; i++, tmp_port++) {
	for (j = 0; j < router->router_vc_num; j++) {
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
	router->port[router->priority_port].priority[router->priority_vc] = 0;
	router->port[tmp_id].priority[tmp_vc] = 0;
	if (tmp_vc == (router->router_vc_num - 1)) {
	    if (tmp_id == (router->router_port_num - 1)) {
		router->port[0].priority[0] = 1;
		router->priority_port = 0;
		router->priority_vc = 0;
	    } else {
		router->priority_port = tmp_id + 1;
		router->priority_vc = 0;
		router->port[router->priority_port].priority[0] = 1;
	    }
	} else {
	    router->priority_port = tmp_id;
	    router->priority_vc = tmp_vc + 1;
	    router->port[tmp_id].priority[router->priority_vc] = 1;
	}
    }
#elif  (ARBITER_ALGORITHM==2)	// matrix Arbiter

    int tmp_vc = 0;
    for (i = 0; i < router->router_port_num; i++) {
	tmp_tick = get_port_min_pkt_tick(router->port[i].local_id);
	for (j = 0; j < router->router_vc_num; j++)
	    if ((Q_state(&router->port[i].recv_fifo[j].queue) == Q_EMPTY) && (router->port[i].recv_fifo[j].tick < tmp_tick))
		router->port[i].recv_fifo[j].tick = tmp_tick;
    }
    tmp_tick = MAX_TIME;
    int priority = 0;
    for (i = 0; i < router->router_port_num; i++, tmp_port++) {
	for (j = 0; j < router->router_vc_num; j++) {
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
	for (i = 0; i < router->router_port_num; i++) {
	    for (j = 0; j < router->router_vc_num; j++) {
		router->port[i].priority[j] = router->port[i].priority[j] + 1;
	    }
	}
        if (router->port[tmp_id].priority[tmp_vc]>=2)
           router->port[tmp_id].priority[tmp_vc] = router->port[tmp_id].priority[tmp_vc] - 2; 
    }
#endif

    for (i = 0; i < router->router_port_num; i++) {
	if (router->port[i].trans_fifo.tick < tmp_tick + ROUTER_DELAY * router->frequency)
	    router->port[i].trans_fifo.tick = tmp_tick + ROUTER_DELAY * router->frequency;
    }

    if (mini_pkt != NULL) {
 	unicast_pkth_t *pkth;
        auto_get_netpkth(mini_pkt, pkth);
	if (get_pkt_tick(mini_pkt) > tmp_tick)
	{
	    printf("error_router");
//	    printf("pkth->TC = %d , pkth->escape = %d ,pkth->DestPort %d, pkth->a_dest = %d,pkth->b_dest =%d, pkth->c_dest=%d,pkth->d_dest=%d\n",pkth->TC,pkth->escape,pkth->DestPort,pkth->a_dest,pkth->b_dest,pkth->c_dest,pkth->d_dest);
	}
	bug_on((get_pkt_tick(mini_pkt) > tmp_tick),"revers_router_abi");
//	printf("router src port id: %d,router dst port  id: %d\n",router->port[tmp_id].local_id,router->port[pkth->DestPort].dst_id);
	*mini_tick = get_pkt_tick(mini_pkt);
	if (priority_change == true)
	    set_pkt_tick(mini_pkt, tmp_tick);	/* update packet's tick
						 * with latest schedule
						 * tick */
	*port_id = tmp_id;
    }
    router->tick = tmp_tick + LINE_DELAY;
    set_compt_tick(router->id, router->tick);

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
static inline bool select_minimum_pkt(struct router *router)
{
    id_t tmp_id = 0;
    void *mini_pkt = NULL;
    htime_t mini_tick = 0;
    bool real_pkt_done = FALSE;
    /*
     * select packet with minimum tick from recv_fifos
     * tmp_id is port_id 
     */
    mini_pkt = router_output_arbiter(router, &tmp_id, &mini_tick, true);

    if (mini_pkt) {
	if (pkt_type(mini_pkt) == NET_UNICAST) {
	    real_pkt_done =(router_handle_unicast(router, mini_pkt, tmp_id, mini_tick));
	}
	router_output_arbiter(router, &tmp_id, &mini_tick, false);
	return real_pkt_done;
    }
    return false;
}

static inline void universal_router_recv_pkt(struct router *router)
{
    int i = 0, j = 0;
    void *pkt;
    for (i = 0; i < router->router_port_num; i++) {
	pkt = sync_recv(router->port[i].local_id);
	if (pkt == NULL) {
	    router->port[i].la_tick = get_port_min_pkt_tick(router->port[i].local_id);
	    for (j = 0; j < router->router_vc_num; j++)
		if ((Q_state(&router->port[i].recv_fifo[j].queue) == Q_EMPTY)&& (router->port[i].recv_fifo[j].tick < router->port[i].la_tick))
		    router->port[i].recv_fifo[j].tick = router->port[i].la_tick;
	}
	while (pkt) {
	    router_port_recv_pkt(&(router->port[i]), pkt,i);
	    pkt = sync_recv(router->port[i].local_id);
	}
    }

}

/* This is a call-back function, do param check */
static run_ret_t router_port_run(void *vrouter)
{
    struct router *router;
    bool real_pkt_get;

    func_enter();

    real_pkt_get = FALSE;
    router = (struct router *) vrouter;
    bug_on(!router, ERR_NULLP);

    /* Receive Packets */
    universal_router_recv_pkt(router);

    /* Update flow control information */
#ifdef FLOW_CONTROL
    router_update_flowcontrol(router);
#endif

    /* Select packets with minimum tick from all recv_fifo, besides it can be sent out from this port */
    real_pkt_get = select_minimum_pkt(router);

    /* Print Statistic Information */
#ifdef TRACE_OPEN
    int i = 0;
    float inband = 0.0000;
    float outband = 0.0000;
    float avg_delay = 0.0000;
    if (router->tick - router->last_trace_time > 100000) {
	router->last_trace_time = router->tick;
	fprintf(router->fp, "at time %" Fu_time ":\n", router->tick);
	for (i = 0; i < router->router_port_num; i++) {
	    inband    += 8 * router->port[i].input_bytes / router->input_stat_tick;
	    outband   += 8 * router->port[i].output_bytes / router->port[i].trans_fifo.tick;
	    avg_delay += router->port[i].avg_delay;
	    fprintf(router->fp,
		    "       port %d:input bandwidth is %0.3f Gbps",
		    router->port[i].port_id,
		    8 * router->port[i].input_bytes / router->input_stat_tick);
	    fprintf(router->fp, "       port %d:output bandwidth is %0.3f Gbps;mean delay is %0.3f;packet number is %0.3f\n", router->port[i].port_id, 8 * router->port[i].output_bytes / router->port[i].trans_fifo.tick, router->port[i].avg_delay, router->port[i].pkt_cnt);	// realtime 
	    // recording 
	    // input 
	    // bandwidth
	}
	fprintf(router->fp,
		"The through put is %0.3f input is %0.3f output is %0.3f mean delay is %0.3f input pkt number is %f:\n",
		outband / inband, inband / (router->router_port_num - 1),
		outband / (router->router_port_num - 1),
		avg_delay / router->router_port_num, router->input_stat_time);
    }
#endif

    return (run_ret_t){real_pkt_get};
}

static htime_t router_get_lookahead(void *vrouter, cid_t cid)
{
    struct router *p_router;
    htime_t ahead_tick = 0;
    int i;
    func_enter();
    p_router = (struct router *) vrouter;
    bug_on(!p_router, ERR_NULLP);
    for (i = 0; i < p_router->router_port_num; i++)
	if (p_router->port[i].local_id == cid) {
	    ahead_tick = p_router->port[i].trans_fifo.tick + LINE_DELAY;
	    break;
	}
    func_return(ahead_tick);
}

void router_init(void *router_desc_ptr)
{
    int router_port;
    struct router *router;
    id_t local_id, dst_id;
    port_t tmp_port;
    router_desc_t *router_desc_uni = (router_desc_t *) router_desc_ptr;
    pmalloc(1, router);
    err_exit_on(!router, -1, ERR_MALLOC);
    router_data_init(router_desc_uni, router);
    entity_t entity;
    // compt_t *compt;
    entity.id = get_compt_id_new();
    sprintf(entity.name, "router%" Fu_compt_id, entity.id);
    entity.owner = router;
    entity.run = router_port_run;
    entity.get_lookahead = router_get_lookahead;
    entity.flag = MSG_WAKE_UP | LAVT_WAKE_UP;
    entity.ch_num = router->router_port_num;
    router->id = entity.id;
    compt_register_new(&entity);
   //init port 
    //printf("router->port_num in int %d \n",router->router_port_num);
    for (router_port = 0; router_port < router->router_port_num; router_port++) {
	local_id = router_desc_uni->port[router_port].local;
	dst_id = router_desc_uni->port[router_port].remote;
	router->port[router_port].local_id = local_id;
	router->port[router_port].dst_id = dst_id;
	debug("Init ROUTER router(%" Fu_compt_id ") port[%d]: local_id = %d, dst_id = %d", entity.id, router_port, local_id, dst_id);
	tmp_port.src_cid = local_id;
	tmp_port.dst_cid = dst_id;
	//printf("router++tem_port.src_id=%d  tem_port.dst_cid=%d\n",tmp_port.src_cid,tmp_port.dst_cid);
	router->port[router_port].port_id = router_port;
	port_register(entity.id, tmp_port);
    }

#ifdef TRACE_OPEN
    int i, j = 0, k;
    char filename[30];
    char tmp_title[] = "res1//router_";
    char tmp_id[8];
    sprintf(tmp_id, "%lu", router->index);
    for (i = 0; i < sizeof(tmp_title) / sizeof(char) - 1; ++i) {
	filename[i] = tmp_title[i];
    }
    for (j = (sizeof(tmp_title) / sizeof(char) - 1), k = 0;
	 j < (sizeof(tmp_title) / sizeof(char) + sizeof(tmp_id) / sizeof(char));) {
	filename[j++] = tmp_id[k++];
    }
    router->fp = fopen(filename, "w");
#endif
    func_return();
}

