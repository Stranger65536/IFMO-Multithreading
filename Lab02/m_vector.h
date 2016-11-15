#ifndef LAB02_VECTOR_H
#define LAB02_VECTOR_H

#include <vector>
#include <memory>
#include <sstream>
#include <fstream>

typedef long double type_t;

class m_vector
{
	typedef std::vector<type_t> data_t;
	const std::shared_ptr<data_t> data;
	const size_t size;
public:
	typedef std::shared_ptr<m_vector> vector_t;
	explicit m_vector(const size_t size);
	static vector_t generate_vector(const size_t size);
	std::shared_ptr<std::vector<type_t>> get_data() const;
	void fill(std::shared_ptr<std::vector<type_t>> &plain_data, const size_t size) const;

	friend std::istream &operator >>(std::istream &input, m_vector &o)
	{
		for (size_t i = 0; i < o.get_size(); ++i)
		{
			if (!input.eof())
			{
				input >> o[i];
			}
			else
			{
				std::stringstream ss;
				ss << "Input stream contains less than " << o.get_size() << " elements!";
				throw std::range_error(ss.str());
			}
		}

		return input;
	}

	friend std::ostream &operator <<(std::ostream &output, m_vector &o)
	{
		for (size_t i = 0; i < o.get_size(); ++i)
		{
			output << o[i];
			if (i != o.get_size() - 1)
			{
				 output << " ";
			}
		}

		return output;
	}

	friend std::ostream &operator <<(std::ofstream &output, m_vector &o)
	{
		if (!output)
		{
			throw std::runtime_error("Output stream is not ready to write!");
		}

		output.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		output << o.get_size() << std::endl;

		for (size_t i = 0; i < o.get_size(); ++i)
		{
			output << o[i] << std::endl;
		}

		return output;
	}

	size_t get_size() const
	{
		return size;
	}

	type_t &operator[](const size_t x) const
	{
		return (*data).at(x);
	}
};

#endif LAB02_VECTOR_H
