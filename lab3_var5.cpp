#include <iostream>
#include <thread>
#include <chrono>
#include <syncstream>
#include <vector>
#include <latch>

void f(char name, int i) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::osyncstream(std::cout) << "From set " << name << " action " << i << " completed.\n";
}

void run(char name, int from, int to) {
    for (int i = from; i <= to; ++i) f(name, i);
}

int main() {
    std::osyncstream(std::cout) << "Computation started.\n";
    auto start_time = std::chrono::steady_clock::now();

    std::latch a_done(4), b_done(5), e_done(4), c_done(5), d_done(3);
    std::latch g_done(2), h_done(2), i_done(1), j_done(2), f_done(1);

    std::vector<std::jthread> threads;
    threads.reserve(5);

    threads.emplace_back([&]() {
        run('a', 1, 1); a_done.count_down();
        run('b', 2, 2); b_done.count_down();
        run('e', 2, 2); e_done.count_down();
        run('c', 1, 1); run('c', 6, 6); c_done.count_down();

        b_done.wait(); e_done.wait();
        run('h', 1, 7); h_done.count_down();

        d_done.wait(); h_done.wait(); j_done.wait();
        f('k', 1);
        });

    threads.emplace_back([&]() {
        run('a', 2, 2); a_done.count_down();
        run('b', 3, 3); b_done.count_down();
        run('e', 3, 3); e_done.count_down();
        run('c', 2, 2); run('c', 7, 7); c_done.count_down();

        b_done.wait(); e_done.wait();
        run('h', 8, 9); h_done.count_down();

        a_done.wait();
        run('g', 1, 1); g_done.count_down();

        run('i', 1, 4); i_done.count_down();

        d_done.wait(); h_done.wait(); j_done.wait();
        f('k', 2);
        });

    threads.emplace_back([&]() {
        run('a', 3, 3); a_done.count_down();
        run('b', 4, 4); b_done.count_down();
        run('e', 4, 4); e_done.count_down();
        run('c', 3, 3); run('c', 8, 8); c_done.count_down();

        a_done.wait();
        run('g', 2, 4); g_done.count_down();
        run('d', 4, 7); d_done.count_down();

        g_done.wait(); i_done.wait(); f_done.wait();
        f('n', 1);
        });

    threads.emplace_back([&]() {
        run('a', 4, 4); a_done.count_down();
        run('b', 5, 5); b_done.count_down();

        a_done.wait();
        run('d', 1, 1);
        run('c', 4, 4); run('c', 9, 9); c_done.count_down();
        run('d', 8, 9); d_done.count_down();

        c_done.wait();
        run('j', 1, 4); j_done.count_down();

        g_done.wait(); i_done.wait(); f_done.wait();
        f('n', 2);
        });

    threads.emplace_back([&]() {
        run('b', 1, 1); b_done.count_down();
        run('e', 1, 1); e_done.count_down();

        a_done.wait();
        run('d', 2, 2);
        run('c', 5, 5); c_done.count_down();
        run('d', 3, 3); d_done.count_down();

        c_done.wait();
        run('j', 5, 6); j_done.count_down();
        run('f', 1, 4); f_done.count_down();

        g_done.wait(); i_done.wait(); f_done.wait();
        f('n', 3);
        });

    for (auto& t : threads) t.join();

    std::chrono::duration<double> duration = std::chrono::steady_clock::now() - start_time;
    std::osyncstream(std::cout) << "Computation completed. Duration of computation "
        << duration.count() << " seconds.\n";
    return 0;
}