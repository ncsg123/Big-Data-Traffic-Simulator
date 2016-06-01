#include <libxml2/libxml/parser.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "simk.h"
#include "parser.h"
#include "hoptions.h"
#include "sw_config_register.h"

#define PRINT_TOPO
//#define A2A_1
#define A2A_2

static struct {
    int inject_rate;
    int nicnum;
    int nic_vcnum;
    int router_vcnum;
    int sw_vcnum;
    int fifo_depth;
    int nic_frequency;
    int sw_frequency;
    int nic_bus_width;
    int sw_bus_width;
} net_param;

static struct {
    int m;
    int n;
} ftree_param;

static struct{
    int m;
    int n;
    int r_m;
}tor_ft_param;

static struct{
    int sw_port_number;
    int layer_number;
    int router_port_number;
}vtor_ft_param;

static struct{
    int k;
    int n;
} flattened_bfly_param;

static struct{
    int m;
    int n;
    char *file_sflbfly;
} sflbfly_param;

static struct {
    int m;
    int n;
    char *file_mesh;
} mesh_param;

static struct {
    int m;	//port number
    int n;	//dimensions
    char *file_torus;
}torus_param;

static struct {
    int m;
    int n;
}all_to_all_param;

static struct {
    int m;	//port number
    int n;	//dimensions
    int o_component;
    bool o_torus;
    bool o_atoa;
    bool o_flbfly;
    char *file1;
    char *file2;
}universal_param;

static struct {
    int m;
    int n;
    int torport; //top of router port number;
}ftree_a2a_param;

nic_desc_t *nic_desc = NULL;
router_desc_t *router_desc = NULL;
sw_desc_t *sw_desc = NULL;

int *cmap  = NULL;	//connetion map of torus
int *direction = NULL;	//port represent the directions
int *dms = NULL;	//dimension size
int *xy_direc = NULL;	//
int **matrix_map = NULL;//teporary
int ***topo_map = NULL;//
int *num_component = NULL;//row
int *column = NULL;//column
int *lev_num =NULL;//every level sw number
int xyz;		//global dimension number
int topo_level = 0;
levnd_desc_t **lev_res = NULL;
int global_swnum = 0;
int global_routernum = 0;
int inject_rate;
int g_pkt_length;
int counter_nic;
int counter_sw;
int router_port_num;
int nic_num;

/* defined in nic.c and sw.c */
extern void nic_init(void *);
extern void sw_init(void *);
extern void router_init(void *);

#define PARSE_BOOL(var, doc, cur_child) do { \
    xmlChar *key = xmlNodeListGetString(doc, cur_child->xmlChildrenNode, 1);	\
    if(!strcmp((char*)key, "no"))  \
	var = false;  \
    else if(!strcmp((char*)key, "yes"))   \
	var = true;   \
    else							\
	fprintf(stderr, "%s flags error: %s", cur_child->name, (char *)key);  \
    xmlFree(key);				\
} while (0)

#define PARSE_INT(var, doc, cur_child) do { \
    xmlChar *key = xmlNodeListGetString(doc, cur_child->xmlChildrenNode, 1);	\
    int int_val = atoi((char *)key);    \
    xmlFree(key);				\
    var = int_val; \
} while (0)

#define PARSE_STRING(var, doc, cur_child) do { \
    xmlChar *key = xmlNodeListGetString(doc, cur_child->xmlChildrenNode, 1);  \
    strcpy(var,(char*)key);                  \
    xmlFree(key);				\
}while(0);

bool parse_ftree_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(ftree_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(ftree_param.n, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_torft_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(tor_ft_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(tor_ft_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"r_m")) {
	    PARSE_INT(tor_ft_param.r_m, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_vtorft_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"sw_port_number")) {
	    PARSE_INT(vtor_ft_param.sw_port_number, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"layer_number")) {
	    PARSE_INT(vtor_ft_param.layer_number, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"router_port_number")) {
	    PARSE_INT(vtor_ft_param.router_port_number, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_flattened_bfly_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"k")) {
	    PARSE_INT(flattened_bfly_param.k, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(flattened_bfly_param.n, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_sflbfly_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(sflbfly_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(sflbfly_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"file")) {
	    PARSE_STRING(sflbfly_param.file_sflbfly, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_mesh_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(mesh_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(mesh_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"file")) {
	    PARSE_STRING(mesh_param.file_mesh, doc, cur);
	} else {
	    fprintf(stderr, "mesh topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_torus_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(torus_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(torus_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"file")) {
	    PARSE_STRING(torus_param.file_torus, doc, cur);
	} else {
	    fprintf(stderr, "torus topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_ftree_a2a_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(ftree_a2a_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(ftree_a2a_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"torport")) {
	    PARSE_INT(ftree_a2a_param.torport, doc, cur);
	} else {
	    fprintf(stderr, "mesh topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}


bool parse_all_to_all_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(all_to_all_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(all_to_all_param.n, doc, cur);
	} else {
	    fprintf(stderr, "mesh topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_universal_topo(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"m")) {
	    PARSE_INT(universal_param.m, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"n")) {
	    PARSE_INT(universal_param.n, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"o_component")) {
	    PARSE_INT(universal_param.o_component, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"o_torus")) {
	    PARSE_BOOL(universal_param.o_torus, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"o_atoa")) {
	    PARSE_BOOL(universal_param.o_atoa, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"file1")) {
	    PARSE_STRING(universal_param.file1, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"file2")) {
	    PARSE_STRING(universal_param.file2, doc, cur);
	} else {
	    fprintf(stderr, "universal topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool parse_nic_sw_param(xmlDocPtr doc, xmlNodePtr nodeptr)
{
    xmlNodePtr cur = nodeptr->children;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"nic_bus_width")) {
	    PARSE_INT(net_param.nic_bus_width, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"sw_bus_width")) {
	    PARSE_INT(net_param.sw_bus_width, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"nic_frequency")) {
	    PARSE_INT(net_param.nic_frequency, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"sw_frequency")) {
	    PARSE_INT(net_param.sw_frequency, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"pkt_length")) {
	    PARSE_INT(g_pkt_length, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"inject_rate")) {
	    PARSE_INT(net_param.inject_rate, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"nic_portnum")) {
	    PARSE_INT(net_param.nicnum, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"nic_vcnum")) {
	    PARSE_INT(net_param.nic_vcnum, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"router_vcnum")) {
	    PARSE_INT(net_param.router_vcnum, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"sw_vcnum")) {
	    PARSE_INT(net_param.sw_vcnum, doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"fifo_depth")) {
	    PARSE_INT(net_param.fifo_depth, doc, cur);
	} else {
	    fprintf(stderr, "ftree topo parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

static bool get_net_type(xmlDocPtr doc, xmlNodePtr cur)
{
    xmlChar *key;

    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *)"select")) {
	    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
	    if (!strcmp("ftree", (char *)key)) {
		hnet_opt.net_type = FAT_TREE;
	    } else if (!strcmp("torft", (char *)key)) {
		hnet_opt.net_type = TORFT;
	    } else if (!strcmp("vtorft", (char *)key)) {
		hnet_opt.net_type = VTORFT;
	    } else if (!strcmp("flattened_butterfly", (char *)key)) {
		hnet_opt.net_type = FLBFLY;
	    } else if (!strcmp("sflbfly", (char *)key)) {
		hnet_opt.net_type = SFLBFLY;
	    } else if (!strcmp("mesh", (char *)key)) {
		hnet_opt.net_type = MESH;
	    } else if (!strcmp("torus", (char *)key)) {
		hnet_opt.net_type = TORUS;
	    } else if (!strcmp("ftreea2a", (char *)key)) {
		hnet_opt.net_type = FTREE_A2A;
	    } else if (!strcmp("all_to_all", (char *)key)) {
		hnet_opt.net_type = ALL_TO_ALL;
	    } else if (!strcmp("universal", (char *)key)) {
		hnet_opt.net_type = UNIVERSAL;
	    } else {
		fprintf(stderr, "unknown network type selection");
		return false;
	    }
	} else if (!xmlStrcmp(cur->name, (xmlChar *)"text")) {
	    ;
	} else {
	    fprintf(stderr, "selection parse error\n");
	    return false;
	}
	cur = cur->next;
    }
    return true;
}

bool get_target_param(const char *filename)
{
    xmlDocPtr doc;
    xmlNodePtr cur;
    bool retval;

    retval = false;
    doc = xmlParseFile(filename);
    if (doc == NULL) {
	fprintf(stderr, "config file %s does not exist\n", filename);
	xmlFreeDoc(doc);
	return false;
    }
    cur = xmlDocGetRootElement(doc);
    if (xmlStrcmp(cur->name, (xmlChar *) "hpp_net_sim")) {
	fprintf(stderr, "root element is not \"hpp_net_sim\"\n");
	xmlFreeDoc(doc);
	return false;
    }

    cur = cur->xmlChildrenNode;
    while (cur != NULL) {
	if (!xmlStrcmp(cur->name, (xmlChar *) "ftree_param")) {
	    retval = parse_ftree_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "nic_sw_param")) {
	    retval = parse_nic_sw_param(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "flattened_butterfly_param")) {
	    retval = parse_flattened_bfly_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "simple_flbfly_param")) {
	    retval = parse_sflbfly_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "torft_param")) {
	    retval = parse_torft_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "vtorft_param")) {
	    retval = parse_vtorft_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "mesh_param")) {
	    retval = parse_mesh_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "torus_param")) {
	    retval = parse_torus_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "ftree_a2a_param")) {
	    retval = parse_ftree_a2a_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "all_to_all_param")) {
	    retval = parse_all_to_all_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "universal_param")) {
	    retval = parse_universal_topo(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "select")) {
	    retval = get_net_type(doc, cur);
	} else if (!xmlStrcmp(cur->name, (xmlChar *) "text")) {
	    retval = true;
	} else {
	    fprintf(stderr, "parse param file error\n");
	    xmlFreeDoc(doc);
	    return false;
	}
	if (!retval) {
	    xmlFreeDoc(doc);
	    return false;
	}
	cur = cur->next;
    }
    xmlFreeDoc(doc);
    return true;
}


bool get_self_param(const char* file_name)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *key;
	int num=0;

	doc = xmlParseFile(file_name);
	if(doc == NULL){
		fprintf(stderr, "self config file %s not exist\n", file_name);
		xmlFreeDoc(doc);
		return false;
	}

	cur = xmlDocGetRootElement(doc);
	if(cur == NULL){
		fprintf(stderr, "self config file %s get root element error\n", file_name);
		xmlFreeDoc(doc);
		return false;
	}
	if(xmlStrcmp(cur->name, (xmlChar*)"simulator")){
		fprintf(stderr, "root element is not \"simulator\"\n");
		xmlFreeDoc(doc);
		return false;
	}

	cur = cur->xmlChildrenNode;
	while(cur != NULL){
		if(!xmlStrcmp(cur->name, (xmlChar*)"thread_num")){
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (!strcmp((char *)key, "auto")) {
				num = 0;
			} else {
				num = atoi((char*)key);
			}
			xmlFree(key);
		} else if (!xmlStrcmp(cur->name, (xmlChar*)"text")) {
		} else {
			fprintf(stderr, "parse param file error\n");
			xmlFreeDoc(doc);
			return false;
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
	hnet_opt.k_mode.thread_num = num;
	return true;
}

void confg_topo_ftree()
{
    int m = ftree_param.m;  //port number
    int n = ftree_param.n;  //level number
    int n_nicport = net_param.nicnum;
    int i,j,k;

    /* calculate the number of nic and sw at each level */
    int nr_nic = 2 * pow(m / 2, n)/n_nicport;
    int nr_sw = (2 * n - 1) * pow(m / 2, n - 1);
    global_swnum = nr_sw;
    int nr_top = pow(m/2, n-1);
    int nr_low = nr_top * 2;
    printf("m = %d, n = %d, nr_nic = %d, nr_sw = %d\n", m, n, nr_nic, nr_sw);

    nic_num = nr_nic;
    nic_desc = malloc(sizeof(nic_desc_t) * nr_nic);
    sw_desc = malloc(sizeof(sw_desc_t) * nr_sw);

    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < nr_nic; i++) {
	nic_desc[i].m          = ftree_param.m;
	nic_desc[i].n          = ftree_param.n;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nr_sw; i++) {
	sw_desc[i].m          = ftree_param.m;
	sw_desc[i].n          = ftree_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < m; j++) {
	    sw_desc[i].port[j].local =(n_nicport + 1) * nr_nic + i * m + j;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = nr_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }


    /* topo */
/*
    for (i = 0; i < nr_nic; i++) {
	if (n == 1) {
	    nic_desc[i].port[0].remote = sw_desc[0].port[i].local;
	    sw_desc[0].port[i].remote = nic_desc[i].port[0].local;
	    simk_connect_node(i, nr_nic, 1);
	} else {
	    nic_desc[i].port[0].remote = sw_desc[2*i/m].port[i%(m/2)].local; 
	    sw_desc[2*i/m].port[i%(m/2)].remote = nic_desc[i].port[0].local;
	    simk_connect_node(i, nr_nic + 2*i/m, 1);
      }
    }
*/
    for (i = 0; i < nr_nic; i++) {
	for(j = 0; j < n_nicport; j++){
	    if (n == 1) {
		nic_desc[i].port[j].remote = sw_desc[0].port[i*n_nicport + j].local;
		sw_desc[0].port[i*n_nicport + j].remote = nic_desc[i].port[j].local;
		simk_connect_node(i, nr_nic, 1);
	    } else {
		nic_desc[i].port[j].remote = sw_desc[2*(i*n_nicport+j)/m].port[(i*n_nicport+j)%(m/2)].local; 
		sw_desc[2*(i*n_nicport+j)/m].port[(i*n_nicport+j)%(m/2)].remote = nic_desc[i].port[j].local;
		simk_connect_node(i, nr_nic + 2*(i*n_nicport+j)/m, 1);
	      }
	}
    }
    if (n == 1)
	return;
    sw_desc_t *sw1, *sw2;
    int	    idx1, idx2;
    int layer;
    for (i = 0; i < nr_sw - nr_top - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nic + (int)(sw1 - sw_desc), nr_nic + (int)(sw2 - sw_desc), 1);
	}
    }
    for (i = nr_sw - 3 * nr_top; i < nr_sw - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nic + (int)(sw1 - sw_desc), nr_nic + (int)(sw2 - sw_desc), 1);

	    sw1= &sw_desc[i + nr_top];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1) + m/2;
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nic + (int)(sw1 - sw_desc), nr_nic + (int)(sw2 - sw_desc), 1);
	}
    }

