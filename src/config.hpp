#pragma once

namespace FRT
{
    enum class log_levels 
    {
        off,
        error,
        warning,
        info,
        debug,
    };

    const inline auto log_level = log_levels::debug;
}; // namespace
