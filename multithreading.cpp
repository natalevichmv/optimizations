#include <bits/stdc++.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
using namespace std;

bool IsHappyNumber(int64_t x) {
    int prev_digit = x % 10;
    x /= 10;
    int cur_digit;
    while (x) {
        cur_digit = x % 10;
        if (abs(prev_digit - cur_digit) > 2) {
            return false;
        }
        x /= 10;
        prev_digit = cur_digit;
    }
    return true;
} 

void Worker(int id, int threads_count, int64_t n, 
            atomic<bool>* show_progress, atomic<int64_t>* progress, 
            int64_t* result, atomic<bool>* thread_finished,
            condition_variable* cv) {
    int64_t steps = 0;
    for (int64_t i = id + 1; i <= n; i += threads_count) {
        if (show_progress->load()) {
            progress->store(steps);
            show_progress->store(false);
        }
        if (IsHappyNumber(i)) {
            ++(*result);
        }
        ++steps;
    }
    progress->store(steps);
    thread_finished->store(true);
    cv->notify_one();
}

int main() {
    cout << "Available cores: " << thread::hardware_concurrency() << endl;
    cout << "Input n, worker threads count" << endl;
    int64_t n;
    cin >> n;
    int threads_count;
    cin >> threads_count;
    auto start_time = chrono::high_resolution_clock::now();

    vector<thread> threads;
    vector<atomic<bool>> show_progress(threads_count);
    vector<atomic<int64_t>> progress(threads_count);
    vector<int64_t> result(threads_count);
    vector<atomic<bool>> thread_finished(threads_count);
    condition_variable cv;
    for (int i = 0; i < threads_count; ++i) {
        threads.emplace_back(
            Worker, i, threads_count, n, &show_progress[i], 
            &progress[i], &result[i], &thread_finished[i], &cv);
    }

    auto next_ask = 
        chrono::high_resolution_clock::now() + chrono::seconds(5);
    mutex mtx;
    while (true) {
        unique_lock<mutex> lock(mtx);
        auto all_finished = cv.wait_until(lock, next_ask, [&] {                
            bool all_finished = true;
            for (int i = 0; i < threads_count; ++i) {
                if (!thread_finished[i].load()) {
                    all_finished = false;
                }
            }
            return all_finished;
        });
        if (all_finished) {
            break;
        }
    
        for (int i = 0; i < threads_count; ++i) {
            show_progress[i].store(true);
        }
        while (true) {
            int sum = 0;
            for (int i = 0; i < threads_count; ++i) {
                if (!thread_finished[i].load()) {
                    sum += show_progress[i].load();
                }
            }
            if (!sum) {
                for (int i = 0; i < threads_count; ++i) {
                    cout << "Thread #"<< i << " processed " 
                            << progress[i].load() << " numbers" << endl;
                }
                cout << endl;
                break;
            }
        }
        next_ask = chrono::high_resolution_clock::now() + chrono::seconds(5);
    }
    int64_t total_result = 0;
    for (int i = 0; i < threads_count; ++i) {
        threads[i].join();
        total_result += result[i];
    }
    auto finish_time = chrono::high_resolution_clock::now();

    cout << "There are " << total_result 
         << " happy numbers from 1 to " << n << endl;
    auto time_elapsed = 
        chrono::duration_cast<chrono::milliseconds>(finish_time - start_time);
    cout << "Time elapsed: " << time_elapsed.count() / 1000.0 << "s" << endl << endl;

    cout << "Brute force to check?" << endl;
    int brute_force;
    cin >> brute_force;
    if (brute_force) {
        int64_t sum = 0;
        for (int i = 1; i <= n; ++i) {
            sum += IsHappyNumber(i);
        }
        cout << "Brute force answer: " << sum << endl;
    }
    return 0;
}