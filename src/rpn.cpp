#include <gtest/gtest.h>
#include "rpn.h"
#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>
#include <cmath>
#include <map>
#include <ctime>
#include <random>
#include <algorithm>
#include <functional>
#include <deque>

std::map<std::string, double> variables;
std::stack<std::stack<double>> undo_stack;
std::stack<std::stack<double>> redo_stack;
std::deque<std::string> history_deque;
const int HISTORY_SIZE = 10;

const double PI = 3.14159265358979323846;
const double E = 2.71828182845904523536;

std::mt19937 gen(std::time(0));
std::uniform_real_distribution<double> dis(0.0, 1.0);

std::vector<std::string> tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::istringstream iss(expression);
    std::string token;
    std::string cleaned_expression = expression;

    size_t comment_pos = cleaned_expression.find('#');
    if (comment_pos != std::string::npos) {
        cleaned_expression = cleaned_expression.substr(0, comment_pos);
    }

    iss.str(cleaned_expression);
    
    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

bool is_number(const std::string& s) {
    if (s.empty()) return false;

    size_t start = 0;
    if (s[0] == '-' || s[0] == '+') {
        if (s.size() == 1) return false;
        start = 1;
    }

    bool has_dot = false;
    for (size_t i = start; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (has_dot) return false;
            has_dot = true;
        } else if (!std::isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

void save_state(const std::stack<double>& stack) {
    undo_stack.push(stack);
    while (!redo_stack.empty()) {
        redo_stack.pop();
    }
}

void add_to_history(const std::string& operation) {
    history_deque.push_front(operation);
    if (history_deque.size() > HISTORY_SIZE) {
        history_deque.pop_back();
    }
}

double evaluate_rpn(const std::string& expression) {
    std::stack<double> stack;
    std::vector<std::string> tokens = tokenize(expression);


    save_state(stack);

    for (const auto& token : tokens) {
        if (is_number(token)) {
            stack.push(std::stod(token));
            add_to_history("push " + token);
        } else if (token == "pi") {
            stack.push(PI);
            add_to_history("push pi");
        } else if (token == "e") {
            stack.push(E);
            add_to_history("push e");
        } else if (variables.find(token) != variables.end()) {
            stack.push(variables[token]);
            add_to_history("recall " + token);
        } else if (token == "store" || token == ">") {
            if (stack.size() < 2) throw std::invalid_argument("Not enough operands for store");
            double value = stack.top(); stack.pop();
            std::string var_name = std::to_string(static_cast<int>(stack.top())); 
            stack.pop(); 
            variables[var_name] = value;
            add_to_history("store " + var_name + " = " + std::to_string(value));
        } else if (token == "recall" || token == "<") {
            if (stack.empty()) throw std::invalid_argument("No variable name specified");
            std::string var_name = std::to_string(static_cast<int>(stack.top()));
            stack.pop();
            if (variables.find(var_name) == variables.end()) {
                throw std::invalid_argument("Variable '" + var_name + "' not found");
            }
            stack.push(variables[var_name]);
            add_to_history("recall " + var_name);
        } else if (token == "rand") {
            stack.push(dis(gen));
            add_to_history("rand");
        } else if (token == "dup") {
            if (stack.empty()) throw std::invalid_argument("Stack is empty");
            stack.push(stack.top());
            add_to_history("dup");
        } else if (token == "swap") {
            if (stack.size() < 2) throw std::invalid_argument("Not enough elements to swap");
            double a = stack.top(); stack.pop();
            double b = stack.top(); stack.pop();
            stack.push(a);
            stack.push(b);
            add_to_history("swap");
        } else if (token == "drop") {
            if (stack.empty()) throw std::invalid_argument("Stack is empty");
            stack.pop();
            add_to_history("drop");
        } else if (token == "clear") {
            while (!stack.empty()) stack.pop();
            add_to_history("clear");
        } else if (token == "undo") {
            if (undo_stack.empty()) throw std::invalid_argument("Nothing to undo");
            redo_stack.push(stack);
            stack = undo_stack.top();
            undo_stack.pop();
            add_to_history("undo");
        } else if (token == "redo") {
            if (redo_stack.empty()) throw std::invalid_argument("Nothing to redo");
            undo_stack.push(stack);
            stack = redo_stack.top();
            redo_stack.pop();
            add_to_history("redo");
        } else {
           
            if (token == "~") { 
                if (stack.empty()) throw std::invalid_argument("Stack is empty");
                double a = stack.top(); stack.pop();
                stack.push(-a);
                add_to_history("~");
            } else if (token == "++") { 
                if (stack.empty()) throw std::invalid_argument("Stack is empty");
                double a = stack.top(); stack.pop();
                stack.push(a + 1);
                add_to_history("++");
            } else if (token == "--") { 
                if (stack.empty()) throw std::invalid_argument("Stack is empty");
                double a = stack.top(); stack.pop();
                stack.push(a - 1);
                add_to_history("--");
            } else if (token == "!") { 
                if (stack.empty()) throw std::invalid_argument("Stack is empty");
                double a = stack.top(); stack.pop();
                if (a < 0 || a != static_cast<int>(a)) {
                    throw std::invalid_argument("Factorial requires non-negative integer");
                }
                double result = 1;
                for (int i = 2; i <= static_cast<int>(a); ++i) {
                    result *= i;
                }
                stack.push(result);
                add_to_history("!");
            } else if (token == "+" || token == "-" || token == "*" || token == "/" || 
                      token == "^" || token == "%" || token == "min" || token == "max" ||
                      token == "==" || token == "!=" || token == "<" || token == ">" || 
                      token == "<=" || token == ">=") {
               
                if (stack.size() < 2) {
                    throw std::invalid_argument("Not enough operands for operator '" + token + "'");
                }

                double b = stack.top(); stack.pop();
                double a = stack.top(); stack.pop();
                double result;

                if (token == "+") result = a + b;
                else if (token == "-") result = a - b;
                else if (token == "*") result = a * b;
                else if (token == "/") {
                    if (b == 0.0) throw std::invalid_argument("Division by zero");
                    result = a / b;
                } else if (token == "^") result = std::pow(a, b);
                else if (token == "%") result = std::fmod(a, b);
                else if (token == "min") result = std::min(a, b);
                else if (token == "max") result = std::max(a, b);
                else if (token == "==") result = (a == b) ? 1.0 : 0.0;
                else if (token == "!=") result = (a != b) ? 1.0 : 0.0;
                else if (token == "<") result = (a < b) ? 1.0 : 0.0;
                else if (token == ">") result = (a > b) ? 1.0 : 0.0;
                else if (token == "<=") result = (a <= b) ? 1.0 : 0.0;
                else if (token == ">=") result = (a >= b) ? 1.0 : 0.0;

                stack.push(result);
                add_to_history(token);
            } else if (token == "sqrt" || token == "abs" || token == "sin" || token == "cos" ||
                      token == "tan" || token == "asin" || token == "acos" || token == "atan" ||
                      token == "ln" || token == "log" || token == "floor" || token == "ceil" ||
                      token == "round" || token == "bin" || token == "hex") {
              
                if (stack.empty()) throw std::invalid_argument("Stack is empty");
                double a = stack.top(); stack.pop();
                double result;

                if (token == "sqrt") {
                    if (a < 0) throw std::invalid_argument("Square root of negative number");
                    result = std::sqrt(a);
                } else if (token == "abs") result = std::abs(a);
                else if (token == "sin") result = std::sin(a);
                else if (token == "cos") result = std::cos(a);
                else if (token == "tan") result = std::tan(a);
                else if (token == "asin") {
                    if (a < -1 || a > 1) throw std::invalid_argument("asin argument out of range");
                    result = std::asin(a);
                } else if (token == "acos") {
                    if (a < -1 || a > 1) throw std::invalid_argument("acos argument out of range");
                    result = std::acos(a);
                } else if (token == "atan") result = std::atan(a);
                else if (token == "ln") {
                    if (a <= 0) throw std::invalid_argument("ln of non-positive number");
                    result = std::log(a);
                } else if (token == "log") {
                    if (a <= 0) throw std::invalid_argument("log of non-positive number");
                    result = std::log10(a);
                } else if (token == "floor") result = std::floor(a);
                else if (token == "ceil") result = std::ceil(a);
                else if (token == "round") result = std::round(a);
                else if (token == "bin" || token == "hex") {

                    result = a;
                }

                stack.push(result);
                add_to_history(token);
            } else if (token == "stack") {
          
                add_to_history("stack");
            } else if (token == "vars") {
                add_to_history("vars");
            } else if (token == "history") {
                add_to_history("history");
            } else {
                throw std::invalid_argument("Invalid operator or function: '" + token + "'");
            }
        }
    }

    if (stack.size() != 1 && !tokens.empty() && tokens.back() != "stack" && 
        tokens.back() != "vars" && tokens.back() != "history") {
        throw std::invalid_argument("Invalid expression: too many operands");
    }

    return stack.empty() ? 0.0 : stack.top();
}


std::string get_stack_contents(const std::stack<double>& stack) {
    std::stack<double> temp = stack;
    std::vector<double> elements;
    
    while (!temp.empty()) {
        elements.push_back(temp.top());
        temp.pop();
    }
    
    std::reverse(elements.begin(), elements.end());
    std::string result;
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) result += " ";
        result += std::to_string(elements[i]);
    }
    return result;
}

std::string get_vars_contents() {
    std::string result;
    for (const auto& [key, value] : variables) {
        if (!result.empty()) result += ", ";
        result += key + " = " + std::to_string(value);
    }
    return result.empty() ? "No variables defined" : result;
}

std::string get_history_contents() {
    if (history_deque.empty()) return "No history";
    
    std::string result;
    for (size_t i = 0; i < history_deque.size(); ++i) {
        result += std::to_string(i + 1) + ". " + history_deque[i] + "\n";
    }
    return result;
}

std::string number_to_bin(double num) {
    int n = static_cast<int>(num);
    if (n == 0) return "0";
    
    std::string result;
    while (n > 0) {
        result = (n % 2 == 0 ? "0" : "1") + result;
        n /= 2;
    }
    return result;
}

std::string number_to_hex(double num) {
    int n = static_cast<int>(num);
    if (n == 0) return "0";
    
    std::string result;
    const char* digits = "0123456789ABCDEF";
    
    while (n > 0) {
        result = digits[n % 16] + result;
        n /= 16;
    }
    return result;
}