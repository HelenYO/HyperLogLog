#include <set>
#include <random>
#include <cassert>
#include <math.h>
#include <iostream>

double std_error = 0.1;


class UniqCounter {
       // no more than 32kb of memory should be used here

private:


        unsigned long long pow_2_32 = 0xFFFFFFFF + 1;

        double m, alpha_m, k;
        unsigned long long k_comp;
//        std::vector<unsigned long long> M;


        unsigned long long rank(unsigned long long hash, long long max) {
            unsigned long long r = 1;
            while((hash & 1) == 0 && r <= max) {
                ++r;
                hash = (hash >> 1); //todo:: check
            }
            return r;
        }

    unsigned long long fnv1a(int x) {//todo:: not
        const unsigned FNV_32_PRIME = 0x01000193;
        unsigned int hval = 0x811c9dc5;
        hval ^= (unsigned int) x;
        hval *= FNV_32_PRIME;
        return hval;
    }



public:

    std::vector<unsigned long long> M;

        UniqCounter() {
            m = (1.04 / std_error)*250;//todo::
            k = ceil(log2(m * m)), k_comp = 32 - static_cast<unsigned long long>(k);
            m = pow(2, k);

            alpha_m = m == 16 ? 0.673
                                  : m == 32 ? 0.697
                                            : m == 64 ? 0.709
                                                      : 0.7213 / (1 + 1.079 / m);


            M.resize(static_cast<unsigned long long>(m), 0);

            //std::cout << m << "\n";
        }

        void add(int x) {
            unsigned long long j = fnv1a(x) >> k_comp;
            //std::cout << j << "\n";
            M[j] = std::max(M[j], rank(fnv1a(x), k_comp));
                // add some code
        }

        int get_uniq_num() const {

            double c = 0.0;
            for (unsigned long long i = 0; i < m; ++i) {
                c += 1 / pow(2, M[i]);
            }
            double E = alpha_m * m * m / c;

            // -- make corrections

            if (E <= 2.5 * m) {
                unsigned long long V = 0;
                for (unsigned long long i = 0; i < m; ++i) {
                    if (M[i] == 0) {
                        ++V;
                    }
                }
                if (V > 0) {
                    E = m * log(m / V);
                }
            } else if (E > 1/30 * pow_2_32) {
                E = -pow_2_32 * log(1 - E / pow_2_32);
            }

            // --

            return E;
        }
};

double relative_error(int expected, int got) {
        return abs(got - expected) / (double) expected;
}

int main() {
        std::random_device rd;
        std::mt19937 gen(rd());

        const int N = (int) 1e6;
        for (int k : {1, 10, 1000, 10000, N / 10, N, N * 10}) {
                std::uniform_int_distribution<> dis(1, k);
                std::set<int> all;
                UniqCounter counter;
                for (int i = 0; i < N; i++) {
                        int value = dis(gen);
                        all.insert(value);
                        counter.add(value);
                }
                int expected = (int) all.size();
                int counter_result = counter.get_uniq_num();
                double error = relative_error(expected, counter_result);
                printf("%d numbers in range [1 .. %d], %d uniq, %d result, %.5f relative error\n", N, k, expected, counter_result, error);
                assert(error <= std_error);
        }

        return 0;
}