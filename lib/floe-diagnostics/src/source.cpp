#include "floe/diagnostics/source.hpp"
#include <algorithm>
#include <cassert>

namespace floe {

Source::Source(SourceId id, std::string name, std::string text)
    : id_(id), name_(std::move(name)), text_(std::move(text))
{
    build_line_offsets();
}

void Source::build_line_offsets() {
    line_offsets_.clear();
    line_offsets_.push_back(0);  // line 1 starts at offset 0
    for (uint32_t i = 0; i < text_.size(); ++i) {
        if (text_[i] == '\n') {
            line_offsets_.push_back(i + 1);
        }
    }
}

Source::LineCol Source::line_col(uint32_t offset) const {
    assert(offset <= text_.size());
    // Binary search: find the last line_offset <= offset
    auto it = std::upper_bound(line_offsets_.begin(), line_offsets_.end(), offset);
    --it;  // safe because line_offsets_[0] == 0 <= offset
    auto line_idx = static_cast<uint32_t>(it - line_offsets_.begin());
    return { line_idx + 1, offset - *it + 1 };
}

std::string_view Source::line_text(uint32_t line) const {
    assert(line >= 1 && line <= line_offsets_.size());
    uint32_t start = line_offsets_[line - 1];
    uint32_t end;
    if (line < line_offsets_.size()) {
        end = line_offsets_[line];
        // Strip the trailing newline
        if (end > start && text_[end - 1] == '\n') --end;
        if (end > start && text_[end - 1] == '\r') --end;
    } else {
        end = static_cast<uint32_t>(text_.size());
    }
    return std::string_view(text_).substr(start, end - start);
}

} // namespace floe
