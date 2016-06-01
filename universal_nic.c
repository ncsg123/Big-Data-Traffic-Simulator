/*
 * File name: universal_nic.c
 * Description: This file provide basic function of NIC.
 * Date:2013/10
 * */
#include"universal_nic.h"
#include"fundcl_nic.h"
#include"hoptions.h"
#include<math.h>
#include"macro_select.h"

htime_t universal_nic_get_positive(htime_t a, htime_t b)
{
   if(a>b)
      return (a-b);
   else
      return 0;
}

htime_t universal_nic_get_max(htime_t a, htime_t b, htime_t c)
{
   htime_t max_time = 0;
   if (max_time<a)
      max_time = a;
   if (max_time <b )
      max_time = b;
   if (max_time <c )
      max_time = c;
   return max_time;
}

static void nic_data_init(nic_desc_t * nic_desc, struct nic *nic)
{
	int i, j;
	func_enter();
	/* set port number */
	nic->m = nic_desc->m;
	nic->n = nic_desc->n;
	nic->nic_vc_num = nic_desc->vcnum;
	nic->nic_port_num = nic_desc->nicnum;
	bug_on(nic->nic_port_num > NIC_XPORT_NUM,
	       "Too many ports (%d ports) for nic", nic->nic_port_num);
	nic->frequency = nic_desc->frequency;
	nic->bus_width = nic_desc->bus_width;
	nic->fifo_depth = nic_desc->fifo_depth;
	nic->index = nic_desc->index;
	bug_on(nic->index > 100000,"nic->index is wrong!");
	nic->vc_allocated = 0;
	nic->priority_port = 0;
	nic->priority_vc = 0;
    /*statstics init*/
	nic->output_bytes = 0;
	nic->output_cnt = 0;
	nic->input_bytes = 0;
	nic->input_cnt = 0;
	nic->input_delay = 0;
	nic->hop_cnt = 0;
	nic->last_stat_tick = 0;
	nic->do_stat = false;
	nic->nb_port_num = 1;
	/* init all port data */
	for (i = 0; i < nic->nic_port_num + 1; i++) {
		nic->port[i].port_id = i;
		nic->port[i].local_id = nic_desc->port[i].local;
		nic->port[i].dst_id = nic_desc->port[i].remote;
		nic->port[i].tick = 0;
		nic->port[i].la_tick = 0;
		nic->port[i].nic = nic;
		nic->port[i].trans_fifo.tick = 0;
		nic->port[i].nic_vc_num = nic->nic_vc_num;

		Q_init(&nic->port[i].trans_fifo.queue, NET_Q_LEN);
		for (j = 0; j < nic->nic_vc_num; j++) {
			nic->port[i].recv_fifo[j].tick = 0;
			nic->port[i].credit[j] = nic->fifo_depth;
			nic->port[i].credit_for_increment[j].tick = 0;
			nic->port[i].priority[j] = 0;
			Q_init(&nic->port[i].credit_for_increment[j].queue,
			       NET_Q_LEN);
			Q_init(&nic->port[i].recv_fifo[j].queue,
			       NET_Q_LEN);
		}
	}
	func_return();
}

static inline void trace_nic(struct nic * nic)
{
	int w;
	if((nic->tick - nic->last_stat_tick) > 100000){
			fprintf(nic->fp,"nic's tick: %"Fu_time"; nic's output: %f; nic's input: %f; ",
					nic->tick, 8*nic->output_bytes/nic->tick, 8*nic->input_bytes/nic->tick);
				fprintf(nic->fp,"input delay is: %f; average hop_cnt: %f input cnt: %f \n",nic->input_delay/nic->input_cnt,nic->hop_cnt/nic->input_cnt,nic->input_cnt);
			for(w = 0 ; w < MAX_STAGE-2; w++)
			{
//				fprintf(nic->fp,"stage_delay[%d] is:%f\n",w,nic->stage_delay[w]/nic->input_cnt);
			}
		nic->last_stat_tick = nic->tick;
	}
}

