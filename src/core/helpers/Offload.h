//
// Created by user on 15.02.2026.
//

#ifndef BEAST_API_OFFLOAD_H
#define BEAST_API_OFFLOAD_H

#pragma once
#include <boost/asio.hpp>
// Needed includes for invoke result
#include <utility>
#include <type_traits>

namespace net = boost::asio;

template <class Executor, class Fn>
auto async_offload(Executor blockingEx, Fn fn)
    -> net::awaitable<std::invoke_result_t<Fn&>>
{
    using R = std::invoke_result_t<Fn&>;

    // 1) save current executor
    auto originalEx = co_await net::this_coro::executor;

    // 2) goto blocking pool
    co_await net::post(blockingEx, net::use_awaitable);

    if constexpr (std::is_void_v<R>) {
        std::invoke(fn);

        // 3) return back
        co_await net::post(originalEx, net::use_awaitable);
        co_return;
    } else {
        R result = std::invoke(fn);

        // 3) return back
        co_await net::post(originalEx, net::use_awaitable);
        co_return result;
    }
}

#endif //BEAST_API_OFFLOAD_H
