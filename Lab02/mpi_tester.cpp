#include <mpi.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "mpi_tester.h"

const std::string mpi_tester::DEFAULT_OUTPUT_FILE_NAME = "output.txt";
const std::string mpi_tester::USE_GENERATED_MATRICES_ARG = "-g";
const std::string mpi_tester::OUTPUT_FILE_ARG = "-o";
const std::string mpi_tester::VERBOSE_ARG = "-v";
const int mpi_tester::ROOT_ID = 0;

void mpi_tester::read_matrix()
{
	std::ifstream input_file(this->input_file_matrix);
	size_t m, n;

	if (!input_file)
	{
		throw std::runtime_error("Input file does not exist or not ready to read: " + this->input_file_matrix);
	}

	input_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	input_file >> m >> n;

	auto full_matrix = std::make_shared<m_matrix>(m, n);

	input_file >> *full_matrix;

	input_file.close();

	auto pair = full_matrix->split();

	this->coeff_matrix = pair.first;
	this->right_hand_side = pair.second;
	this->matrix_rows = (*this->coeff_matrix).get_rows();
	this->matrix_columns = this->matrix_rows + 1;
}

void mpi_tester::check_range() const
{
	if (this->coeff_matrix == nullptr || this->approximation == nullptr)
	{
		throw std::invalid_argument("Both coefficient and approximation matrices must be initialized!");
	}

	if (this->coeff_matrix->get_rows() != this->approximation->get_size() ||
		this->coeff_matrix->get_columns() != this->coeff_matrix->get_rows())
	{
		std::stringstream ss;
		ss << "Can't apply Jacobi method for matrices with not arranged rows and columns number: "
			<< "coeff_matrix[" << this->coeff_matrix->get_rows() << "][" << this->coeff_matrix->get_columns() << "] and "
			<< "approximation[" << this->approximation->get_size() << "]";
		throw std::invalid_argument(ss.str());
	}
}

void mpi_tester::print_answer() const
{
	std::ofstream out_file(this->output_file, std::ofstream::trunc);

	if (!out_file)
	{
		throw std::runtime_error("Output file does not exist or not ready to read: " + this->output_file);
	}

	out_file.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	out_file << *this->approximation;

	out_file.close();
}

double mpi_tester::distance(m_vector::vector_t &new_appr, m_vector::vector_t &old_appr)
{
	auto size = new_appr->get_size();
	auto sum = 0.0;

	for (auto i = 0; i < size; ++i)
	{
		sum += ((*new_appr)[i] - (*old_appr)[i]) * ((*new_appr)[i] - (*old_appr)[i]);
	}

	return sqrt(sum);
}

void mpi_tester::check_arguments_available(const int total, const int current, const int required)
{
	if (current + required >= total)
	{
		throw std::invalid_argument("Not enough arguments to resolve argument! " + get_help());
	}
}

std::string mpi_tester::get_help()
{
	std::stringstream ss;
	ss << "Usage: Lab02 "
		<< "[" << OUTPUT_FILE_ARG << " output_path] "
		<< "[" << USE_GENERATED_MATRICES_ARG << " rows] "
		<< "input_file_matrix input_file_approximation precision max_iterations";
	return ss.str();
}

void mpi_tester::calculate_data_distribution()
{
	auto div = std::div(static_cast<int>(this->get_size()), this->total_processes);
	this->rows_per_process = (div.rem ? div.quot + 1 : div.quot);
	this->cells_per_process = this->get_size() * this->rows_per_process;
	this->rows_number_distribution = std::make_shared<std::vector<int>>(this->total_processes, 0);
	this->rows_positions_distribution = std::make_shared<std::vector<int>>(this->total_processes, 0);
	this->cells_number_distribution = std::make_shared<std::vector<int>>(this->total_processes, 0);
	this->cells_positions_distribution = std::make_shared<std::vector<int>>(this->total_processes, 0);

	auto row = 0;
	for (auto i = 0; i < this->total_processes; ++i)
	{
		auto left = this->get_size() - row;
		auto current = static_cast<int>(std::min(left, this->rows_per_process));
		(*this->rows_number_distribution)[i] = current;
		(*this->rows_positions_distribution)[i] = row;
		(*this->cells_number_distribution)[i] = current * static_cast<int>(this->get_size());
		(*this->cells_positions_distribution)[i] = row * static_cast<int>(this->get_size());
		row += current;
	}

	this->print_data_distribution();
}

void mpi_tester::print_data_distribution() const
{
	if (this->verbose)
	{
		std::stringstream ss;
		ss << "Rows to process: " << (*this->rows_number_distribution)[this->process_id] << ", "
			<< "rows position: " << (*this->rows_positions_distribution)[this->process_id] << ", "
			<< "cells to process: " << (*this->cells_number_distribution)[this->process_id] << ", "
			<< "cells position: " << (*this->cells_positions_distribution)[this->process_id];
		auto msg = ss.str();
		log(msg);
	}
}

void mpi_tester::log(std::string &msg) const
{
	if (this->verbose)
	{
		print_process_id();
		std::cout << msg << std::endl;
	}
}

