#ifndef BEAST_API_OFFLOAD_H
#define BEAST_API_OFFLOAD_H

#pragma once
#include <boost/asio.hpp>
#include <future>
#include <memory>

namespace net = boost::asio;

/**
* Safe offload without migrating the coroutine frame.
* 1. The coroutine remains on the IO thread.
* 2. The task (lambda) is moved to the ThreadPool.
* 3. The IO thread "sleeps" (awaitable timer) until the Future becomes ready.
*/
template <class Executor, class Fn>
auto async_offload(Executor blockingEx, Fn fn)
    -> net::awaitable<std::invoke_result_t<Fn>>
{
    using ResultType = std::invoke_result_t<Fn>;

    auto promise = std::make_shared<std::promise<ResultType>>();
    auto future = promise->get_future();

    net::post(blockingEx, [p = promise, func = std::move(fn)]() mutable {
        try {
            if constexpr (std::is_void_v<ResultType>) {
                func();
                p->set_value();
            } else {
                p->set_value(func());
            }
        } catch (...) {
            p->set_exception(std::current_exception());
        }
    });

    net::steady_timer timer(co_await net::this_coro::executor);
    
    while (future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::milliseconds(2)); // Небольшая задержка
        co_await timer.async_wait(net::use_awaitable);
    }

    co_return future.get();
}

#endif //BEAST_API_OFFLOAD_H
