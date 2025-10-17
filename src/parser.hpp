#pragma once

#include "lexer.hpp"
#include <variant>
#include <cassert>
#include "arena.hpp"

struct NodeTermInt {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeTermParen {
    NodeExpr* expr;
};

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMulti {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprEqual {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr {
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMulti*, NodeBinExprDiv*, NodeBinExprEqual*> var;
};

struct  NodeTerm {
    std::variant<NodeTermIdent*, NodeTermInt*, NodeTermParen*> var;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtReturn {
    NodeExpr* expr;
};

struct NodeStmtInt {
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt;

struct NodeScope {
    std::vector<NodeStmt*> stmts;
};

struct NodeIfPred;

struct NodeIfPredElif {
    NodeExpr* expr {};
    NodeScope* scope{};
    std::optional<NodeIfPred*> pred;
};

struct  NodeIfPredElse {
    NodeScope* scope;
};

struct NodeIfPred {
    std::variant<NodeIfPredElif*, NodeIfPredElse*> var;
};

struct NodeStmtIf {
    NodeExpr* expr {};
    NodeScope* scope{};
    std::optional<NodeIfPred*> pred;
};

struct NodeStmtAssign {
    Token ident;
    NodeExpr* expr{};
};

struct NodeStmt {
    std::variant<NodeStmtInt*, NodeScope*, NodeStmtIf*, NodeStmtAssign*, NodeStmtReturn*> var;
};

struct  NodeProg {
    std::vector<NodeStmt*> stmts;
};

class Parser {
public:

    explicit Parser(std::vector<Token> tokens) : data(std::move(tokens)), m_allocator(1024 * 1024 * 4) {};

    void error_expected(const std::string& msg) const{
        std::cerr << "[Parse Error] Expected " << msg << " on line " << peek(0).value().line << " on column " << peek(0).value().column << std::endl;
        exit(EXIT_FAILURE);
    }

    std::optional<NodeTerm*> parse_term() {
        if (auto int_lit = try_consume(TokenType::Token_IntLit)) {
            auto term_int_lit = m_allocator.emplace<NodeTermInt>(int_lit.value());
            auto term = m_allocator.emplace<NodeTerm>(term_int_lit);
            return term;
        }
        if (auto ident = try_consume(TokenType::Token_Identifier)) {
            auto term_ident = m_allocator.emplace<NodeTermIdent>(ident.value());
            auto term = m_allocator.emplace<NodeTerm>(term_ident);
            return term;
        }
        if (const auto open_paren = try_consume(TokenType::Token_LBracket)) {
            auto expr = parse_expr();
            if (!expr.has_value()) {
                error_expected("expression");
            }
            try_consume_err(TokenType::Token_RParen);
            auto term_paren = m_allocator.emplace<NodeTermParen>(expr.value());
            auto term = m_allocator.emplace<NodeTerm>(term_paren);
            return term;
        }
        return {};
    }

    std::optional<NodeExpr*> parse_expr(const int min_prec = 0) {
        std::optional<NodeTerm*> term_lhs = parse_term();
        if (!term_lhs.has_value()) {
            return {};
        }

        auto expr_lhs =  m_allocator.emplace<NodeExpr>(term_lhs.value());
        while (true) {
            std::optional<Token> curr_tok = peek();
            std::optional<int> prec;
            if (curr_tok.has_value()) {
                    prec = bin_prec(curr_tok->type);
                if (!prec.has_value() || prec < min_prec) {
                    break;
                }
            }
            else {
                break;
            }

            auto [type, line, column, val] = consume();
            const int next_min_prec = prec.value() + 1;

            auto expr_rhs = parse_expr(next_min_prec);
            if (!expr_rhs.has_value()) {
                error_expected("Expression");
            }
            auto expr = m_allocator.emplace<NodeBinExpr>();
            auto expr_lhs2 = m_allocator.emplace<NodeExpr>();

            if (type == TokenType::Token_Plus) {
                expr_lhs2->var = expr_lhs->var;
                auto add = m_allocator.emplace<NodeBinExprAdd>(expr_lhs2, expr_rhs.value());
                expr->var = add;
            }else if (type == TokenType::Token_Minus) {
                expr_lhs2->var = expr_lhs->var;
                auto sub = m_allocator.emplace<NodeBinExprSub>(expr_lhs2, expr_rhs.value());
                expr->var = sub;
            }else if (type == TokenType::Token_Star) {
                expr_lhs2->var = expr_lhs->var;
                auto multi = m_allocator.emplace<NodeBinExprMulti>(expr_lhs2, expr_rhs.value());
                expr->var = multi;
            }else if (type == TokenType::Token_Dividend) {
                expr_lhs2->var = expr_lhs->var;
                auto div = m_allocator.emplace<NodeBinExprDiv>(expr_lhs2, expr_rhs.value());
                expr->var = div;
            } else if (type == TokenType::Token_Equal) {
                expr_lhs2->var = expr_lhs->var;
                auto equal = m_allocator.emplace<NodeBinExprEqual>(expr_lhs2, expr_rhs.value());
                expr->var = equal;
            }

            else {
                assert(false);
            }
            expr_lhs->var = expr;
        }
        return expr_lhs;
    }

    std::optional<NodeScope*> parse_scope() {
        if (!try_consume(TokenType::Token_LBracket)) {
            return {};
        }

        auto scope = m_allocator.emplace<NodeScope>();
        while (auto stmt = parse_stmt()) {
            scope->stmts.push_back(stmt.value());
        }

        try_consume_err(TokenType::Token_RBracket);
        return scope;
    }

    std::optional<NodeIfPred*> parse_if_pred() {
        if (try_consume(TokenType::Token_Elif)) {
            try_consume_err(TokenType::Token_LParen);
            const auto elif = m_allocator.alloc<NodeIfPredElif>();
            if (const auto expr = parse_expr()) {
                elif->expr = expr.value();
            }else {
                error_expected("Expression");
            }

            try_consume_err(TokenType::Token_RParen);
            if (const auto scope = parse_scope()) {
                elif->scope = scope.value();
            }else {
                error_expected("scope");
            }

            elif->pred = parse_if_pred();
            auto pred = m_allocator.emplace<NodeIfPred>(elif);
            return pred;
        }
        if (try_consume(TokenType::Token_Else)) {
            auto else_ = m_allocator.alloc<NodeIfPredElse>();
            if (const auto scope = parse_scope()) {
                else_->scope = scope.value();
            }
            else {
                error_expected("scope");
            }
            auto pred = m_allocator.emplace<NodeIfPred>(else_);
            return pred;
        }
        return {};
    }

    NodeStmtReturn* parse_return_stmt() {
        try_consume_err(TokenType::Token_LParen);

        NodeExpr* expr = nullptr;
        if (auto e = parse_expr()) {
            expr = e.value();
        } else {
            error_expected("expression after return");
        }

        try_consume_err(TokenType::Token_RParen);
        try_consume_err(TokenType::Token_Semi);

        auto node = m_allocator.alloc<NodeStmtReturn>();
        node->expr = expr;
        return node;
    }

    std::optional<NodeStmt*> parse_stmt() {
        if (peek().has_value() && peek().value().type == TokenType::Token_Int &&
                peek(1).has_value() && peek(1).value().type == TokenType::Token_Identifier &&
                peek(2).has_value() && peek(2).value().type == TokenType::Token_Assign) {

            consume();
            auto stmt_int = m_allocator.emplace<NodeStmtInt> ();
            stmt_int->ident = consume();

            consume();

            if (const auto expr = parse_expr()) {
                stmt_int->expr = expr.value();
            }else {
                error_expected("Expression");
            }

            try_consume_err(TokenType::Token_Semi);
            auto stmt = m_allocator.emplace<NodeStmt> ();
            stmt->var = stmt_int;
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::Token_Int &&
                peek(1).has_value() && peek(1).value().type == TokenType::Token_Identifier) {
            consume();

            auto stmt_int = m_allocator.emplace<NodeStmtInt>();
            stmt_int->ident = consume();

            if (try_consume(TokenType::Token_Assign)) {
                if (auto expr = parse_expr()) {
                    stmt_int->expr = expr.value();
                }else {
                    error_expected("expression");
                }
            }

            try_consume_err(TokenType::Token_Semi);
            auto stmt = m_allocator.emplace<NodeStmt> (stmt_int);
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::Token_Identifier &&
                peek(1).has_value() && peek(1).value().type == TokenType::Token_Assign) {
            const auto assign = m_allocator.emplace<NodeStmtAssign>();
            assign->ident = consume();
            consume();
            if (const auto expr = parse_expr()) {
                assign->expr = expr.value();
            }
            else {
                error_expected("expression");
            }
            try_consume_err(TokenType::Token_Semi);
            auto stmt = m_allocator.emplace<NodeStmt>(assign);
            std::cout << assign->ident.line << std::endl;
            return stmt;

        }
        if (peek().has_value() && peek().value().type == TokenType::Token_LBracket) {
            if (auto scope = parse_scope()) {
                auto stmt = m_allocator.emplace<NodeStmt>(scope.value());
                return stmt;
            }
            error_expected("scope");
        }
        if (auto if_ = try_consume(TokenType::Token_If)) {
            try_consume_err(TokenType::Token_LParen);
            auto stmt_if = m_allocator.emplace<NodeStmtIf>();
            if (const auto expr = parse_expr()) {
                stmt_if->expr = expr.value();
            }
            else {
                error_expected("expression");
            }

            try_consume_err(TokenType::Token_RParen);
            if (const auto scope = parse_scope()) {
                stmt_if->scope = scope.value();
            }
            else {
                error_expected("scope");
            }
            stmt_if->pred = parse_if_pred();
            auto stmt = m_allocator.emplace<NodeStmt>(stmt_if);
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::Token_Return) {
            consume();
            auto ret = parse_return_stmt();
            auto stmt = m_allocator.emplace<NodeStmt>(ret);
            return stmt;
        }
        if (peek().has_value() && peek().value().type == TokenType::Token_EOF) {
            return {}; // stop parsing statements
        }
        return {};
    }

    std::optional<NodeProg> parse_program() {
        NodeProg prog;
        while (peek().has_value() && peek()->type != TokenType::Token_EOF) {
            auto stmt = parse_stmt();
            if (!stmt) {
                error_expected("statement");
            }else {
                prog.stmts.push_back(stmt.value());
            }
        }
        return prog;
    }
private:
    const std::vector<Token> data;
    int c_Index = 0;
    ArenaAllocator m_allocator;

    [[nodiscard]]std::optional<Token> peek(const size_t offset=0) const {
        if (c_Index + offset < data.size()) {
            auto tok = data[c_Index + offset];
            return tok;
        }
        return {};
    }

    Token consume() {
        if (c_Index < data.size()) {
            return data[c_Index++];
        }
        return {TokenType::Token_EOF, 0, 0, ""};
    }

    Token try_consume_err(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        error_expected(tokenToString(type));
        return {};
    }

    std::optional<Token> try_consume(const TokenType type) {
        if (peek().has_value() && peek().value().type == type) {
            return consume();
        }
        return {};
    }
};