static inline void universal_nic_recv_pkt(struct nic *nic)
{
	void *pkt = NULL;
	int i = 0;
	int j = 0;
	int nic_port_num = nic->nic_port_num;
	/* Make unicast packets, send them to nic */
	for (i = 0; i < nic->nic_vc_num; i++) {
		if (!Q_full(&nic->port[nic_port_num].recv_fifo[i].queue) && judge_vc(i)) {
			j = rand() % 100;
#ifdef NETWORK_TYPE_D7K_VERSION1
			if(i < 4 )
			{
			    if(j < inject_rate)
			    {
				       pkt = universal_nic_make_unicast_pkt(nic,i);
				       universal_nic_form_nic_packet(&(nic->port[nic_port_num]), pkt,i);
			    }
			    else
				nic->port[nic_port_num].la_tick += (g_pkt_length / nic->bus_width)*nic->frequency;
			}
			else
			{
			    if(j < 15)
			    {
				       pkt = universal_nic_make_unicast_pkt(nic,i);
				       universal_nic_form_nic_packet(&(nic->port[nic_port_num]), pkt,i);
			    }

			}

#else
			if (j < inject_rate) {
				       pkt = universal_nic_make_unicast_pkt(nic,i);
				       universal_nic_form_nic_packet(&(nic->port[nic_port_num]), pkt,i);
			} 
			else
				nic->port[nic_port_num].la_tick += (g_pkt_length / nic->bus_width)*nic->frequency;
#endif
		}
	}
	/*  Receive Packets from nic ports */
	for (i = 0; i < nic->nic_port_num; i++) {
		pkt = sync_recv(nic->port[i].local_id);
		/*  adjust time tick */
		if (pkt == NULL) {
			nic->port[i].la_tick =
			    get_port_min_pkt_tick(nic->port[i].local_id);
			for (j = 0; j < nic->nic_vc_num; j++)
				if ((Q_state(&nic->port[i].recv_fifo[j].queue) == Q_EMPTY)
				    && (nic->port[i].recv_fifo[j].tick < nic->port[i].la_tick))
					nic->port[i].recv_fifo[j].tick = nic->port[i].la_tick;
			continue;
		}
		while (pkt) {
			universal_nic_port_recv_pkt(&(nic->port[i]), pkt);
			pkt = sync_recv(nic->port[i].local_id);
		}
	}
}

static inline void universal_update_flowcontrol(struct nic *nic)
{
   struct nic_port *base_port = &nic->port[0];
   void *head_pkt;
   fc_pkth_t *head_pkth;
   htime_t head_tick;
   int i,j;
   for (i = 0; i< nic->nic_port_num; i++, base_port++){
      for (j = 0; j< nic->nic_vc_num; j++){
         if(Q_state(&base_port->credit_for_increment[j].queue) != Q_EMPTY){
            head_tick = base_port->recv_fifo[j].tick + NIC_DELAY*nic->frequency;
            while(base_port->credit_for_increment[j].tick <= head_tick){
               head_pkt = Q_dequeue_ele(&base_port->credit_for_increment[j].queue);
               auto_get_pkth(head_pkt, head_pkth);
               base_port->credit[j] = base_port->credit[j] + head_pkth->credit;
               sync_release_buf(head_pkt);
               if (Q_state(&base_port->credit_for_increment[j].queue) != Q_EMPTY){
                  head_pkt = Q_front_ele(&(base_port->credit_for_increment[j].queue));
                  base_port->credit_for_increment[j].tick = get_pkt_tick(head_pkt);
               }
               else
                  break;
            }
         }
      }
   }
}   

