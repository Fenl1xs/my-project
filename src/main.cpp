#include <iostream>
#include <string>
#include "rpn.h"

int main() {
    std::string expression;
    std::cout << "Enter RPN expression: ";
    std::getline(std::cin, expression);

    size_t start = expression.find_first_not_of(" \t");
    if (start == std::string::npos) {
        std::cout << "Empty expression" << std::endl;
        return 0;
    }
    size_t end = expression.find_last_not_of(" \t");
    expression = expression.substr(start, end - start + 1);

    if (expression.empty()) {
        std::cout << "Empty expression" << std::endl;
        return 0;
    }

    try {
        double result = evaluate_rpn(expression);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
