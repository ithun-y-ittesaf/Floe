#pragma once

/// @file token.hpp
/// @brief Token types and the Token struct for the Floe lexer.

#include <cstdint>
#include <string>
#include <string_view>
#include "floe/diagnostics/span.hpp"

namespace floe {

/// Every distinct token the lexer can produce.
/// Grouped logically: literals, punctuation, keywords, special.
enum class TokenKind : uint8_t {
    // ── Literals ────────────────────────────────────────────────────────
    IntLit,         // 42, 1280
    FloatLit,       // 2.5, 0.75
    StringLit,      // "hello.mp4"
    BoolLit,        // true, false
    DurationLit,    // 2.5s, 100ms  (number + time-unit suffix)
    SizeLit,        // 128k, 4M     (number + size suffix)

    // ── Identifiers & keywords ──────────────────────────────────────────
    Ident,          // user names, codec names (h264, aac)
    KwLet,          // let
    KwFn,           // fn
    KwIf,           // if
    KwElse,         // else
    KwTrue,         // true
    KwFalse,        // false

    // ── Punctuation / operators ─────────────────────────────────────────
    LParen,         // (
    RParen,         // )
    LBrace,         // {
    RBrace,         // }
    LBracket,       // [
    RBracket,       // ]
    Comma,          // ,
    Colon,          // :
    Dot,            // .
    Pipe,           // |>
    Arrow,          // ->
    Eq,             // =
    EqEq,           // ==
    BangEq,         // !=
    Lt,             // <
    Gt,             // >
    LtEq,           // <=
    GtEq,           // >=
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /

    // ── Special ─────────────────────────────────────────────────────────
    Newline,        // significant for statement separation (filtered later)
    Eof,            // end of input
    Error,          // a lexer error (unexpected character, unterminated string, etc.)
};

/// Human-readable name for a TokenKind (for debug output and diagnostics).
[[nodiscard]] std::string_view token_kind_name(TokenKind kind) noexcept;

/// A single token produced by the lexer.
struct Token {
    TokenKind   kind = TokenKind::Eof;
    Span        span;
    std::string lexeme;  ///< The raw text of the token.

    /// For IntLit / FloatLit / DurationLit / SizeLit, the parsed numeric value.
    /// For StringLit, the unescaped string content (without quotes).
    /// For other tokens this is empty.
    std::string value;
};

} // namespace floe
