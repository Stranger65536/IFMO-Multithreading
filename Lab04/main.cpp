#include <iostream>
#include "omp_tester.h"

int main(const int argc, const char *const argv[])
{
	try
	{
		auto tester = omp_tester(argc, argv);
		tester.process();
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error occurred: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
