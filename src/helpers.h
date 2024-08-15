#pragma once

#include <exception>
#include <string>

inline auto panic( std::string const& message ) -> void
{
    throw std::runtime_error( message );
}

namespace helpers {

} // namespace helpers
