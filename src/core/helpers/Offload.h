#ifndef BEAST_API_OFFLOAD_H
#define BEAST_API_OFFLOAD_H

#pragma once
#include <boost/asio.hpp>
#include <optional>
#include <exception>
#include <atomic>
#include <memory>

namespace net = boost::asio;

/// NEED TO STRICTLY REVIEWED
/// Written with GPT guide, but need to be tested on production env

inline std::exception_ptr make_operation_aborted_ep()
{
    return std::make_exception_ptr(
        boost::system::system_error(net::error::operation_aborted)
    );
}

template <class Handler, class R>
struct OffloadState {
    explicit OffloadState(net::any_io_executor ex)
        : work(std::move(ex)) {}

    net::executor_work_guard<net::any_io_executor> work;
    std::optional<Handler> h;

    std::shared_ptr<std::atomic_bool> cancelled =
        std::make_shared<std::atomic_bool>(false);

    std::exception_ptr ep;
    std::optional<R> result;

    net::cancellation_type cancel_type = net::cancellation_type::none;
};

template <class Executor, class Fn>
auto async_offload(Executor blockingEx, Fn fn)
    -> net::awaitable<std::invoke_result_t<Fn>>
{
    using R = std::invoke_result_t<Fn>;

    auto ioEx  = co_await net::this_coro::executor;
    auto token = net::as_tuple(net::use_awaitable);

    if constexpr (std::is_void_v<R>)
    {
        auto [ep] = co_await net::async_initiate<decltype(token), void(std::exception_ptr)>(
            [ioEx, blockingEx, func = std::move(fn)](auto handler) mutable {

                using H  = std::decay_t<decltype(handler)>;
                using St = OffloadState<H, int>; // int-фиктивный, result не используется при void

                auto st = std::make_shared<St>(net::any_io_executor(ioEx));
                st->h.emplace(std::move(handler));

                auto slot = net::get_associated_cancellation_slot(*st->h);
                if (slot.is_connected()) {
                    slot.assign([st](net::cancellation_type ct) {
                        st->cancel_type = ct;
                        st->cancelled->store(true, std::memory_order_relaxed);
                    });
                }

                net::post(blockingEx, [ioEx, st, func = std::move(func)]() mutable {
                    try {
                        if (!st->cancelled->load(std::memory_order_relaxed)) {
                            func();
                        } else {
                            st->ep = make_operation_aborted_ep();
                        }
                    } catch (...) {
                        st->ep = std::current_exception();
                    }

                    net::dispatch(ioEx, [st]() mutable {
                        auto ep = st->ep;
                        (*st->h)(ep);

                        // critical: destroy handler on ioEx
                        st->h.reset();
                    });
                });
            },
            token
        );

        if (ep) std::rethrow_exception(ep);
        co_return;
    }
    else
    {
        auto [ep, resultOpt] =
            co_await net::async_initiate<decltype(token), void(std::exception_ptr, std::optional<R>)>(
                [ioEx, blockingEx, func = std::move(fn)](auto handler) mutable {

                    using H  = std::decay_t<decltype(handler)>;
                    using St = OffloadState<H, R>;

                    auto st = std::make_shared<St>(net::any_io_executor(ioEx));
                    st->h.emplace(std::move(handler));

                    auto slot = net::get_associated_cancellation_slot(*st->h);
                    if (slot.is_connected()) {
                        slot.assign([st](net::cancellation_type ct) {
                            st->cancel_type = ct;
                            st->cancelled->store(true, std::memory_order_relaxed);
                        });
                    }

                    net::post(blockingEx, [ioEx, st, func = std::move(func)]() mutable {
                        try {
                            if (!st->cancelled->load(std::memory_order_relaxed)) {
                                st->result.emplace(func());
                            } else {
                                st->ep = make_operation_aborted_ep();
                            }
                        } catch (...) {
                            st->ep = std::current_exception();
                        }

                        net::dispatch(ioEx, [st]() mutable {
                            auto ep  = st->ep;
                            auto res = std::move(st->result);

                            (*st->h)(ep, std::move(res));

                            // critical: destroy handler on ioEx
                            st->h.reset();
                        });
                    });
                },
                token
            );

        if (ep) std::rethrow_exception(ep);
        co_return std::move(*resultOpt);
    }
}

#endif //BEAST_API_OFFLOAD_H