#ifdef PRINT_TOPO
    for (i = 0; i < nr_nic; i++) {
	printf("nic  %d:", i);
	for(j = 0; j < n_nicport; j++)
		printf(" %d-%d\n", nic_desc[i].port[j].local, nic_desc[i].port[j].remote);
    }
    for (i = 0; i < nr_sw; i++) {
	printf("sw %d: ", i);
	for (j = 0; j < ftree_param.m; j++) {
	    printf("%d-%d ", sw_desc[i].port[j].local, sw_desc[i].port[j].remote);
	}
	printf("\n");
    }
#endif
}

void confg_topo_torft()
{
    int m = tor_ft_param.m;  //port number
    int n = tor_ft_param.n;  //level number
    int r_m = tor_ft_param.r_m;//router port number
    router_port_num = r_m;
   // printf("r_m = %d\n",r_m);
    int n_nicport = net_param.nicnum;
    int i,j,k;

    /* calculate the number of nic and sw at each level */
    int nr_router = 2 * pow(m/2,n-1); 
    int nr_nic = nr_router * (r_m - m/2)/n_nicport;
//    int nr_sw = (2 * n - 1) * pow(m / 2, n - 1);
    int nr_sw = (2 * n - 3) * pow(m / 2, n - 1);
    global_swnum = nr_sw;
    global_routernum = nr_router;
    nic_num = nr_nic;

    int nr_top = pow(m/2, n-1);
    int nr_low = nr_top * 2;
    printf("m = %d, n = %d, nr_nic = %d,nr_router = %d, nr_sw = %d\n", m, n, nr_nic,nr_router, nr_sw);

    nic_desc = malloc(sizeof(nic_desc_t) * nr_nic);
    router_desc = malloc(sizeof(router_desc_t) * nr_router);
    sw_desc = malloc(sizeof(sw_desc_t) * nr_sw);

    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < nr_nic; i++) {
	nic_desc[i].m          = tor_ft_param.m;
	nic_desc[i].n          = tor_ft_param.n;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t router_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = router_init,
	.data_size = sizeof(router_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };

    for (i = 0; i < nr_router; i++) {
	router_desc[i].m          = tor_ft_param.r_m;
	router_desc[i].n          = tor_ft_param.n;
	router_desc[i].vcnum      = net_param.router_vcnum;
	router_desc[i].fifo_depth = net_param.fifo_depth;
	router_desc[i].frequency = net_param.nic_frequency;
	router_desc[i].bus_width = net_param.nic_bus_width;
	router_desc[i].index      = i+nic_num;
	for(j=0; j < r_m;j++)
	{
	    router_desc[i].port[j].local = k++;
	    router_desc[i].port[j].remote = -1;
	}

	router_node.data_buf = &router_desc[i];
	router_node.id = nr_nic+i;
	simk_add_node(router_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nr_sw; i++) {
	sw_desc[i].m          = tor_ft_param.m;
	sw_desc[i].n          = tor_ft_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num+nr_router;
	for (j = 0; j < m; j++) {
	    sw_desc[i].port[j].local = k++;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = nr_nic + nr_router + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

    /* topo */

    for (i = 0; i < nr_nic; i++) {
	for(j = 0; j < n_nicport; j++){
		nic_desc[i].port[j].remote = router_desc[(i*n_nicport+j)/(r_m - m/2)].port[(i*n_nicport+j)%(r_m - m/2)].local; 
		router_desc[(i*n_nicport+j)/(r_m - m/2)].port[(i*n_nicport+j)%(r_m - m/2)].remote = nic_desc[i].port[j].local;
		//printf("id %d connect %d\n",i,nr_nic + (i*n_nicport + j)/(r_m - m/2));
		simk_connect_node(i, nr_nic + (i*n_nicport +j) / (r_m - m/2), 1);
	}
    }

    router_desc_t *router1;
    sw_desc_t  *sw1,*sw2;
    int	    idx1, idx2;
    int nr_nr = nr_nic + nr_router;
    int layer;
    for (i = 0; i < nr_router; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    router1= &router_desc[i];
	    idx1 = j + r_m -m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    router1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = router1->port[idx1].local;
	    simk_connect_node(nr_nic+i, nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    for (i = 0; i < nr_sw - nr_top - nr_low; i++) {
	layer = i / nr_low + 2;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    for (i = nr_sw - 3 * nr_top; i < nr_sw - nr_low; i++) {
	layer = i / nr_low + 2;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);

	    sw1= &sw_desc[i + nr_top];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1) + m/2;
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }

    /*
    
    for (i = 0; i < nr_router; i++) {
	for(j = 0; j < m/2; j++){
	    if (n == 1) {
		router_desc[i].port[r_m - m/2 + j].remote = sw_desc[0].port[i*m/2 + j].local;
		sw_desc[0].port[i*m/2 + j].remote = router_desc[i].port[r_m - m/2 +j].local;
		simk_connect_node(nr_nic+i, nr_nic+nr_router, 1);
	    } else {
		router_desc[i].port[r_m - m/2 +j].remote = sw_desc[2*(i*m/2+j)/m].port[(i*m/2+j)%(m/2)].local; 
		sw_desc[2*(i*m/2+j)/m].port[(i*m/2+j)%(m/2)].remote = router_desc[i].port[r_m - m/2 +j].local;
		simk_connect_node(nr_nic+i, nr_nic + nr_router + 2*(i*m/2+j)/m, 1);
	      }
	}
    }
    */
/*
    if (n != 1)
    {
 *TOR_FT_MORE LAYER 
    sw_desc_t *sw1, *sw2;
    int	    idx1, idx2;
    int nr_nr = nr_nic + nr_router;
    int layer;
    for (i = 0; i < nr_sw - nr_top - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    for (i = nr_sw - 3 * nr_top; i < nr_sw - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);

	    sw1= &sw_desc[i + nr_top];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1) + m/2;
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    }
*/
#ifdef PRINT_TOPO
    for (i = 0; i < nr_nic; i++) {
	printf("nic  %d:", i);
	for(j = 0; j < n_nicport; j++)
		printf(" %d-%d", nic_desc[i].port[j].local, nic_desc[i].port[j].remote);
	printf("\n");
    }
    for (i = 0; i < nr_router; i++) {
	printf("router %d: ", i);
	for (j = 0; j < r_m ; j++) {
	    printf("%d-%d ", router_desc[i].port[j].local, router_desc[i].port[j].remote);
	}
	printf("\n");
    }
    for (i = 0; i < nr_sw; i++) {
	printf("sw %d: ", i);
	for (j = 0; j < m; j++) {
	    printf("%d-%d ", sw_desc[i].port[j].local, sw_desc[i].port[j].remote);
	}
	printf("\n");
    }
#endif
}

void confg_topo_vtorft()
{
    int m = vtor_ft_param.sw_port_number;  //port number
    int n = vtor_ft_param.layer_number;  //level number
    int r_m = vtor_ft_param.router_port_number;//router port number
    router_port_num = r_m;
    printf("r_m = %d, m = %d \n",r_m, m);
    int n_nicport = net_param.nicnum;
    int i,j,k;

    /* calculate the number of nic and sw at each level */
    int nr_router = 2 * pow(m/2,n-1)*n_nicport; 
    int nr_nic = nr_router * (r_m - m/2 /n_nicport)/n_nicport;
    int nr_sw = (2 * n - 1) * pow(m / 2, n - 1);
    global_swnum = nr_sw;
    global_routernum = nr_router;
    nic_num = nr_nic;

    int nr_top = pow(m/2, n-1);
    int nr_low = nr_top * 2;
    printf("m = %d, n = %d, nr_nic = %d,nr_router = %d, nr_sw = %d\n", m, n, nr_nic,nr_router, nr_sw);

    nic_desc = malloc(sizeof(nic_desc_t) * nr_nic);
    router_desc = malloc(sizeof(router_desc_t) * nr_router);
    sw_desc = malloc(sizeof(sw_desc_t) * nr_sw);

    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < nr_nic; i++) {
	nic_desc[i].m          = vtor_ft_param.sw_port_number;
	nic_desc[i].n          = vtor_ft_param.layer_number;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t router_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = router_init,
	.data_size = sizeof(router_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };

    for (i = 0; i < nr_router; i++) {
	router_desc[i].m          = vtor_ft_param.router_port_number;
	router_desc[i].n          = vtor_ft_param.layer_number;
	router_desc[i].vcnum      = net_param.router_vcnum;
	router_desc[i].fifo_depth = net_param.fifo_depth;
	router_desc[i].frequency = net_param.nic_frequency;
	router_desc[i].bus_width = net_param.nic_bus_width;
	router_desc[i].index      = i+nic_num;
	for(j=0; j < r_m;j++)
	{
	    router_desc[i].port[j].local = k++;
	    router_desc[i].port[j].remote = -1;
	}

	router_node.data_buf = &router_desc[i];
	router_node.id = nr_nic+i;
	simk_add_node(router_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nr_sw; i++) {
	sw_desc[i].m          = vtor_ft_param.sw_port_number;
	sw_desc[i].n          = vtor_ft_param.layer_number;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num+nr_router;
	for (j = 0; j < m; j++) {
	    sw_desc[i].port[j].local = k++;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = nr_nic + nr_router + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

    /* topo */

    int r_id,r_idp;
    for (i = 0; i < nr_nic; i++) {
	for(j = 0; j < n_nicport; j++){
	    	r_id = i / (r_m - m/2/n_nicport) * n_nicport +j;
		r_idp = i%(r_m - m/2/n_nicport);
		nic_desc[i].port[j].remote = router_desc[r_id].port[r_idp].local; 
		router_desc[r_id].port[r_idp].remote = nic_desc[i].port[j].local;
		//printf("id %d connect %d\n",i,nr_nic + (i*n_nicport + j)/(r_m - m/2));
		simk_connect_node(i, nr_nic + r_id, 1);
	}
    }
    
    int r_port_sw = m/2/n_nicport;
    for (i = 0; i < nr_router; i++) {
	for(j = 0; j < r_port_sw; j++){
	    if (n == 1) {
		router_desc[i].port[r_m - r_port_sw + j].remote = sw_desc[0].port[i*r_port_sw + j].local;
		sw_desc[0].port[i*r_port_sw + j].remote = router_desc[i].port[r_m - r_port_sw +j].local;
		simk_connect_node(nr_nic+i, nr_nic+nr_router, 1);
	    } else {
		router_desc[i].port[r_m - r_port_sw +j].remote = sw_desc[2*(i*r_port_sw+j)/m].port[(i*r_port_sw+j)%(m/2)].local; 
		sw_desc[2*(i*r_port_sw+j)/m].port[(i*r_port_sw+j)%(m/2)].remote = router_desc[i].port[r_m - r_port_sw +j].local;
		simk_connect_node(nr_nic+i, nr_nic + nr_router + 2*(i*r_port_sw+j)/m, 1);
	      }
	}
    }
    if (n != 1)
    {
    sw_desc_t *sw1, *sw2;
    int	    idx1, idx2;
    int nr_nr = nr_nic + nr_router;
    int layer;
    for (i = 0; i < nr_sw - nr_top - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    for (i = nr_sw - 3 * nr_top; i < nr_sw - nr_low; i++) {
	layer = i / nr_low + 1;
	for (j = 0; j < m/2; j++) {
	    sw1= &sw_desc[i];
	    idx1 = j + m / 2;
	    sw2 = &sw_desc[i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);

	    sw1= &sw_desc[i + nr_top];
	    idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1) + m/2;
	    sw1->port[idx1].remote = sw2->port[idx2].local;
	    sw2->port[idx2].remote = sw1->port[idx1].local;
	    simk_connect_node(nr_nr + (int)(sw1 - sw_desc), nr_nr + (int)(sw2 - sw_desc), 1);
	}
    }
    }
#ifdef PRINT_TOPO
    for (i = 0; i < nr_nic; i++) {
	printf("nic  %d:", i);
	for(j = 0; j < n_nicport; j++)
		printf(" %d-%d", nic_desc[i].port[j].local, nic_desc[i].port[j].remote);
	printf("\n");
    }
    for (i = 0; i < nr_router; i++) {
	printf("router %d: ", i);
	for (j = 0; j < r_m ; j++) {
	    printf("%d-%d ", router_desc[i].port[j].local, router_desc[i].port[j].remote);
	}
	printf("\n");
    }
    for (i = 0; i < nr_sw; i++) {
	printf("sw %d: ", i);
	for (j = 0; j < m; j++) {
	    printf("%d-%d ", sw_desc[i].port[j].local, sw_desc[i].port[j].remote);
	}
	printf("\n");
    }
#endif
}

void confg_topo_flbfly()
{
    int k = flattened_bfly_param.k;
    int n = flattened_bfly_param.n;
    int n_nicport = net_param.nicnum;

    int i,j,w;
    int radix = (k-1)*n + 1;
    int enicp = k;
    int dnum = n - 1;
    nic_num = pow(k,n);
    int sw_num = pow(k,n-1);
    int id = 0;
    int index;
    int seq;
    int cmp;
    global_swnum  = sw_num;

    sw_desc = (sw_desc_t*)malloc(sizeof(sw_desc_t) * sw_num);
    nic_desc = (nic_desc_t*)malloc(sizeof(nic_desc_t) * nic_num);
    dms = (int *)malloc(sizeof(int) * 2);
    dms[0] = k;
    dms[1] = n;

    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    w = 0;
    for (i = 0; i < nic_num; i++) {
	nic_desc[i].m          = radix;
	nic_desc[i].n          = flattened_bfly_param.n;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = w++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < sw_num; i++) {
	sw_desc[i].m          = radix;
	sw_desc[i].n          = flattened_bfly_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < radix; j++) {
	    sw_desc[i].port[j].local =(n_nicport + 1) * nic_num + i * radix + j;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = nic_num + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

    id = 0;
    for(i = 0; i < sw_num; i++)
    {
	for(j = 0; j < dnum; j++)
	{
	    index = pow(k,j);
	    seq = (int)(i/pow(k,j))%k;
	    for(w = 0,cmp = 0; w < k -1; w++,cmp++)
	    {
		if(w == seq) cmp++;
		sw_desc[i].port[j*(k-1) + w].remote = sw_desc[i + (cmp - seq) * index].port[j*(k-1) + seq - 1 + cmp - w].local;
	        simk_connect_node(nic_num + i, nic_num + i + (cmp - seq) * index, 1);
	    }
	}
	for(j = 0; j < enicp; j++)
	{
	    sw_desc[i].port[dnum * (k-1) + j].remote = nic_desc[id].port[0].local;
	    nic_desc[id++].port[0].remote = sw_desc[i].port[dnum * (k-1) + j].local;
	    simk_connect_node(nic_num + i, id - 1, 1);
	}
    }
#ifdef PRINT_TOPO
    for(i = 0; i < sw_num; i++)
    {
	for(j = 0; j < radix; j++)
	{
	    printf("sw_desc[%d].port[%d].remote = %d\n",i,j,sw_desc[i].port[j].remote);
	}

    }
    for(i = 0; i < nic_num; i++)
	for(j = 0; j < 2; j++)
    {
	printf("nic_desc[%d].port[%d].remote = %d\n",i,j,nic_desc[i].port[j].remote);
    }
#endif

}

void sflbfly_read_file()
{
    /*Get cofiguration information from file config/confg_sflbfly*/
    int i = 0;
    int j = 0;
    char c;
    FILE *fcfg;
    char rubi[300];
    char num[10];
    char *file_confg_sflblfy = sflbfly_param.file_sflbfly;
    fcfg = fopen(file_confg_sflblfy,"r");
    if(NULL == fcfg)
    {
	printf("No mesh config file!");
	exit(0);
    }
    while((c = fgetc(fcfg) )!= EOF)
    {
	if(c == '#')
	{
	    fgets(rubi,300,fcfg);
	}
	else if (c >= 0x30 && c<= 0x39)
	{
	    num[i++] = c ;
	}
	else if ( c == ' ' || c == '\n' )
	{
	    num[i] ='\0';
	    i = 0;
	    dms[++j]  = atoi(num);
	    //printf("dms[%d] = %d\n",j-1, dms[j-1]);
	}
    }

}

void confg_topo_sflbfly()
{
    int m = sflbfly_param.m;//dimentsion
    int n = sflbfly_param.n;//number of port for nic
    int n_nicport = net_param.nicnum;
    int i,j,k,w,id;
    int seq;
    int cmp;
    int row;
    int pbase = 0;
    int index = 1;
    int sw_num = 1;
    int radix = n; //swisch nic port number
    /* compute port number of swtich*/
    bug_on(n%n_nicport != 0,"nic port wrong!");
    row = n/n_nicport;
    dms = (int *)malloc(sizeof(int) * m + 1);
    dms[0] = m;
    sflbfly_read_file();
    for(i = 1; i < m+1; i++)
    {
	radix +=(dms[i]-1);
    }
    /* compute switch number */
    for(i = 1; i< m+1; i++)
    {
	sw_num *= dms[i];
    }
    global_swnum = sw_num;
    nic_num = row * sw_num;

    sw_desc = (sw_desc_t*)malloc(sizeof(sw_desc_t) * sw_num);
    nic_desc = (nic_desc_t*)malloc(sizeof(nic_desc_t) * nic_num);

    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    w = 0;
    for (i = 0; i < nic_num; i++) {
	nic_desc[i].m          = sflbfly_param.m;
	nic_desc[i].n          = sflbfly_param.n;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = w++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < sw_num; i++) {
	sw_desc[i].m          = radix;
	sw_desc[i].n          = sflbfly_param.m;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < radix; j++) {
	    sw_desc[i].port[j].local =(n_nicport + 1) * nic_num + i * radix + j;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = nic_num + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }
    id = 0;
    for(i = 0; i < sw_num; i++)
    {
	pbase = 0;
	index = 1;
	for(j = 0; j < m; j++)
	{
	    seq = i / index % dms[j+1];
	    for(w = 0, cmp = 0; w < dms[j+1] -1; w++,cmp++ )
	    {
		if(w == seq) cmp++;
		sw_desc[i].port[pbase + w].remote = sw_desc[i + (cmp - seq) * index].port[pbase + seq - 1 + cmp - w].local;
	//	printf("sw_desc[%d].port[%d].remote = %d\n",i,pbase+w,sw_desc[i].port[pbase+w].remote);
		simk_connect_node(nic_num + i, nic_num + i +(cmp - seq)*index,1);
	    }
	    index = index * dms[j+1];
	    pbase += (dms[j+1] - 1);
	}
	printf("row%d\n",row);
	for(j = 0; j < row; j++)
	{
	    for(k = 0; k < n_nicport; k++)
	    {
		sw_desc[i].port[pbase + j*n_nicport+k].remote = nic_desc[id].port[k].local;
		nic_desc[id].port[k].remote = sw_desc[i].port[pbase + j*n_nicport+k].local;
		simk_connect_node(nic_num + i, id, 1);
	    }
	    id++;
	}

    }
#ifdef PRINT_TOPO
    for(i = 0; i < sw_num; i++)
    {
	for(j = 0; j < radix; j++)
	{
	    printf("sw_desc[%d].port[%d].remote = %d\n",i,j,sw_desc[i].port[j].remote);
	}

    }
    for(i = 0; i < nic_num; i++)
	for(j = 0; j < n_nicport; j++)
    {
	printf("nic_desc[%d].port[%d].remote = %d\n",i,j,nic_desc[i].port[j].remote);
    }
#endif

}

void mesh_read_file(FILE * fcfg)
{
    int i = 0;
    int j = 0;
    int k = 0;
    char c;
    char rubi[300];
    char num[10] = {'\0'};
    int temp[3][30] = {{0}};
    while((c = fgetc(fcfg) )!= EOF)
    {
	if(c == '#')
	{
	    fgets(rubi,300,fcfg);
	}
	else if ((c >= 0x30 && c<= 0x39) || c == '-')
	{
	    num[i++] = c ;
	}
	else if ( c == ' ' || c == '\n' )
	{
	    num[i] ='\0';
	    i = 0;
	    temp[j][k++] = atoi(num);
	    if(c == '\n') 
	    {
		j++;
		k = 0;
	    }
	}
    }
    for(i = 0; i < torus_param.m; i++)
    {
	cmap[i] = temp[0][i];
    }
    for(i = 0; i < torus_param.n; i++)
    {
	direction[i] = temp[1][i];
    }
    for(i = 0; i < torus_param.n; i++)
    {
	dms[i] = temp[2][i];
    }
}

void find_nic_port(int nport,int cmap[], int *num_unused, int port_unused[])
{
	int i;
	*num_unused = 0;
	for(i = 0; i < nport ; i++)
	{
		if(cmap[i] == -1)
		       	port_unused[(*num_unused)++] = i;
	}
}	

void confg_topo_mesh()
{
    int nport = mesh_param.m;
    int dimensions = mesh_param.n;
    int n_nicport = net_param.nicnum;
    int i,j,k;

    cmap = (int*)malloc(sizeof(int) * mesh_param.m);
    direction = (int*)malloc(sizeof(int) * mesh_param.n);
    dms = (int*)malloc(sizeof(int) * mesh_param.n);

    /*Get cofiguration information from file config/confg_mesh*/
    FILE *fcfg;
    char *file_confg_mesh = mesh_param.file_mesh;
    fcfg = fopen(file_confg_mesh,"r");
    if(NULL == fcfg)
    {
	printf("No mesh config file!");
	exit(0);
    }

    mesh_read_file(fcfg);

    int nsw = 1;
    //caculate number of sw is used
    for(i = 0; i < dimensions; i++)
	nsw *= dms[i];
    //caulate number of nic
    int n_nic = (nport - 2*dimensions) * nsw/n_nicport;
    nic_num = n_nic;
    global_swnum = nsw;
    printf("global_sw: %d\n",global_swnum);

    sw_desc = (sw_desc_t*)malloc(nsw * sizeof(sw_desc_t));
    nic_desc =(nic_desc_t*)malloc(n_nic *sizeof(nic_desc_t));
    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < n_nic; i++) {
	nic_desc[i].m          = mesh_param.m;
	nic_desc[i].n          = mesh_param.n;
	nic_desc[i].nicnum     = net_param.nicnum; 
	nic_desc[i].vcnum      = net_param.nic_vcnum; 
	nic_desc[i].fifo_depth = net_param.fifo_depth; 
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;
	for(j = 0; j < n_nicport + 1 ; j++)
	{
	    nic_desc[i].port[j].local = k++;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nsw; i++) {
	sw_desc[i].m          = mesh_param.m;
	sw_desc[i].n          = mesh_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum; 
	sw_desc[i].fifo_depth = net_param.fifo_depth; 
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < nport; j++) {
	    sw_desc[i].port[j].local = k++;
	}

	sw_node.id = n_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

	
    int d_level1 = 1;	//use in loop ,first is dx
    int d_level2 = 1;	//use in loop , first is 1
    int idx1,idx2;
    for(i = 0; i<dimensions; i++)
    {
	 d_level1 = d_level1*dms[i];
	 for(j = 0; j < nsw; j++)
	 {
		if((j+1)%d_level1 == 0||(d_level1 - (j+1)%d_level1 < d_level2))
		{
		    //do noting! 
		    //sw_desc[j].port[direction[i]].remote = sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].local;
		    //sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].remote = sw_desc[j].port[direction[i]].local;
		    sw_desc[j].port[direction[i]].remote = sw_desc[j].port[direction[i]].local;
		    sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].remote = sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].local;
		}
		else 
		{
		    sw_desc[j].port[direction[i]].remote = sw_desc[j+d_level2].port[cmap[direction[i]]].local;
		    sw_desc[j+d_level2].port[cmap[direction[i]]].remote = sw_desc[j].port[direction[i]].local;
		    idx1 = j + n_nic;
		    idx2 = j + d_level2 + n_nic;
		    simk_connect_node(idx1,idx2,1);

#ifdef PRINT_TOPO
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,j,direction[i],sw_desc[j].port[direction[i]].remote);
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,j+d_level2,cmap[direction[i]],sw_desc[j].port[direction[i]].local);
#endif
		}	
	 }
		    d_level2 = d_level2*dms[i];
    }
    int num_unused = 0;
    int port_unused[20];
    find_nic_port(nport,cmap,&num_unused,port_unused);
    if(num_unused%n_nicport !=0)
    {
	    printf("error!\n");
	    exit(-1);
    }
    else
    {
	    for(i = 0; i < nsw; i++)
	    {
		    for(j = 0; j < num_unused; j++)
		    {
			    sw_desc[i].port[port_unused[j]].remote = nic_desc[(i*num_unused+j)/n_nicport].port[j%n_nicport].local;
			    nic_desc[(i*num_unused+j)/n_nicport].port[j%n_nicport].remote = sw_desc[i].port[port_unused[j]].local;
			    idx1 = i + n_nic;
			    idx2 = (i*num_unused + j)/n_nicport;
			    simk_connect_node(idx1,idx2,1); 
		    }
	    }
    }
#ifdef PRINT_TOPO
    for(i = 0; i< nsw; i++)
    {
	    for(j = 0; j < nport; j++)
	    {
		    printf("sw_desc[%d].port[%d].remote = %d\n",i,j,sw_desc[i].port[j].remote);
	    }
    }
    for(i = 0; i< n_nic; i++)
    {
	    for(j = 0; j < n_nicport; j++)
	    {
		    printf("nic_desc[%d].port[%d].remote = %d\n",i,j,nic_desc[i].port[j].remote);
	    }
    }
#endif    

}


void ftree_task_partition()
{
    int m = ftree_param.m;
    int n = ftree_param.n;
    int nr_nic = 2 * pow(m / 2, n)/net_param.nicnum;
    int nr_sw = (2 * n - 1) * pow(m / 2, n - 1);
    int nr_top = pow(m/2, n-1);
    int nr_low = nr_top * 2;
    int thrd_idx, base;
    int gap;

    int i, j;
    int proc_num = get_host_num();
    int thread_num = get_thread_num();

    gap = nr_nic / proc_num;
    gap = nr_nic % proc_num == 0? gap : gap+1;
    thrd_idx = 0;
    for (i = 0; i < nr_nic; i++) {
	simk_set_proc_idx(i, i / gap);
	if (thrd_idx >= thread_num || i % gap == 0) thrd_idx = 0;
	//printf("id: %d, proc: %d, thread: %d\n", i, i / gap, thrd_idx);
	simk_set_thread_idx(i, thrd_idx++);
    }

    thrd_idx = 0;
    base = 0;
    gap = nr_low / proc_num;
    gap = nr_low % proc_num == 0 ? gap : gap + 1;
    for (j = 0; j < nr_sw; j++) {
	if (j > 0 && j % nr_low == 0)
	    base += nr_low;
	if (j / nr_low == n-1) {    // evaluated to be true once
	    gap = nr_top / proc_num;
	    gap = nr_top % proc_num == 0 ? gap : gap + 1;
	}
	simk_set_proc_idx(nr_nic + j, (j-base) / gap);
	if (thrd_idx >= thread_num || (j-base) % gap == 0) thrd_idx = 0;
	//printf("id: %d, proc: %d, thread: %d\n", nr_nic+j, (j-base) / gap, thrd_idx);
	simk_set_thread_idx(nr_nic+j, thrd_idx++);
    }
}

void torus_read_file(FILE * fcfg)
{
    int i = 0;
    int j = 0;
    int k = 0;
    char c;
    char rubi[300];
    char num[10] = {'\0'};
    int temp[3][30] = {{0}};
    while((c = fgetc(fcfg) )!= EOF)
    {
	if(c == '#')
	{
	    fgets(rubi,300,fcfg);
	}
	else if ((c >= 0x30 && c<= 0x39) || c == '-')
	{
	    num[i++] = c ;
	}
	else if ( c == ' ' || c == '\n' )
	{
	    num[i] ='\0';
	    i = 0;
	    temp[j][k++] = atoi(num);
	    if(c == '\n') 
	    {
		j++;
		k = 0;
	    }
	}
    }
    for(i = 0; i < torus_param.m; i++)
    {
	cmap[i] = temp[0][i];
    }
    for(i = 0; i < torus_param.n; i++)
    {
	direction[i] = temp[1][i];
    }
    for(i = 0; i < torus_param.n; i++)
    {
	dms[i] = temp[2][i];
    }
}

void confg_topo_torus()
{
    int nport = torus_param.m;
    int dimensions = torus_param.n;
    int n_nicport = net_param.nicnum;
    int i,j,k;

    cmap = (int*)malloc(sizeof(int) * torus_param.m);
    direction = (int*)malloc(sizeof(int) * torus_param.n);
    dms = (int*)malloc(sizeof(int) * torus_param.n);

    /*Get cofiguration information from file config/confg_torus*/
    FILE *fcfg;
    char *file_confg_torus = torus_param.file_torus;
    fcfg = fopen(file_confg_torus,"r");
    if(NULL == fcfg)
    {
	printf("No torus config file!");
	exit(0);

    }

    torus_read_file(fcfg);

    int nsw = 1;
    //caculate number of sw is used
    for(i = 0; i < dimensions; i++)
	nsw *= dms[i];
    //caulate number of nic
    int n_nic = (nport - 2*dimensions) * nsw/n_nicport;
    nic_num = n_nic;
    global_swnum = nsw;
    printf("global_sw: %d\n",global_swnum);

    sw_desc = (sw_desc_t*)malloc(nsw * sizeof(sw_desc_t));
    nic_desc =(nic_desc_t*)malloc(n_nic *sizeof(nic_desc_t));
    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < n_nic; i++) {
	nic_desc[i].m          = torus_param.m;
	nic_desc[i].n          = torus_param.n;
	nic_desc[i].nicnum     = net_param.nicnum; 
	nic_desc[i].vcnum      = net_param.nic_vcnum; 
	nic_desc[i].fifo_depth = net_param.fifo_depth; 
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;
	for(j = 0; j < n_nicport + 1 ; j++)
	{
	    nic_desc[i].port[j].local = k++;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nsw; i++) {
	sw_desc[i].m          = torus_param.m;
	sw_desc[i].n          = torus_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum; 
	sw_desc[i].fifo_depth = net_param.fifo_depth; 
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < nport; j++) {
	    sw_desc[i].port[j].local = k++;
	}

	sw_node.id = n_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

	
    int d_level1 = 1;	//use in loop ,first is dx
    int d_level2 = 1;	//use in loop , first is 1
    int idx1,idx2;
    for(i = 0; i<dimensions; i++)
    {
	 d_level1 = d_level1*dms[i];
	 for(j = 0; j < nsw; j++)
	 {
		if((j+1)%d_level1 == 0||(d_level1 - (j+1)%d_level1 < d_level2))
		{

		    sw_desc[j].port[direction[i]].remote = sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].local;
		    sw_desc[j -(d_level1-d_level2)].port[cmap[direction[i]]].remote = sw_desc[j].port[direction[i]].local;
		    idx1 = j + n_nic;
		    idx2 = j -(d_level1-d_level2) + n_nic;
		    simk_connect_node(idx1,idx2,1);
#ifdef PRINT_TOPO
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,j,direction[i],sw_desc[j].port[direction[i]].remote);
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,(j-(d_level1-d_level2)),cmap[direction[i]],sw_desc[j].port[direction[i]].local);
#endif
		}
		else 
		{
		    sw_desc[j].port[direction[i]].remote = sw_desc[j+d_level2].port[cmap[direction[i]]].local;
		    sw_desc[j+d_level2].port[cmap[direction[i]]].remote = sw_desc[j].port[direction[i]].local;
		    idx1 = j + n_nic;
		    idx2 = j + d_level2 + n_nic;
		    simk_connect_node(idx1,idx2,1);

#ifdef PRINT_TOPO
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,j,direction[i],sw_desc[j].port[direction[i]].remote);
		    printf("dimension %d: sw_desc[%d].port[%d].remote = %d\n",i,j+d_level2,cmap[direction[i]],sw_desc[j].port[direction[i]].local);
#endif
		}	
	 }
		    d_level2 = d_level2*dms[i];
    }
    int num_unused = 0;
    int port_unused[20];
    find_nic_port(nport,cmap,&num_unused,port_unused);
    if(num_unused%n_nicport !=0)
    {
	    printf("error!\n");
	    exit(-1);
    }
    else
    {
	    for(i = 0; i < nsw; i++)
	    {
		    for(j = 0; j < num_unused; j++)
		    {
			    sw_desc[i].port[port_unused[j]].remote = nic_desc[(i*num_unused+j)/n_nicport].port[j%n_nicport].local;
			    nic_desc[(i*num_unused+j)/n_nicport].port[j%n_nicport].remote = sw_desc[i].port[port_unused[j]].local;
			    idx1 = i + n_nic;
			    idx2 = (i*num_unused + j)/n_nicport;
			    simk_connect_node(idx1,idx2,1); 
		    }
	    }
    }
#ifdef PRINT_TOPO
    for(i = 0; i< nsw; i++)
    {
	    for(j = 0; j < nport; j++)
	    {
		    printf("sw_desc[%d].port[%d].remote = %d\n",i,j,sw_desc[i].port[j].remote);
	    }
    }
    for(i = 0; i< n_nic; i++)
    {
	    for(j = 0; j < n_nicport; j++)
	    {
		    printf("nic_desc[%d].port[%d].remote = %d\n",i,j,nic_desc[i].port[j].remote);
	    }
    }
#endif    
}
  
