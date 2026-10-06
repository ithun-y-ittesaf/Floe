#pragma once

/// @file lexer.hpp
/// @brief Hand-written lexer for the Floe language.
///
/// The lexer scans a Source and produces a sequence of Tokens.
/// It is designed for single-pass, zero-copy scanning with full span tracking.

#include <vector>
#include "floe/diagnostics/source.hpp"
#include "floe/diagnostics/diagnostic.hpp"
#include "floe/syntax/token.hpp"

namespace floe {

/// Result of lexing an entire source file.
struct LexResult {
    std::vector<Token>      tokens;
    std::vector<Diagnostic> diagnostics;

    [[nodiscard]] bool has_errors() const noexcept;
};

/// Lex an entire Source into a token stream.
/// Always succeeds (never throws). Errors are reported as TokenKind::Error
/// tokens and as Diagnostic entries in the result.
[[nodiscard]] LexResult lex(const Source& source);

/// Convenience: lex a string directly (creates a temporary Source).
[[nodiscard]] LexResult lex(std::string_view text,
                            std::string_view filename = "<input>");

} // namespace floe
