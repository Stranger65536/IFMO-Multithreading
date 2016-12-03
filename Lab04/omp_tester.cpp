#include <omp.h>
#include <chrono>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "omp_tester.h"


const std::string omp_tester::DEFAULT_OUTPUT_FILE_NAME = "output.txt";
const std::string omp_tester::USE_GENERATED_GRAPH_ARG = "-g";
const std::string omp_tester::OUTPUT_FILE_ARG = "-o";
const std::string omp_tester::VERBOSE_ARG = "-v";
const std::string omp_tester::ITERATIONS_NUMBER_ARG = "-i";
const std::string omp_tester::THREADS_NUMBER_ARG = "-t";
const omp_tester::type_t omp_tester::NO_PATH_VALUE = -1;

void omp_tester::log(std::stringstream &ss) const
{
	if (verbose)
	{
		std::cout << ss.str() << std::endl;
	}
}

template <typename H, typename... T>
void omp_tester::log(std::stringstream &ss, H &p, T ... t) const
{
	if (verbose)
	{
		ss << p;
		log(ss, t...);
	}
}

template <typename H, typename... T>
void omp_tester::log(H &p, T ... t) const
{
	if (verbose)
	{
		std::stringstream ss;
		ss << p;
		log(ss, t...);
	}
}

int64_t omp_tester::get_average_execution_time() const
{
	return std::accumulate(time_accumulator.cbegin(), time_accumulator.cend(), static_cast<int64_t>(0)) / time_accumulator.size();
}


std::unique_ptr<std::vector<omp_tester::type_t>> omp_tester::dijkstra_run()
{
	std::vector<bool> visited(nodes, false);
	auto dist = std::make_unique<std::vector<type_t>>(nodes, std::numeric_limits<type_t>::max());
	auto node = start_node;
	(*dist)[start_node] = 0;
	auto before = m_clock::now();
	for (auto i = 0; i < nodes; ++i)
	{
		visited[node] = true;

#pragma omp parallel for schedule(static)
		for (auto to = 0; to < nodes; ++to)
		{
			if ((*data)[node][to] != NO_PATH_VALUE)
			{
				if ((*dist)[node] + (*data)[node][to] < (*dist)[to])
				{
					(*dist)[to] = (*dist)[node] + (*data)[node][to];
				}
			}
		}

		node = get_next_node(visited, *dist);
	}

	auto after = m_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();

	std::cout << "Time taken: " << format_time(time) << std::endl;

	time_accumulator.push_back(time);

	return dist;
}

std::string omp_tester::format_time(int64_t nanos)
{
	const auto seconds = nanos / 1000000000;
	nanos %= 1000000000;
	const auto milliseconds = nanos / 1000000;
	nanos %= 1000000;
	const auto microseconds = nanos / 1000;
	nanos %= 1000;
	const auto nanoseconds = nanos;

	std::stringstream ss;
	ss << std::setw(3) << seconds << "s " <<
		std::setw(3) << milliseconds << "ms " <<
		std::setw(3) << microseconds << "mcs " <<
		std::setw(3) << nanoseconds << "ns";

	return ss.str();
}


size_t omp_tester::get_next_node(std::vector<bool> &visited, std::vector<type_t> &dist) const
{
	type_t min;

	auto nextNode = -1;
	min = std::numeric_limits<type_t>::max();

	for (auto i = 0; i < nodes; ++i)
	{
		if (!visited[i] && dist[i] < min)
		{
			min = dist[i];
			nextNode = i;
		}
	}

	return nextNode;
}

void omp_tester::generate_matrix(const size_t &n)
{
	log("Generating matrix ", n, "x", n);
	data = std::make_unique<matrix<type_t>>(n, n);
	data->generate_matrix(-1, 100);
}

void omp_tester::read_matrix(const std::string &file_path)
{
	log("Reading matrix from file ", file_path);

	std::ifstream input_file(file_path);
	size_t n;

	if (!input_file)
	{
		throw std::runtime_error("Input file does not exist or not ready to read: " + file_path);
	}

	input_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	input_file >> n;
	data = std::make_unique<matrix<type_t>>(n, n);
	input_file >> *data;
	nodes = n;
}

template <typename T>
std::ostream &operator <<(std::ostream &output, const std::vector<T> &o)
{
	auto indx_width = std::to_string(o.size() - 1).length();
	auto elem_width = std::to_string(*std::max_element(o.cbegin(), o.cend())).length();

	for (auto i = 0; i < o.size(); ++i)
	{
		output << std::setw(indx_width) << i << " " << std::setw(elem_width) << o[i] << std::endl;
	}

	return output;
}

