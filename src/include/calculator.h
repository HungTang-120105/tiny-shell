#pragma once

#include <string>
#include <vector>
#include <stdexcept> // For std::runtime_error

namespace TinyCalculator {

// Represents a token in an expression
struct Token {
    enum class Type { 
        NUMBER, 
        OPERATOR,       // Binary operators like +, -, *, /, %, ^
        UNARY_OPERATOR, // Unary operators like negation (-), factorial (!)
        LEFT_PAREN, 
        RIGHT_PAREN, 
        FUNCTION,       // sin, cos, log, etc.
        CONSTANT,       // pi, e
        COMMA,          // Argument separator for functions (future use)
        VARIABLE,       // (Future use for function definition)
        UNKNOWN 
    };
    Type type;
    std::string value; // For numbers, operator symbols, function names, variable names, constants
    int precedence;    // For operators (binary and unary)
    bool left_associative; // For operators
    int arg_count;     // For functions (number of arguments expected)

    Token(Type t, std::string val = "", int prec = 0, bool left_assoc = true, int args = 0)
        : type(t), value(val), precedence(prec), left_associative(left_assoc), arg_count(args) {}
};

// Tokenizes an infix expression string.
std::vector<Token> tokenize(const std::string& expression);

// Converts a tokenized infix expression to postfix (RPN) using Shunting-Yard.
std::vector<Token> shunting_yard(const std::vector<Token>& infix_tokens);

// Evaluates a postfix (RPN) expression.
double evaluate_postfix(const std::vector<Token>& postfix_tokens);

// Main function to calculate an infix expression string.
double calculate_expression(const std::string& expression);

} // namespace TinyCalculator
