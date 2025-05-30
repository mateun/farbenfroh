//
// Created by mgrus on 21.04.2025.
//

#ifndef LANG_PARSER_H
#define LANG_PARSER_H

#include <map>
#include <vector>
#include <string>

namespace blang {
    struct Value;
}

namespace blang {
    enum class TokenType {
        IDENT  ,
        NUMBER_FLOAT ,
        NUMBER_INT ,
        STRING ,
        LINE_COMMENT,
        BLOCK_COMMENT_START,
        BLOCK_COMMENT_END,
        WS     ,
        IF,
        THEN,
        ELSEIF,
        ELSE, END,
        WHILE, DO, FOR, FUNC, RETURN,
        AND, OR, NOT,
        TRUE, FALSE, NIL,
        EQUALS,
        PLUS, MINUS, MUL, DIV,
        BRAC_OPEN,
        BRAC_CLOSE,
        SQ_BRAC_OPEN,
        SQ_BRAC_CLOSE,
        CURL_BRAC_OPEN,
        CURL_BRAC_CLOSE,
        COMMA,

    };

    struct Token {
        TokenType type;
        int int_val;
        float float_val;
        std::string string_val;
    };


    const std::map<std::string, TokenType> keyword_map = {
        {"if", TokenType::IF},
        {"then", TokenType::THEN},
        {"elseif", TokenType::ELSEIF},
        {"else", TokenType::ELSE},
        {"end", TokenType::END},
        {"while", TokenType::WHILE},
        {"do", TokenType::DO},
        {"for", TokenType::FOR},
        {"func", TokenType::FUNC},
        {"return", TokenType::RETURN},
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE},
        {"nil", TokenType::NIL},
        {"==", TokenType::EQUALS},
        {"+", TokenType::PLUS},
        {"-", TokenType::MINUS},
        {"*", TokenType::MUL},
        {"/", TokenType::DIV},
        {"(", TokenType::BRAC_OPEN},
        {")", TokenType::BRAC_CLOSE},
        {"[", TokenType::SQ_BRAC_OPEN},
        {"]", TokenType::SQ_BRAC_CLOSE},
        {"{", TokenType::CURL_BRAC_OPEN},
        {"}", TokenType::CURL_BRAC_CLOSE},

    };


    std::vector<blang::Token> lex(const std::string& sourceCode);

    struct AstNode {
        virtual ~AstNode() {}
    };

    struct StmtNode : AstNode {

    };

    struct IfStmtNode : StmtNode {

    };



    struct ProgNode : AstNode {

        std::vector<StmtNode*> stmtList;
    };

    struct ExpressionNode: StmtNode {
        virtual Value reduce() = 0;
    };

    enum class BinaryOp {
        Add,
        Subtract,
        Multiply,
        Divide,
    };

    struct FactorNode;
    struct TermTail {
        BinaryOp op;
        FactorNode* factor;
    };

    struct TermNode: ExpressionNode {
        FactorNode* left;
        std::vector<TermTail> term_tails;

        Value reduce() override;


    };

    struct UnaryNode;
    struct FactorTail {
        BinaryOp op;
        UnaryNode* unary;
    };



    struct FactorNode: TermNode {
        UnaryNode* left;
        std::vector<FactorTail> factor_tails;
        Value reduce() override;
    };

    enum class UnaryType {
        unary_minus,
        unary_not,
        none,
    };
    struct UnaryNode : FactorNode {
        UnaryType type;
        UnaryNode* rhsUnary;
        Value reduce() override;
    };

    struct PrimaryNode : UnaryNode {

    };

    struct NumPrimary : PrimaryNode {

    };

    struct FloatNumPrimary : NumPrimary {
        Value reduce() override;
        float num;
    };

    struct IntNumPrimary : NumPrimary {
        Value reduce() override;
        int num;
    };

    struct StringPrimary : PrimaryNode {
        std::string str;
    };
    struct BoolPrimary : PrimaryNode {
        bool bool_value;
    };
    struct IdentPrimary : PrimaryNode {
        IdentPrimary(const std::string& ident);
        std::string ident;
    };
    struct FuncCallPrimary : PrimaryNode {
      std::string func_name;
      Value reduce() override;
    };
    struct BracedPrimary : PrimaryNode {
        ExpressionNode* expr;
    };

    struct AssignmentNode : StmtNode {
        AssignmentNode(IdentPrimary* id, ExpressionNode* expr);
        IdentPrimary* id_ = nullptr;
        ExpressionNode* expr_ = nullptr;
    };

    struct ReturnStatementNode : StmtNode {
        ExpressionNode* expr;
    };

    struct FuncCallNode : PrimaryNode {
        IdentPrimary* func_name = nullptr;
        std::vector<ExpressionNode*> args;
    };

    struct FunctionDeclNode : StmtNode {
        IdentPrimary* func_name = nullptr;
        std::vector<IdentPrimary*> params;
        std::vector<StmtNode*> body_stmts;
    };

    AstNode* parse(const std::vector<Token>& tokenStream);

}


#endif //LANG_PARSER_H
