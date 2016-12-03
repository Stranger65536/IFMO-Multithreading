#ifndef LAB04_MATRIX_H
#define LAB04_MATRIX_H

#include <chrono>
#include <vector>
#include <sstream>
#include <iomanip>
#include "random.h"

template <typename T>
class matrix
{
	const size_t rows, columns;
	random<T> rnd{-1, 100};
	std::vector<std::vector<T>> data;
public:
	matrix(const size_t rows, const size_t columns) : rows(rows),
	                                                  columns(columns),
	                                                  data(rows, std::vector<T>(columns))
	{
	}

	void generate_matrix(const T min, const T max)
	{
		for (auto i = 0; i < rows; ++i)
		{
			for (auto j = 0; j < columns; ++j)
			{
				data[i][j] = i == j ? 0 : rnd.next();
			}
		}
	}

	size_t get_rows() const
	{
		return rows;
	}

	size_t get_columns() const
	{
		return columns;
	}

	friend std::istream &operator>>(std::istream &input, matrix &o)
	{
		for (size_t i = 0; i < o.rows; ++i)
		{
			for (size_t j = 0; j < o.columns; ++j)
			{
				if (!input.eof())
				{
					input >> o[i][j];
				}
				else
				{
					std::stringstream ss;
					ss << "Input stream contains less than "
						<< o.rows << "x" << o.columns << " elements!";
					throw std::range_error(ss.str());
				}
			}
		}

		return input;
	}

	friend std::ostream &operator<<(std::ostream &output, const matrix &o)
	{
		for (size_t i = 0; i < o.rows; ++i)
		{
			for (size_t j = 0; j < o.columns; ++j)
			{
				output << std::setw(3) << o[i][j] << " ";
			}
			output << std::endl;
		}

		return output;
	}

	std::vector<T> &operator[](const size_t x)
	{
		return data[x];
	}

	std::vector<T> operator[](const size_t x) const
	{
		return data[x];
	}
};

#endif //LAB04_MATRIX_H
