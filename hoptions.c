#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "hoptions.h"
#include "simk.h"		// simk_mode_t


// return value for long options that have no corresponding short ones
enum { HELP = 300, VERSION };
/**
 * Use this struct variable to make project-wide decisions, 
 * not by D_FLAGS or something else like that.
 */
hnet_opt_t hnet_opt = {
    .net_type = FAT_TREE,
    .trace_type = INNER_UNIFORM,
    .universal_tick_num = 300000,
    .trace_num = 1000000,
    .thread_num = INV_THREAD_NUM,
    .thread_binding = false,
    .k_mode = DEFAULT_SIMK_MODE
};


void version()
{
    printf("cHPPNetSim 1.0, using Simk 1.0. \n"
	   "Copyright (C) 2013, NCIC, ICT.\n");
    exit(0);
}

/**
 * usage -- print the help info 
 *
 * Edit it carefully and make it clear enough to command line users.
 */
void usage(char *name)
{
    printf("Usage: %s [OPTION]... \n", name);
    puts("Simulate several kinds of system area networks in detail.\n\n"
	 "Mandatory arguments to long options are mandatory for short options too.\n"
	 "  -t, --trace=NAME       NAME of the trace generator in simscript mode\n"
	 "                            file, uni, rand, bitrev, shuffle, hotregion;\n"
	 "                            default value is file\n"
	 "  -i, --universal-tick-num=NUM	scripting ticks to run, default to 1000\n"
	 "  -e, --trace-num=NUM     NUM trace entries will be generated internally\n"
	 "  -n, --net=NAME          NAME of the network type:\n"
	 "                            fat-tree, mesh\n"
	 "                            default to fat-tree\n"
	 "  -b, --thread-binding    enable thread binding; disabled by default\n"
	 "  -r, --thread-num=NUM    NUM threads per process, in range [1, #core-per-node]\n"
	 "                            0 or any number bigger than #core-per-node will be\n"
	 "                            considered as #core-per-node; default to 1\n"
	 "  -x, --exclusive-sr      run MPI send/recv procedures exclusively in a thread\n"
	 "                            this is for speedup when running in multi-process\n"
	 "  -m, --migration=MODE    the task migration MODE that simk runs in:\n"
	 "                            none, atom, lock, nego; default to none\n"
	 "  -f, --flow-control      enable flow control in simk library\n"
	 "  -p, --msg-pack          packing messages delivered between processes via MPI\n"
	 "                            for communication efficiency\n"
	 "      --help              display this help and exit\n"
	 "      --version           print version information and exit\n"
	 "\n" "Report bugs to <fanzhiguo@ncic.ac.cn>.");
    exit(0);
}

/**
 * process_trace_type -- assign hnet_opt.trace_type
 *
 * @optarg - string that represents the name of trace generator
 */
void process_trace_type(char *optarg)
{
    if (!strcmp(optarg, "file")) {
	hnet_opt.trace_type = TRACE_FILE;
    } else if (!strcmp(optarg, "uni")) {
	hnet_opt.trace_type = INNER_UNIFORM;
    } else if (!strcmp(optarg, "rand")) {
	hnet_opt.trace_type = INNER_RANDOM;
    } else if (!strcmp(optarg, "bitrev")){
	hnet_opt.trace_type = INNER_BIT;
	} else if (!strcmp(optarg, "shuffle")){
	hnet_opt.trace_type = INNER_SHUF;
	} else if (!strcmp(optarg, "hotregion")){
	hnet_opt.trace_type = INNER_HOT;
	} else {
	printf("Invalid trace generator '%s'\n", optarg);
	exit(1);
    }
}

/**
 * process_net_type -- assign hnet_opt.net_type
 *
 * @optarg - string that represents the name of network module;
 */
void process_net_type(char *optarg)
{
    if (!strcmp(optarg, "fat-tree")) {
	hnet_opt.net_type = FAT_TREE;
    } else if (!strcmp(optarg, "mesh")) {
	hnet_opt.net_type = MESH;
    } else if (!strcmp(optarg, "torus")) {
	hnet_opt.net_type = TORUS;
    } else if (!strcmp(optarg, "all_to_all")) {
	hnet_opt.net_type = ALL_TO_ALL;
    } else if (!strcmp(optarg, "universal")) {
	hnet_opt.net_type = UNIVERSAL;
    } else {
	printf("Invalid network module '%s'\n", optarg);
	exit(1);
    }
}

