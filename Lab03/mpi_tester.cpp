#include <mpi.h>
#include <queue>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include "mpi_tester.h"
#include <iomanip>


const std::string mpi_tester::DEFAULT_OUTPUT_FILE_NAME = "output.txt";
const std::string mpi_tester::USE_GENERATED_VECTOR_ARG = "-g";
const std::string mpi_tester::OUTPUT_FILE_ARG = "-o";
const std::string mpi_tester::VERBOSE_ARG = "-v";
const int mpi_tester::ROOT_ID = 0;

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v)
{
	for (auto i : v)
	{
		os << i << ' ';
	}
	return os;
}

void mpi_tester::print_answer() const
{
	std::ofstream out_file(this->output_file, std::ofstream::trunc);

	if (!out_file)
	{
		throw std::runtime_error("Output file does not exist or not ready to read: " + this->output_file);
	}

	out_file.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	out_file << this->sorted_data;

	out_file.close();
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
	ss << "Usage: Lab03 "
		<< "[" << OUTPUT_FILE_ARG << " output_path] "
		<< "[" << USE_GENERATED_VECTOR_ARG << " size] "
		<< "input_file";
	return ss.str();
}

void mpi_tester::log(std::stringstream &ss) const
{
	if (verbose)
	{
		std::cout << ss.str() << std::endl;
	}
}

template <typename H, typename... T>
void mpi_tester::log(std::stringstream &ss, H &p, T ... t) const
{
	if (verbose)
	{
		ss << p;
		log(ss, t...);
	}
}

