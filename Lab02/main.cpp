#include "mpi_tester.h"
#include <iostream>
#include <chrono>
#include <mpi.h>

int main(int argc, char * argv[]) {
	MPI_Init(&argc, &argv);

    try {
	    auto tester = mpi_tester(argc, argv);
		auto before = std::chrono::high_resolution_clock::now();
		tester.init();
        tester.process();
		auto after = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();
		std::cout << "Time taken: " << time << "ms" << std::endl;
    } catch (std::exception const &e) {
        std::cerr << "Error occurred: " << e.what() << std::endl;
    }
	
	MPI_Finalize();

    return 0;
}