/**
 * process_mig_mode -- assign hnet_opt.mig_mode
 *
 * @optarg - string that represents the name of task migration mode;
 */
void process_mig_mode(char *optarg)
{
    if (!strcmp(optarg, "none")) {
	hnet_opt.k_mode.migrate = MIG_NONE;
    } else if (!strcmp(optarg, "atom")) {
	hnet_opt.k_mode.migrate = MIG_ATOM;
    } else if (!strcmp(optarg, "lock")) {
	hnet_opt.k_mode.migrate = MIG_LOCK;
    } else if (!strcmp(optarg, "desk")) {
	hnet_opt.k_mode.migrate = MIG_DESK;
    } else {
	printf("Invalid task migration module '%s'\n", optarg);
	exit(1);
    }
}

/**
 * process_thread_num -- assign hnet_opt.thread_num
 *   @optarg - string that represents the number of threads per process
 *
 * if the number specified in command line is 0 or larger than nprocs (the 
 * number of processors on the machine), then nprocs is used instead.
 */
static inline void process_thread_num(char *optarg)
{
    int n = atoi(optarg);
    int nprocs = get_nprocs_conf();

    bug_on(n < 0, "Invalid thread num: %d", n);
    hnet_opt.k_mode.thread_num = (n == 0 || n > nprocs) ? nprocs : n;
}

/*
 * This argument is used to control how many ticks should the
 * scripting simulator run.
 */
static inline void process_script_tick_num(char *optarg)
{
    int n = atoi(optarg);
    bug_on(n < 0, "Invalid script tick num: %d", n);
    hnet_opt.universal_tick_num = n;
}

static inline void process_trace_num(char *optarg)
{
    int n = atoi(optarg);
    bug_on(n < 0, "Invalid trace num: %d", n);
    hnet_opt.trace_num = n;
}

/**
 * get_hnet_opt -- process argv[] and give hnet_opt the right value.
 * 
 * @argc : argc from main()
 * @argv : argv from main()
 */
void get_hnet_opt(int argc, char *argv[])
{
    int c;
    char short_opts[] = "bxt:e:n:r:m:fpi:";
    static struct option long_opts[] = {
	{"net", 1, 0, 'n'},
	{"trace", 1, 0, 't'},
	{"script_tick_num", 1, 0, 'i'},
	{"trace-num", 1, 0, 'e'},
	{"thread-num", 1, 0, 'r'},
	{"thread-binding", 0, 0, 'b'},
	{"flow-control", 0, 0, 'f'},
	{"migrate", 0, 0, 'm'},
	{"help", 0, 0, HELP},
	{"version", 0, 0, VERSION},
	{"exclusive-sr", 0, 0, 'x'},
	{"msg-pack", 0, 0, 'p'},
	{0, 0, 0, 0}
    };

    while (1) {
	int opt_index = 0;
	c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
	if (c == -1)
	    break;
	switch (c) {
	case 't':		// -t, --trace
	    process_trace_type(optarg);
	    break;
	case 'i':		// -i, --script-tick-num
	    process_script_tick_num(optarg);
	    break;
	case 'e':		// -e, --trace-num
	    process_trace_num(optarg);
	    break;
	case 'n':		// -n, --net
	    process_net_type(optarg);
	    break;
	case 'r':		// -r, --thread-num
	    process_thread_num(optarg);
	    break;
	case 'b':		// -b, --thread-binding
	    hnet_opt.thread_binding = true;
	    break;
	case 'x':		// -x, --exclusive-sr
	    hnet_opt.k_mode.exclusive_sr = true;
	    break;
	case 'p':		// -p, --msg-pack
	    hnet_opt.k_mode.msg_pack = true;
	    break;
	case 'f':		// -f, --flow-control
	    // hnet_opt.flow_control not used currently
	    hnet_opt.k_mode.fc_enabled = true;
	    break;
	case 'm':		// -m, --migrate
	    process_mig_mode(optarg);
	    break;
	case HELP:		// --help
	    usage(argv[0]);
	    break;
	case VERSION:		// --version
	    version();
	    break;
	default:
	    break;
	}
    }
}
