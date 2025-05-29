#include "type.hpp"

struct Symbol {
    std::shared_ptr<Type> type;

};
// ta zhe structura kak v type
struct IntegerSymbol : public Symbol {
    int value;

    IntegerSymbol(int v) : value(v) {}
};

struct FloatSymbol : public Symbol {
    float value;

    FloatSymbol(float v) : value(v) {}
};

struct StringSymbol : public Symbol {
    std::string value;

    StringSymbol(const std::string& v) : value(v) {}
};

struct BoolSymbol : public Symbol {
    bool value;

    BoolSymbol(bool v) : value(v) {}
};
// symbol - type, value
// v analizatore nuzhno proveryat type, a v executor value