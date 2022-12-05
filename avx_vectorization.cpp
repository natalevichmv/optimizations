#include <iostream>
#include <chrono>
#include <algorithm>
#include <random>
#include <immintrin.h>

float func_initial(const float* a, const float* b, int n) {
    float result = 0.0f;
    for (int i = 0; i < n; ++i) {
        result += b[i] - a[i];
    }
    return result;
}

float func2(const float* a, const float* b, int n) {
    float result = 0.0f;
    for (int j = 0; j < n % 8; ++j) {
        result += b[j] - a[j];
    }

    const float zeros[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    __m256 sums = _mm256_loadu_ps(zeros);
    int i = n % 8;
    for (; i < n; i += 8) {
        __m256 x = _mm256_loadu_ps(b + i);
        __m256 y = _mm256_loadu_ps(a + i);
        __m256 prod = _mm256_sub_ps(x, y);
        sums = _mm256_add_ps(sums, prod);
    }
    float columns[8];
    _mm256_storeu_ps(columns, sums);
    for (int j = 0; j < 8; ++j) {
        result += columns[j];
    }
    return result;
}

void Gen(float* a, float* b, int n, std::mt19937& gen, 
         std::normal_distribution<float>& dist) {
    for (int i = 0; i < n; ++i) {
        a[i] = dist(gen);
        b[i] = dist(gen);
    }
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        return 1;
    }
    int n = std::stoi(argv[1]);
    float* a = new float[n];
    float* b = new float[n];

    std::mt19937 gen(42);
    std::normal_distribution<float> dist(0.0f, 1.0f);
    printf("Warm-up\n");
    for (int i = 0; i < 10; ++i) {
        Gen(a, b, n, gen, dist);
        printf("Iter #%d: %.4f\n", i, func2(a, b, n));
        //printf("Iter #%d: %.4f\n", i, func_initial(a, b, n));
    }

    printf("Benchmark\n");
    int64_t total_time = 0;
    for (int i = 10; i < 110; ++i) {
        Gen(a, b, n, gen, dist);
        auto start = std::chrono::high_resolution_clock::now();
        //float result = func2(a, b, n);
        float result = func_initial(a, b, n);
        auto finish = std::chrono::high_resolution_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        total_time += elapsed.count();
        printf("Iter #%d: %.4f\n", i, result);
    }
    
    printf("Avg time per function launch: %f microseconds\n", total_time / 100.0);


    delete[] a;
    delete[] b;
    return 0;
}