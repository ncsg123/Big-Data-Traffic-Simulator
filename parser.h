#ifndef NETSIM_PARSER_H
#define NETSIM_PARSER_H

#include <stdbool.h>

#define MAX_SW_PORT 64
#define MAX_NIC_PORT 64
#define MAX_LEVEL_PORT 100

// direction of NIC ports: Centra, East, South, West, North
typedef enum {C=0, E, S, W, N} direct_t;

typedef struct {
    int local;		//local port number, port's own number
    int remote;		//remote port number, the number port connected to 
} x_port_t;

/* information for components connection and initialization*/
typedef struct {
    int m;				//information for nic, it can be switch port number or others
    int n;				//it can be topology level number, or others	
    int nicnum;				//nic port number
    int vcnum;				//vc number
    int fifo_depth;			//queue depth
    int frequency;			//one cycle time, unit (ns)
    int bus_width;			//bus width
    x_port_t port[MAX_NIC_PORT];	//port information
    unsigned int index;			//componet ID number
} nic_desc_t;

typedef struct {
    int m;				//sw port number
    int n;                              //it can be topology level number, or others	
    int vcnum;				//vc number
    int fifo_depth;
    int frequency;
    int bus_width;
    x_port_t port[MAX_SW_PORT];
    unsigned int index;
} sw_desc_t;

typedef struct {
    int m;
    int n;
    int vcnum;
    int fifo_depth;
    int frequency;
    int bus_width;
    x_port_t port[MAX_SW_PORT];
    unsigned int index;
} router_desc_t;

typedef struct levnd_desc_t
{
    int level;				//the topology level number, used for universal topology 
    int inner_id;			//level node inner number
    int whole_id;			//level node global number
    x_port_t *port_ptr[MAX_LEVEL_PORT]; //port information
    int index[MAX_LEVEL_PORT];		//port's owner
    struct levnd_desc_t *outside;	//point to the upper level 
}levnd_desc_t;

extern int *dms;	//dimension size
extern int global_swnum;
extern int global_routernum;
extern int nic_num;
extern int router_port_num;
void confg_from_file();
void make_topo();
void ftree_task_partition();
void release_resource();


#endif  // NETSIM_PARSER_H
