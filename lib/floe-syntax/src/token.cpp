#include "floe/syntax/token.hpp"

namespace floe {

std::string_view token_kind_name(TokenKind kind) noexcept {
    switch (kind) {
        // Literals
        case TokenKind::IntLit:      return "INT";
        case TokenKind::FloatLit:    return "FLOAT";
        case TokenKind::StringLit:   return "STRING";
        case TokenKind::BoolLit:     return "BOOL";
        case TokenKind::DurationLit: return "DURATION";
        case TokenKind::SizeLit:     return "SIZE";
        // Identifiers & keywords
        case TokenKind::Ident:       return "IDENT";
        case TokenKind::KwLet:       return "KW_LET";
        case TokenKind::KwFn:        return "KW_FN";
        case TokenKind::KwIf:        return "KW_IF";
        case TokenKind::KwElse:      return "KW_ELSE";
        case TokenKind::KwTrue:      return "KW_TRUE";
        case TokenKind::KwFalse:     return "KW_FALSE";
        // Punctuation
        case TokenKind::LParen:      return "LPAREN";
        case TokenKind::RParen:      return "RPAREN";
        case TokenKind::LBrace:      return "LBRACE";
        case TokenKind::RBrace:      return "RBRACE";
        case TokenKind::LBracket:    return "LBRACKET";
        case TokenKind::RBracket:    return "RBRACKET";
        case TokenKind::Comma:       return "COMMA";
        case TokenKind::Colon:       return "COLON";
        case TokenKind::Dot:         return "DOT";
        case TokenKind::Pipe:        return "PIPE";
        case TokenKind::Arrow:       return "ARROW";
        case TokenKind::Eq:          return "EQ";
        case TokenKind::EqEq:        return "EQEQ";
        case TokenKind::BangEq:      return "BANGEQ";
        case TokenKind::Lt:          return "LT";
        case TokenKind::Gt:          return "GT";
        case TokenKind::LtEq:        return "LTEQ";
        case TokenKind::GtEq:        return "GTEQ";
        case TokenKind::Plus:        return "PLUS";
        case TokenKind::Minus:       return "MINUS";
        case TokenKind::Star:        return "STAR";
        case TokenKind::Slash:       return "SLASH";
        // Special
        case TokenKind::Newline:     return "NEWLINE";
        case TokenKind::Eof:         return "EOF";
        case TokenKind::Error:       return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace floe
