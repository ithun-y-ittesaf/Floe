#pragma once

/// @file diagnostic.hpp
/// @brief Structured diagnostics with source spans (errors, warnings, notes).

#include <string>
#include <vector>
#include "floe/diagnostics/span.hpp"

namespace floe {

enum class Severity { Error, Warning, Note };

/// A single label pointing at a span with a short message.
struct Label {
    Span        span;
    std::string message;
};

/// A structured diagnostic: a primary message, a severity, and zero or more
/// source-span labels.
struct Diagnostic {
    Severity           severity = Severity::Error;
    std::string        message;
    std::vector<Label> labels;

    static Diagnostic error(std::string msg) {
        return { Severity::Error, std::move(msg), {} };
    }
    static Diagnostic warning(std::string msg) {
        return { Severity::Warning, std::move(msg), {} };
    }

    Diagnostic& with_label(Span span, std::string msg) {
        labels.push_back({ span, std::move(msg) });
        return *this;
    }
};

} // namespace floe
