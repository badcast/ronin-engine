#include "ronin_exception.h"
#include "ronin_debug.h"

namespace RoninEngine::Exception
{
    ronin_base_error::ronin_base_error(const char *error_str) : std::runtime_error(error_str)
    {
        ronin_err(error_str);
    }

} // namespace RoninEngine::Exception
