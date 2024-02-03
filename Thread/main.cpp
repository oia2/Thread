#include <iostream>
#include <thread>
#include <chrono>

#define NUM_THREADS 4

void sumFunction(int thread_number, uint64_t N, uint64_t* results)
{
    uint64_t sum = 0;
    uint64_t count = N / NUM_THREADS;
    for (uint64_t i = count * thread_number + 1; i <= count * (thread_number + 1); i++) {
        sum += i;
    }
    results[thread_number] = sum;
}

int main()
{
    std::thread threads[NUM_THREADS];
    uint64_t results[NUM_THREADS];
    uint64_t sum1 = 0, N = 10000000000;

    std::cout << "Threads:" << std::endl;
    auto start = std::chrono::system_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(sumFunction, i, N, results);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
        std::cout << "Thread " << i << " end calculation " << std::endl;
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        sum1 += results[i];
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Sum = " << sum1 << std::endl;
    std::cout << "Time to calculate " << diff.count() << " s" << std::endl;

    uint64_t sum2 = 0;
    std::cout << "One thread:" << std::endl;
    start = std::chrono::system_clock::now();

    for (uint64_t i = 1; i <= N; i++) {
        sum2 += i;
    }

    end = std::chrono::system_clock::now();
    diff = end - start;
    std::cout << "Sum = " << sum2 << std::endl;
    std::cout << "Time to calculate " << diff.count() << " s" << std::endl;

    if (sum1 == sum2) {
        std::cout << "The result is correct" << std::endl;
    }
    else {
        std::cout << "The result is incorrect" << std::endl;
    }
}