void universal_set_compt_tick(compt_id_t id, htime_t tick) 
{
   int i;
   kcompt_t * kc;
   cid_t src_cid, dst_cid;
   htime_t lavt, min_lavt;
   kc = get_kcompt(id);

   min_lavt = (htime_t)-1;
   for (i = 0; i < kc->ch_num; i++) {
      lavt = kc->ch[i]->lavt;  //Note: must copy first, then test and set
      if(lavt < min_lavt)
         min_lavt = lavt;
   }
   if(min_lavt < kc->lavt){
      bug("compt(%s)'s lavt reverse, min_lavt:%"Fu_time" is less than component lavt %"Fu_time, 
            kc->compt->name, min_lavt, kc->lavt);
      printf("compt(%s)'s lavt reverse, min_lavt:%"Fu_time" is less than component lavt %"Fu_time, 
            kc->compt->name, min_lavt, kc->lavt);
   }
   kc->lavt = min_lavt;
   if (tick >= min_lavt) {
      block_compt(kc, WAIT_LAVT);
      kc->blk_lavt = min_lavt;
      for (i = 0; i < kc->ch_num - 1 ; i++) {
         src_cid = kc->ch[i]->lcid;
         dst_cid = kc->ch[i]->rcid;
         lavt = kc->get_lookahead(kc->compt->owner, kc->ch[i]->lcid);
         send_lookahead(src_cid, dst_cid, lavt, SYNC_LAR);
      }
   }
}

