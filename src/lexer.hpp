#pragma once
#include <iostream>
#include <ostream>
#include <vector>
#include <string>
#include <optional>

enum class TokenType {
    Token_Int,
    Token_If,
	Token_Elif,
	Token_Else,
    Token_Assign,
    Token_Identifier,
    Token_IntLit ,
    Token_Plus,
    Token_Minus,
	Token_Star,
	Token_Dividend,
    Token_LParen,
    Token_RParen,
    Token_LBracket,
    Token_RBracket,
    Token_Equal,
    Token_Less,
    Token_LessEqual,
    Token_Greater,
    Token_GreaterEqual,
    Token_Semi,
    Token_Return,
    Token_EOF
};

inline std::string tokenToString(const TokenType t) {
    if (t == TokenType::Token_Int) { return "int"; }
    else if (t == TokenType::Token_If) { return "if"; }
    else if (t == TokenType::Token_Elif) { return "else if"; }
    else if (t == TokenType::Token_Else) { return "else"; }
    else if (t == TokenType::Token_Plus) { return "+"; }
    else if (t == TokenType::Token_Minus) { return "-"; }
    else if (t == TokenType::Token_Star) { return "*"; }
    else if (t == TokenType::Token_Dividend) { return "/"; }
    else if (t == TokenType::Token_Assign) { return "="; }
    else if (t == TokenType::Token_Equal) { return "=="; }
    else if (t == TokenType::Token_Less) { return "<"; }
    else if (t == TokenType::Token_LessEqual) { return "<="; }
    else if (t == TokenType::Token_Greater) { return ">"; }
    else if (t == TokenType::Token_GreaterEqual) { return ">="; }
    else if (t == TokenType::Token_Semi) { return ";"; }
    else if (t == TokenType::Token_LBracket) { return "{"; }
    else if (t == TokenType::Token_RBracket) { return "}"; }
    else if (t == TokenType::Token_LParen) { return "("; }
    else if (t == TokenType::Token_LParen) { return ")"; }
    else if (t == TokenType::Token_Identifier) { return "Identifier"; }
    else if (t == TokenType::Token_IntLit) { return "IntLit"; }
    else if (t == TokenType::Token_Return) { return "return"; }
    else if (t == TokenType::Token_EOF) {return "EOF";}
    return "";
}

inline std::optional<int> bin_prec(const TokenType type) {
    switch (type) {
        case TokenType::Token_Minus:
        case TokenType::Token_Plus:
            return 0;
        case TokenType::Token_Star:
        case TokenType::Token_Dividend:
            return 1;
        case TokenType::Token_Equal: return 1;
        default:
            return {};
    }
}

struct Token {
    TokenType type;
    int line;
    int column;
    std::optional<std::string> value{};
};

class Tokenizer {
public:
    explicit Tokenizer(std::string src) : data(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> token;
        std::string buf;

        while (peek().has_value()) {
            if (isspace(peek().value())) {
                consume();
            }
            else if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && isalnum(peek().value())) {
                    buf.push_back(consume());
                }

                if (buf == "int") {
                    token.push_back({TokenType::Token_Int, lineNum, colNum, buf });
                }else if (buf == "if") {
                    token.push_back({TokenType::Token_If, lineNum, colNum, buf });
                }else if (buf == "elif") {
                    token.push_back({TokenType::Token_Elif, lineNum, colNum, buf });
                }else if (buf == "else") {
                    token.push_back({TokenType::Token_Else, lineNum, colNum, buf });
                }else if (buf == "return") {
                    token.push_back({TokenType::Token_Return, lineNum, colNum, buf });
                }else {
                    token.push_back({TokenType::Token_Identifier, lineNum, colNum, buf });
                }

                buf.clear();
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());

                while (peek().has_value() && isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                if (peek().has_value() && isalpha(peek().value())) {
                    buf.push_back(consume());
                    std::cout << "Invaild identifier: " << buf << std::endl;
                    exit(EXIT_FAILURE);
                }

                token.push_back({TokenType::Token_IntLit, lineNum, colNum, buf });
                buf.clear();
            }else if (peek().value() == '/' && peek(1).has_value() && peek().value() == '/') {
                consume();
                consume();
                while (peek().value() != '\n') {
                    consume();
                }
                ++lineNum;
                colNum=1;
            }else if (peek().value() == '/' && peek(1).has_value() && peek().value() == '*') {
                consume();
                consume();
                while (peek().value()) {
                    if (peek().value() == '*' && peek().has_value() && peek(1).value() == '/') {
                        break;
                    }
                    if (peek().value() != '\n') {
                        ++lineNum;
                    }
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                if (peek().has_value()) {
                    consume();
                }
                colNum=1;
            }else if (peek().value() == '+') {
                consume();
                token.push_back({TokenType::Token_Plus, lineNum, colNum, "+"});
            }else if (peek().value() == '-') {
                consume();
                token.push_back({TokenType::Token_Minus, lineNum, colNum, "-"});
            }else if (peek().value() == '*') {
                consume();
                token.push_back({TokenType::Token_Star, lineNum, colNum, "*"});
            }else if (peek().value() == '/') {
                consume();
                token.push_back({TokenType::Token_Dividend, lineNum, colNum, "/"});
            }
            else if (peek().value() == '=') {
                consume();
                if (peek().value() == '=') {
                    consume();
                    token.push_back({TokenType::Token_Equal, lineNum, colNum, "=="});
                }else {
                    token.push_back({TokenType::Token_Assign, lineNum, colNum, "="});
                }
            }else if (peek().value() == '<') {
                consume();
                if (peek().value() == '=') {
                    consume();
                    token.push_back({TokenType::Token_LessEqual, lineNum, colNum, "<="});
                } else {
                    token.push_back({TokenType::Token_Less, lineNum, colNum, "<"});
                }
            }else if (peek().value() == '>') {
                consume();
                if (peek().value() == '=') {
                    consume();
                    token.push_back({TokenType::Token_GreaterEqual, lineNum, colNum, ">="});
                }else {
                    token.push_back({TokenType::Token_Greater, lineNum, colNum, ">"});
                }
            }else if (peek().value() == ';') {
                consume();
                token.push_back({TokenType::Token_Semi, lineNum, colNum, ";"});
                ++lineNum;
                colNum= 1;
            }else if (peek().value() == '\n') {
                consume();
                ++lineNum;
                colNum= 1;
            }else if (peek().value() == '(') {
                consume();
                token.push_back({TokenType::Token_LParen, lineNum, colNum, "("});
            }else if (peek().value() == ')') {
                consume();
                token.push_back({TokenType::Token_RParen, lineNum, colNum, ")"});
            }else if (peek().value() == '{') {
                consume();
                token.push_back({TokenType::Token_LBracket, lineNum, colNum, "{"});
            }else if (peek().value() == '}') {
                consume();
                token.push_back({TokenType::Token_RBracket, lineNum, colNum, "}"});
            }
            else {
                std::cerr << "Unexpected char: " << peek().value() << std::endl;
                exit(EXIT_FAILURE);
            }

        }
        token.push_back({TokenType::Token_EOF, lineNum,0, ""});
        return token;
    }

private:
    const std::string data;
    int c_Index = 0;
    int lineNum = 1, colNum = 1;

    [[nodiscard]]std::optional<char> peek(size_t offset=0) const {
        if (c_Index + offset < data.length()) {
            return  data[c_Index + offset];
        }
        return {};
    }

    char consume() {
        colNum++;
        return data[c_Index++];
    }
};

