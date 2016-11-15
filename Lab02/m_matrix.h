#ifndef LAB02_MATRIX_H
#define LAB02_MATRIX_H

#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include "m_vector.h"

class m_matrix
{
	typedef std::vector<std::shared_ptr<m_vector>> data_t;
	const std::shared_ptr<data_t> data;
	const size_t rows, columns;
public:
	typedef std::shared_ptr<m_matrix> matrix_t;
	explicit m_matrix(const size_t rows, const size_t columns);
	static matrix_t generate_matrix(const size_t rows, const size_t columns);
	std::pair<matrix_t, m_vector::vector_t> split() const;
	std::shared_ptr<std::vector<type_t>> get_plain_data() const;
	void fill(std::shared_ptr<std::vector<type_t>> &plain_data, const size_t size) const;

	size_t get_rows() const
	{
		return rows;
	}

	size_t get_columns() const
	{
		return columns;
	}

	friend std::istream &operator >>(std::istream &input, m_matrix &o)
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

	friend std::ostream &operator <<(std::ostream &output, const m_matrix &o)
	{
		for (size_t i = 0; i < o.rows; ++i)
		{
			for (size_t j = 0; j < o.columns; ++j)
			{
				output << o[i][j] << " ";
			}
		}

		return output;
	}

	m_vector &operator[](const size_t x) const
	{
		return *(*data).at(x);
	}
};

#endif LAB02_MATRIX_H
