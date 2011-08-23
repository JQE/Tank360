#include "input.h"

static struct controller_data_s ctrl[4];
static int valid[4];

int get_controller_data(struct controller_data_s *d, int port)
{
	int r= 0;
	if (port >= 4)
		return 0;
	*d = ctrl[port];
	r = valid[port];
	valid[port] = 0;
	return r;
}

void set_controller_data(int port, const struct controller_data_s *d)
{
	if (port >= 4)
		return;
	ctrl[port] = *d;
	valid[port] = 1;
}
