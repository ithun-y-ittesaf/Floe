#include "floe/syntax/lexer.hpp"
#include <cassert>
#include <cctype>
#include <unordered_map>

namespace floe {

// ═════════════════════════════════════════════════════════════════════════════
// Scanner — internal implementation
// ═════════════════════════════════════════════════════════════════════════════

namespace {

/// Keyword lookup table (populated once, thread-safe after init).
const std::unordered_map<std::string_view, TokenKind>& keywords() {
    static const std::unordered_map<std::string_view, TokenKind> kw = {
        { "let",   TokenKind::KwLet   },
        { "fn",    TokenKind::KwFn    },
        { "if",    TokenKind::KwIf    },
        { "else",  TokenKind::KwElse  },
        { "true",  TokenKind::KwTrue  },
        { "false", TokenKind::KwFalse },
    };
    return kw;
}

class Scanner {
public:
    Scanner(const Source& source)
        : src_(source)
        , text_(source.text())
        , source_id_(source.id())
        , pos_(0)
    {}

    LexResult scan_all() {
        LexResult result;
        while (true) {
            Token tok = next_token();
            bool is_eof = (tok.kind == TokenKind::Eof);
            if (tok.kind == TokenKind::Error) {
                result.diagnostics.push_back(
                    Diagnostic::error(std::string(tok.value))
                        .with_label(tok.span, "here")
                );
            }
            result.tokens.push_back(std::move(tok));
            if (is_eof) break;
        }
        return result;
    }

private:
    const Source&    src_;
    std::string_view text_;
    SourceId        source_id_;
    uint32_t        pos_;

    // ── Helpers ─────────────────────────────────────────────────────────

    [[nodiscard]] bool at_end() const noexcept {
        return pos_ >= text_.size();
    }

    [[nodiscard]] char peek() const noexcept {
        return at_end() ? '\0' : text_[pos_];
    }

    [[nodiscard]] char peek_next() const noexcept {
        return (pos_ + 1 >= text_.size()) ? '\0' : text_[pos_ + 1];
    }

    char advance() noexcept {
        assert(!at_end());
        return text_[pos_++];
    }

    bool match(char expected) noexcept {
        if (at_end() || text_[pos_] != expected) return false;
        ++pos_;
        return true;
    }

    [[nodiscard]] Span make_span(uint32_t start) const noexcept {
        return { source_id_, start, pos_ };
    }

    [[nodiscard]] std::string lexeme(uint32_t start) const {
        return std::string(text_.substr(start, pos_ - start));
    }

    Token make_token(TokenKind kind, uint32_t start) const {
        return { kind, make_span(start), lexeme(start), {} };
    }

    Token make_token(TokenKind kind, uint32_t start, std::string value) const {
        return { kind, make_span(start), lexeme(start), std::move(value) };
    }

    Token error_token(uint32_t start, std::string message) const {
        return { TokenKind::Error, make_span(start), lexeme(start), std::move(message) };
    }

    // ── Skip whitespace and comments ────────────────────────────────────

    void skip_whitespace_and_comments() {
        while (!at_end()) {
            char c = peek();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else if (c == '/' && peek_next() == '/') {
                // Line comment: skip to end of line
                while (!at_end() && peek() != '\n') advance();
            } else {
                break;
            }
        }
    }

    // ── Number scanning ─────────────────────────────────────────────────

    Token scan_number(uint32_t start) {
        // Consume digits
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }

        bool is_float = false;

        // Check for fractional part
        if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
            is_float = true;
            advance();  // consume '.'
            while (!at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }

        std::string num_text(text_.substr(start, pos_ - start));

        // Check for unit suffixes
        char suffix = peek();
        if (suffix == 's' && !std::isalnum(static_cast<unsigned char>(peek_next()))) {
            // Duration: e.g. 2.5s
            advance();
            return make_token(TokenKind::DurationLit, start, num_text);
        }
        if ((suffix == 'm' || suffix == 'M') && peek_next() == 's') {
            // Duration: e.g. 100ms
            advance(); advance();
            return make_token(TokenKind::DurationLit, start, num_text);
        }
        if (suffix == 'k' || suffix == 'K' || suffix == 'M') {
            // Size: e.g. 128k, 4M — but only if not followed by more alpha
            char after = peek_next();
            if (!std::isalnum(static_cast<unsigned char>(after)) && after != '_') {
                advance();
                return make_token(TokenKind::SizeLit, start, num_text);
            }
        }

        if (is_float) {
            return make_token(TokenKind::FloatLit, start, num_text);
        }
        return make_token(TokenKind::IntLit, start, num_text);
    }

    // ── String scanning ─────────────────────────────────────────────────