static inline void * universal_nic_output_arbiter(struct nic *nic, id_t *port_id, htime_t *mini_tick, bool priority_change)//this is a vc_num*port_num x vc_num*port_num crossbar
{     
   id_t tmp_id = 0;
   struct nic_port *tmp_port = nic->port;
   void *tmp_pkt = NULL;
   void *mini_pkt = NULL;
   htime_t tmp_tick = MAX_TIME;
   int i = 0, j = 0;

#if (ARBITER_ALGORITHM==0) //loop priority
   int tmp_vc=0;
   for (i = 0; i< (nic->nic_port_num+nic->nb_port_num); i++, tmp_port++){
      for (j = 0;j < nic->nic_vc_num; j++){
         tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
         if(tmp_tick > tmp_port->recv_fifo[j].tick){
            tmp_tick = tmp_port->recv_fifo[j].tick;
            mini_pkt = tmp_pkt;
            tmp_id = i;
            tmp_vc = j;

         }
         else if((tmp_tick == tmp_port->recv_fifo[j].tick)&&tmp_pkt){
            if((tmp_port->priority[j]==1) ||(mini_pkt==NULL)){
               mini_pkt = tmp_pkt;
               tmp_id = i;
               tmp_vc = j;
            }
         }
      }
   }
   if(mini_pkt!=NULL){
      nic->port[nic->priority_port].priority[nic->priority_vc] = 0;
      nic->port[tmp_id].priority[tmp_vc] = 0;
         if(tmp_vc==(nic->nic_vc_num-1)){ 
            if(tmp_id==((nic->nic_port_num+nic->nb_port_num)-1)){
               nic->port[0].priority[0] = 1;
               nic->priority_port = 0;
               nic->priority_vc = 0;
            }
            else{
               nic->priority_port = tmp_id+1;
               nic->priority_vc = 0;
               nic->port[nic->priority_port].priority[0] = 1;
            }
         }
         else{
            nic->priority_vc = tmp_vc+1;
            nic->port[nic->priority_port].priority[nic->priority_vc] = 1;
         }
      }

#elif  (ARBITER_ALGORITHM==1)//round robin

      int tmp_vc=0;
      for (i = 0; i < (nic->nic_port_num+nic->nb_port_num); i++) {
         tmp_tick = get_port_min_pkt_tick(nic->port[i].local_id);
         for(j = 0; j < nic->nic_vc_num-1; j++)
            if((Q_state(&nic->port[i].recv_fifo[j].queue)==Q_EMPTY)&&(nic->port[i].recv_fifo[j].tick < tmp_tick))
               nic->port[i].recv_fifo[j].tick = tmp_tick;
      }

      if(Q_state(&nic->port[0].recv_fifo[nic->nic_vc_num-1].queue)==Q_EMPTY){
         if((NIC_DELAY*nic->frequency+nic->port[0].recv_fifo[nic->nic_vc_num-1].tick)<nic->port[1].trans_fifo.tick)
            nic->port[0].recv_fifo[nic->nic_vc_num-1].tick = nic->port[1].trans_fifo.tick-NIC_DELAY*nic->frequency;
         else {
            for(i=0;i<nic->nic_vc_num-1;i++){
               if(nic->port[0].recv_fifo[i].tick>nic->port[0].recv_fifo[nic->nic_vc_num-1].tick)
                  nic->port[0].recv_fifo[nic->nic_vc_num-1].tick = nic->port[0].recv_fifo[i].tick;
            }
         }
      }

      if(Q_state(&nic->port[1].recv_fifo[nic->nic_vc_num-1].queue)==Q_EMPTY)
         if((NIC_DELAY*nic->frequency+nic->port[1].recv_fifo[nic->nic_vc_num-1].tick)<nic->port[0].trans_fifo.tick)
            nic->port[1].recv_fifo[nic->nic_vc_num-1].tick =nic->port[0].trans_fifo.tick-NIC_DELAY*nic->frequency;	

      tmp_tick=MAX_TIME;
      for (i = 0; i< (nic->nic_port_num+nic->nb_port_num); i++, tmp_port++){
         for (j = 0;j < nic->nic_vc_num; j++){
            tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
            if(tmp_tick > tmp_port->recv_fifo[j].tick){
               tmp_tick = tmp_port->recv_fifo[j].tick;
               mini_pkt = tmp_pkt;
               tmp_id = i;
               tmp_vc = j;
            }
            else if((tmp_tick == tmp_port->recv_fifo[j].tick)&&tmp_pkt){
               if((tmp_port->priority[j]==1) ||(mini_pkt==NULL)){
                  mini_pkt = tmp_pkt;
                  tmp_id = i;
                  tmp_vc = j;
               }
            }
         }
      }
      if((priority_change==true) && (mini_pkt!=NULL)){
         nic->port[nic->priority_port].priority[nic->priority_vc] = 0;
         nic->port[tmp_id].priority[tmp_vc] = 0;
            if(tmp_vc==(nic->nic_vc_num-1)){
               if(tmp_id==((nic->nic_port_num+nic->nb_port_num)-1)){
                  nic->port[0].priority[0] = 1;
                  nic->priority_port = 0;
                  nic->priority_vc = 0;
               }
               else{
                  nic->priority_port = tmp_id+1;
                  nic->priority_vc = 0;
                  nic->port[nic->priority_port].priority[0] = 1;
               }
            }
            else{
               nic->priority_port = tmp_id;
               nic->priority_vc = tmp_vc+1;
               nic->port[tmp_id].priority[nic->priority_vc] = 1;
            }
         }

#elif  (ARBITER_ALGORITHM==2)//matrix Arbiter

         int tmp_vc=0; 
	 htime_t trans_max = 0;
	 htime_t recv_max = 0;

	 /* Find the max trans_fifo time stamp from the sending fifo */
	 for(i = 0; i < nic->nic_port_num; i++)
	 {
	     if(nic->port[i].trans_fifo.tick > trans_max)
		 trans_max = nic->port[i].trans_fifo.tick;
	 }
	 /* Find the max time stamp from the north bridge port */
	 for(i = 0; i < nic->nic_vc_num; i++)
	 {
	     if(judge_vc(i) && nic->port[nic->nic_port_num].recv_fifo[i].tick > recv_max )
		 recv_max = nic->port[nic->nic_port_num].recv_fifo[i].tick;
	 }
	 for(i = 0; i < nic->nic_vc_num; i++)
	 {
	     if(!judge_vc(i) && Q_state(&nic->port[nic->nic_port_num].recv_fifo[i].queue) == Q_EMPTY)
	     {
		 if(NIC_DELAY * nic->frequency + nic->port[nic->nic_port_num].recv_fifo[i].tick < trans_max)
		 {
		     nic->port[nic->nic_port_num].recv_fifo[i].tick = trans_max - NIC_DELAY * nic->frequency;
		 }
		 else
		 {
		     if(nic->port[nic->nic_port_num].recv_fifo[i].tick < recv_max)
			 nic->port[nic->nic_port_num].recv_fifo[i].tick = recv_max;
		 }

	     }
	 }
	
	 for (i = 0; i < nic->nic_port_num+1; i++) {
	     if(i == nic->nic_port_num)
		 tmp_tick = nic->port[nic->nic_port_num].la_tick;
	     else
	     	 tmp_tick = get_port_min_pkt_tick(nic->port[i].local_id);
	     for (j = 0; j < nic->nic_vc_num; j++)
	         if ((Q_state(&nic->port[i].recv_fifo[j].queue) == Q_EMPTY) && (nic->port[i].recv_fifo[j].tick < tmp_tick))
	         nic->port[i].recv_fifo[j].tick = tmp_tick;
	 }
	 
	 tmp_tick = MAX_TIME;
	 
         int priority = 0;
         for (i = 0; i< (nic->nic_port_num+nic->nb_port_num); i++, tmp_port++){
            for (j = 0;j < nic->nic_vc_num; j++){
               tmp_pkt = Q_front_ele(&tmp_port->recv_fifo[j].queue);
               if(tmp_tick > tmp_port->recv_fifo[j].tick){
                  tmp_tick = tmp_port->recv_fifo[j].tick;
                  mini_pkt = tmp_pkt;
                  tmp_id = i;
                  tmp_vc = j;
                  priority = tmp_port->priority[j];
               }
               else if((tmp_tick == tmp_port->recv_fifo[j].tick)&&tmp_pkt){
                  if((tmp_port->priority[j] >= priority)||(!mini_pkt)){
                     mini_pkt = tmp_pkt;
                     tmp_id = i;
                     tmp_vc = j;
                     priority = tmp_port->priority[j];
                  }
               }
            }
         }
         if((priority_change==true) && (mini_pkt!=NULL)){ 
            for (i = 0; i< (nic->nic_port_num+nic->nb_port_num); i++){
               for (j = 0;j < nic->nic_vc_num; j++){
                  nic->port[i].priority[j] = nic->port[i].priority[j] + 1; 
               }
            }
            if (nic->port[tmp_id].priority[tmp_vc]>=2)
               nic->port[tmp_id].priority[tmp_vc] = nic->port[tmp_id].priority[tmp_vc] - 2; 
         }

#endif

         for (i = 0; i< (nic->nic_port_num+nic->nb_port_num); i++){
            if (nic->port[i].trans_fifo.tick < tmp_tick+NIC_DELAY*nic->frequency) 
               nic->port[i].trans_fifo.tick = tmp_tick+NIC_DELAY*nic->frequency;  
         }
         if (mini_pkt != NULL) {
            if(get_pkt_tick(mini_pkt)>tmp_tick)
               printf("error__nic\n");
            bug_on(get_pkt_tick(mini_pkt)>tmp_tick, ERR_TICKREVERSE "tick in packet is bigger than in recv_fifo");
            *mini_tick = get_pkt_tick(mini_pkt);
            set_pkt_tick(mini_pkt, tmp_tick); /* update packet's tick with latest schedule tick */
            *port_id = tmp_id;
         }
         nic->tick = tmp_tick+LINE_DELAY;
         universal_set_compt_tick(nic->id, nic->tick); 
         return mini_pkt;
} 

