/// @file lexer_test.cpp
/// @brief Unit tests for the Floe lexer.
///
/// Test strategy from PLAN.md §8:
///   "Lexer: source → token stream snapshots.
///    For example, input("video.mp4") → IDENT LPAREN STRING RPAREN."

#include <gtest/gtest.h>
#include "floe/syntax/lexer.hpp"
#include "floe/syntax/token.hpp"

using namespace floe;

// ── Helpers ─────────────────────────────────────────────────────────────────

/// Lex and return only the non-EOF, non-Newline token kinds.
static std::vector<TokenKind> lex_kinds(std::string_view src) {
    auto result = lex(src);
    std::vector<TokenKind> kinds;
    for (auto& tok : result.tokens) {
        if (tok.kind != TokenKind::Eof && tok.kind != TokenKind::Newline)
            kinds.push_back(tok.kind);
    }
    return kinds;
}

/// Lex and return a string of "KIND KIND KIND ..." (excluding EOF/Newline).
static std::string lex_snapshot(std::string_view src) {
    auto kinds = lex_kinds(src);
    std::string out;
    for (size_t i = 0; i < kinds.size(); ++i) {
        if (i > 0) out += ' ';
        out += token_kind_name(kinds[i]);
    }
    return out;
}

/// Lex and return the first non-trivial token.
static Token lex_first(std::string_view src) {
    auto result = lex(src);
    for (auto& tok : result.tokens) {
        if (tok.kind != TokenKind::Newline) return tok;
    }
    return result.tokens.back(); // EOF
}

// ═════════════════════════════════════════════════════════════════════════════
// Basic tokens
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, EmptyInput) {
    auto result = lex("");
    ASSERT_EQ(result.tokens.size(), 1u);
    EXPECT_EQ(result.tokens[0].kind, TokenKind::Eof);
    EXPECT_FALSE(result.has_errors());
}

TEST(Lexer, SingleInteger) {
    auto tok = lex_first("42");
    EXPECT_EQ(tok.kind, TokenKind::IntLit);
    EXPECT_EQ(tok.lexeme, "42");
    EXPECT_EQ(tok.value, "42");
}

TEST(Lexer, FloatLiteral) {
    auto tok = lex_first("2.5");
    EXPECT_EQ(tok.kind, TokenKind::FloatLit);
    EXPECT_EQ(tok.value, "2.5");
}

TEST(Lexer, StringLiteral) {
    auto tok = lex_first("\"hello.mp4\"");
    EXPECT_EQ(tok.kind, TokenKind::StringLit);
    EXPECT_EQ(tok.value, "hello.mp4");
}

TEST(Lexer, StringWithEscapes) {
    auto tok = lex_first(R"("line\nbreak")");
    EXPECT_EQ(tok.kind, TokenKind::StringLit);
    EXPECT_EQ(tok.value, "line\nbreak");
}

TEST(Lexer, BoolLiterals) {
    auto t = lex_first("true");
    EXPECT_EQ(t.kind, TokenKind::BoolLit);
    EXPECT_EQ(t.value, "true");

    auto f = lex_first("false");
    EXPECT_EQ(f.kind, TokenKind::BoolLit);
    EXPECT_EQ(f.value, "false");
}

// ═════════════════════════════════════════════════════════════════════════════
// Unit suffixes (from PLAN.md §3.2: duration `2.5s`, size/bitrate `128k`)
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, DurationSuffix_Seconds) {
    auto tok = lex_first("2.5s");
    EXPECT_EQ(tok.kind, TokenKind::DurationLit);
    EXPECT_EQ(tok.value, "2.5");
    EXPECT_EQ(tok.lexeme, "2.5s");
}

TEST(Lexer, DurationSuffix_Milliseconds) {
    auto tok = lex_first("100ms");
    EXPECT_EQ(tok.kind, TokenKind::DurationLit);
    EXPECT_EQ(tok.value, "100");
    EXPECT_EQ(tok.lexeme, "100ms");
}

TEST(Lexer, SizeSuffix_Kilo) {
    auto tok = lex_first("128k");
    EXPECT_EQ(tok.kind, TokenKind::SizeLit);
    EXPECT_EQ(tok.value, "128");
    EXPECT_EQ(tok.lexeme, "128k");
}

TEST(Lexer, SizeSuffix_Mega) {
    auto tok = lex_first("4M");
    EXPECT_EQ(tok.kind, TokenKind::SizeLit);
    EXPECT_EQ(tok.value, "4");
    EXPECT_EQ(tok.lexeme, "4M");
}

// ═════════════════════════════════════════════════════════════════════════════
// Keywords and identifiers
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, Keywords) {
    EXPECT_EQ(lex_first("let").kind, TokenKind::KwLet);
    EXPECT_EQ(lex_first("fn").kind, TokenKind::KwFn);
    EXPECT_EQ(lex_first("if").kind, TokenKind::KwIf);
    EXPECT_EQ(lex_first("else").kind, TokenKind::KwElse);
}

TEST(Lexer, IdentNotKeyword) {
    auto tok = lex_first("h264");
    EXPECT_EQ(tok.kind, TokenKind::Ident);
    EXPECT_EQ(tok.lexeme, "h264");
}

TEST(Lexer, IdentWithUnderscores) {
    auto tok = lex_first("my_var");
    EXPECT_EQ(tok.kind, TokenKind::Ident);
    EXPECT_EQ(tok.lexeme, "my_var");
}