int find_max(int arr[],int size)
{
    int i;
    int max = 0;
    for(i = 0; i < size; i++)
    {
	if(arr[i] > max) max = arr[i];
    }
    return max;
}

void make_atoamatrix(int ***map,int j)
{
    int k;
    int w_i,w_j,w_k;
    int count = 0;
    matrix_map = (int**)malloc(sizeof(int*) * (num_component[j]) + sizeof(int) * column[j] * num_component[j]);
    if(matrix_map != NULL)
    {
	memset(matrix_map, 0, sizeof(int*) * num_component[j] + sizeof(int) * column[j] * num_component[j]);
	for(k = 0 ; k < num_component[j]; k++)
	   matrix_map[k] = (int*)((unsigned long)matrix_map + sizeof(int*) * num_component[j] + sizeof(int) * k * column[j]);
	printf("ok ");
    }
    else
    {
	printf("in make_atoamatrix!\n");
	exit(0);
    }

    /*consider some ports are the same direction*/
#ifdef A2A_1
    for(w_i = 0; w_i < num_component[j]; w_i++)
    {
	printf("ok 2\n");
	w_j = 0;
	w_k = 0;
	while(w_j < column[j])
	{
	    if(w_j < num_component[j]-1)
	    {
		if(w_j != w_i)
		{
		    matrix_map[w_i][w_j] = w_k++;
		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
		else
		{
		    w_k++;
		    matrix_map[w_i][w_j] = w_k++;
		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
	    }
	    else
	    {
		matrix_map[w_i][w_j] = matrix_map[w_i][count];
		printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		count++;
		if(count == num_component[j] - 1)
		    count = 0;
	    }
	    w_j++;
	}
    }
#endif
#ifdef A2A_2
    for(w_i = 0; w_i < num_component[j]; w_i++)
    {
	printf("ok 2\n");
	w_j = 0;
	w_k = 0;
	count = column[j] / (num_component[j]-1);
	while(w_j < column[j])
	{
		if(w_j/count != w_i)
		{
		    matrix_map[w_i][w_j] = w_j/count + w_k;
		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
		else
		{
		    w_k = 1;
		    matrix_map[w_i][w_j] = w_j/count + w_k;
		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
	    w_j++;
	}
    }
#endif

    *map = matrix_map;
}

void universal_param_config()
{
    topo_level = universal_param.n;
    int sport_num = universal_param.m;
    bool o_torus = universal_param.o_torus;
    topo_map = (int ***)malloc(sizeof(int**)*(topo_level));
    int i,j,k,i_temp,j_temp;
    int w_i,w_j;
    FILE *fcfg;
    char c;
    char num[10];
    char rubi[300];
    fcfg = fopen(universal_param.file1,"r");
    if(fcfg == NULL) 
    {
	printf("Can't Open the file!\n");
	exit(0);
    }
    num_component = (int *)malloc(sizeof(int) * (topo_level));
    column = (int*)malloc(sizeof(int)*(topo_level));
    memset(column,0,sizeof(int) * (topo_level));
    i = 0;
    j = 0;
    k = 0;
    i_temp = 0;
    j_temp = 0;
    while((c = fgetc(fcfg) )!= EOF)
    {
	if(c == '#')
	{
	    fgets(rubi,300,fcfg);
	}
	else if (c >= 0x30 && c<= 0x39)
	{
	    num[i++] = c ;
	}
	else if(c == '\n'||c == ' ')
	{
	    num[i] = '\0';
	    if(i == 0) continue;
	    num_component[j] = atoi(num);
	    if(j == 0) column[0] = sport_num;
	    printf("num_component:%d  column%d \n",num_component[j],column[j]);
	    matrix_map = (int**)malloc(sizeof(int*) * (num_component[j]) + sizeof(int) * column[j] * num_component[j]);
	    if(matrix_map != NULL)
	    {
		memset(matrix_map, 0, sizeof(int*) * num_component[j] + sizeof(int) * column[j] * num_component[j]);
		for(k = 0 ; k < num_component[j]; k++)
		   matrix_map[k] = (int*)((unsigned long)matrix_map + sizeof(int*) * num_component[j] + sizeof(int) * k * column[j]);
	    }
	    else exit(0);
	    for(i = 0; i < num_component[j]; i++){
		k = 0;
		while(k< column[j])
		{
		    while((c = fgetc(fcfg))!=EOF)
			{
			    if(c != ' '&& c != '\n') num[i_temp++] = c;
			    else {
				    if(i_temp == 0) continue;
				    num[i_temp] ='\0';
				    i_temp = 0;
				    break;
			         }
			}
			matrix_map[i][k] = atoi(num);
			printf("matrix_map[%d][%d]:%d \n",i,k,matrix_map[i][k]);
			if(matrix_map[i][k] == -2) {
			    j_temp++;
			}
			/*change here*/
			if(c == '\n' && k < column[j] - 1)
			{
			    if(column[j]%(k+1) != 0)
			    {
				printf("column[%d]=%d k = %d\n",j,column[j],k);
				printf("file number in level %d feed wrong!\n",j);
				exit(0);
			    }
			    w_i = k + 1;
			    w_j = 0;
			    while(w_i != column[j])
			    {
				matrix_map[i][w_i] = matrix_map[i][w_j];
				if(matrix_map[i][w_i] == -2) j_temp++;
				w_j++;
				if(w_j == k+1 ) w_j = 0;
				w_i++;
			    }
			    break;
			}
		    k++;	
		}
	    }
	    topo_map[j] = matrix_map;
	    printf("j is :%d\n",j);
	    j++;
	    if(j < topo_level) column[j] = j_temp;
	    if(j > topo_level)
	    {
		
	        printf("error !j is :%d\n",j);
		exit(0);
	    }
	    j_temp = 0;
	    i = 0;
	}
    }
    fclose(fcfg);
////////////////////////////////////////////////// 
    if(universal_param.o_atoa)
    {
	if(j != topo_level-1 )
	{
	    printf("all to all topo wrong ,j is %d\n",j);
	}
	printf("all to all  ,j is %d\n",j);
	num_component[j] = universal_param.o_component;
	printf("column[%d] = %d\n",j,column[j]);
	printf("universal_param.o_component is %d\n",universal_param.o_component);
	make_atoamatrix(&topo_map[j],j);
	dms = (int*)malloc(sizeof(int) * 4);
	dms[0] = column[0];
	dms[1] = universal_param.m;
	dms[2] = universal_param.n;
	dms[3] = universal_param.o_component;
    }
    if(o_torus) i_temp = topo_level-1;
    else i_temp = topo_level;
    for(i = 0 ; i < i_temp; i++){
	for(j = 0; j < num_component[i]; j++){
	    for(k = 0; k < column[i]; k++) {
		printf("%d ",topo_map[i][j][k]);
	    }
	    printf("\n");
	}
    }
///////////////////////////////////////////////////////////  
    if(o_torus)
    {
	printf("remain port num:%d\n",column[topo_level-1]);
	cmap = (int*)malloc(sizeof(int) *column[topo_level-1]);
	direction = (int*)malloc(sizeof(int) * column[topo_level-1]/2);
	xy_direc  = (int*)malloc(sizeof(int) * column[topo_level-1]/2);
	dms = (int*)malloc(sizeof(int) * column[topo_level-1]/2);
	char *config_universal = universal_param.file2;
	fcfg = fopen(config_universal,"r");
	if(NULL == fcfg)
	{
	    printf("No outside_torus config file!");
	    exit(0);
	};
	int temp[4][100] = {{0}};
	i = 0;
	j = 0;
	k = 0;
	while((c = fgetc(fcfg) )!= EOF)
	{
	    if(c == '#')
	    {
		fgets(rubi,300,fcfg);
	    }
	    else if ((c >= 0x30 && c<= 0x39) || c == '-')
	    {
		num[i++] = c ;
	    }
	    else if ( c == ' ' || c == '\n' )
	    {
		num[i] ='\0';
		i = 0;
		temp[j][k++] = atoi(num);
		if(c == '\n') 
		{
		    j++;
		    k = 0;
		}
	    }
	}
	fclose(fcfg);
	for(i = 0; i < column[topo_level-1]; i++)
	{
	    cmap[i] = temp[0][i];
	    printf("%d ",temp[0][i]);
	}
	printf("\n");
	for(i = 0; i < column[topo_level-1]/2; i++)
	{
	    direction[i] = temp[1][i];
	    printf("%d ",temp[1][i]);
	}
	printf("\n");
	for(i = 0; i < column[topo_level-1]/2; i++)
	{
	    xy_direc[i] = temp[2][i];
	    printf("%d ",temp[2][i]);
	}
	printf("\n");
	xyz = find_max(xy_direc,column[topo_level-1]/2) + 1;
	for(i = 0; i < xyz; i++)
	{
	    dms[i] = temp[3][i];
	    printf("%d ",temp[3][i]);
	}
	printf("\n");

    }
    
}

//find the number of -1 in the array
int find_nic_total(int** arr,int size1,int size2)
{
    int i,j,num = 0;
    for(i = 0; i < size1; i++)
	for(j = 0; j < size2; j++)
	{
	    if(arr[i][j] == -1) num++;
	}
 //   printf("nic port in small node is :%d \n");
    return num;
}

int get_map_rank(int *map, int size, int find)
{
    int i;
    int cnt = 1;
    for(i = 0; i < size; i++)
    {
	if(map[i] == find)
	    cnt++;
    }
    return cnt;
}

int get_map_val(int *map, int size, int rank ,int find)
{
    int i;
    int cnt = 0;
    for(i = 0; i < size; i++)
    {
	if(map[i] == find)
	{
	    cnt++;
	    if(cnt == rank)
		 return i;
	}
    }
    return i;
}


/*connect first level node:
1  level node info ,
2  rank of sw, 
3  rank of nic, number of sw be used,
4  number of nic be used,
5  connction map,
6  sw port num,
7  nic port num,
*/
void connect_flevel_node(levnd_desc_t *lres,int sw_res,int nic_res,int nr_sw,int nr_nic,int **map,int sp_num,int np_num)
{
    int i,j;
    int id1;
    int id2;
    int count = 0;
    int cnt_levl_port = 0;
    int lres_wid = sw_res/nr_sw;
    int id_rank;

    lres->level = 0;
    lres->whole_id = lres_wid;
    for(i = 0; i < nr_sw; i++)
    {
	for(j = 0; j < sp_num; j++)
	{
	    if( map[i][j] >= 0)
	    {
		id1 = sw_res + map[i][j];
		id_rank = get_map_rank(map[i],j,map[i][j]);
		id2 = get_map_val(map[map[i][j]],sp_num,id_rank,i);
		if(id2 == sp_num)
		{
		    printf("In connect_flevel_node!\n"); 
		    exit(0);
		}
		sw_desc[i+sw_res].port[j].remote = sw_desc[id1].port[id2].local;
		simk_connect_node(sw_desc[i+sw_res].index,sw_desc[id1].index,1);
//		printf("sw_desc[%d].port[%d].remote = %d \n",i+sw_res, j ,sw_desc[i+sw_res].port[j].remote);
	    }
	    else if(map[i][j] == -1)
	    {
		sw_desc[i+sw_res].port[j].remote = nic_desc[nic_res+count/np_num].port[count%np_num].local;
		nic_desc[nic_res+count/np_num].port[count%np_num].remote =sw_desc[i+sw_res].port[j].local;
		simk_connect_node(sw_desc[i+sw_res].index, nic_desc[nic_res+count/np_num].index,1);
	//	printf("sw_desc[%d].port[%d].remote = %d \n",i+sw_res,j,sw_desc[i+sw_res].port[j].remote);
	//	printf("nic_desc[%d].port[%d].remote = %d \n",nic_res+count/np_num,count%np_num,nic_desc[nic_res+count/np_num].port[count%np_num].remote);
		count++;
	    }
	    else if(map[i][j] == -2)
	    {
		lres->port_ptr[cnt_levl_port] = &(sw_desc[i+sw_res].port[j]);
		lres->index[cnt_levl_port] = sw_desc[i+sw_res].index;
//		printf("lres->port_ptr[%d]->local = %d\n", cnt_levl_port,lres->port_ptr[cnt_levl_port]->local);
//		printf("lres->index[%d] = %d \n ",cnt_levl_port,sw_desc[i+sw_res].index);
		cnt_levl_port++;
	    }
	}

    }
}

/*
 * levnd_desc_t *lres1 is the current level node whitch is need to configurate
 * levnd_desc_t *lres2 is the lower level node provide information to lres1
 * int nr_comp is the number of lres2
 * int col is lres2 port number
 * int level is level number, it count from bottom
*/
void connect_level_node(levnd_desc_t *lres1, levnd_desc_t *lres2, int whole_id, int nr_comp, int col, int level, int **map)
{
    int i,j;
    int id;
    int cnt_levl_port = 0;
    int id_rank;
    lres1->level = level;
    lres1->whole_id = whole_id; 
    for(i = 0; i < nr_comp; i++)
    {
	lres2[i].inner_id = i;
	lres2[i].outside = lres1;
	for(j = 0; j < col; j++)
	{
	    if(map[i][j] >= 0)
	    {

		id_rank = get_map_rank(map[i],j,map[i][j]);
		id = get_map_val(map[map[i][j]],col,id_rank,i);
		if(id == col)
		{
		    printf("In connect_level_node! level:%d\n",level);
		    exit(0);
		}
		lres2[i].port_ptr[j]->remote = lres2[map[i][j]].port_ptr[id]->local;
		simk_connect_node(lres2[i].index[j],lres2[map[i][j]].index[id],1);
		//printf("lres2[%d].port_ptr[%d]->remote = %d \n",i,j,lres2[i].port_ptr[j]->remote);
	    }
	    else if(map[i][j] == -2)
	    {
		lres1->port_ptr[cnt_levl_port] = lres2[i].port_ptr[j];
		lres1->index[cnt_levl_port] = lres2[i].index[j];
		cnt_levl_port++;
	    }

	}
    }
}

void make_outside_torus(levnd_desc_t *levs, int dimensions,int ln_port_num,int topo_level)
{
	int i,j,k;
	int d_level1 = 1;//use in loop ,first is dx
	int d_level2 = 1;//use in loop , first is 1
	int id1,id2;
	int arr1,arr2;
	for(i = 0; i<dimensions; i++)
	{
		d_level1 = d_level1*dms[i];
		for(j = 0; j < lev_num[topo_level-2]; j++)
		{
		    for(k = 0; k < ln_port_num/2; k++)
		    {
			if(xy_direc[k] == i)
			{

			    if((j+1)%d_level1 == 0||(d_level1 - (j+1)%d_level1 < d_level2))
			    {
				    levs[j].port_ptr[direction[k]]->remote = levs[j -(d_level1-d_level2)].port_ptr[cmap[direction[k]]]->local;
				    levs[j -(d_level1-d_level2)].port_ptr[cmap[direction[k]]]->remote = levs[j].port_ptr[direction[k]]->local;
				    id1 =j;
				    id2 =j -(d_level1-d_level2);
				    arr1 = direction[k];
				    arr2 = cmap[direction[k]];
				    simk_connect_node(levs[id1].index[arr1],levs[id2].index[arr2],1);
//				    printf("dimension %d: levs[%d].port_ptr[%d]->remote = %d\n",i,j,direction[k],levs[j].port_ptr[direction[k]]->remote);
//				    printf("dimension %d: levs[%d].port_ptr[%d]->remote = %d\n",i,(j-(d_level1-d_level2)),cmap[direction[k]],levs[j].port_ptr[direction[k]]->local);
			    }
			    else 
			    {
				    levs[j].port_ptr[direction[k]]->remote = levs[j+d_level2].port_ptr[cmap[direction[k]]]->local;
				    levs[j+d_level2].port_ptr[cmap[direction[k]]]->remote = levs[j].port_ptr[direction[k]]->local;
				    id1 =j;
				    id2 =j + d_level2;
				    arr1 = direction[k];
				    arr2 = cmap[direction[k]];
				    simk_connect_node(levs[id1].index[arr1],levs[id2].index[arr2],1);
//				    printf("dimension %d: levs[%d].port_ptr[%d]->remote = %d\n",i,j,direction[k],levs[j].port_ptr[direction[k]]->remote);
//				    printf("dimension %d: levs[%d].port_ptr[%d]->remote = %d\n",i,j+d_level2,cmap[direction[k]],levs[j].port_ptr[direction[k]]->local);
			    }
			}
		    }

		}
			d_level2 = d_level2*dms[i];
	}

}


void confg_topo_universal()
{
    int topo_level = universal_param.n;
    int sport_num = universal_param.m;
    int n_nicport = net_param.nicnum;
    bool o_torus = universal_param.o_torus;
    int i,j,k;
    int nsw = 1;
    int n_nic;
    int n_part_port;
    /*caculate number of sw is used*/
    if(o_torus)
    {
	for( i = 0; i < xyz; i++)
	{
	    nsw *= dms[i];
	}
	for( i = 0; i < topo_level-1; i++)
	{
	    nsw *= num_component[i]; 
	}
	printf("the numer of sw is :%d \n",nsw);
    }
    else
    {
	for( i = 0; i < topo_level; i++)
	{
	    nsw *= num_component[i]; 
	}
    }
    global_swnum = nsw;
  
    n_part_port = find_nic_total(topo_map[0],num_component[0],column[0]);
    printf("in small node the need of nic port is:%d\n",n_part_port);
    
    /*calculate the use of nic*/
    if(0 != n_part_port%n_nicport) 
    {
	printf("nic_port information is wrong!");
	exit(0);
    }
    i = n_part_port / n_nicport;
    int part_nic_num = i;
    n_nic = i * nsw / num_component[0];
    printf("the whole system nic num is :%d\n",n_nic);

    nic_num = n_nic;
    /*above on all basic information is geted, now I can configurate!*/

    sw_desc = (sw_desc_t*)malloc(nsw * sizeof(sw_desc_t));
    nic_desc =(nic_desc_t*)malloc(n_nic *sizeof(nic_desc_t));
    printf("sizeof nic_desc %ld sizeof sw_desc %ld \n",sizeof(nic_desc_t),sizeof(sw_desc_t));

    /*alloc resource for level node*/
    lev_res = (levnd_desc_t**)malloc(sizeof(levnd_desc_t*) * topo_level);
//    printf("lev_res: %p \n",lev_res);
    lev_num =(int*)malloc(sizeof(int*) * topo_level);

    printf("topo_level is :%d\n",topo_level);
    
    int loop_level;
    if(o_torus) loop_level = topo_level - 1;
    else loop_level = topo_level;

    for(i = 0; i<loop_level; i++)
    {
	if(i == 0) lev_num[0] = nsw / num_component[0];
	else
	    lev_num[i] = lev_num[i-1]/num_component[i];
	lev_res[i] = (levnd_desc_t*)malloc(sizeof(levnd_desc_t) * lev_num[i]);
      //  printf("lev_res[%d]: %p \n",i,lev_res[i]);
	printf("lev_num[%d] is :%d \n",i,lev_num[i]);
    }
/* fix here */
    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < n_nic; i++) {
	nic_desc[i].m          = universal_param.m;
	nic_desc[i].n          = universal_param.n;
	nic_desc[i].nicnum     = net_param.nicnum; 
	nic_desc[i].vcnum      = net_param.nic_vcnum; 
	nic_desc[i].fifo_depth = net_param.fifo_depth; 
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;
	for(j = 0; j < n_nicport + 1; j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nsw; i++) {
	sw_desc[i].m          = universal_param.m;
	sw_desc[i].n          = universal_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum; 
	sw_desc[i].fifo_depth = net_param.fifo_depth; 
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = n_nic + i;
	for (j = 0; j < sport_num; j++) {
	    sw_desc[i].port[j].local = k++;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = n_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

    for(i = 0; i < lev_num[0]; i++)
    {
	connect_flevel_node(&lev_res[0][i],num_component[0]*i,part_nic_num*i,num_component[0],part_nic_num,topo_map[0],sport_num,n_nicport);
    }

    for(i = 1; i < loop_level ; i++)
    {
	for(j = 0; j < lev_num[i]; j++)
	{
	    connect_level_node(&lev_res[i][j],&lev_res[i-1][num_component[i] * j],j,num_component[i],column[i],i,topo_map[i]);
	}
    }

    if(o_torus)
    {
	make_outside_torus(&lev_res[topo_level-2][0],xyz,column[topo_level-1] ,topo_level);
    }

/*    ////////////////////////////////
    for(i = 0 ; i < nsw; i++)
	for(j = 0; j < sport_num; j++)
	{
//	    printf("sw_desc[%d].port[%d].remote = %d \n",i,j,sw_desc[i].port[j].remote);

	}
*/  ////////////////////
}

void make_alltoall_matrix(int component, int spn)
{
    int i;
    int w_i,w_j,w_k;
    matrix_map = (int**)malloc( component * sizeof(int *) + component * spn * sizeof(int));
    memset(matrix_map,0,component * sizeof(int *) + component * spn * sizeof(int));
    for(i = 0; i < component; i++)
    {
	matrix_map[i] =(int*)((unsigned long int)matrix_map + component * sizeof(int *) + i * spn * sizeof(int));
    }
    for(w_i = 0; w_i < component; w_i++)
    {
//	printf("ok 2\n");
	w_j = 0;
	w_k = 0;
	while(w_j < component - 1)
	{
		if(w_j != w_i)
		{
		    matrix_map[w_i][w_j] = w_k++;
//		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
		else
		{
		    w_k++;
		    matrix_map[w_i][w_j] = w_k++;
//		    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
		}
	    w_j++;
	}
	while(w_j < spn)
	{
	    matrix_map[w_i][w_j] = -1;
//	    printf("matrix_map[%d][%d]:%d \n",w_i,w_j,matrix_map[w_i][w_j]);
	    w_j++;
	}
    }
   
}

void connect_alltoall_node(int **map, int nr_sw, int sp_num,int num_nic,int np_num)
{
    int i,j;
    int id1,id2;
    int id_rank;
    int count = 0;
    for(i = 0; i < nr_sw; i++)
    {
	for(j = 0; j < sp_num; j++)
	{
	    if( map[i][j] >= 0)
	    {
		id1 = map[i][j];
		id_rank = get_map_rank(map[i],j,map[i][j]);
		id2 = get_map_val(map[map[i][j]],sp_num,id_rank,i);
		if(id2 == sp_num)
		{
		    printf("In alltoall_node!\n");
		    exit(0);
		}
		sw_desc[i].port[j].remote = sw_desc[id1].port[id2].local;
		simk_connect_node(sw_desc[i].index,sw_desc[id1].index,1);
		printf("sw_desc[%d].port[%d].remote = %d \n",i, j ,sw_desc[i].port[j].remote);
	    }
	    else
	    {
		sw_desc[i].port[j].remote = nic_desc[count/np_num].port[count%np_num].local;
		nic_desc[count/np_num].port[count%np_num].remote =sw_desc[i].port[j].local;
		simk_connect_node(sw_desc[i].index,nic_desc[count/np_num].index,1);
		printf("sw_desc[%d].port[%d].remote = %d \n",i,j,sw_desc[i].port[j].remote);
		printf("nic_desc[%d].port[%d].remote = %d \n",count/np_num,count%np_num,nic_desc[count/np_num].port[count%np_num].remote);
		count++;
	    }
	}
    }
    if(count != num_nic * np_num) 
    {
	printf("count = %d num_nic*np_num = %d \n",count,num_nic*np_num);
    }

}

void confg_topo_ftreea2a()
{
    int m = ftree_a2a_param.m;
    int n = ftree_a2a_param.n;
    int torport = ftree_a2a_param.torport;
    int n_nicport = net_param.nicnum;
    int i,j,k;

    if (n == 1){
	printf("ftree alltoall topology is wrong\n");
	return ;
    }

    /* calculate the number of nic and sw at each level */
    int nr_nic = 2 * pow(m / 2, n)/n_nicport;
    int origin_nr_sw = (2*n-1) * pow(m/2,n-1);
    int nr_sw = 2 * (n - 1) * pow(m / 2, n - 1);
    int nr_router = pow(m/2,n-1);
    int anr_nic = nr_nic * (nr_router*m + 1);
    int anr_sw = nr_sw * (nr_router*m +1);
    int anr_router = nr_router * (nr_router*m +1);


    global_swnum = anr_sw;
    int nr_top = pow(m/2, n-1);
    int nr_low = nr_top * 2;
    printf("m = %d, n = %d, nr_nic = %d, nr_sw = %d\n", m, n, nr_nic, nr_sw);

    nic_num = anr_nic;
    nic_desc = malloc(sizeof(nic_desc_t) * anr_nic);
    sw_desc = malloc(sizeof(sw_desc_t) * anr_sw);
    router_desc = malloc(sizeof(router_desc_t) * anr_router);

    topo_level = 2;
    topo_map = (int ***)malloc(sizeof(int**)* (topo_level));
    num_component = (int *)malloc(sizeof(int) * (topo_level));
    column = (int*)malloc(sizeof(int)*(topo_level));
    memset(column,0,sizeof(int) * (topo_level));
    num_component[0] = nr_router;
    column[0] = torport; 
    num_component[1] = nr_router * m + 1;
    column[1] = nr_router * (torport - m);
    lev_res = (levnd_desc_t**)malloc(sizeof(levnd_desc_t*) * topo_level);
    lev_num =(int*)malloc(sizeof(int*) * topo_level);
    printf("topo_level is :%d\n",topo_level);

   
    for(i = 0; i<topo_level; i++)
    {
	if(i == 0) lev_num[0] = anr_router / num_component[0];
	else
	    lev_num[i] = lev_num[i-1]/num_component[i];
	lev_res[i] = (levnd_desc_t*)malloc(sizeof(levnd_desc_t) * lev_num[i]);
      //  printf("lev_res[%d]: %p \n",i,lev_res[i]);
	//printf("lev_num[%d] is :%d \n",i,lev_num[i]);
    }



    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < anr_nic; i++) {
	nic_desc[i].m          = ftree_a2a_param.m;
	nic_desc[i].n          = ftree_a2a_param.n;
	nic_desc[i].nicnum     = net_param.nicnum;
	nic_desc[i].vcnum      = net_param.nic_vcnum;
	nic_desc[i].fifo_depth = net_param.fifo_depth;
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;

	for(j=0; j < n_nicport + 1;j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < anr_sw; i++) {
	sw_desc[i].m          = ftree_a2a_param.m;
	sw_desc[i].n          = ftree_a2a_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum;
	sw_desc[i].fifo_depth = net_param.fifo_depth;
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = i + nic_num;
	for (j = 0; j < m; j++) {
	    sw_desc[i].port[j].local = k++;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = anr_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }

    task_node_t router_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = router_init,
	.data_size = sizeof(router_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };

    for (i = 0; i < anr_router; i++) {
	router_desc[i].m          = ftree_a2a_param.torport;
	router_desc[i].n          = ftree_a2a_param.n;
	router_desc[i].vcnum      = net_param.router_vcnum;
	router_desc[i].fifo_depth = net_param.fifo_depth;
	router_desc[i].frequency = net_param.nic_frequency;
	router_desc[i].bus_width = net_param.nic_bus_width;
	router_desc[i].index      = i+nic_num+global_swnum;
	for(j=0; j < torport;j++)
	{
	    router_desc[i].port[j].local = k++;
	    router_desc[i].port[j].remote = -1;
	}

	router_node.data_buf = &router_desc[i];
	router_node.id = anr_nic+anr_sw+i;
	simk_add_node(router_node);
    }

    /* topo */
/*
    for (i = 0; i < nr_nic; i++) {
	if (n == 1) {
	    nic_desc[i].port[0].remote = sw_desc[0].port[i].local;
	    sw_desc[0].port[i].remote = nic_desc[i].port[0].local;
	    simk_connect_node(i, nr_nic, 1);
	} else {
	    nic_desc[i].port[0].remote = sw_desc[2*i/m].port[i%(m/2)].local; 
	    sw_desc[2*i/m].port[i%(m/2)].remote = nic_desc[i].port[0].local;
	    simk_connect_node(i, nr_nic + 2*i/m, 1);
      }
    }
*/

    for(k = 0; k < anr_nic/nr_nic; k++){
	for (i = 0; i < nr_nic; i++) {
	    for(j = 0; j < n_nicport; j++){
		    nic_desc[k*nr_nic+i].port[j].remote = sw_desc[k * nr_sw + 2*(i*n_nicport+j)/m].port[(i*n_nicport+j)%(m/2)].local; 
		    sw_desc[k * nr_sw + 2*(i*n_nicport+j)/m].port[(i*n_nicport+j)%(m/2)].remote = nic_desc[k*nr_nic + i].port[j].local;
		    simk_connect_node(k*nr_nic+i, anr_nic + k*nr_sw+ 2*(i*n_nicport+j)/m, 1);
	    }
	}
    }

    sw_desc_t *sw1, *sw2;
    int	    idx1, idx2;
    int layer;
    for(k = 0; k < anr_sw/nr_sw; k++)
    {
	for (i = 0; i < origin_nr_sw - nr_top - nr_low; i++) {
	    layer = i / nr_low + 1;
	    for (j = 0; j < m/2; j++) {
		sw1= &sw_desc[i+k * nr_sw];
		idx1 = j + m / 2;
		sw2 = &sw_desc[k* nr_sw + i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
		idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
		sw1->port[idx1].remote = sw2->port[idx2].local;
		sw2->port[idx2].remote = sw1->port[idx1].local;
		simk_connect_node(anr_nic + (int)(sw1 - sw_desc), anr_nic + (int)(sw2 - sw_desc), 1);
	    }
	}
    }

    router_desc_t *router2;
    int tmp_sw2;
    for( k = 0; k < anr_router/nr_router; k++){
	for (i = origin_nr_sw - 3 * nr_top; i < origin_nr_sw - nr_low; i++) {
	    layer = i / nr_low + 1;
	    for (j = 0; j < m/2; j++) {
		sw1= &sw_desc[k * nr_sw+i];
		idx1 = j + m / 2;
		//sw2 = &sw_desc[k* nr_sw + i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j];
		tmp_sw2 = i - i % (int)pow(m/2, layer) + nr_low + m/2 * (i % (int)pow(m/2, layer-1)) + j;
		router2 = &router_desc[k * nr_router + tmp_sw2%nr_router];
		
		idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1);
		sw1->port[idx1].remote = router2->port[idx2].local;
		router2->port[idx2].remote = sw1->port[idx1].local;
		simk_connect_node(anr_nic + (int)(sw1 - sw_desc), anr_nic + (int)(router2 - router_desc), 1);

		sw1= &sw_desc[k*nr_sw+i + nr_top];
		idx2 = (i % (int)pow(m/2, layer)) / (int)pow(m/2, layer-1) + m/2;
		sw1->port[idx1].remote = router2->port[idx2].local;
		router2->port[idx2].remote = sw1->port[idx1].local;
		simk_connect_node(nr_nic + (int)(sw1 - sw_desc), nr_nic + (int)(router2 - router_desc), 1);
	    }
	}
    }

    make_atoamatrix(&topo_map[1],1);

    int acc;
    for( k = 0; k < anr_router/nr_router; k++){
	acc = 0;
	for(i = 0; i < nr_router; i++){
	    for(j = m; j < torport; j++){
		lev_res[0][k].port_ptr[acc] = &router_desc[k * nr_router + i].port[j];
		lev_res[0][k].index[acc] = router_desc[k*nr_router + i].index;
		//printf("router_desc[k*nr_router+i].index  = %d \n",router_desc[k*nr_router+i].index);
		acc++;
	    }
	}
    }

    i = 1;
    j = 0;
   // printf("num_componet[i] = %d, column[i] = %d \n",num_component[i],column[i]);
    connect_level_node(&lev_res[i][j],&lev_res[i-1][num_component[i] * j],j,num_component[i],column[i],i,topo_map[i]);

#ifdef PRINT_TOPO
    for (i = 0; i < anr_nic; i++) {
	printf("nic  %d:", i);
	for(j = 0; j < n_nicport; j++)
		printf(" %d-%d\n", nic_desc[i].port[j].local, nic_desc[i].port[j].remote);
    }
    for (i = 0; i < anr_sw; i++) {
	printf("sw %d: ", i);
	for (j = 0; j < m; j++) {
	    printf("%d-%d ", sw_desc[i].port[j].local, sw_desc[i].port[j].remote);
	}
	printf("\n");
    }
    for (i = 0; i < anr_router; i++) {
	printf("router %d: ", i);
	for (j = 0; j < torport; j++) {
	    printf("%d-%d ", router_desc[i].port[j].local, router_desc[i].port[j].remote);
	}
	printf("\n");
    }
#endif

}

void confg_topo_alltoall()
{
    int i,j,k;
    int sport_num = all_to_all_param.m;
    int nsw = all_to_all_param.n;
    int n_nicport = net_param.nicnum;
    int n_nic = (sport_num - nsw + 1) * nsw / n_nicport;
    nic_num = n_nic;
    global_swnum = nsw;
    if((sport_num - nsw + 1)%n_nicport!=0)
    {
	printf("sport_num set wrong!\n");
	exit(0);
    }
    make_alltoall_matrix(nsw,sport_num);
    sw_desc = (sw_desc_t *)malloc( nsw * sizeof(sw_desc_t));
    nic_desc = (nic_desc_t *)malloc(n_nic * sizeof(nic_desc_t));
/* fix here */
    task_node_t nic_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = nic_init,
	.data_size = sizeof(nic_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    k = 0;
    for (i = 0; i < n_nic; i++) {
	nic_desc[i].m          = all_to_all_param.m;
	nic_desc[i].n          = all_to_all_param.n;
	nic_desc[i].nicnum     = net_param.nicnum; 
	nic_desc[i].vcnum      = net_param.nic_vcnum; 
	nic_desc[i].fifo_depth = net_param.fifo_depth; 
	nic_desc[i].frequency = net_param.nic_frequency;
	nic_desc[i].bus_width = net_param.nic_bus_width;
	nic_desc[i].index      = i;
	for(j = 0; j < n_nicport + 1; j++)
	{
	    nic_desc[i].port[j].local = k++;
	    nic_desc[i].port[j].remote = -1;
	}

	nic_node.data_buf = &nic_desc[i];
	nic_node.id = i;
	simk_add_node(nic_node);
    }

    task_node_t sw_node = {
	.id = -1,
	.vwgt = 1,
	.init_func = sw_init,
	.data_size = sizeof(sw_desc_t),
	.data_buf = NULL,
	.need_copy = false
    };
    for (i = 0; i < nsw; i++) {
	sw_desc[i].m          = all_to_all_param.m;
	sw_desc[i].n          = all_to_all_param.n;
	sw_desc[i].vcnum      = net_param.sw_vcnum; 
	sw_desc[i].fifo_depth = net_param.fifo_depth; 
	sw_desc[i].frequency = net_param.sw_frequency;
	sw_desc[i].bus_width = net_param.sw_bus_width;
	sw_desc[i].index      = n_nic + i;
	for (j = 0; j < sport_num; j++) {
	    sw_desc[i].port[j].local = k++;
	    sw_desc[i].port[j].remote = -1;
	}

	sw_node.id = n_nic + i;
	sw_node.data_buf = &sw_desc[i];
	simk_add_node(sw_node);
    }
    connect_alltoall_node(matrix_map,nsw,sport_num,n_nic,n_nicport);
}

void confg_from_file()
{
    char *topo_config_file = "./config/topo.xml";
    char *simu_config_file  = "./config/self_conf.xml";

    sflbfly_param.file_sflbfly  = (char*)malloc(sizeof(char)*100);
    mesh_param.file_mesh   = (char*)malloc(sizeof(char)*100);
    torus_param.file_torus = (char*)malloc(sizeof(char)*100);
    universal_param.file1  = (char*)malloc(sizeof(char)*100);
    universal_param.file2  = (char*)malloc(sizeof(char)*100);
    get_target_param(topo_config_file);
    get_self_param(simu_config_file);
    inject_rate = net_param.inject_rate;
}

void release_resource()
{
   /*In resource copy mode, the below can be free*/
   int i = 0;
   if(lev_res != NULL )
   {
       int loop_level;
       if(universal_param.o_torus) loop_level = topo_level - 1;
       else loop_level = topo_level;
      
       for(i = 0; i < loop_level; i++)
       {
	   free(*(lev_res+i));
       }
       free(lev_res);
   }
   printf("relese levernode!\n");
   
    free(nic_desc);
    free(sw_desc);
   // The above two are released in universal_nic and universal_sw
   
   printf("relese nic and sw!\n");

}

void make_topo()
{
    if (hnet_opt.net_type == FAT_TREE)
	confg_topo_ftree();
    else if (hnet_opt.net_type ==TORFT)
	confg_topo_torft();
    else if (hnet_opt.net_type ==VTORFT)
	confg_topo_vtorft();
    else if (hnet_opt.net_type == FLBFLY)
	confg_topo_flbfly();
    else if (hnet_opt.net_type == SFLBFLY)
	confg_topo_sflbfly();
    else if (hnet_opt.net_type == MESH)
	confg_topo_mesh();
    else if (hnet_opt.net_type == TORUS)
	confg_topo_torus();
    else if (hnet_opt.net_type == ALL_TO_ALL)
	confg_topo_alltoall();
    else if (hnet_opt.net_type == FTREE_A2A)
	confg_topo_ftreea2a();
    else if (hnet_opt.net_type == UNIVERSAL)
    {
	universal_param_config();
	confg_topo_universal();
	printf("global_swnum = %d\n",global_swnum);
    }
    sw_confg_regiser_create(global_swnum);
    counter_nic = nic_num;
    counter_sw = global_swnum; 
}

