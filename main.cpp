#include <set>
#include <random>
#include <cassert>
#include <math.h>
#include <iostream>

double std_error = 0.1;

class UniqCounter {

private:

    double  k_norm,  m;
    unsigned int max_k, k;
    std::vector<unsigned int> buckets;

    unsigned int rank(unsigned int hash, unsigned int max) {
        unsigned int r = 1;
        while ((hash & 1) == 0 && r <= max) {
            ++r;
            hash = (hash >> 1);
        }
        return r;
    }

    unsigned int make_hash(int x) {
        const unsigned salt = 27644437;
        unsigned int temp = 2166136261;
        temp ^= (unsigned int) x;
        temp *= salt;
        return temp;
    }

public:

    UniqCounter() {
        double temp = (1.04 / std_error) * 170;
        k = static_cast<unsigned int>(ceil(log2(temp * temp)));
        max_k = 32 - k;
        m = pow(2, k);
        k_norm = 0.7213 / (1 + 1.079 / m);
        buckets.resize(static_cast<unsigned int>(m), 0);
    }

    void add(int x) {
        unsigned int hash = make_hash(x);
        unsigned int j = hash >> max_k;
        buckets[j] = std::max(buckets[j], rank(hash, max_k));
    }

    int get_uniq_num() const {
        double c = 0;
        for (size_t i = 0; i < m; ++i) {
            c += 1 / pow(2, buckets[i]);
        }
        double E = k_norm * m * m / c;
        unsigned int V = 0;
        for (size_t i = 0; i < m; ++i) {
            if (buckets[i] == 0) {
                ++V;
            }
        }
        if (V > 0) {
            E = m * log(m / V);
        }
        return static_cast<int>(E);
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
        printf("%d numbers in range [1 .. %d], %d uniq, %d result, %.5f relative error\n", N, k, expected,
               counter_result, error);
        assert(error <= std_error);
    }
    return 0;
}