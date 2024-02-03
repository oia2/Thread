#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <immintrin.h> // библиотека для SIMD-инструкций

using namespace std;
#define NUM_THREADS 4

void print_matrix(__int64** matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            cout << matrix[i][j] << " ";
        }
        cout << endl;
    }
}

void matrixThread(int thread_number, int size, __int64** Am, __int64** Bm, __int64** Rt)
{
    int count = size / NUM_THREADS;
    for (int i = count * thread_number; i < count * thread_number + count; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < size; k++) {
                Rt[i][j] += Am[i][k] * Bm[k][j];
            }
        }
    }
}

void matrixThreadVector(int thread_number, int size, __int64** Am, __int64** Bm, __int64** BT, __int64** Rtv)
{
    int count = size / NUM_THREADS;

    __m128i mult;
    __m128i sum;
    __m128i vector_a_1;
    __m128i vector_b_1;

    for (int i = count * thread_number; i < count * thread_number + count; i++)
    {
        for (int j = 0; j < size; j++)
        {
            mult = _mm_setzero_si128();
            sum = _mm_setzero_si128();
            for (int k = 0; k < size; k += 2)
            {
                vector_a_1 = _mm_load_si128((__m128i*) & Am[i][k]);
                vector_b_1 = _mm_load_si128((__m128i*) & BT[j][k]);
                mult = _mm_mul_epi32(vector_a_1, vector_b_1);
                sum = _mm_add_epi64(mult, sum);
            }
            Rtv[i][j] = sum.m128i_i64[1] + sum.m128i_i64[0];
        }
    }
}


int main() {
    int size = 4096;

    __int64** Am = new __int64* [size];
    __int64** Bm = new __int64* [size];
    __int64** Rs = new __int64* [size];
    __int64** Rv = new __int64* [size];
    __int64** Rt = new __int64* [size];
    __int64** Rtv = new __int64* [size];


    for (int i = 0; i < size; i++) {
        Am[i] = new __int64[size];
        Bm[i] = new __int64[size];
        Rs[i] = new __int64[size];
        Rv[i] = new __int64[size];
        Rt[i] = new __int64[size];
        Rtv[i] = new __int64[size];
        for (int j = 0; j < size; j++) {
            Am[i][j] = rand() % 10;
            Bm[i][j] = rand() % 10;
            Rs[i][j] = 0;
            Rv[i][j] = 0;
            Rt[i][j] = 0;
            Rtv[i][j] = 0;
        }
    }

    cout << "Scalar: \n";
    auto start = chrono::system_clock::now();

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                Rs[i][j] += Am[i][k] * Bm[k][j];
            }
        }
    }
    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;
    cout << "Time to calculate " << diff.count() << " s" << endl;

    __int64** BT = new __int64* [size];
    for (int i = 0; i < size; i++) {
        BT[i] = new __int64[size];
        for (int j = 0; j < size; j++) {
            BT[i][j] = Bm[j][i];
        }
    }

    cout << "Vector: \n";
    start = chrono::system_clock::now();

    __m128i mult;
    __m128i sum;
    __m128i vector_a_1;
    __m128i vector_b_1;
    for (int i = 0; i < size; i++)
    {

        for (int j = 0; j < size; j++)
        {
            mult = _mm_setzero_si128();
            sum = _mm_setzero_si128();
            for (int k = 0; k < size; k += 2)
            {
                vector_a_1 = _mm_load_si128((__m128i*) & Am[i][k]);
                vector_b_1 = _mm_load_si128((__m128i*) & BT[j][k]);
                mult = _mm_mul_epi32(vector_a_1, vector_b_1);
                sum = _mm_add_epi64(mult, sum);
            }
            Rv[i][j] = sum.m128i_i64[1] + sum.m128i_i64[0];
        }
    }

    end = chrono::system_clock::now();
    diff = end - start;
    cout << "Time to calculate " << diff.count() << " s" << endl;

    cout << "Threads: \n";

    thread threads[NUM_THREADS];
    start = chrono::system_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = thread(matrixThread, i, size, Am, Bm, Rt);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
        std::cout << "Thread " << i << " end calculation " << std::endl;
    }

    end = chrono::system_clock::now();
    diff = end - start;
    cout << "Time to calculate " << diff.count() << " s" << endl;

    cout << "ThreadVector: \n";

    start = chrono::system_clock::now();

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = thread(matrixThreadVector, i, size, Am, Bm, BT, Rtv);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
        cout << "Thread " << i << " end calculation " << endl;
    }

    end = chrono::system_clock::now();
    diff = end - start;
    cout << "Time to calculate " << diff.count() << " s" << endl;


    bool equal = true;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (Rs[i][j] != Rv[i][j] || Rs[i][j] != Rt[i][j] || Rs[i][j] != Rtv[i][j]) {
                equal = false;
            }
        }
    }
    if (equal) {
        cout << "Matrix is equal\n";
    }
    else {
        cout << "Matrix not equal\n";
    }

    for (int i = 0; i < size; i++) {
        delete[] Am[i];
        delete[] Bm[i];
        delete[] BT[i];
        delete[] Rs[i];
        delete[] Rv[i];
        delete[] Rt[i];
        delete[] Rtv[i];
    }

    delete[] Am;
    delete[] Bm;
    delete[] BT;
    delete[] Rs;
    delete[] Rv;
    delete[] Rt;
    delete[] Rtv;

    return 0;
}
