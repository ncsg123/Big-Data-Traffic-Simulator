/**
 * hoptions.h
 *
 * global options that determine the behavior of cHppNetSim.
 * Copyright (C) 2013 NCIC, ICT.
 *
 *
 */

#ifndef HPPNETSIM_OPTIONS_H
#define HPPNETSIM_OPTIONS_H

#include "simk.h"
/**
 * net_type_t - network type of the target system
 * 	FAT_TREE : m-tree, n-port fat tree
 * 	MESH : mesh networks
 * 	TORFT: Data Center Network, Fat tree topology, Sever with TOR
 * 	VTORFT:Data Center Network using PCIE NIC, which contains virtual function.
 * 	FLBFLY: Standrad Flattened butterfly topology
 * 	SFLBFLY: User defined Flattened butterfly topology
 * 	ALL_TO_ALL: All to all network
 * 	Universal: User defined topologys
 */
typedef enum { NET_UNDEF, FAT_TREE,TORFT,VTORFT,FLBFLY,SFLBFLY, MESH,TORUS,ALL_TO_ALL,FTREE_A2A, UNIVERSAL} net_type_t; 
/**
 * universal_type_t - universal generator type in SIMuniversal mode
 *	TRACE_FILE : trace read from file
 *	INNER_UNIFORM : inner trace generator; constant-interval L/S addresses
 *	INNER_RANDOM : inner trace generator; random L/S addr across the whole 
 *	               virutal memory space
 */
typedef enum { TRACE_FILE = 0, INNER_UNIFORM, INNER_RANDOM, INNER_BIT, INNER_SHUF, INNER_HOT } trace_type_t;

// FIXME: do not put it here
#define INV_THREAD_NUM -1

typedef struct {
    net_type_t net_type;
    trace_type_t trace_type;
    int universal_tick_num;
    int trace_num;
    bool thread_binding;
    int thread_num;

    simk_mode_t k_mode;
} hnet_opt_t;

extern hnet_opt_t hnet_opt;

void get_hnet_opt(int argc, char *argv[]);

#endif				// GLOBAL_OPTIONS_H
