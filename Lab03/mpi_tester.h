#ifndef LAB03_MPI_TESTER_H
#define LAB03_MPI_TESTER_H

#define mpi_type_t MPI_LONG_LONG_INT

#include <vector>
#include "random.h"

class mpi_tester
{
	typedef int64_t type_t;

	static const std::string DEFAULT_OUTPUT_FILE_NAME;
	static const std::string USE_GENERATED_VECTOR_ARG;
	static const std::string OUTPUT_FILE_ARG;
	static const std::string VERBOSE_ARG;
	static const int ROOT_ID;

	std::string input_file = "";
	std::string output_file = "";
	std::vector<type_t> data;
	std::vector<type_t> sorted_data;
	random<type_t> rnd{std::numeric_limits<type_t>::min(), std::numeric_limits<type_t>::max()};
	bool use_gen_input = false;
	bool verbose = false;
	size_t size = 0;
	int process_id = 0;
	int total_processes = 0;
	int last_start = 0;
	size_t elems_per_process = 0;
	std::vector<int> elems_distribution;
	std::vector<int> positions_distribution;
	std::vector<int> class_starts;
	std::vector<int> class_lengths;
	std::vector<type_t> pivot_buffer;
	size_t pivot_buffer_size;
	double start_time, end_time;

	static std::string get_help();
	static void fill_vector_randomly(std::vector<type_t> &v, const size_t size, random<type_t> &rnd);
	static void check_arguments_available(const int total, const int current, const int required);
	static size_t parse_size_t(const char *value, const char *parse_error, const char *overflow_error);

	template <typename H, typename... T>
	void log(std::stringstream &ss, H &p, T ... t) const;
	template <typename H, typename... T>
	void log(H &p, T ... t) const;

	bool is_root() const;
	void scatter_data();
	void local_sort();
	void collect_regular_samples();
	void gather_samples();
	void merge_samples();
	void broadcast_samples();
	void partition_local_data();
	void read_vector();
	void check_arguments() const;
	void print_answer() const;
	void print_process_id() const;
	void log(std::stringstream &ss) const;
	void calculate_data_distribution();
	void gather_and_multimerge_data();
	void collect_data();
	void multimerge(std::vector<std::vector<type_t>> &starts, std::vector<type_t> &result) const;
public:
	mpi_tester(const int argc, const char * const argv[]);
	void init();
	void process();
};

template <typename T>
struct mm_data
{
	int start_index;
	int index;
	T start_value;

	explicit mm_data(int st = 0, int id = 0, T stv = 0) : start_index(st), index(id), start_value(stv)
	{
	}

	bool operator<(const mm_data &o) const
	{
		return this->start_value > o.start_value;
	}
};


#endif //LAB03_MPI_TESTER_H
