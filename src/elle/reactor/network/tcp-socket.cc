#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <elle/reactor/network/exception.hh>
#include <elle/reactor/network/resolve.hh>
#include <elle/reactor/network/tcp-socket.hh>
#include <elle/reactor/scheduler.hh>
#include <elle/reactor/thread.hh>

namespace elle
{
  namespace reactor
  {
    namespace network
    {
      /*-------------.
      | Construction |
      `-------------*/

      TCPSocket::TCPSocket(std::unique_ptr<AsioSocket> socket,
                           AsioSocket::endpoint_type const& endpoint)
        : Super(std::move(socket), endpoint)
      {}

      TCPSocket::TCPSocket(boost::asio::ip::tcp::endpoint const& endpoint,
                           DurationOpt timeout)
        : Super(std::make_unique<boost::asio::ip::tcp::socket>
                (reactor::Scheduler::scheduler()->io_service()),
                endpoint, timeout)
      {}

      TCPSocket::TCPSocket(TCPSocket&& socket)
        : Super(std::move(socket))
      {}

      TCPSocket::TCPSocket(const std::string& hostname,
                           const std::string& port,
                           DurationOpt timeout)
        : TCPSocket(resolve_tcp(hostname, port), timeout)
      {}

      TCPSocket::TCPSocket(const std::string& hostname,
                           int port,
                           DurationOpt timeout)
        : TCPSocket(hostname, std::to_string(port), timeout)
      {}

      TCPSocket::~TCPSocket()
      {}
    }
  }
}