void omp_tester::print_answer(const std::string &file_path, const std::vector<type_t> &result)
{
	std::ofstream out_file(file_path, std::ofstream::trunc);

	if (!out_file)
	{
		throw std::runtime_error("Output file is not ready to write: " + file_path);
	}

	out_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	out_file << result;
}

void omp_tester::check_arguments_available(const int total, const int current, const int required)
{
	if (current + required >= total)
	{
		throw std::invalid_argument("Not enough arguments to resolve argument! " + get_help());
	}
}

std::string omp_tester::get_help()
{
	std::stringstream ss;
	ss << "Usage: Lab04 "
		<< "[" << OUTPUT_FILE_ARG << " output_path] "
		<< "[" << USE_GENERATED_GRAPH_ARG << " size] "
		<< "[" << ITERATIONS_NUMBER_ARG << " iterations_number] "
		<< "[" << THREADS_NUMBER_ARG << " threads_number] "
		<< "input_file start_node";
	return ss.str();
}

size_t omp_tester::parse_size_t(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoull(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()) + ": " + value);
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()) + ": " + value);
	}
}

int omp_tester::parse_int(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoi(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()) + ": " + value);
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()) + ": " + value);
	}
}

omp_tester::omp_tester(const int argc, const char *const argv[])
{
	for (auto i = 1; i < argc; ++i)
	{
		auto current = std::string(argv[i]);
		if (current == OUTPUT_FILE_ARG)
		{
			check_arguments_available(argc, i, 1);
			output_file = std::string(argv[++i]);
		}
		else if (current == USE_GENERATED_GRAPH_ARG)
		{
			check_arguments_available(argc, i, 1);
			nodes = parse_size_t(argv[++i],
			                     "Non-integer parameter passed as matrices dimension!",
			                     "Too large value passed as matrices dimension!");
			use_gen_input = true;
		}
		else if (current == ITERATIONS_NUMBER_ARG)
		{
			check_arguments_available(argc, i, 1);
			iterations = parse_int(argv[++i],
			                       "Non-integer parameter passed as iterations number!",
			                       "Too large value passed as iterations number!");
			time_accumulator.reserve(iterations);
		}
		else if (current == THREADS_NUMBER_ARG)
		{
			check_arguments_available(argc, i, 1);
			threads = parse_int(argv[++i],
			                    "Non-integer parameter passed as threads number!",
			                    "Too large value passed as iterations number!");
			omp_set_num_threads(threads);
		}
		else if (current == VERBOSE_ARG)
		{
			verbose = true;
		}
		else if (input_file == "")
		{
			input_file = current;
		}
		else if (!start_node_specified)
		{
			start_node = parse_size_t(argv[i],
			                          "Non-integer parameter passed as matrices dimension!",
			                          "Too large value passed as matrices dimension!");
			start_node_specified = true;
		}
	}

	if (output_file.empty())
	{
		output_file = DEFAULT_OUTPUT_FILE_NAME;
	}

	check_arguments();
}

void omp_tester::check_arguments() const
{
	if (input_file.empty())
	{
		throw std::invalid_argument("Input file is required!");
	}
	if (!nodes && use_gen_input)
	{
		throw std::invalid_argument("Nodes number must be positive!");
	}
	if (!start_node_specified)
	{
		throw std::invalid_argument("Start node is required!");
	}
}

void omp_tester::process()
{
	log("Loading graph");

	if (use_gen_input)
	{
		generate_matrix(nodes);
	}
	else if (!use_gen_input)
	{
		read_matrix(input_file);
	}

	log("Threads: ", threads);
	log("Nodes: ", nodes);
	log("Start node: ", start_node);
	log("Graph: ");
	log(*data);

	if (start_node >= nodes)
	{
		throw std::invalid_argument("Start node number must be less or equal than nodes number!");
	}

	std::unique_ptr<std::vector<type_t>> prev_result;
	std::unique_ptr<std::vector<type_t>> result;

	for (auto i = 0; i < iterations; ++i)
	{
		std::cout << "[Iteration " << std::setw(std::to_string(iterations).length() - 1) << i << "] ";
		result = dijkstra_run();

		if (prev_result)
		{
			if (!std::equal(result->cbegin(), result->cend(), prev_result->cbegin()))
			{
				throw std::logic_error("Solutions are different for the same input!");
			}
		}

		prev_result.swap(result);
	}

	std::cout << "[Average] " << format_time(get_average_execution_time()) << std::endl;

	log("Distances:");
	log(*result);

	print_answer(output_file, *result);
}
