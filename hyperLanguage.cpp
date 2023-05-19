#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <locale.h>
#include <cmath>
#include <regex>

std::map<std::string, std::string> variables;
std::map<std::string, bool> booleanVariables;
std::map<std::string, std::vector<std::string>> functions;

class Interpreter {
public:
    void interpret(const std::vector<std::string>& tokens) {
        if (tokens.empty()) {
            return;
        }

        const std::string& command = tokens[0];
        if (command == "if") {
            interpretIf(tokens);
        }
        else if (command == "let") {
            interpretLet(tokens);
        }
        else if (command == "print") {
            interpretPrint(tokens);
        }
        else if (command == "foreach") {
            interpretForEach(tokens);
        }
        else if (command == "def") {
            interpretFunction(tokens);
        }
        else if (command == "sqrt") {
            interpretSqrt(tokens);
        }
        else if (command == "abs") {
            interpretAbs(tokens);
        }
        else if (command == "round") {
            interpretRound(tokens);
        }
        else if (command == "floor") {
            interpretFloor(tokens);
        }
        else if (command == "ceil") {
            interpretCeil(tokens);
        }
        else if (command == "sin") {
            interpretSin(tokens);
        }
        else if (command == "cos") {
            interpretCos(tokens);
        }
        else if (command == "tan") {
            interpretTan(tokens);
        }
        else if (command == "log") {
            interpretLog(tokens);
        }
        else if (command == "exp") {
            interpretExp(tokens);
        }
        else if (command == "tolower") {
            interpretToLower(tokens);
        }
        else if (command == "toupper") {
            interpretToUpper(tokens);
        }
        else if (functions.count(command)) {
            interpretFunctionCall(tokens);
        }
        else {
            throw std::runtime_error("Comando desconhecido: " + command);
        }
    }

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        bool inQuotes = false;

