#include <tuple>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include "omp_tester.h"

const std::string omp_tester::DEFAULT_OUTPUT_FILE_NAME = "output.txt";
const std::string omp_tester::RUN_ALL_MULTIPLIERS_ARG = "--all";
const std::string omp_tester::USE_GENERATED_MATRICES_ARG = "-g";
const std::string omp_tester::OUTPUT_FILE_ARG = "-o";
const std::string omp_tester::ITERATIONS_NUMBER_ARG = "-c";

std::shared_ptr<matrix> omp_tester::generate_matrix(const size_t &m, const size_t &n) {
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> dist(-10, 10);
    auto result = std::make_shared<matrix>(m, m_vector(n));

    for (auto i = 0; i < m; ++i) {
        for (auto j = 0; j < n; ++j) {
            (*result)[i][j] = dist(gen);
        }
    }

    return result;
}

std::shared_ptr<matrix> omp_tester::read_matrix(const std::string &file_path) {
    std::ifstream input_file(file_path);
    size_t m, n;

    if (!input_file) {
        throw std::runtime_error("Input file does not exist or not ready to read: " + file_path);
    }

    input_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
    input_file >> m >> n;

    auto result = std::make_shared<matrix>(m, m_vector(n));

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            if (!input_file.eof()) {
                input_file >> (*result)[i][j];
            } else {
                std::stringstream ss;
                ss << "Input file " << file_path << " contains less than "
                   << m << "x" << n << " elements!";
                throw std::range_error(ss.str());
            }
        }
    }

    return result;
}

void omp_tester::print_matrix(const std::string &file_path, const std::shared_ptr<matrix> &result) {
    std::ofstream out_file(file_path, std::ofstream::trunc);

    if (!out_file) {
        throw std::runtime_error("Output file is not ready to write: " + file_path);
    }

    out_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);

    const auto m = result->size();
    const auto n = (m == 0) ? 0 : (*result)[0].size();

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            out_file << (*result)[i][j] << " ";
        }
        out_file << std::endl;
    }

    return;
}

dimensions omp_tester::check_range(const std::shared_ptr<matrix> &a, const std::shared_ptr<matrix> &b) {
    if (a->empty() || b->empty()) {
        throw std::invalid_argument("Can't multiply matrix with zero rows!");
    }

    const auto m = a->size();
    const auto n1 = (*a)[0].size();
    const auto m1 = b->size();
    const auto n = (*b)[0].size();

    if (n1 != m1) {
        std::stringstream ss;
        ss << "Can't multiply matrices with not arranged rows and columns number: "
           << "a[" << m << "][" << n1 << "] and  "
           << "b[" << m1 << "][" << n << "]";
        throw std::invalid_argument(ss.str());
    }

    return dimensions(m, n, n1);
}

void omp_tester::no_mp_multiplier(
        const std::shared_ptr<matrix> a,
        const std::shared_ptr<matrix> b,
        const std::shared_ptr<matrix> result,
        const size_t m,
        const size_t n,
        const size_t c) {
    //@formatter:off
    {
        for (size_t r = 0; r < m * n * c; ++r) {
            const auto i = r / n / c;
            const auto j = r / c % n;
            const auto k = r % c;

                (*result)[i][j] += (*a)[i][k] * (*b)[k][j];
        }
    }
    //@formatter:on
}

void omp_tester::mp_static_multiplier(
        const std::shared_ptr<matrix> a,
        const std::shared_ptr<matrix> b,
        const std::shared_ptr<matrix> result,
        const size_t m,
        const size_t n,
        const size_t c) {
    //@formatter:off
    #pragma omp parallel
    {
        #pragma omp for ordered schedule(static)
        for (auto r = 0; r < m * n * c; ++r) {
            const auto i = r / n / c;
            const auto j = r / c % n;
            const auto k = r % c;
            (*result)[i][j] += (*a)[i][k] * (*b)[k][j];
        }
    }
    //@formatter:on
}

void omp_tester::mp_dynamic_multiplier(
        const std::shared_ptr<matrix> a,
        const std::shared_ptr<matrix> b,
        const std::shared_ptr<matrix> result,
        const size_t m,
        const size_t n,
        const size_t c) {
    //@formatter:off
    #pragma omp parallel
    {
        #pragma omp for ordered schedule(dynamic)
        for (auto r = 0; r < m * n * c; ++r) {
            const auto i = r / n / c;
            const auto j = r / c % n;
            const auto k = r % c;
            (*result)[i][j] += (*a)[i][k] * (*b)[k][j];
        }
    }
    //@formatter:on
}

void omp_tester::mp_guided_multiplier(
        const std::shared_ptr<matrix> a,
        const std::shared_ptr<matrix> b,
        const std::shared_ptr<matrix> result,
        const size_t m,
        const size_t n,
        const size_t c) {
    //@formatter:off
    #pragma omp parallel
    {
        #pragma omp for ordered schedule(guided)
        for (auto r = 0; r < m * n * c; ++r) {
            const auto i = r / n / c;
            const auto j = r / c % n;
            const auto k = r % c;
            (*result)[i][j] += (*a)[i][k] * (*b)[k][j];
        }
    }
    //@formatter:on
}