bool universal_select_minimum_pkt(struct nic *nic)
{
   id_t tmp_id = 0;
   htime_t mini_tick=0;
   void *mini_pkt=NULL;
   bool real_pkt_done=FALSE;

   /* select packet with minimum tick from recv_fifos */
   mini_pkt = universal_nic_output_arbiter(nic,&tmp_id,&mini_tick,true);

   if (mini_pkt){
       	if(pkt_type(mini_pkt)== NET_UNICAST){
	    /* tmp_id is port id which will send pkt */
            real_pkt_done = (universal_nic_handle_unicast(nic,mini_pkt,tmp_id));
      	}
       
   	universal_nic_output_arbiter(nic,&tmp_id,&mini_tick,false);
     
   	return real_pkt_done;
   } 

   return false;
}

static run_ret_t universal_nic_port_run(void *vnic)
{

	struct nic *nic;
	bool real_pkt_get;

	func_enter();
	real_pkt_get = false;

	/* check param */
	bug_on(!vnic, ERR_NULLP);
	nic = (struct nic *) vnic;
	bug_on(!nic, ERR_NULLP);

	bug_on((unsigned int)nic->index > 100000,"nic->index is wrong!%d",nic->index);
	//debug("=====nic->tick=%lu=====\n",nic->tick);
	debug("%d\n", hnet_opt.universal_tick_num);
	if (nic->tick > hnet_opt.universal_tick_num) {
		compt_finish();
	}
#ifdef TRACE_OPEN
	trace_nic(nic);
#endif
	universal_nic_recv_pkt(nic);

	universal_update_flowcontrol(nic);

	real_pkt_get = universal_select_minimum_pkt(nic);

	        return (run_ret_t){real_pkt_get};
}


