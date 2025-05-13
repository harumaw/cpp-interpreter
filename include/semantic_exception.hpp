#pragma once
#include <exception>
#include <string>

class SemanticException : public std::exception {
public:
    explicit SemanticException(const std::string& message)
        : _message(message) {}

    const char* what() const noexcept override {
        return _message.c_str();
    }

private:
    std::string _message;
};
