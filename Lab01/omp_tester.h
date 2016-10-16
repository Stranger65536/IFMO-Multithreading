#ifndef LAB01_OMP_TESTER_H
#define LAB01_OMP_TESTER_H

#include <memory>
#include <vector>
#include <chrono>

typedef std::tuple<size_t, size_t, size_t> dimensions;
typedef std::vector<int64_t> m_vector;
typedef std::vector<m_vector> matrix;
typedef std::chrono::high_resolution_clock m_clock;

class omp_tester {
    static const std::string DEFAULT_OUTPUT_FILE_NAME;
    static const std::string RUN_ALL_MULTIPLIERS_ARG;
    static const std::string USE_GENERATED_MATRICES_ARG;
    static const std::string OUTPUT_FILE_ARG;
    static const std::string ITERATIONS_NUMBER_ARG;

    std::string input_file_1 = "";
    std::string input_file_2 = "";
    std::string output_file = "";
    bool run_all_multipliers = false;
    bool use_gen_input = false;
    size_t matrix_1_rows = 0;
    size_t matrix_1_columns = 0;
    size_t matrix_2_rows = 0;
    size_t matrix_2_columns = 0;
    int iterations = 1;

    static std::shared_ptr<matrix> generate_matrix(const size_t &m, const size_t &n);

    static std::shared_ptr<matrix> read_matrix(const std::string &file_path);

    static void print_matrix(const std::string &file_path, const std::shared_ptr<matrix> &result);

    static dimensions check_range(const std::shared_ptr<matrix> &a, const std::shared_ptr<matrix> &b);

    static std::shared_ptr<matrix> perform_timed_calculation(
            void (*multiplier)(const std::shared_ptr<matrix>,
                               const std::shared_ptr<matrix>,
                               const std::shared_ptr<matrix>,
                               const size_t,
                               const size_t,
                               const size_t),
            const std::shared_ptr<matrix> a,
            const std::shared_ptr<matrix> b);

    //@formatter:off
    static void no_mp_multiplier(const std::shared_ptr<matrix> a, const std::shared_ptr<matrix> b, const std::shared_ptr<matrix> result, const size_t m, const size_t n, const size_t c);
    static void mp_static_multiplier(const std::shared_ptr<matrix> a, const std::shared_ptr<matrix> b, const std::shared_ptr<matrix> result, const size_t m, const size_t n, const size_t c);
    static void mp_dynamic_multiplier(const std::shared_ptr<matrix> a, const std::shared_ptr<matrix> b, const std::shared_ptr<matrix> result, const size_t m, const size_t n, const size_t c);
    static void mp_guided_multiplier(const std::shared_ptr<matrix> a, const std::shared_ptr<matrix> b, const std::shared_ptr<matrix> result, const size_t m, const size_t n, const size_t c);
    //@formatter:on

    static void check_arguments_available(const int total, const int current, const int required);

    static std::string get_help();

public:
    omp_tester(const int argc, const char *const argv[]);

    void process() const;
};

#endif //LAB01_OMP_TESTER_H
