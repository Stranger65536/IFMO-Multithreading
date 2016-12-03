#include <chrono>
#include <random>
#include "matrix.h"

matrix::matrix(const size_t rows, const size_t columns) : rows(rows),
                                                          columns(columns),
                                                          data(rows, std::vector<type_t>(columns))
{
}

void matrix::generate_matrix(const type_t min, const type_t max)
{
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine gen(seed);
	std::uniform_int_distribution<type_t> dist(min, max);

	for (auto i = 0; i < rows; ++i)
	{
		for (auto j = 0; j < columns; ++j)
		{
			data[i][j] = i == j ? 0 : dist(gen);
		}
	}
}
