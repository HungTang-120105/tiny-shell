#include "../include/calculator.h"
#include <stack>
#include <cmath>     // For std::stod, std::pow, trig functions, M_PI, M_E etc.
#include <algorithm> 
#include <map>     
#include <sstream> 
#include <iomanip>   // For std::setprecision in output (though not directly in calc logic)

// Define M_E and M_PI if not defined by cmath (some compilers might not define them without _USE_MATH_DEFINES)
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif
#ifndef M_E
    #define M_E 2.71828182845904523536
#endif

namespace TinyCalculator {

// Helper to check if a character can be part of an operator or is a parenthesis/comma
bool is_operator_char_or_paren(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^' || c == '!' || c == '(' || c == ')' || c == ',';
}

// Corrected is_number from previous step
bool is_number(const std::string& s) {
    if (s.empty()) return false;
    std::size_t pos = 0;
    try {
        std::stod(s, &pos);
        return pos == s.length();
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

// Operator properties (now includes unary and new binary ops)
struct OperatorInfo {
    int precedence;
    bool left_associative;
    Token::Type type; // To distinguish between OPERATOR and UNARY_OPERATOR
};

const std::map<std::string, OperatorInfo>& get_operator_info_map() {
    static std::map<std::string, OperatorInfo> props;
    if (props.empty()) { // Initialize only once
        // Binary Operators
        props["+"] = {2, true, Token::Type::OPERATOR};
        props["-"] = {2, true, Token::Type::OPERATOR};
        props["*"] = {3, true, Token::Type::OPERATOR};
        props["/"] = {3, true, Token::Type::OPERATOR};
        props["%"] = {3, true, Token::Type::OPERATOR};       // Modulo
        props["^"] = {5, false, Token::Type::OPERATOR};      // Exponentiation (right-associative)
        
        // Unary Operators (tokenized with these specific names)
        // The tokenizer will map a leading '-' to "neg" and a leading '+' to "pos"
        props["neg"] = {4, false, Token::Type::UNARY_OPERATOR}; // Unary minus (right-assoc, higher precedence than * /)
        props["pos"] = {4, false, Token::Type::UNARY_OPERATOR}; // Unary plus (same as neg)
        props["!"]   = {6, true, Token::Type::UNARY_OPERATOR}; // Factorial (postfix, but precedence helps in Shunting if treated as op)
                                                              // Factorial is typically handled specially due to its postfix nature.
                                                              // Let's treat it as a high-precedence left-associative unary op for now.
    }
    return props;
}

// Function information (name and argument count)
struct FunctionInfo {
    int arg_count;
    // Potentially add a pointer to the actual function later: double (*func_ptr)(double); or std::function
};

const std::map<std::string, FunctionInfo>& get_function_info_map() {
    static std::map<std::string, FunctionInfo> funcs;
    if (funcs.empty()) {
        funcs["sqrt"] = {1};
        funcs["sin"]  = {1};
        funcs["cos"]  = {1};
        funcs["tan"]  = {1};
        funcs["cot"]  = {1};
        funcs["ln"]   = {1}; // Natural log
        funcs["log10"]= {1}; // Base 10 log
        funcs["log2"] = {1}; // Base 2 log
        // log8 and log16 can be done via change of base: log_b(x) = ln(x)/ln(b)
        // Or add them if explicit functions are desired
        funcs["log8"] = {1};
        funcs["log16"]= {1};
    }
    return funcs;
}

// Constants map
const std::map<std::string, double>& get_constants_map() {
    static std::map<std::string, double> consts;
    if (consts.empty()) {
        consts["pi"] = M_PI;
        consts["e"]  = M_E;
    }
    return consts;
}

std::vector<Token> tokenize(const std::string& expression) {
    std::vector<Token> tokens;
    std::string current_token_str;
    const auto& op_map = get_operator_info_map();
    const auto& func_map = get_function_info_map();
    const auto& const_map = get_constants_map();

    for (size_t i = 0; i < expression.length(); ++i) {
        char c = expression[i];
        Token::Type prev_token_type = tokens.empty() ? Token::Type::UNKNOWN : tokens.back().type;

        if (std::isspace(c)) {
            continue; 
        } else if (std::isdigit(c) || (c == '.' && (i + 1 < expression.length() && std::isdigit(expression[i+1])) && (current_token_str.empty() || is_number(current_token_str)) ) ) {
            // Start or continuation of a number
            current_token_str += c;
            while (i + 1 < expression.length() && (std::isdigit(expression[i+1]) || (expression[i+1] == '.' && current_token_str.find('.') == std::string::npos) )) {
                current_token_str += expression[++i];
            }
            if (!is_number(current_token_str)) {
                 throw std::runtime_error("Invalid number format: '" + current_token_str + "'");
            }
            tokens.push_back(Token(Token::Type::NUMBER, current_token_str));
            current_token_str.clear();
        } else if (c == '-' || c == '+') { // Potential unary or binary operator
            bool is_unary = tokens.empty() || 
                            prev_token_type == Token::Type::OPERATOR || 
                            prev_token_type == Token::Type::UNARY_OPERATOR || 
                            prev_token_type == Token::Type::LEFT_PAREN ||
                            prev_token_type == Token::Type::COMMA;
            if (is_unary) {
                std::string op_val = (c == '-') ? "neg" : "pos";
                auto it = op_map.find(op_val);
                tokens.push_back(Token(Token::Type::UNARY_OPERATOR, op_val, it->second.precedence, it->second.left_associative));
            } else {
                std::string op_val = std::string(1, c);
                auto it = op_map.find(op_val);
                tokens.push_back(Token(Token::Type::OPERATOR, op_val, it->second.precedence, it->second.left_associative));
            }
        } else if (is_operator_char_or_paren(c)) { // Other operators, parentheses, comma
             std::string op_val = std::string(1, c);
            if (c == '(') {
                tokens.push_back(Token(Token::Type::LEFT_PAREN, "("));
            } else if (c == ')') {
                tokens.push_back(Token(Token::Type::RIGHT_PAREN, ")"));
            } else if (c == ',') {
                tokens.push_back(Token(Token::Type::COMMA, ","));
            } else { // Must be an operator like *, /, %, ^, !
                auto it = op_map.find(op_val);
                if (it == op_map.end()) throw std::runtime_error("Unknown operator symbol: '" + op_val + "'");
                // Factorial is postfix, special handling here? For now, treat as other ops.
                // If '!' is found, its type should be UNARY_OPERATOR from the map.
                tokens.push_back(Token(it->second.type, op_val, it->second.precedence, it->second.left_associative));
            }
        } else if (std::isalpha(c)) { // Functions or Constants
            current_token_str += c;
            while (i + 1 < expression.length() && (std::isalnum(expression[i+1]) || expression[i+1] == '_')) {
                current_token_str += expression[++i];
            }
            if (func_map.count(current_token_str)) {
                tokens.push_back(Token(Token::Type::FUNCTION, current_token_str, 0, true, func_map.at(current_token_str).arg_count));
            } else if (const_map.count(current_token_str)) {
                tokens.push_back(Token(Token::Type::CONSTANT, current_token_str)); 
            } else {
                throw std::runtime_error("Unknown function or constant: '" + current_token_str + "'");
            }
            current_token_str.clear();
        } else {
            throw std::runtime_error("Unexpected character in expression: '" + std::string(1, c) + "'");
        }
    }
    return tokens;
}

// ... (Shunting-Yard and Evaluate Postfix will need significant updates) ...
// ... (calculate_expression remains the same structurally) ...

std::vector<Token> shunting_yard(const std::vector<Token>& infix_tokens) {
    std::vector<Token> output_queue;
    std::stack<Token> operator_stack;
    const auto& op_info_provider = get_operator_info_map(); // For properties
    const auto& func_info_provider = get_function_info_map(); // For properties

    for (const auto& token : infix_tokens) {
        switch (token.type) {
            case Token::Type::NUMBER:
            case Token::Type::CONSTANT:
                output_queue.push_back(token);
                break;

            case Token::Type::FUNCTION:
                operator_stack.push(token);
                break;

            case Token::Type::OPERATOR:
            case Token::Type::UNARY_OPERATOR: { // Both handled similarly based on properties
                // Assume token itself now carries its properties from tokenizer, or fetch them.
                // The tokenizer should fill precedence and associativity for ops.
                // Let's re-fetch from map to be safe, or ensure tokenizer does it.
                // For simplicity, assume the token passed in has .precedence and .left_associative set correctly by tokenizer if it's an operator.
                // The current tokenizer above sets precedence for unary +/- but not other operators yet.
                // This part needs careful re-evaluation based on how tokenizer populates Token objects for operators.
                // For now: assume token.precedence and token.left_associative are correctly set for OPERATOR and UNARY_OPERATOR types.
                // This implies tokenizer needs to use get_operator_info_map for all ops.
                
                // Re-doing logic from Phase 1 and adapting:
                OperatorInfo current_op_info = op_info_provider.at(token.value);

                while (!operator_stack.empty()) {
                    Token top_op = operator_stack.top();
                    if (top_op.type == Token::Type::LEFT_PAREN) break;

                    OperatorInfo stacked_op_info;                    
                    if (top_op.type == Token::Type::OPERATOR || top_op.type == Token::Type::UNARY_OPERATOR) {
                        stacked_op_info = op_info_provider.at(top_op.value);
                    } else if (top_op.type == Token::Type::FUNCTION) {
                        // Functions always have higher precedence than operators when on stack vs incoming operator
                        // Or rather, an incoming operator should pop functions.
                        // This needs to be thought through carefully. Usually, functions are popped by ')' or end of expression.
                        // Let's assume operators do not pop functions from stack unless it's an operator of higher precedence.
                        // Standard Shunting Yard: if o1 is an operator and o2 is a function, then o1 is popped.
                        // Let's treat functions like operators with very high precedence for this rule.
                         output_queue.push_back(top_op);
                         operator_stack.pop();
                         continue;
                    } else {
                         break; // Should not happen if stack only has ops, funcs, or (
                    }
                    
                    if ((current_op_info.left_associative && current_op_info.precedence <= stacked_op_info.precedence) ||
                        (!current_op_info.left_associative && current_op_info.precedence < stacked_op_info.precedence)) {
                        output_queue.push_back(top_op);
                        operator_stack.pop();
                    } else {
                        break;
                    }
                }
                operator_stack.push(Token(current_op_info.type, token.value, current_op_info.precedence, current_op_info.left_associative));
                break;
            }

            case Token::Type::COMMA: // For function arguments
                // Pop operators until a left parenthesis is found. Error if not found.
                while (!operator_stack.empty() && operator_stack.top().type != Token::Type::LEFT_PAREN) {
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                if (operator_stack.empty()) {
                    throw std::runtime_error("Misplaced comma or missing left parenthesis for function arguments.");
                }
                break;

            case Token::Type::LEFT_PAREN:
                operator_stack.push(token);
                break;

            case Token::Type::RIGHT_PAREN: {
                bool found_left_paren = false;
                while (!operator_stack.empty()) {
                    if (operator_stack.top().type == Token::Type::LEFT_PAREN) {
                        operator_stack.pop(); // Discard left paren
                        found_left_paren = true;
                        break;
                    }
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                if (!found_left_paren) {
                    throw std::runtime_error("Mismatched parentheses: missing '('.");
                }
                // If token at top of stack is a function, pop it to output queue.
                if (!operator_stack.empty() && operator_stack.top().type == Token::Type::FUNCTION) {
                    output_queue.push_back(operator_stack.top());
                    operator_stack.pop();
                }
                break;
            }
            default:
                throw std::runtime_error("Unsupported token type in Shunting-Yard: " + token.value);
        }
    }

    while (!operator_stack.empty()) {
        Token top_op = operator_stack.top();
        if (top_op.type == Token::Type::LEFT_PAREN || top_op.type == Token::Type::RIGHT_PAREN) {
            throw std::runtime_error("Mismatched parentheses on stack at end.");
        }
        output_queue.push_back(top_op);
        operator_stack.pop();
    }
    return output_queue;
}

double evaluate_postfix(const std::vector<Token>& postfix_tokens) {
    std::stack<double> eval_stack;
    const auto& const_map = get_constants_map();
    const auto& op_map = get_operator_info_map(); // For type check
    const auto& func_map = get_function_info_map(); // For arg count

    for (const auto& token : postfix_tokens) {
        if (token.type == Token::Type::NUMBER) {
            try {
                eval_stack.push(std::stod(token.value));
            } catch (const std::exception& e) {
                throw std::runtime_error("Invalid number during evaluation ('" + token.value + "'): " + e.what());
            }
        } else if (token.type == Token::Type::CONSTANT) {
            eval_stack.push(const_map.at(token.value));
        } else if (token.type == Token::Type::OPERATOR) { // Binary operators
            if (eval_stack.size() < 2) throw std::runtime_error("Invalid RPN: insufficient operands for binary op '" + token.value + "'");
            double val2 = eval_stack.top(); eval_stack.pop();
            double val1 = eval_stack.top(); eval_stack.pop();
            double result = 0.0;

            if (token.value == "+") result = val1 + val2;
            else if (token.value == "-") result = val1 - val2;
            else if (token.value == "*") result = val1 * val2;
            else if (token.value == "/") {
                if (val2 == 0.0) throw std::runtime_error("Division by zero.");
                result = val1 / val2;
            } else if (token.value == "%") {
                if (val2 == 0.0) throw std::runtime_error("Modulo by zero.");
                result = std::fmod(val1, val2);
            } else if (token.value == "^") {
                result = std::pow(val1, val2);
            } else {
                throw std::runtime_error("Unknown binary operator in RPN: '" + token.value + "'");
            }
            eval_stack.push(result);
        } else if (token.type == Token::Type::UNARY_OPERATOR) {
            if (eval_stack.empty()) throw std::runtime_error("Invalid RPN: insufficient operands for unary op '" + token.value + "'");
            double val = eval_stack.top(); eval_stack.pop();
            double result = 0.0;
            if (token.value == "neg") result = -val;
            else if (token.value == "pos") result = +val; // or just val
            else if (token.value == "!") { // Factorial
                if (val < 0.0 || val != static_cast<long long>(val)) throw std::runtime_error("Factorial undefined for non-integer or negative numbers.");
                if (val > 20) throw std::runtime_error("Factorial input too large (overflow risk for double)."); // Practical limit
                result = 1.0;
                for (long long i = 1; i <= static_cast<long long>(val); ++i) result *= i;
            } else {
                throw std::runtime_error("Unknown unary operator in RPN: '" + token.value + "'");
            }
            eval_stack.push(result);
        } else if (token.type == Token::Type::FUNCTION) {
            FunctionInfo f_info = func_map.at(token.value);
            if (eval_stack.size() < static_cast<size_t>(f_info.arg_count)) throw std::runtime_error("Invalid RPN: insufficient operands for func '" + token.value + "'");
            // All current functions take 1 argument
            double arg = eval_stack.top(); eval_stack.pop();
            double result = 0.0;
                 if (token.value == "sqrt") { if(arg < 0) throw std::runtime_error("sqrt of negative number"); result = std::sqrt(arg); }
            else if (token.value == "sin")  result = std::sin(arg);
            else if (token.value == "cos")  result = std::cos(arg);
            else if (token.value == "tan")  result = std::tan(arg);
            else if (token.value == "cot")  { if(std::tan(arg)==0) throw std::runtime_error("cot undefined (tan is zero)"); result = 1.0/std::tan(arg); }
            else if (token.value == "ln")   { if(arg <= 0) throw std::runtime_error("ln of non-positive number"); result = std::log(arg); }
            else if (token.value == "log10"){ if(arg <= 0) throw std::runtime_error("log10 of non-positive number"); result = std::log10(arg); }
            else if (token.value == "log2") { if(arg <= 0) throw std::runtime_error("log2 of non-positive number"); result = std::log2(arg); }
            else if (token.value == "log8") { if(arg <= 0) throw std::runtime_error("log8 of non-positive number"); result = std::log(arg) / std::log(8.0); }
            else if (token.value == "log16"){ if(arg <= 0) throw std::runtime_error("log16 of non-positive number"); result = std::log(arg) / std::log(16.0); }
            else { throw std::runtime_error("Unknown function in RPN: '" + token.value + "'");}
            eval_stack.push(result);
        }
        else {
             throw std::runtime_error("Unsupported token in RPN: '" + token.value + "' type: " + std::to_string(static_cast<int>(token.type)));
        }
    }

    if (eval_stack.size() != 1) {
        std::stringstream err_ss;
        err_ss << "Invalid RPN result. Stack size: " << eval_stack.size() << ". Expected 1.";
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
             throw std::runtime_error("Tokenization resulted in no tokens.");
        }
        std::vector<Token> postfix = shunting_yard(tokens);
        if (postfix.empty() && !tokens.empty()) { 
             throw std::runtime_error("Expression resulted in empty postfix (e.g., empty parentheses).");
        }
         if (postfix.empty() && tokens.empty()){ 
             throw std::runtime_error("Cannot evaluate empty expression.");
        }
        return evaluate_postfix(postfix);
    } catch (const std::runtime_error& e) {
        // Consider logging the original expression for better debugging
        throw std::runtime_error(std::string("Calculation error processing '" + expression + "': ") + e.what());
    }
}


} // namespace TinyCalculator
