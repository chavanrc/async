#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

void SetThreadName(const char *name) {
#ifdef HAVE_PRCTL
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name), 0, 0, 0);
#endif
}

void async_send_handler_2(int number) {
    std::cout << "Number: " << number << ", threadID: " << std::this_thread::get_id() << std::endl;
}

void Jarle() {
    boost::asio::io_context ctx;
    std::list<std::thread> workers;
    boost::asio::io_service::work asio_work{ctx};

    for (size_t t = 0; t < 1; ++t) {
        std::thread thd([t, &ctx]() {
            std::cout << "Starting asio loop " << t << '\n';
            auto name = "asio-" + std::to_string(t);
            SetThreadName(name.c_str());
            ctx.run();
            std::cout << "Done with asio loop " << t << '\n';
        });
        workers.push_back(move(thd));
    }

    boost::asio::io_service::strand strand(ctx);
    strand.post([] { return async_send_handler_2(1); });
    strand.post([] { return async_send_handler_2(2); });
    strand.post([] { return async_send_handler_2(3); });
    strand.post([] { return async_send_handler_2(4); });
    strand.post([] { return async_send_handler_2(5); });

    boost::this_thread::sleep(boost::posix_time::seconds(1)); // Simulate work

    ctx.stop();
    for (auto &w: workers) {
        w.join();
    }
}