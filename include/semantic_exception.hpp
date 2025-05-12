#include <exception>
#include <string>

struct SemanticException : public std::exception {
    std::string message;
    explicit SemanticException(std::string msg)
        : message("Semantic error: " + std::move(msg)) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

};