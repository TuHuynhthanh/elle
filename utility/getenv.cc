#include <cstdlib>
#include <stdexcept>

#include "getenv.hh"

namespace elle
{
  namespace utility
  {

    std::string getenv(std::string const& key)
    {
      char const* val = ::getenv(key.c_str());
      if (val == nullptr)
        throw std::runtime_error("KeyError: '" + key + "'");
      return std::string{val};
    }

    std::string getenv(std::string const& key,
                       std::string const& default_)
    {
      char const* val = ::getenv(key.c_str());
      if (val != nullptr)
        return std::string{val};
      else
        return default_;
    }

  }
}
