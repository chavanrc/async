#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include "jarle.hpp"
#include "worker.hpp"

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter) {
    //global_stream_lock.lock();
    //std::cout << "Thread " << std::this_thread::get_id() << ", " << counter << " Start.\n";
    //global_stream_lock.unlock();

    iosvc->run();

    //global_stream_lock.lock();
    //std::cout << "Thread " << counter << " End.\n";
    //global_stream_lock.unlock();
}

void async_send_handler(int number) {
    std::cout << "Number: " << number << ", threadID: " << std::this_thread::get_id() << std::endl;
}

int main() {
    //boost::shared_ptr<boost::asio::io_service> io_svc(new boost::asio::io_service);

    /**
     * If you want to make sure service_.run() continues to run, you have to assign more
     * work to it. There are two ways of accomplishing this. One way is to assign more
     * work inside connect_handler by starting another asynchronous operation.
     * The other way is to simulate some work for it. The preceding code will make sure that
     * iosvc->run() never stops unless you either iosvc->stop() or worker.reset();
     */
    /*boost::shared_ptr<boost::asio::io_service::work> worker(
            new boost::asio::io_service::work(*io_svc)
            );

    boost::asio::io_service::strand strand(*io_svc);

    global_stream_lock.lock();
    std::cout << "The program will exit once all work has finished.\n";
    global_stream_lock.unlock();

    boost::thread_group threads;
    for( int i = 1; i <= 1; i++ )
        threads.create_thread([io_svc, i] { return WorkerThread(io_svc, i); });*/

    //boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    // Imagine you are invoking async_send on tcp or udp socket several times
    // and you want the handlers of this async_send call to be invoked sequentially

    // This code is almost equal to calling handlers of socket.async_send.
    // The handlers are invoked concurently and the order might be arbitrary
    /*io_svc->post([] { return async_send_handler(1); });
    io_svc->post([] { return async_send_handler(2); });
    io_svc->post([] { return async_send_handler(3); });
    io_svc->post([] { return async_send_handler(4); });
    io_svc->post([] { return async_send_handler(5); });*/

    // This code will do what you exactly want;
    // It will execute the handlers sequentially in that order
    /*strand.post([] { return async_send_handler(1); });
    strand.post([] { return async_send_handler(2); });
    strand.post([] { return async_send_handler(3); });
    strand.post([] { return async_send_handler(4); });
    strand.post([] { return async_send_handler(5); });

    worker.reset();

    threads.join_all();

    return 0;*/

    //Jarle();

    {
        boost::asio::io_context ctx;
        std::vector<std::thread> workers;
        std::vector<std::future<void>> futures;
        boost::asio::io_service::work asio_work{ctx};
        std::size_t asio_threads{2};

        for (size_t t = 0; t < asio_threads; ++t) {
            std::thread thd([t, &ctx]() {
                std::cout << "Starting asio loop " << t << '\n';
                auto name = "asio-" + std::to_string(t);
                SetThreadName(name.c_str());
                ctx.run();
                std::cout << "Done with asio loop " << t << '\n';
            });
            workers.push_back(move(thd));
        }
        std::size_t worker_threads{asio_threads};
        std::list<std::unique_ptr<Producer>> producers;
        for(size_t w = 0; w < worker_threads; ++w) {
            producers.emplace_back(std::make_unique<Producer>(ctx));
            futures.push_back(producers.back()->Run());
        }

        for(auto& f: futures) {
            f.get();
        }
        ctx.stop();
        for (auto &w: workers) {
            w.join();
        }
    }
}