std::shared_ptr<matrix> omp_tester::perform_timed_calculation(
        void (*multiplier)(const std::shared_ptr<matrix>,
                           const std::shared_ptr<matrix>,
                           const std::shared_ptr<matrix>,
                           const size_t,
                           const size_t,
                           const size_t),
        const std::shared_ptr<matrix> a,
        const std::shared_ptr<matrix> b) {
    const dimensions dimensions = check_range(a, b);
    const auto m = std::get<0>(dimensions);
    const auto n = std::get<1>(dimensions);
    const auto c = std::get<2>(dimensions);
    const auto result = std::make_shared<matrix>(m, m_vector(n));

    auto before = m_clock::now();
    multiplier(a, b, result, m, n, c);
    auto after = m_clock::now();

    auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();
    const auto seconds = time / 1000000000;
    time %= 1000000000;
    const auto milliseconds = time / 1000000;
    time %= 1000000;
    const auto microseconds = time / 1000;
    time %= 1000;
    const auto nanoseconds = time;

    std::cout << "time taken for matrices "
         << "a[" << m << "][" << c << "] "
         << "b[" << c << "][" << n << "] : "
         << seconds << "s "
         << milliseconds << "ms "
         << microseconds << "mcs "
         << nanoseconds << "ns" << std::endl;

    return result;
}

void omp_tester::check_arguments_available(const int total, const int current, const int required) {
    if (current + required >= total) {
        throw std::invalid_argument("Not enough arguments to resolve argument! " + get_help());
    }
}

std::string omp_tester::get_help() {
    std::stringstream ss;
    ss << "Usage: Lab01 "
       << "[" << OUTPUT_FILE_ARG << " output_path] "
       << "[" << USE_GENERATED_MATRICES_ARG << " m1_rows m1_columns m2_rows m2_columns] "
       << "[" << ITERATIONS_NUMBER_ARG << " iterations_number] "
       << "[" << RUN_ALL_MULTIPLIERS_ARG << "] "
       << "input_file_1 input_file_2";
    return ss.str();
}

omp_tester::omp_tester(const int argc, const char *const argv[]) {
    for (auto i = 1; i < argc; ++i) {
        auto current = std::string(argv[i]);
        if (current == OUTPUT_FILE_ARG) {
            check_arguments_available(argc, i, 1);
            output_file = std::string(argv[i + 1]);
            i += 1;
        } else if (current == USE_GENERATED_MATRICES_ARG) {
            check_arguments_available(argc, i, 4);
            try {
                matrix_1_rows = stoull(std::string(argv[i + 1]));
                matrix_1_columns = stoull(std::string(argv[i + 2]));
                matrix_2_rows = stoull(std::string(argv[i + 3]));
                matrix_2_columns = stoull(std::string(argv[i + 4]));
                use_gen_input = true;
                i += 4;
            } catch (std::invalid_argument const &e) {
                throw std::invalid_argument("Non-integer parameter passed as matrices dimension!");
            } catch (std::out_of_range const &e) {
                throw std::invalid_argument("Too large value passed as matrices dimension!");
            }
        } else if (current == omp_tester::ITERATIONS_NUMBER_ARG) {
            check_arguments_available(argc, i, 1);
            try {
                iterations = stoi(std::string(argv[i + 1]));
                i += 1;
            } catch (std::invalid_argument const &e) {
                throw std::invalid_argument("Non-integer parameter passed as iterations number!");
            } catch (std::out_of_range const &e) {
                throw std::invalid_argument("Too large value passed as iterations number!");
            }
        } else if (current == RUN_ALL_MULTIPLIERS_ARG) {
            check_arguments_available(argc, i, 0);
            run_all_multipliers = true;
        } else if (input_file_1 == "") {
            input_file_1 = current;
        } else if (input_file_2 == "") {
            input_file_2 = current;
        }
    }

    if (output_file.empty()) {
        output_file = DEFAULT_OUTPUT_FILE_NAME;
    }
}

void omp_tester::process() const {
    if (input_file_1.empty() || input_file_2.empty()) {
        throw std::invalid_argument(get_help());
    }

    std::shared_ptr<matrix> matrix_1;
    std::shared_ptr<matrix> matrix_2;

    if (use_gen_input) {
        matrix_1 = generate_matrix(matrix_1_rows, matrix_1_columns);
        matrix_2 = generate_matrix(matrix_2_rows, matrix_2_columns);
    } else if (!use_gen_input) {
        matrix_1 = read_matrix(input_file_1);
        matrix_2 = read_matrix(input_file_2);
    }

    std::shared_ptr<matrix> result;
    if (run_all_multipliers) {
        std::cout << "No OpenMP configuration:" << std::endl;
        for (auto i = 0; i < iterations; ++i) {
            result = perform_timed_calculation(no_mp_multiplier, matrix_1, matrix_2);
        }
        std::cout << "Static OpenMP configuration:" << std::endl;
        for (auto i = 0; i < iterations; ++i) {
            result = perform_timed_calculation(mp_static_multiplier, matrix_1, matrix_2);
        }
        std::cout << "Dynamic OpenMP configuration:" << std::endl;
        for (auto i = 0; i < iterations; ++i) {
            result = perform_timed_calculation(mp_dynamic_multiplier, matrix_1, matrix_2);
        }
        std::cout << "Guided OpenMP configuration:" << std::endl;
        for (auto i = 0; i < iterations; ++i) {
            result = perform_timed_calculation(mp_guided_multiplier, matrix_1, matrix_2);
        }
    } else {
        std::cout << "No OpenMP configuration:" << std::endl;
        result = perform_timed_calculation(no_mp_multiplier, matrix_1, matrix_2);
    }
    print_matrix(output_file, result);
}
