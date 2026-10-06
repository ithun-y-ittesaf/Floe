#include "floe/diagnostics/span.hpp"
#include <algorithm>
#include <cassert>

namespace floe {

Span Span::merge(Span other) const noexcept {
    assert(source == other.source);
    return { source, std::min(start, other.start), std::max(end, other.end) };
}

} // namespace floe
