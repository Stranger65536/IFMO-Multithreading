#include "m_vector.h"

m_vector::m_vector(const size_t size) : data(std::make_shared<data_t>(size, 0.0)), size(size)
{
}

m_vector::vector_t m_vector::generate_vector(const size_t size)
{
	auto result = std::make_shared<m_vector>(size);

	for (auto i = 0; i < size; ++i)
	{
		(*result)[i] = static_cast<type_t>(i % 2 ? -1.0 / 3.0 : 1);
	}

	return result;
}

std::shared_ptr<std::vector<type_t>> m_vector::get_data() const
{
	return this->data;
}

void m_vector::fill(std::shared_ptr<std::vector<type_t>> &plain_data, const size_t size) const
{
	for (auto i = 0; i < size; ++i)
	{
		(*this->data).at(i) = (*plain_data)[i];
	}
}
