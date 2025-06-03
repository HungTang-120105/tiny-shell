#include "../include/calculator.h"
#include <stack>
#include <cmath> // For std::stod, std::pow (later)
#include <algorithm> // For std::remove_if
#include <map>     // For operator properties
#include <sstream> // For string to double conversion and error messages

namespace TinyCalculator {

// Helper to check if a character is an operator
bool is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '!';
}

// Helper to check if a string is a number (CORRECTED)
bool is_number(const std::string& s) {
    if (s.empty()) return false;
    std::size_t pos = 0;
    try {
        std::stod(s, &pos);
        // The conversion was successful if stod did not throw an exception
        // and if all characters in the string were processed.
        return pos == s.length();
    } catch (const std::invalid_argument&) {
        // This means no conversion could be performed (e.g., "abc", or just ".")
        return false;
    } catch (const std::out_of_range&) {
        // This means the number was valid in format but out of double's range
        return false;
    }
}


// Operator properties
struct OperatorProperties {
    int precedence;
    bool left_associative;
};

std::map<std::string, OperatorProperties> get_operator_properties_map() { // Renamed to avoid conflict
    std::map<std::string, OperatorProperties> props;
    // Phase 1: Basic operators
    props["+"] = {2, true};
    props["-"] = {2, true};
    props["*"] = {3, true};
    props["/"] = {3, true};
    // Phase 2 operators will be added here (%, ^)
    // Unary minus/plus will be handled by a transformation or special token type if needed
    return props;
}

// std::map<std::string, int> get_function_arg_count() { // Not used in Phase 1
//     std::map<std::string, int> counts;
//     counts["sqrt"] = 1;
//     return counts;
// }


std::vector<Token> tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    std::string current_token_str;

    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];

        if (std::isspace(c)) {
            continue; // Skip whitespace
        } else if (std::isdigit(c) || c == '.') {
            current_token_str += c;
            // Greedily consume digits and one optional decimal point.
            // This loop needs to be careful not to over-consume or form invalid numbers like "1.2.3"
            // The current loop is fine for forming a candidate string, is_number will validate.
            while (i + 1 < expression.length() && (std::isdigit(expression[i+1]) || expression[i+1] == '.')) {
                // Only add if it potentially forms a valid number part. Avoid adding a second decimal.
                // if (expression[i+1] == '.' && current_token_str.find('.') != std::string::npos) {
                //     break; // Already have a decimal, stop here.
                // }
                current_token_str += expression[++i];
            }
            
            if (!is_number(current_token_str)) { // Validation of the formed number string
                 throw std::runtime_error("Invalid number format: '" + current_token_str + "'");
            }
            tokens.push_back(Token(Token::Type::NUMBER, current_token_str));
            current_token_str.clear();
        } else if (is_operator_char(c)) {
            tokens.push_back(Token(Token::Type::OPERATOR, std::string(1, c)));
        } else if (c == '(') {
            tokens.push_back(Token(Token::Type::LEFT_PAREN, "("));
        } else if (c == ')') {
            tokens.push_back(Token(Token::Type::RIGHT_PAREN, ")"));
        } else if (std::isalpha(c)) { 
            current_token_str += c;
            while (i + 1 < expression.length() && (std::isalnum(expression[i+1]) || expression[i+1] == '_')) {
                current_token_str += expression[++i];
            }
            throw std::runtime_error("Functions/variables ('" + current_token_str + "') not supported in this version.");
        } else {
            throw std::runtime_error("Unknown character in expression: '" + std::string(1, c) + "'");
        }
    }
    return tokens;
}

