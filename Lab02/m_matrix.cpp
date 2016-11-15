#include "m_matrix.h"

m_matrix::m_matrix(const size_t rows, const size_t columns): data(std::make_shared<data_t>(rows, nullptr)),
                                                             rows(rows),
                                                             columns(columns)
{
	for (auto i = 0; i < rows; ++i)
	{
		(*this->data)[i] = std::make_shared<m_vector>(columns);
	}
}

std::pair<m_matrix::matrix_t, m_vector::vector_t> m_matrix::split() const
{
	if (this->get_columns() < 2)
	{
		throw std::range_error("Can't extract the right-hand values from matrix with less than 2 columns!");
	}

	auto a = std::make_shared<m_matrix>(this->get_rows(), this->get_rows());
	auto b = std::make_shared<m_vector>(this->get_rows());

	for (size_t i = 0; i < this->get_rows(); ++i)
	{
		for (size_t j = 0; j < this->get_columns() - 1; ++j)
		{
			(*a)[i][j] = (*(*this->data)[i])[j];
		}
		(*b)[i] = (*(*this->data)[i])[this->get_columns() - 1];
	}

	return make_pair(a, b);
}


std::shared_ptr<std::vector<type_t>> m_matrix::get_plain_data() const
{
	auto result = std::make_shared<std::vector<type_t>>(this->get_rows() * this->get_columns());

	for (auto i = 0; i < (*this->data).size(); ++i)
	{
		auto row = (*this->data)[i];
		for (auto j = 0; j < row->get_size(); ++j)
		{
			(*result)[i * row->get_size() + j] = (*(*this->data)[i])[j];
		}
	}

	return result;
}


void m_matrix::fill(std::shared_ptr<std::vector<type_t>> &plain_data, const size_t size) const
{
	for (auto i = 0; i < size; ++i)
	{
		(*(*this->data)[i / this->columns])[i % columns] = (*plain_data)[i];
	}
}

m_matrix::matrix_t m_matrix::generate_matrix(const size_t m, const size_t n)
{
	auto result = std::make_shared<m_matrix>(m, n);

	for (auto i = 0; i < m; ++i)
	{
		for (auto j = 0; j < n; ++j)
		{
			if (i == j)
			{
				(*result)[i][j] = static_cast<type_t>(1);
			}
			else if (i < j)
			{
				(*result)[i][j] = static_cast<type_t>(-2);
			}
		}
	}

	return result;
}
