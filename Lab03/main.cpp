#include "mpi_tester.h"
#include <iostream>
#include <chrono>
#include <mpi.h>

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	try
	{
		auto tester = mpi_tester(argc, argv);
		tester.init();
		tester.process();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error occurred: " << e.what() << std::endl;
		return 1;
	}

	MPI_Finalize();

	return 0;
}
