#include "cartridge.h"
#include "graphics.h"
#include "launcher.h"
#include <stdio.h>

int		main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s cartridge_file\n", *argv);
		return (1);
	}

	open_cartridge(argv[1]);

	gr_init_window();

	start_game();
	return (0);
}
