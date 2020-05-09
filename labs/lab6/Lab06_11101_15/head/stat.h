#ifndef STAT_H
#define STAT_H

#include <linux/ioctl.h>
#define WR_CMD _IOW('a','a',stat_t*)
#define RD_CMD _IOR('a','b',stat_t*)


typedef struct{
	int num_liv_cells;
	int num_born_cells;
	int num_dead_cells;
	int gen_counter;
}stat_t;

#endif
