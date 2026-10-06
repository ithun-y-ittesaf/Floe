#pragma once

/// @file source.hpp
/// @brief Represents a named source file and its contents.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace floe {

/// A unique identifier for a loaded source file.
/// Allows spans to refer back to their origin cheaply (by value).
enum class SourceId : uint32_t {};

/// Owns the text of a single source file and provides line-offset lookup.
class Source {
public:
    Source(SourceId id, std::string name, std::string text);

    [[nodiscard]] SourceId          id()   const noexcept { return id_; }
    [[nodiscard]] std::string_view  name() const noexcept { return name_; }
    [[nodiscard]] std::string_view  text() const noexcept { return text_; }
    [[nodiscard]] std::size_t       size() const noexcept { return text_.size(); }

    /// Convert a byte offset into a (line, column) pair (both 1-based).
    struct LineCol { uint32_t line; uint32_t col; };
    [[nodiscard]] LineCol line_col(uint32_t offset) const;

    /// Return the text of a given 1-based line number.
    [[nodiscard]] std::string_view line_text(uint32_t line) const;

private:
    void build_line_offsets();

    SourceId               id_;
    std::string            name_;
    std::string            text_;
    std::vector<uint32_t>  line_offsets_;  ///< byte offset of each line start
};

} // namespace floe