std::vector<Token> shunting_yard(const std::vector<Token>& infix_tokens) {
    std::vector<Token> output_queue;
    std::stack<Token> operator_stack;
    auto op_props_map = get_operator_properties_map();

    for (const auto& token : infix_tokens) {
        switch (token.type) {
            case Token::Type::NUMBER:
                output_queue.push_back(token);
                break;
            case Token::Type::OPERATOR: {
                auto it = op_props_map.find(token.value);
                if (it == op_props_map.end()) throw std::runtime_error("Unknown operator: " + token.value);
                int current_op_prec = it->second.precedence;
                // bool current_op_left_assoc = it->second.left_associative; // Already stored in token if created with props

                // Create token with full properties for stack consistency
                Token current_op_token = Token(token.type, token.value, it->second.precedence, it->second.left_associative);

                while (!operator_stack.empty() &&
                       operator_stack.top().type == Token::Type::OPERATOR &&
                       ( (operator_stack.top().left_associative && operator_stack.top().precedence >= current_op_token.precedence) ||
                         (!operator_stack.top().left_associative && operator_stack.top().precedence > current_op_token.precedence) ) ) {
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                operator_stack.push(current_op_token);
                break;
            }
            case Token::Type::LEFT_PAREN:
                operator_stack.push(token);
                break;
            case Token::Type::RIGHT_PAREN: {
                bool found_left_paren = false;
                while (!operator_stack.empty()) {
                    if (operator_stack.top().type == Token::Type::LEFT_PAREN) {
                        operator_stack.pop(); 
                        found_left_paren = true;
                        break;
                    } else {
                        output_queue.push_back(operator_stack.top());
                        operator_stack.pop();
                    }
                }
                if (!found_left_paren) {
                    throw std::runtime_error("Mismatched parentheses: missing '('.");
                }
                break;
            }
            default:
                throw std::runtime_error("Unsupported token type in Shunting-Yard: " + token.value);
        }
    }

    while (!operator_stack.empty()) {
        if (operator_stack.top().type == Token::Type::LEFT_PAREN || operator_stack.top().type == Token::Type::RIGHT_PAREN) {
            throw std::runtime_error("Mismatched parentheses on stack at end.");
        }
        output_queue.push_back(operator_stack.top());
        operator_stack.pop();
    }
    return output_queue;
}


double evaluate_postfix(const std::vector<Token>& postfix_tokens) {
    std::stack<double> eval_stack;

    for (const auto& token : postfix_tokens) {
        if (token.type == Token::Type::NUMBER) {
            try {
                eval_stack.push(std::stod(token.value));
            } catch (const std::exception& e) { // Catching std::exception for broader coverage
                throw std::runtime_error("Invalid number during evaluation ('" + token.value + "'): " + e.what());
            }
        } else if (token.type == Token::Type::OPERATOR) {
            if (eval_stack.size() < 2) { 
                throw std::runtime_error("Invalid postfix: insufficient operands for '" + token.value + "'");
            }
            double val2 = eval_stack.top(); eval_stack.pop();
            double val1 = eval_stack.top(); eval_stack.pop();
            double result = 0.0;

            if (token.value == "+") result = val1 + val2;
            else if (token.value == "-") result = val1 - val2;
            else if (token.value == "*") result = val1 * val2;
            else if (token.value == "/") {
                if (val2 == 0.0) throw std::runtime_error("Division by zero.");
                result = val1 / val2;
            }
            else {
                throw std::runtime_error("Unknown operator in postfix: '" + token.value + "'");
            }
            eval_stack.push(result);
        } else {
             throw std::runtime_error("Unsupported token in postfix: '" + token.value + "'");
        }
    }

    if (eval_stack.size() != 1) {
        std::stringstream err_ss;
        err_ss << "Invalid postfix expression result. Stack size: " << eval_stack.size() << ". Expected 1.";
        if (!eval_stack.empty()) err_ss << " Top: " << eval_stack.top();
        throw std::runtime_error(err_ss.str());
    }
    return eval_stack.top();
}

double calculate_expression(const std::string& expression) {
    if (expression.empty() || std::all_of(expression.begin(), expression.end(), ::isspace)) {
        throw std::runtime_error("Expression is empty or contains only whitespace.");
    }
    try {
        std::vector<Token> tokens = tokenize(expression);
        if (tokens.empty()) { 
             throw std::runtime_error("Failed to tokenize non-empty expression. Resulted in no tokens.");
        }
        std::vector<Token> postfix = shunting_yard(tokens);
        if (postfix.empty() && !tokens.empty()) { // e.g. if expression was just "()"
             throw std::runtime_error("Expression resulted in empty postfix (e.g., empty parentheses).");
        }
        if (postfix.empty() && tokens.empty()){ // Should be caught by initial check, but for safety
             throw std::runtime_error("Cannot evaluate empty expression.");
        }
        return evaluate_postfix(postfix);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(std::string("Calculation error: ") + e.what());
    }
}

} // namespace TinyCalculator 