static htime_t universal_nic_get_lookahead(void *vnic, cid_t cid)
{
   struct nic *p_nic;
   htime_t ahead_tick=0;
   func_enter();
   p_nic = (struct nic*)vnic;
   bug_on(!p_nic, ERR_NULLP);
   int i = 0;
   for(i=0;i<(p_nic->nic_port_num+1);i++)
      if(p_nic->port[i].local_id == cid){
         ahead_tick = p_nic->port[i].trans_fifo.tick+LINE_DELAY;
         break;
      }
   func_return(ahead_tick);
}


void inline trace_create(struct nic * nic)
{
	int j = 0, i, k;
	char filename[100];
	char tmp_title[] = "res2//nic_";
	char tmp_id[8];
		sprintf(tmp_id, "%u", nic->index);
		for (i = 0; i < sizeof(tmp_title) / sizeof(char) - 1; ++i) {
			filename[i] = tmp_title[i];
		}
		for (j = (sizeof(tmp_title) / sizeof(char) - 1), k = 0;
			       	j < (sizeof(tmp_title) / sizeof(char) +sizeof(tmp_id) / sizeof(char));) {
			filename[j++] = tmp_id[k++];
		}
		nic->fp = fopen(filename, "w");
}

void nic_init(void *nic_desc_ptr)
{
       
      // printf("nic_counter:%d\n",counter_nic);
	struct nic *nic;
	port_t tmp_port;	/*define in /kernel/commu.h */
	nic_desc_t *nic_desc_uni = (nic_desc_t *) nic_desc_ptr;

	pmalloc(1, nic);
	nic_data_init(nic_desc_uni, nic);

	entity_t entity;

	entity.id = get_compt_id_new();
	sprintf(entity.name, "nic%" Fu_compt_id, entity.id);
	entity.owner = nic;
	entity.run = universal_nic_port_run;
	entity.get_lookahead = universal_nic_get_lookahead;
	entity.flag = NO_BLK;// LAVT_WAKE_UP| MSG_WAKE_UP;
	entity.ch_num = nic->nic_port_num + 1;
	entity.halt_dm = true;
	nic->id = entity.id;

	compt_register_new(&entity);
	/* init nic port */
	int i;
	for (i = 0; i < entity.ch_num; i++) {
		tmp_port.src_cid = nic->port[i].local_id;	// port[n] is not nic port
		tmp_port.dst_cid = nic->port[i].dst_id;
		//printf("tem_port.src_id=%d  tem_port.dst_cid=%d\n",tmp_port.src_cid,tmp_port.dst_cid);
		port_register(entity.id, tmp_port);
	}
#ifdef TRACE_OPEN
	trace_create(nic);
#endif
       counter_nic--;
       if(counter_nic == 0)
       {
//	   printf("pointer address:%p\n",nic_desc);
//	   printf("sizeof nic_desc %lu\n" ,sizeof(nic_desc));
	   free(nic_desc);
       }
}
