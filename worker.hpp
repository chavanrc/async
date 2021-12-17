#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

volatile sig_atomic_t stop;

void handler(int signum) {
    stop = 1;
}

class Worker {
public:
    using ptr_t = std::unique_ptr<Worker>;

    explicit Worker(boost::asio::io_context &ctx)
            : ctx_{ctx} {
        signal(SIGINT, handler);
        signal(SIGTERM, handler);
    }

    virtual ~Worker() = default;

    std::future<void> Run() {
        Run_();
        return promise_.get_future();
    }

protected:
    virtual void Run_() = 0;

    void Shutdown() {
        std::call_once(done_, [&]() {
            promise_.set_value();
        });
    }

    boost::asio::io_context &ctx_;
private:
    std::once_flag done_;
    std::promise<void> promise_;
};

class Producer : public Worker {
public:
    explicit Producer(boost::asio::io_context &ctx) : Worker(ctx), strand_{ctx}, asio_timer_{
            std::make_unique<boost::asio::deadline_timer>(ctx_,
                                                          boost::posix_time::milliseconds(interval_in_millisec))} {
    }

protected:
    void Run_() override {
        Produce();
    }

private:
    void Produce() {
        strand_.post([this] {
            // Approach 1
            {
                data_ += 1;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
                Schedule();
            }
            // Approach 2
            //return Work();
        });
    }

    void Schedule() {
        asio_timer_->async_wait([this](const boost::system::error_code &err) {
            if (err) {
                std::cout << "Timer failed: " << err.message();
            }
            std::cout << "Schedule " << data_ << '\n';
            if(!stop) {
                Produce();
            } else {
                Shutdown();
            }
        });
    }

    void Work() {
        while (!stop) {
            //while (data_ < 5) {
            std::cout << "Work " << data_ << '\n';
            data_ += 1;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate work
        }
        Shutdown();
    }

    boost::asio::io_context::strand strand_;
    std::unique_ptr<boost::asio::deadline_timer> asio_timer_;
    uint64_t interval_in_millisec{0};
    size_t data_{0};
};