        while (std::getline(ss, token, delimiter)) {
            if (!inQuotes) {
                // Remove leading and trailing whitespace
                token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](int ch) {
                    return !std::isspace(ch);
                    }));
                token.erase(std::find_if(token.rbegin(), token.rend(), [](int ch) {
                    return !std::isspace(ch);
                    }).base(), token.end());

                if (!token.empty()) {
                    tokens.push_back(token);
                }
            }
            else {
                tokens.back() += delimiter + token;
            }

            if (token.front() == '"' && token.back() == '"') {
                inQuotes = false;
            }
            else if (token.front() == '"' && token.back() != '"') {
                inQuotes = true;
            }
        }

        return tokens;
    }

    std::string join(const std::vector<std::string>& tokens, char delimiter) {
        std::stringstream ss;
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i != 0) {
                ss << delimiter;
            }
            ss << tokens[i];
        }
        return ss.str();
    }

    void interpretIf(const std::vector<std::string>& tokens) {
        if (tokens.size() < 4 || tokens[2] != "then") {
            throw std::runtime_error("Sintaxe incorreta para o comando 'if'.");
        }

        const std::string& condition = tokens[1];
        const std::vector<std::string>& ifBlock = getBlock(tokens, 3);
        const std::vector<std::string>& elseBlock = getBlock(tokens, 3 + ifBlock.size() + 1);

        if (evaluateCondition(condition)) {
            interpret(ifBlock);
        }
        else if (!elseBlock.empty()) {
            interpret(elseBlock);
        }
    }

    double getVariableValue(const std::string& variable) {
        if (isNumber(variable)) {
            return std::stod(variable);
        }

        if (variables.count(variable)) {
            return std::stod(variables[variable]);
        }

        throw std::runtime_error("Variável não encontrada: " + variable);
    }

    bool evaluateCondition(const std::string& condition) {
        std::vector<std::string> tokens = split(condition, ' ');
        std::string lhs = tokens[0];
        std::string op = tokens[1];
        std::string rhs = tokens[2];

        double lhsValue = getVariableValue(lhs);
        double rhsValue = getVariableValue(rhs);

        if (op == "==") {
            return lhsValue == rhsValue;
        }
        else if (op == "!=") {
            return lhsValue != rhsValue;
        }
        else if (op == ">") {
            return lhsValue > rhsValue;
        }
        else if (op == ">=") {
            return lhsValue >= rhsValue;
        }
        else if (op == "<") {
            return lhsValue < rhsValue;
        }
        else if (op == "<=") {
            return lhsValue <= rhsValue;
        }
        else {
            throw std::runtime_error("Operador desconhecido: " + op);
        }
    }

    void interpretLet(const std::vector<std::string>& tokens) {
        if (tokens.size() < 4 || tokens[2] != "=") {
            throw std::runtime_error("Sintaxe incorreta para o comando 'let'.");
        }

        std::string variable = tokens[1];
        std::string valueStr = join(std::vector<std::string>(tokens.begin() + 3, tokens.end()), ' ');

        if (valueStr.front() == '"' && valueStr.back() == '"') {
            // Valor de texto (string)
            std::string value = valueStr.substr(1, valueStr.size() - 2);
            variables[variable] = value;
        }
        else if (valueStr == "true" || valueStr == "false") {
            // Valor booleano
            bool value = (valueStr == "true");
            variables[variable] = (value ? "true" : "false");
        }
        else {
            // Valor numérico
            try {
                double value = std::stod(valueStr);
                variables[variable] = std::to_string(value);
            }
            catch (const std::exception&) {
                throw std::runtime_error("Valor inválido para atribuição.");
            }
        }
    }

    double evaluateExpression(const std::string& expression) {
        std::vector<std::string> tokens = split(expression, ' ');

        // Check for function calls
        if (tokens.size() > 1 && functions.count(tokens[0])) {
            return evaluateFunctionCall(tokens);
        }

        // Check for arithmetic expressions
        std::vector<std::string> postfix = infixToPostfix(tokens);
        return evaluatePostfix(postfix);
    }

    std::vector<std::string> infixToPostfix(const std::vector<std::string>& infix) {
        std::vector<std::string> postfix;
        std::vector<std::string> operators;

        for (const std::string& token : infix) {
            if (isNumber(token)) {
                postfix.push_back(token);
            }
            else if (isFunction(token)) {
                operators.push_back(token);
            }
            else if (token == "(") {
                operators.push_back(token);
            }
            else if (token == ")") {
                while (!operators.empty() && operators.back() != "(") {
                    postfix.push_back(operators.back());
                    operators.pop_back();
                }

                if (operators.empty() || operators.back() != "(") {
                    throw std::runtime_error("Expressão inválida: parênteses não correspondentes.");
                }

                operators.pop_back();  // Remove the "("
            }
            else {
                while (!operators.empty() && getPrecedence(operators.back()) >= getPrecedence(token)) {
                    postfix.push_back(operators.back());
                    operators.pop_back();
                }

                operators.push_back(token);
            }
        }

        while (!operators.empty()) {
            if (operators.back() == "(") {
                throw std::runtime_error("Expressão inválida: parênteses não correspondentes.");
            }

            postfix.push_back(operators.back());
            operators.pop_back();
        }

        return postfix;
    }

    double evaluatePostfix(const std::vector<std::string>& postfix) {
        std::vector<double> stack;

        for (const std::string& token : postfix) {
            if (isNumber(token)) {
                stack.push_back(std::stod(token));
            }
            else if (isFunction(token)) {
                if (stack.empty()) {
                    throw std::runtime_error("Expressão inválida: função sem argumentos.");
                }

                double arg = stack.back();
                stack.pop_back();

                double result = evaluateFunction(token, arg);
                stack.push_back(result);
            }
            else {
                if (stack.size() < 2) {
                    throw std::runtime_error("Expressão inválida: operador sem operandos suficientes.");
                }

                double rhs = stack.back();
                stack.pop_back();

                double lhs = stack.back();
                stack.pop_back();

                double result = evaluateOperator(token, lhs, rhs);
                stack.push_back(result);
            }
        }

        if (stack.size() != 1) {
            throw std::runtime_error("Expressão inválida: sobraram valores na pilha.");
        }

        return stack.back();
    }

    double evaluateFunction(const std::string& function, double arg) {
        if (function == "sqrt") {
            return std::sqrt(arg);
        }
        else if (function == "abs") {
            return std::abs(arg);
        }
        else if (function == "round") {
            return std::round(arg);
        }
        else if (function == "floor") {
            return std::floor(arg);
        }
        else if (function == "ceil") {
            return std::ceil(arg);
        }
        else if (function == "sin") {
            return std::sin(arg);
        }
        else if (function == "cos") {
            return std::cos(arg);
        }
        else if (function == "tan") {
            return std::tan(arg);
        }
        else if (function == "log") {
            return std::log(arg);
        }
        else if (function == "exp") {
            return std::exp(arg);
        }
        else {
            throw std::runtime_error("Função desconhecida: " + function);
        }
    }

    std::vector<std::string> getBlock(const std::vector<std::string>& tokens, size_t startIndex) {
        std::vector<std::string> block;
        int nestingLevel = 0;

        for (size_t i = startIndex; i < tokens.size(); ++i) {
            const std::string& token = tokens[i];

            if (token == "if" || token == "foreach") {
                ++nestingLevel;
            }
            else if (token == "end") {
                if (nestingLevel == 0) {
                    return block;
                }
                else {
                    --nestingLevel;
                }
            }

            block.push_back(token);
        }

        throw std::runtime_error("Estrutura de bloco inválida: faltando comando 'end'.");
    }

    double calculateExpression(const std::string& operand1, const std::string& operatorSymbol, const std::string& operand2) {
        double value1 = std::stod(operand1);
        double value2 = std::stod(operand2);
        double result = 0.0;

        if (operatorSymbol == "+") {
            result = value1 + value2;
        }
        else if (operatorSymbol == "-") {
            result = value1 - value2;
        }
        else if (operatorSymbol == "*") {
            result = value1 * value2;
        }
        else if (operatorSymbol == "/") {
            if (value2 == 0.0) {
                throw std::runtime_error("Erro: divisão por zero.");
            }
            result = value1 / value2;
        }
        else {
            throw std::runtime_error("Operador inválido: " + operatorSymbol);
        }

        return result;
    }
    
    void interpretPrint(const std::vector<std::string>& tokens) {
        if (tokens.size() < 2) {
            throw std::runtime_error("Sintaxe incorreta para o comando 'print'.");
        }

        std::ostringstream resultStream;
        for (size_t i = 1; i < tokens.size(); ++i) {
            std::string token = tokens[i];

            if (token.front() == '"' && token.back() == '"') {
                // Remover as aspas no início e no final do token
                token = token.substr(1, token.length() - 2);
                resultStream << token;
            }
            else if (token.front() == '{' && token.back() == '}') {
                std::string variableName = token.substr(1, token.length() - 2);
                if (variables.count(variableName) > 0) {
                    resultStream << variables[variableName];
                }
                else {
                    throw std::runtime_error("Variável '" + variableName + "' não foi definida.");
                }
            }
            else if (token.front() == 'f' && token.find('"') != std::string::npos) {
                // Processar f-string (formatação de string)
                std::string formatString = token.substr(token.find('"') + 1, token.rfind('"') - token.find('"') - 1);

                std::regex variableRegex("\\{([^}]+)\\}");
                std::string::const_iterator searchStartPos = formatString.begin();
                std::string::const_iterator searchEndPos = formatString.end();
                std::regex_iterator<std::string::const_iterator> regexIterator(searchStartPos, searchEndPos, variableRegex);
                std::regex_iterator<std::string::const_iterator> regexEnd;

                size_t formatIndex = 0;
                while (regexIterator != regexEnd) {
                    std::smatch match = *regexIterator;
                    std::string variableName = match[1].str();
                    if (variables.count(variableName) > 0) {
                        resultStream << formatString.substr(formatIndex, match[0].first - formatString.begin())
                            << variables[variableName];
                        formatIndex = match[0].second - formatString.begin();
                    }
                    else {
                        throw std::runtime_error("Variável '" + variableName + "' não foi definida.");
                    }
                    ++regexIterator;
                }

                resultStream << formatString.substr(formatIndex, formatString.length() - formatIndex);
            }
            else {
                resultStream << token;
            }

            // Adicionar espaço em branco entre os tokens, exceto o último
            if (i < tokens.size() - 1) {
                resultStream << " ";
            }
        }

        std::cout << resultStream.str() << std::endl;
    }

    std::string evaluateExpressionAsString(const std::string& expression) {
        std::vector<std::string> tokens = split(expression, ' ');

        // Check for function calls
        if (tokens.size() > 1 && functions.count(tokens[0])) {
            double result = evaluateFunctionCall(tokens);
            return std::to_string(result);
        }

        // Check for string literals
        if (tokens.size() == 1 && tokens[0].front() == '"' && tokens[0].back() == '"') {
            return tokens[0].substr(1, tokens[0].size() - 2);
        }

        // Check for variables
        if (tokens.size() == 1 && variables.count(tokens[0])) {
            return variables[tokens[0]];
        }

        // Check for arithmetic expressions
        std::vector<std::string> postfix = infixToPostfix(tokens);
        double result = evaluatePostfix(postfix);
        return std::to_string(result);
    }

    void interpretForEach(const std::vector<std::string>& tokens) {
        if (tokens.size() < 5 || tokens[2] != "in" || tokens[4] != "do") {
            throw std::runtime_error("Sintaxe incorreta para o comando 'foreach'.");
        }

        const std::string& variable = tokens[1];
        const std::string& iterable = tokens[3];
        const std::vector<std::string>& block = getBlock(tokens, 5);

        if (!variables.count(iterable)) {
            throw std::runtime_error("Variável não encontrada: " + iterable);
        }

        const std::vector<std::string>& values = split(variables[iterable], ',');

        for (const std::string& value : values) {
            variables[variable] = value;
            interpret(block);
        }
    }

    void interpretFunction(const std::vector<std::string>& tokens) {
        if (tokens.size() < 4 || tokens[2] != "=>") {
            throw std::runtime_error("Sintaxe incorreta para o comando 'def'.");
        }

        const std::string& functionName = tokens[1];
        const std::vector<std::string>& parameters = split(tokens[3], ',');

        std::vector<std::string> block = getBlock(tokens, 4 + parameters.size());
        functions[functionName] = block;

        for (const std::string& parameter : parameters) {
            if (parameter.back() == ':') {
                booleanVariables[parameter.substr(0, parameter.size() - 1)] = false;
            }
            else {
                variables[parameter] = "0";
            }
        }
    }

    void interpretFunctionCall(const std::vector<std::string>& tokens) {
        std::string function = tokens[0];
        if (functions.count(function)) {
            std::vector<std::string> args(tokens.begin() + 1, tokens.end());

            std::vector<std::string> params = functions[function];
            if (params.size() != args.size()) {
                throw std::runtime_error("Número incorreto de argumentos para a função: " + function);
            }

            for (size_t i = 0; i < params.size(); i++) {
                variables[params[i]] = args[i];
            }

            interpret(params);
        }
    }


    double evaluateFunctionCall(const std::vector<std::string>& tokens) {
        const std::string& functionName = tokens[0];
        const std::vector<std::string>& arguments = split(tokens[1], ',');

        if (!functions.count(functionName)) {
            throw std::runtime_error("Função não encontrada: " + functionName);
        }

        const std::vector<std::string>& functionBlock = functions[functionName];

        if (arguments.size() != functionBlock.size()) {
            throw std::runtime_error("Número incorreto de argumentos para a função " + functionName);
        }

        for (size_t i = 0; i < arguments.size(); ++i) {
            const std::string& parameter = functionBlock[i];
            const std::string& argument = arguments[i];

            if (parameter.back() == ':') {
                booleanVariables[parameter.substr(0, parameter.size() - 1)] = evaluateCondition(argument);
            }
            else {
                variables[parameter] = std::to_string(evaluateExpression(argument));
            }
        }

        interpret(functionBlock);

        if (variables.count(functionName)) {
            return std::stod(variables[functionName]);
        }
        else {
            throw std::runtime_error("Função não retornou um valor: " + functionName);
        }
    }

    void interpretSqrt(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'sqrt'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::sqrt(arg);
        std::cout << result << std::endl;
    }

    void interpretAbs(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'abs'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::abs(arg);
        std::cout << result << std::endl;
    }

    void interpretRound(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'round'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::round(arg);
        std::cout << result << std::endl;
    }

    void interpretFloor(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'floor'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::floor(arg);
        std::cout << result << std::endl;
    }

    void interpretCeil(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'ceil'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::ceil(arg);
        std::cout << result << std::endl;
    }

    void interpretSin(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'sin'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::sin(arg);
        std::cout << result << std::endl;
    }

    void interpretCos(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'cos'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::cos(arg);
        std::cout << result << std::endl;
    }

    void interpretTan(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'tan'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::tan(arg);
        std::cout << result << std::endl;
    }

    void interpretLog(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'log'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::log(arg);
        std::cout << result << std::endl;
    }

    void interpretExp(const std::vector<std::string>& tokens) {
        if (tokens.size() != 2) {
            throw std::runtime_error("Sintaxe incorreta para a função 'exp'.");
        }

        double arg = evaluateExpression(tokens[1]);
        double result = std::exp(arg);
        std::cout << result << std::endl;
    }

    void interpretToLower(const std::vector<std::string>& tokens) {
        if (tokens.size() != 3) {
            throw std::runtime_error("Uso incorreto da função tolower");
        }

        std::string variable = tokens[1];
        std::string value = tokens[2];

        if (variables.count(value)) {
            std::string str = variables[value];
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            variables[variable] = str;
        }
        else {
            throw std::runtime_error("Variável não encontrada: " + value);
        }
    }

    void interpretToUpper(const std::vector<std::string>& tokens) {
        if (tokens.size() != 3) {
            throw std::runtime_error("Uso incorreto da função toupper");
        }

        std::string variable = tokens[1];
        std::string value = tokens[2];

        if (variables.count(value)) {
            std::string str = variables[value];
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            variables[variable] = str;
        }
        else {
            throw std::runtime_error("Variável não encontrada: " + value);
        }
    }

    bool isNumber(const std::string& token) {
        std::size_t pos = 0;
        std::stod(token, &pos);
        return pos == token.size();
    }

    bool isFunction(const std::string& token) {
        return functions.count(token);
    }

    int getPrecedence(const std::string& op) {
        if (op == "+" || op == "-") {
            return 1;
        }
        else if (op == "*" || op == "/") {
            return 2;
        }
        else if (op == "^") {
            return 3;
        }
        else {
            return 0;
        }
    }

    double evaluateOperator(const std::string& op, double lhs, double rhs) {
        if (op == "+") {
            return lhs + rhs;
        }
        else if (op == "-") {
            return lhs - rhs;
        }
        else if (op == "*") {
            return lhs * rhs;
        }
        else if (op == "/") {
            if (rhs == 0) {
                throw std::runtime_error("Divisão por zero.");
            }
            return lhs / rhs;
        }
        else if (op == "^") {
            return std::pow(lhs, rhs);
        }
        else {
            throw std::runtime_error("Operador desconhecido: " + op);
        }
    }
};

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Portuguese");

    if (argc > 2) {
        std::cout << "Uso incorreto do interpretador. Utilize o comando \"interpreter help\" para obter ajuda." << std::endl;
        return 1;
    }

    if (argc == 2 && std::string(argv[1]) == "help") {
        std::cout << "Este é um interpretador de linguagens de script." << std::endl;
        std::cout << "Para executar um arquivo de script, utilize o comando \"interpreter <arquivo>\"." << std::endl;
        std::cout << "Se nenhum arquivo for fornecido, o programa será executado em modo de teste." << std::endl;
        return 0;
    }

    Interpreter interpreter;

    if (argc == 1) {
        std::cout << "Modo de teste ativado. Digite os comandos linha a linha." << std::endl;
        std::cout << "Pressione Ctrl + C para sair." << std::endl;

        while (true) {
            std::string line;
            std::cout << "> ";
            std::getline(std::cin, line);

            std::vector<std::string> tokens = interpreter.split(line, ' ');

            try {
                interpreter.interpret(tokens);
            }
            catch (const std::exception& e) {
                std::cout << "Erro: " << e.what() << std::endl;
            }
        }
    }
    else {
        std::ifstream file(argv[1]);

        if (!file) {
            std::cout << "Arquivo não encontrado: " << argv[1] << std::endl;
            return 1;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::vector<std::string> tokens = interpreter.split(line, ' ');

            try {
                interpreter.interpret(tokens);
            }
            catch (const std::exception& e) {
                std::cout << "Erro: " << e.what() << std::endl;
                return 1;
            }
        }
    }

    return 0;
}