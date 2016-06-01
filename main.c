#include "simk.h"
#include "parser.h"
#include "hoptions.h"
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    char name[32];
    int rank, len;
    MPI_simk_start(&argc, &argv);
    get_processor_name(name, &len);
    rank = get_my_rank();
    printf("I am rank %d on %s\n", rank, name);

    if (get_my_rank() == 0) {	// slaves do nothing
	confg_from_file(); 
	get_hnet_opt(argc, argv);
	simk_init(hnet_opt.k_mode);
	make_topo();
	if (hnet_opt.net_type == FAT_TREE) {
	    ftree_task_partition();   // user defined partition algorithm
	} else {
	    simk_partition();
	}
	simk_dispatch_task();
    } else {
	get_hnet_opt(argc, argv);
	simk_init(hnet_opt.k_mode);
    }

    simk_run();

    simk_fini();

    MPI_simk_finalize();
    return 0;
}
