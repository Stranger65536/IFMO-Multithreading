#ifndef LAB02_MPI_TESTER_H
#define LAB02_MPI_TESTER_H

#include "m_matrix.h"
#include <string>

class mpi_tester
{
	static const std::string DEFAULT_OUTPUT_FILE_NAME;
	static const std::string USE_GENERATED_MATRICES_ARG;
	static const std::string OUTPUT_FILE_ARG;
	static const std::string VERBOSE_ARG;
	static const int ROOT_ID;

	std::string input_file_matrix = "";
	std::string output_file = "";
	bool use_gen_input = false;
	bool verbose = false;
	size_t matrix_rows = 0;
	size_t matrix_columns = 0;
	int total_processes = 0;
	int process_id = 0;
	size_t cells_per_process;
	size_t rows_per_process;
	double precision = -1;
	size_t max_iterations = 0;
	m_matrix::matrix_t coeff_matrix;
	m_vector::vector_t right_hand_side;
	m_vector::vector_t approximation;
	m_vector::vector_t answer;
	std::shared_ptr<std::vector<int>> rows_number_distribution;
	std::shared_ptr<std::vector<int>> rows_positions_distribution;
	std::shared_ptr<std::vector<int>> cells_number_distribution;
	std::shared_ptr<std::vector<int>> cells_positions_distribution;

	static void check_arguments_available(const int total, const int current, const int required);
	static double distance(m_vector::vector_t &new_appr, m_vector::vector_t &old_appr);
	static std::string get_help();
	static double parse_double(const char *value, const char *parse_error, const char *overflow_error);
	static int parse_int(const char *value, const char *parse_error, const char *overflow_error);
	static size_t parse_size_t(const char *value, const char *parse_error, const char *overflow_error);

	void calculate_data_distribution();
	void read_matrix();
	void receive_meta_data();
	void receive_initial_data();
	void send_initial_data() const;
	void send_meta_data() const;
	void check_range() const;
	void check_arguments() const;
	void print_answer() const;
	void print_process_id() const;
	void print_data_distribution() const;
	void log(std::string &msg) const;
	void log(char *const msg) const;
	std::pair<bool, size_t> apply_jacobi() const;
	size_t get_size() const;
	std::string get_metadata() const;
public:
	mpi_tester(const int argc, const char * const argv[]);
	void init();
	void process() const;

	friend std::ostream &operator <<(std::ostream &os, const mpi_tester &obj)
	{
		return os
			<< "input_file_matrix: " << obj.input_file_matrix 
			<< " output_file: " << obj.output_file 
			<< " use_gen_input: " << obj.use_gen_input 
			<< " matrix_rows: " << obj.matrix_rows 
			<< " matrix_columns: " << obj.matrix_columns 
			<< " total_processes: " << obj.total_processes 
			<< " process_id: " << obj.process_id 
			<< " cells_per_process: " << obj.cells_per_process 
			<< " rows_per_process: " << obj.rows_per_process 
			<< " precision: " << obj.precision 
			<< " max_iterations: " << obj.max_iterations 
			<< " coeff_matrix: " << *obj.coeff_matrix
			<< " right_hand_side: " << *obj.right_hand_side
			<< " approximation: " << *obj.approximation;
	}
};

#endif //LAB02_MPI_TESTER_H