void mpi_tester::log(char *const msg) const
{
	if (this->verbose)
	{
		print_process_id();
		std::cout << msg << std::endl;
	}
}

size_t mpi_tester::parse_size_t(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoull(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()));
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()));
	}
}

double mpi_tester::parse_double(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stod(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()));
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()));
	}
}

int mpi_tester::parse_int(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoi(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()));
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()));
	}
}

mpi_tester::mpi_tester(const int argc, const char *const argv[])
{
	for (auto i = 1; i < argc; ++i)
	{
		auto current = std::string(argv[i]);
		if (current == this->OUTPUT_FILE_ARG)
		{
			check_arguments_available(argc, i, 1);
			this->output_file = std::string(argv[i++ + 1]);
		}
		else if (current == this->USE_GENERATED_MATRICES_ARG)
		{
			check_arguments_available(argc, i, 1);
			this->matrix_rows = parse_size_t(argv[i++ + 1],
			                                 "Non-integer parameter passed as matrices dimension! ",
			                                 "Too large value passed as matrices dimension! ");
			this->matrix_columns = this->matrix_rows + 1;
			this->use_gen_input = true;
		}
		else if (current == this->VERBOSE_ARG)
		{
			this->verbose = true;
		}
		else if (this->input_file_matrix == "")
		{
			this->input_file_matrix = current;
		}
		else if (this->precision == -1)
		{
			this->precision = parse_double(current.c_str(),
			                               "Non-floating-point parameter passed as precision! ",
			                               "Too large value passed as precision! ");
		}
		else if (this->max_iterations == 0)
		{
			this->max_iterations = parse_size_t(current.c_str(),
			                                    "Non-integer parameter passed as max iterations parameter! ",
			                                    "Too large value passed as max iterations parameter! ");
		}
	}

	if (this->output_file.empty())
	{
		this->output_file = this->DEFAULT_OUTPUT_FILE_NAME;
	}

	try
	{
		check_arguments();
	}
	catch (std::exception const &e)
	{
		std::cout << e.what() << std::endl << get_help() << std::endl;
	}
}

void mpi_tester::check_arguments() const
{
	if (this->input_file_matrix == "")
	{
		throw std::invalid_argument("File with matrix to solve has not been passed!");
	}
	if (this->precision == -1)
	{
		throw std::invalid_argument("Precision has not been passed!");
	}
	if (this->max_iterations == 0)
	{
		throw std::invalid_argument("Maximum iterations number has not been passed!");
	}
}

void mpi_tester::print_process_id() const
{
	if (this->verbose)
	{
		std::cout << "[" << this->process_id << "] ";
	}
}

void mpi_tester::init()
{
	MPI_Comm_size(MPI_COMM_WORLD, &this->total_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &this->process_id);

	if (this->process_id == this->ROOT_ID)
	{
		if (this->use_gen_input)
		{
			log("Generating matrix and approximation");
			this->coeff_matrix = m_matrix::generate_matrix(this->matrix_rows, this->matrix_rows);
			this->right_hand_side = m_vector::generate_vector(this->matrix_rows);
			this->approximation = m_vector::generate_vector(this->matrix_rows);
		}
		else
		{
			log("Reading matrix and approximation");
			this->read_matrix();
			this->approximation = std::make_shared<m_vector>(this->matrix_rows);
		}
		this->calculate_data_distribution();
		this->check_range();
		this->send_meta_data();
		this->send_initial_data();
	}
	else
	{
		this->receive_meta_data();
		this->calculate_data_distribution();
		this->receive_initial_data();
	}
}

std::string mpi_tester::get_metadata() const
{
	std::stringstream ss;
	ss << "n: " << this->get_size()
		<< ", precision: " << this->precision
		<< ", max_iterations: " << this->max_iterations
		<< ", rows_per_process: " << this->rows_per_process
		<< ", cells_per_process: " << this->cells_per_process;
	return ss.str();
}

