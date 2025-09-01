#include <boost/version.hpp>
#include <boost/asio.hpp>
#include <boost/asio/experimental/channel.hpp>

namespace net = boost::asio;

template <class T>
net::awaitable<T>

channel_recv(boost::asio::experimental::channel<void(T)>& ch) {
#if BOOST_VERSION >= 108200  // 1.82.0+
    co_return co_await ch.async_receive(boost::asio::use_awaitable);
#else
    T tmp;
    co_await ch.async_receive(tmp, boost::asio::use_awaitable);
    co_return tmp;
#endif
}