    Token scan_string(uint32_t start) {
        // start points at the opening quote, which we already consumed
        std::string value;
        while (!at_end() && peek() != '"') {
            if (peek() == '\n') {
                return error_token(start, "unterminated string literal");
            }
            if (peek() == '\\') {
                advance(); // consume backslash
                if (at_end()) {
                    return error_token(start, "unterminated string escape");
                }
                char esc = advance();
                switch (esc) {
                    case 'n':  value += '\n'; break;
                    case 't':  value += '\t'; break;
                    case 'r':  value += '\r'; break;
                    case '\\': value += '\\'; break;
                    case '"':  value += '"';  break;
                    default:
                        return error_token(start,
                            std::string("unknown escape sequence: \\") + esc);
                }
            } else {
                value += advance();
            }
        }

        if (at_end()) {
            return error_token(start, "unterminated string literal");
        }

        advance(); // consume closing quote
        return make_token(TokenKind::StringLit, start, std::move(value));
    }

    // ── Identifier / keyword scanning ───────────────────────────────────

    Token scan_identifier(uint32_t start) {
        while (!at_end()) {
            char c = peek();
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                advance();
            } else {
                break;
            }
        }

        auto text = text_.substr(start, pos_ - start);
        auto& kw = keywords();
        auto it = kw.find(text);
        if (it != kw.end()) {
            TokenKind kind = it->second;
            // true / false are also BoolLit
            if (kind == TokenKind::KwTrue || kind == TokenKind::KwFalse) {
                return make_token(TokenKind::BoolLit, start,
                                  std::string(text));
            }
            return make_token(kind, start);
        }
        return make_token(TokenKind::Ident, start);
    }

    // ── Main dispatch ───────────────────────────────────────────────────

    Token next_token() {
        skip_whitespace_and_comments();

        if (at_end()) {
            uint32_t pos = pos_;
            return { TokenKind::Eof, { source_id_, pos, pos }, "", {} };
        }

        uint32_t start = pos_;
        char c = advance();

        // Newline (significant for statement separation)
        if (c == '\n') {
            return make_token(TokenKind::Newline, start);
        }

        // Numbers
        if (std::isdigit(static_cast<unsigned char>(c))) {
            // We already consumed one digit; back up and re-scan.
            pos_ = start;
            return scan_number(start);
        }

        // Strings
        if (c == '"') {
            return scan_string(start);
        }

        // Identifiers and keywords
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            pos_ = start;
            return scan_identifier(start);
        }

        // Punctuation and operators
        switch (c) {
            case '(': return make_token(TokenKind::LParen,   start);
            case ')': return make_token(TokenKind::RParen,   start);
            case '{': return make_token(TokenKind::LBrace,   start);
            case '}': return make_token(TokenKind::RBrace,   start);
            case '[': return make_token(TokenKind::LBracket, start);
            case ']': return make_token(TokenKind::RBracket, start);
            case ',': return make_token(TokenKind::Comma,    start);
            case ':': return make_token(TokenKind::Colon,    start);
            case '.': return make_token(TokenKind::Dot,      start);
            case '+': return make_token(TokenKind::Plus,     start);
            case '*': return make_token(TokenKind::Star,     start);
            case '/': return make_token(TokenKind::Slash,    start);

            case '|':
                if (match('>')) return make_token(TokenKind::Pipe, start);
                return error_token(start, "expected '>' after '|' (pipe operator is |>)");

            case '-':
                if (match('>')) return make_token(TokenKind::Arrow, start);
                return make_token(TokenKind::Minus, start);

            case '=':
                if (match('=')) return make_token(TokenKind::EqEq, start);
                return make_token(TokenKind::Eq, start);

            case '!':
                if (match('=')) return make_token(TokenKind::BangEq, start);
                return error_token(start, "expected '=' after '!' (did you mean !=?)");

            case '<':
                if (match('=')) return make_token(TokenKind::LtEq, start);
                return make_token(TokenKind::Lt, start);

            case '>':
                if (match('=')) return make_token(TokenKind::GtEq, start);
                return make_token(TokenKind::Gt, start);

            default:
                return error_token(start,
                    std::string("unexpected character: '") + c + "'");
        }
    }
};

} // namespace (anonymous)

// ═════════════════════════════════════════════════════════════════════════════
// Public API
// ═════════════════════════════════════════════════════════════════════════════

bool LexResult::has_errors() const noexcept {
    return !diagnostics.empty();
}

LexResult lex(const Source& source) {
    Scanner scanner(source);
    return scanner.scan_all();
}

LexResult lex(std::string_view text, std::string_view filename) {
    Source source(SourceId{0}, std::string(filename), std::string(text));
    return lex(source);
}

} // namespace floe