void mpi_tester::send_meta_data() const
{
	auto msg = "Sending metadata: " + this->get_metadata();
	log(msg);

	auto size = static_cast<long>(this->get_size());
	auto precision = this->precision;
	auto max_iterations = static_cast<long>(this->max_iterations);

	MPI_Bcast(&size, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&precision, 1, MPI_DOUBLE, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&max_iterations, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
}

void mpi_tester::receive_meta_data()
{
	log("Receiving metadata");

	long size = 0;
	auto precision = 0.0;
	auto max_iterations = static_cast<long>(this->max_iterations);

	MPI_Bcast(&size, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&precision, 1, MPI_DOUBLE, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&max_iterations, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);

	this->matrix_rows = static_cast<size_t>(size);
	this->matrix_columns = matrix_rows + 1;
	this->precision = precision;
	this->max_iterations = max_iterations;
}

void mpi_tester::send_initial_data() const
{
	log("Sending initial data");

	auto rows = (*this->rows_number_distribution)[this->process_id];
	auto cells = (*this->cells_number_distribution)[this->process_id];

	auto plain_matrix_data = std::make_shared<std::vector<type_t>>(cells);
	auto plain_right_hand_data = std::make_shared<std::vector<type_t>>(rows);

	MPI_Scatterv(this->coeff_matrix->get_plain_data()->data(),
	             this->cells_number_distribution->data(),
	             this->cells_positions_distribution->data(), MPI_DOUBLE,
	             plain_matrix_data->data(), static_cast<int>(cells), MPI_DOUBLE,
	             this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Scatterv(this->right_hand_side->get_data()->data(),
	             this->rows_number_distribution->data(),
	             this->rows_positions_distribution->data(), MPI_DOUBLE,
	             plain_right_hand_data->data(), static_cast<int>(rows), MPI_DOUBLE,
	             this->ROOT_ID, MPI_COMM_WORLD);
}

void mpi_tester::receive_initial_data()
{
	log("Receiving initial data");

	auto rows = (*this->rows_number_distribution)[this->process_id];
	auto cells = (*this->cells_number_distribution)[this->process_id];

	this->coeff_matrix = std::make_shared<m_matrix>(rows, this->get_size());
	this->right_hand_side = std::make_shared<m_vector>(rows);
	this->approximation = std::make_shared<m_vector>(this->matrix_rows);

	auto plain_matrix_data = std::make_shared<std::vector<type_t>>(cells);
	auto plain_right_hand_data = std::make_shared<std::vector<type_t>>(rows);

	MPI_Scatterv(nullptr,
	             this->cells_number_distribution->data(),
	             this->cells_positions_distribution->data(), MPI_DOUBLE,
	             plain_matrix_data->data(), cells, MPI_DOUBLE,
	             this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Scatterv(nullptr,
	             this->rows_number_distribution->data(),
	             this->rows_positions_distribution->data(), MPI_DOUBLE,
	             plain_right_hand_data->data(), rows, MPI_DOUBLE,
	             this->ROOT_ID, MPI_COMM_WORLD);

	this->coeff_matrix->fill(plain_matrix_data, cells);
	this->right_hand_side->fill(plain_right_hand_data, rows);
}

std::pair<bool, size_t> mpi_tester::apply_jacobi() const
{
	auto x_old = std::make_shared<m_vector>(this->get_size());
	auto x_new = std::make_shared<m_vector>(this->get_size());

	auto rows = (*this->rows_number_distribution)[this->process_id];

	MPI_Allgatherv(this->right_hand_side->get_data()->data(), rows, MPI_DOUBLE,
	               x_new->get_data()->data(),
	               this->rows_number_distribution->data(),
	               this->rows_positions_distribution->data(), MPI_DOUBLE, MPI_COMM_WORLD);

	auto iteration = 0;

	do
	{
		iteration++;
		x_old.swap(x_new);

		for (auto i = 0; i < rows; ++i)
		{
			auto g = i + this->process_id * rows;
			(*approximation)[i] = (*this->right_hand_side)[i];
			for (auto j = 0; j < g; ++j)
				(*approximation)[i] -= (*coeff_matrix)[i][j] * (*x_old)[j];
			for (auto j = g + 1; j < this->get_size(); ++j)
				(*approximation)[i] -= (*coeff_matrix)[i][j] * (*x_old)[j];
			(*approximation)[i] /= (*coeff_matrix)[i][g];
		}

		MPI_Allgatherv(approximation->get_data()->data(), rows, MPI_DOUBLE,
		               x_new->get_data()->data(), this->rows_number_distribution->data(),
		               this->rows_positions_distribution->data(), MPI_DOUBLE, MPI_COMM_WORLD);
	}
	while (iteration < this->max_iterations && this->distance(x_new, x_old) >= this->precision);

	return std::make_pair(this->distance(x_new, x_old) < this->precision, iteration);
}

size_t mpi_tester::get_size() const
{
	return this->matrix_rows;
}

void mpi_tester::process() const
{
	log("Processing");

	auto converged = this->apply_jacobi();
	auto rows = (*this->rows_number_distribution)[this->process_id];
	auto plain_approximation_data = std::make_shared<std::vector<type_t>>(this->matrix_rows);

	std::stringstream ss;
	ss << "Local approximation: " << *approximation;
	auto msg = ss.str();
	log(msg);

	auto x_new = std::make_shared<m_vector>(this->get_size());

	MPI_Gatherv(this->approximation->get_data()->data(), rows, MPI_DOUBLE,
	            plain_approximation_data->data(), this->rows_number_distribution->data(),
	            this->rows_positions_distribution->data(), MPI_DOUBLE,
	            this->ROOT_ID, MPI_COMM_WORLD);

	if (this->process_id == ROOT_ID)
	{
		this->approximation->fill(plain_approximation_data, this->get_size());

		std::stringstream ss2;
		ss2 << "Global approximation: " << *approximation << ", "
			<< "Converged: " << (converged.first ? "true" : "false") << ", iterations: " << converged.second;
		msg = ss2.str();
		log(msg);
		print_answer();
	}
}
