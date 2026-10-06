#pragma once

/// @file span.hpp
/// @brief Source-location spans used throughout the compiler.

#include <cstdint>
#include "floe/diagnostics/source.hpp"

namespace floe {

/// A half-open byte range [start, end) within a Source.
struct Span {
    SourceId  source;
    uint32_t  start = 0;
    uint32_t  end   = 0;

    [[nodiscard]] uint32_t length() const noexcept { return end - start; }
    [[nodiscard]] bool     empty()  const noexcept { return start == end; }

    /// Merge two spans into one that covers both (must be from the same source).
    [[nodiscard]] Span merge(Span other) const noexcept;
};

} // namespace floe