template <typename H, typename... T>
void mpi_tester::log(H &p, T ... t) const
{
	if (verbose)
	{
		print_process_id();
		std::stringstream ss;
		ss << p;
		log(ss, t...);
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


void mpi_tester::read_vector()
{
	std::ifstream input(this->input_file);

	if (!input)
	{
		throw std::invalid_argument("Can't read from file " + input_file + "!");
	}

	copy(std::istream_iterator<type_t>(input), std::istream_iterator<type_t>(), back_inserter(this->data));
	this->size = this->data.size();
}

void mpi_tester::fill_vector_randomly(std::vector<type_t> &v, const size_t size, random<type_t> &rnd)
{
	v.reserve(size);

	for (auto i = 0; i < size; ++i)
	{
		v.push_back(rnd.next());
	}
}

void mpi_tester::calculate_data_distribution()
{
	auto div = std::div(static_cast<int>(this->size), this->total_processes);
	this->elems_per_process = (div.rem ? div.quot + 1 : div.quot);

	auto row = 0;
	for (auto i = 0; i < this->total_processes; ++i)
	{
		auto left = this->size - row;
		auto current = static_cast<int>(std::min(left, this->elems_per_process));
		this->elems_distribution.push_back(current);
		this->positions_distribution.push_back(row);
		row += current;
	}

	if (this->process_id != this->ROOT_ID)
	{
		this->data.assign(this->size, 0);
	}

	this->pivot_buffer.assign(this->total_processes * this->total_processes, 0);
	this->class_starts.assign(this->total_processes, 0);
	this->class_lengths.assign(this->total_processes, 0);
	this->sorted_data.assign(this->size, 0);

	log("Elems to process: ", this->elems_distribution[this->process_id]);
	log("Elems position: ", this->positions_distribution[this->process_id]);
}

void mpi_tester::init()
{
	MPI_Comm_size(MPI_COMM_WORLD, &this->total_processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &this->process_id);

	if (this->process_id == this->ROOT_ID)
	{
		if (this->use_gen_input)
		{
			log("Generating vector");
			this->fill_vector_randomly(this->data, this->size, this->rnd);
			log(this->data);
		}
		else
		{
			log("Reading vector");
			this->read_vector();
			log(this->data);
		}

		this->size = this->data.size();
	}

	this->start_time = MPI_Wtime();

	MPI_Bcast(&this->size, 1, MPI_UNSIGNED_LONG_LONG, this->ROOT_ID, MPI_COMM_WORLD);
	log("Total data size: ", this->size);

	calculate_data_distribution();
}

mpi_tester::mpi_tester(const int argc, const char *const argv[])
{
	for (auto i = 1; i < argc; ++i)
	{
		auto current = std::string(argv[i]);
		if (current == this->OUTPUT_FILE_ARG)
		{
			check_arguments_available(argc, i, 1);
			this->output_file = std::string(argv[++i]);
		}
		else if (current == this->USE_GENERATED_VECTOR_ARG)
		{
			check_arguments_available(argc, i, 1);
			this->size = parse_size_t(argv[++i],
			                          "Non-integer parameter passed as array size! ",
			                          "Too large value passed as array size! ");
			this->use_gen_input = true;
		}
		else if (current == this->VERBOSE_ARG)
		{
			this->verbose = true;
		}
		else if (this->input_file == "")
		{
			this->input_file = current;
		}
	}

	if (this->output_file.empty())
	{
		this->output_file = this->DEFAULT_OUTPUT_FILE_NAME;
	}

	check_arguments();
}

void mpi_tester::check_arguments() const
{
	if (this->input_file == "")
	{
		throw std::invalid_argument("File with vector to sort has not been passed!");
	}
}

void mpi_tester::print_process_id() const
{
	if (this->verbose)
	{
		std::cout << "[" << this->process_id << "] ";
	}
}

bool mpi_tester::is_root() const
{
	return this->process_id == this->ROOT_ID;
}

void mpi_tester::scatter_data()
{
	log("Splitting data");

	if (is_root())
	{
		MPI_Scatterv(this->data.data(), this->elems_distribution.data(), this->positions_distribution.data(), mpi_type_t,
		             MPI_IN_PLACE, this->elems_distribution[this->process_id], mpi_type_t,
		             this->ROOT_ID, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Scatterv(nullptr, this->elems_distribution.data(), this->positions_distribution.data(), mpi_type_t,
		             const_cast<long long*>(this->data.data()), this->elems_distribution[this->process_id], mpi_type_t,
		             this->ROOT_ID, MPI_COMM_WORLD);
	}

	log("Local data: ", this->data);
}

void mpi_tester::local_sort()
{
	log("Soring local parts");
	qsort(this->data.data(), this->elems_distribution[this->process_id], sizeof(type_t), [](const void *a, const void *b)
	      {
		      auto arg1 = *static_cast<const type_t*>(a);
		      auto arg2 = *static_cast<const type_t*>(b);

		      if (arg1 < arg2) return -1;
		      if (arg1 > arg2) return 1;
		      return 0;
	      });
}

void mpi_tester::collect_regular_samples()
{
	log("Calculating pivots");

	for (auto i = 0; i < this->total_processes; ++i)
	{
		this->pivot_buffer[i] = this->data[i * this->elems_distribution[this->process_id] / this->total_processes];
	}

	log("Pivots: ", this->pivot_buffer);
}

void mpi_tester::gather_samples()
{
	log("Gathering pivots");

	if (is_root())
	{
		MPI_Gather(MPI_IN_PLACE, this->total_processes, mpi_type_t,
		                       this->pivot_buffer.data(), this->total_processes, mpi_type_t,
		                       this->ROOT_ID, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Gather(this->pivot_buffer.data(), this->total_processes, mpi_type_t,
		           nullptr, this->total_processes, mpi_type_t,
		           this->ROOT_ID, MPI_COMM_WORLD);
	}

	log("Pivots after gather: ", this->pivot_buffer);
}

void mpi_tester::merge_samples()
{
	log("Multimerge samples");

	if (is_root())
	{
		std::vector<std::vector<type_t>> samples(this->total_processes);

		for (auto i = 0; i < this->total_processes; ++i)
		{
			samples[i] = std::vector<type_t>(&pivot_buffer[i * this->total_processes],
			                                 &pivot_buffer[i * this->total_processes] + this->total_processes);
		}

		std::vector<type_t> result(this->total_processes * this->total_processes, 0);
		multimerge(samples, result);

		for (auto i = 0; i < this->total_processes - 1; ++i)
		{
			this->pivot_buffer[i] = result[(i + 1) * this->total_processes];
		}
	}
}

void mpi_tester::broadcast_samples()
{
	log("Broadcasting pivots");

	MPI_Bcast(this->pivot_buffer.data(), this->total_processes - 1, mpi_type_t, this->ROOT_ID, MPI_COMM_WORLD);

	log("Pivots after multimerge: ", this->pivot_buffer);
}

void mpi_tester::partition_local_data()
{
	// All processes partition their data members based on the pivot values stored in pivotbuffer[]
	// Partition information for data: 
	// index of beginning of ith class is class_starts[i],
	// length of ith class is class_lengths[i], and
	// members of ith class, data[j], have the property pivot_buffer[i-1] <= data[j] < pivot_buffer[i]

	log("Partitioning local data");

	auto dataindex = 0;
	for (auto classindex = 0; classindex < this->total_processes - 1; ++classindex)
	{
		this->class_starts[classindex] = dataindex;
		this->class_lengths[classindex] = 0;

		// as long as dataindex refers to data in the current class
		while ((dataindex < this->elems_distribution[this->process_id])
			&& (this->data[dataindex] <= pivot_buffer[classindex]))
		{
			this->class_lengths[classindex]++;
			dataindex++;
		}
	}
	// set start and length for last class
	this->class_starts[this->total_processes - 1] = dataindex;
	this->class_lengths[this->total_processes - 1] = this->elems_distribution[this->process_id] - dataindex;
}

void mpi_tester::gather_and_multimerge_data()
{
	log("Gathering classes between processed");
	// All ith classes are gathered by ith process 
	std::vector<type_t> recv_buffer(this->size, 0); // buffer to hold all members of ith class 
	std::vector<int> recv_counts(this->total_processes, 0);
	std::vector<int> displs(this->total_processes, 0);

	for (auto process = 0; process < this->total_processes; ++process)
	{
		MPI_Gather(&this->class_lengths[process], 1, MPI_INT,
		           recv_counts.data(), 1, MPI_INT,
		           process, MPI_COMM_WORLD);

		if (this->process_id == process)
		{
			displs[0] = 0;
			for (auto i = 1; i < this->total_processes; ++i)
			{
				displs[i] = displs[i - 1] + recv_counts[i - 1];
			}
		}

		// each process gathers up all the members of the ith process classes from the other nodes
		MPI_Gatherv(&this->data[class_starts[process]], class_lengths[process], mpi_type_t,
		            recv_buffer.data(), recv_counts.data(), displs.data(), mpi_type_t,
		            process, MPI_COMM_WORLD);
	}

	log("Local classes after gathering: ", recv_buffer);
	log("Multimerge classes");

	this->last_start = displs[this->total_processes - 1] + recv_counts[this->total_processes - 1];

	std::vector<std::vector<type_t>> starts(this->total_processes);
	for (auto i = 0; i < this->total_processes; i++)
	{
		starts[i] = std::vector<type_t>(recv_buffer.data() + displs[i], recv_buffer.data() + displs[i] + recv_counts[i]);
	}

	multimerge(starts, this->data);

	log("Local data after multimerge: ", this->data);
}

void mpi_tester::collect_data()
{
	log("Collecting data");

	std::vector<int> recv_count(this->total_processes);
	std::vector<int> displs(this->total_processes);

	MPI_Gather(&this->last_start, 1, MPI_INT,
	           recv_count.data(), 1, MPI_INT,
	           this->ROOT_ID, MPI_COMM_WORLD);

	if (is_root())
	{
		displs[0] = 0;
		for (auto i = 1; i < this->total_processes; ++i)
		{
			displs[i] = displs[i - 1] + recv_count[i - 1];
		}
	}

	MPI_Gatherv(this->data.data(), this->last_start, mpi_type_t,
	            this->sorted_data.data(), recv_count.data(), displs.data(), mpi_type_t,
	            this->ROOT_ID, MPI_COMM_WORLD);

	if (is_root())
	{
		log("Sorted data: ", sorted_data);
	}
}

void mpi_tester::process()
{
	scatter_data();
	local_sort();
	collect_regular_samples();
	gather_samples();
	merge_samples();
	broadcast_samples();
	partition_local_data();
	gather_and_multimerge_data();
	collect_data();

	this->end_time = MPI_Wtime();

	if (is_root())
	{
		auto duration = this->end_time - this->start_time;
		double seconds, millis, micros, fract;
		fract = 1000 * std::modf(duration, &seconds);
		fract = 1000 * std::modf(fract, &millis);
		std::modf(fract, &micros);
		std::cout << "Time taken: " 
			<< seconds << "s "
			<< millis << "ms "
			<< micros << "mcs "
			<< std::endl;
		print_answer();
	}
}

void mpi_tester::multimerge(std::vector<std::vector<type_t>> &starts, std::vector<type_t> &result) const
{
	std::priority_queue<mm_data<type_t>> priorities;

	for (auto i = 0; i < this->total_processes; ++i)
	{
		if (starts[i].size() > 0)
		{
			priorities.push(mm_data<type_t>(i, 0, starts[i][0]));
		}
	}

	// As long as priorities is not empty, pull off the top member (the smallest value from list i), 
	// push it into the result, and place the next element from list i in the priority queue
	auto result_index = 0; // index into the merged array
	while (!priorities.empty() && (result_index < result.size()))
	{
		// grab the smallest element, and remove it from the priority queue
		auto elem = priorities.top();
		priorities.pop();

		// insert this smallest element into the merged array
		result[result_index++] = starts[elem.start_index][elem.index];

		// if start[xxx.start_index] is not empty, place the next member into priority
		if (starts[elem.start_index].size() > (elem.index + 1))
		{
			priorities.push(mm_data<type_t>(elem.start_index, elem.index + 1, starts[elem.start_index][elem.index + 1]));
		}
	}
}
