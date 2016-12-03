#ifndef LAB04_OMP_TESTER_H
#define LAB04_OMP_TESTER_H

#include <memory>
#include "matrix.h"

class omp_tester
{
	typedef int64_t type_t;
	typedef std::chrono::high_resolution_clock m_clock;

	static const std::string DEFAULT_OUTPUT_FILE_NAME;
	static const std::string USE_GENERATED_GRAPH_ARG;
	static const std::string OUTPUT_FILE_ARG;
	static const std::string ITERATIONS_NUMBER_ARG;
	static const std::string THREADS_NUMBER_ARG;
	static const std::string VERBOSE_ARG;
	static const type_t NO_PATH_VALUE;

	std::string input_file = "";
	std::string output_file = "";
	bool use_gen_input = false;
	bool verbose = false;
	size_t nodes = 0;
	bool start_node_specified = false;
	size_t start_node = 0;
	int iterations = 1;
	int threads = 1;
	std::unique_ptr<matrix<type_t>> data;
	std::vector<long long> time_accumulator;

	template <typename H, typename... T>
	void log(std::stringstream &ss, H &p, T ... t) const;
	template <typename H, typename... T>
	void log(H &p, T ... t) const;

	void check_arguments() const;
	void generate_matrix(const size_t &n);
	void read_matrix(const std::string &file_path);
	void omp_tester::log(std::stringstream &ss) const;
	int64_t get_average_execution_time() const;
	std::unique_ptr<std::vector<type_t>> dijkstra_run();
	size_t get_next_node(std::vector<bool> &visited, std::vector<type_t> &dist) const;

	static void print_answer(const std::string &file_path, const std::vector<type_t> &result);
	static void check_arguments_available(const int total, const int current, const int required);
	static size_t parse_size_t(const char *value, const char *parse_error, const char *overflow_error);
	static int parse_int(const char *value, const char *parse_error, const char *overflow_error);
	static std::string get_help();
	static std::string format_time(int64_t nanos);
public:
	omp_tester(const int argc, const char *const argv[]);
	void process();
};

#endif //LAB04_OMP_TESTER_H