// ═════════════════════════════════════════════════════════════════════════════
// Operators and punctuation
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, PipeOperator) {
    auto tok = lex_first("|>");
    EXPECT_EQ(tok.kind, TokenKind::Pipe);
    EXPECT_EQ(tok.lexeme, "|>");
}

TEST(Lexer, ArrowOperator) {
    auto tok = lex_first("->");
    EXPECT_EQ(tok.kind, TokenKind::Arrow);
    EXPECT_EQ(tok.lexeme, "->");
}

TEST(Lexer, ComparisonOperators) {
    EXPECT_EQ(lex_first("==").kind, TokenKind::EqEq);
    EXPECT_EQ(lex_first("!=").kind, TokenKind::BangEq);
    EXPECT_EQ(lex_first("<=").kind, TokenKind::LtEq);
    EXPECT_EQ(lex_first(">=").kind, TokenKind::GtEq);
    EXPECT_EQ(lex_first("<").kind,  TokenKind::Lt);
    EXPECT_EQ(lex_first(">").kind,  TokenKind::Gt);
}

// ═════════════════════════════════════════════════════════════════════════════
// Comments
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, LineComment) {
    auto result = lex("// this is a comment\n42");
    auto kinds = lex_kinds("// this is a comment\n42");
    ASSERT_EQ(kinds.size(), 1u);
    EXPECT_EQ(kinds[0], TokenKind::IntLit);
}

TEST(Lexer, CommentAtEndOfLine) {
    auto snapshot = lex_snapshot("let x = 10 // comment");
    EXPECT_EQ(snapshot, "KW_LET IDENT EQ INT");
}

// ═════════════════════════════════════════════════════════════════════════════
// Snapshot tests — full token sequences from PLAN.md examples
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, Snapshot_InputCall) {
    // From PLAN.md §8: input("video.mp4") → IDENT LPAREN STRING RPAREN
    auto snapshot = lex_snapshot(R"(input("video.mp4"))");
    EXPECT_EQ(snapshot, "IDENT LPAREN STRING RPAREN");
}

TEST(Lexer, Snapshot_LetBinding) {
    auto snapshot = lex_snapshot(R"(let src = input("input.mp4"))");
    EXPECT_EQ(snapshot, "KW_LET IDENT EQ IDENT LPAREN STRING RPAREN");
}

TEST(Lexer, Snapshot_PipeChain) {
    auto snapshot = lex_snapshot("src.video |> scale(1280, 720) |> fps(30)");
    EXPECT_EQ(snapshot,
        "IDENT DOT IDENT PIPE IDENT LPAREN INT COMMA INT RPAREN "
        "PIPE IDENT LPAREN INT RPAREN");
}

TEST(Lexer, Snapshot_EncodeWithNamedArg) {
    auto snapshot = lex_snapshot("encode(h264, crf: 23)");
    EXPECT_EQ(snapshot, "IDENT LPAREN IDENT COMMA IDENT COLON INT RPAREN");
}

TEST(Lexer, Snapshot_FnDefinition) {
    auto snapshot = lex_snapshot("fn web720(v: Video) -> Video {");
    EXPECT_EQ(snapshot,
        "KW_FN IDENT LPAREN IDENT COLON IDENT RPAREN ARROW IDENT LBRACE");
}

TEST(Lexer, Snapshot_BitrateArg) {
    auto snapshot = lex_snapshot("encode(aac, bitrate: 128k)");
    EXPECT_EQ(snapshot, "IDENT LPAREN IDENT COMMA IDENT COLON SIZE RPAREN");
}

TEST(Lexer, Snapshot_IfCondition) {
    auto snapshot = lex_snapshot("if src.video.height > 720 {");
    EXPECT_EQ(snapshot, "KW_IF IDENT DOT IDENT DOT IDENT GT INT LBRACE");
}

// ═════════════════════════════════════════════════════════════════════════════
// Span accuracy
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, SpanPositions) {
    auto result = lex("let x");
    // "let" at [0,3), " ", "x" at [4,5)
    ASSERT_GE(result.tokens.size(), 3u); // KwLet, Ident, Eof
    EXPECT_EQ(result.tokens[0].span.start, 0u);
    EXPECT_EQ(result.tokens[0].span.end,   3u);
    EXPECT_EQ(result.tokens[1].span.start, 4u);
    EXPECT_EQ(result.tokens[1].span.end,   5u);
}

// ═════════════════════════════════════════════════════════════════════════════
// Error recovery
// ═════════════════════════════════════════════════════════════════════════════

TEST(Lexer, UnterminatedString) {
    auto result = lex("\"hello");
    EXPECT_TRUE(result.has_errors());
    bool found_error = false;
    for (auto& tok : result.tokens) {
        if (tok.kind == TokenKind::Error) {
            found_error = true;
            EXPECT_NE(tok.value.find("unterminated"), std::string::npos);
        }
    }
    EXPECT_TRUE(found_error);
}

TEST(Lexer, UnexpectedCharacter) {
    auto result = lex("let x = @");
    EXPECT_TRUE(result.has_errors());
}

TEST(Lexer, BarePipe) {
    // '|' without '>' is an error
    auto result = lex("x | y");
    EXPECT_TRUE(result.has_errors());
}

TEST(Lexer, ErrorRecovery_ContinuesAfterError) {
    // After an error, the lexer should still produce remaining tokens
    auto result = lex("@ 42");
    auto kinds = lex_kinds("@ 42");
    // Should contain ERROR and INT
    EXPECT_EQ(kinds.size(), 2u);
    EXPECT_EQ(kinds[0], TokenKind::Error);
    EXPECT_EQ(kinds[1], TokenKind::IntLit);
}
