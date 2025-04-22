//
// Created by mgrus on 21.04.2025.
//

#include <algorithm>

#include "blang_parser.h"
#include <vector>



blang::IdentPrimary::IdentPrimary(const std::string &ident)  : ident(ident) {
}

blang::AssignmentNode::AssignmentNode(IdentPrimary *id, ExpressionNode *expr) : id_(std::move(id)), expr_(std::move(expr)) {

}



blang::TermNode* term(const std::vector<blang::Token>& tokens,  int& index);
blang::FactorNode* factor(const std::vector<blang::Token>& tokens,  int& index);
blang::PrimaryNode* primary(const std::vector<blang::Token>& tokens,  int& index);
blang::UnaryNode* unary(const std::vector<blang::Token>& tokens,  int& index);

void matchWithExit(blang::Token token, blang::TokenType type) {
    if (token.type != type) {
        printf("syntax error: expected %d but got: %d", type, token.type);
    }
}

bool match(blang::Token token, blang::TokenType type) {
    if (token.type == type) {
        return true;
    }
    return false;
}

blang::ExpressionNode* expressionStmt(const std::vector<blang::Token>& tokens,  int& index) {
    auto termNode = term(tokens, index);
    if (termNode) return termNode;

    auto factorNode = factor(tokens, index);
    if (factorNode) return factorNode;

    auto unaryNode = unary(tokens, index);
    if (unaryNode) return unaryNode;

    auto primaryNode = primary(tokens, index);
    if (primaryNode) return primaryNode;

    return nullptr;

}

blang::PrimaryNode* primary(const std::vector<blang::Token>& tokens,  int& index) {
    auto token = tokens[index++];
    if (match(token, blang::TokenType::NUMBER_FLOAT)) {
        auto fp = new blang::FloatNumPrimary();
        fp->num = token.float_val;
        return fp;
    }
    if (match(token, blang::TokenType::NUMBER_INT)) {
        auto ip = new blang::IntNumPrimary();
        ip->num = token.int_val;
        return ip;
    }

    if (match(token, blang::TokenType::STRING)) {
        return new blang::PrimaryNode();
    }
    if (match(token, blang::TokenType::IDENT)) {
        return new blang::PrimaryNode();
    }
    if (match(token, blang::TokenType::BRAC_OPEN)) {
        auto expr = expressionStmt(tokens, index);
        if (!expr) {
            return nullptr;
        }
        auto token2 = tokens[index++];
        if (!match(token2, blang::TokenType::BRAC_CLOSE)) {
            return nullptr;
        }
            // TODO fill actual bracelete, expr data!
            return new blang::PrimaryNode();

    }

    return nullptr;
}


blang::UnaryNode* unary(const std::vector<blang::Token>& tokens,  int& index) {
    auto unaryNode = new blang::UnaryNode();
    auto token = tokens[index++];
    if (match(token, blang::TokenType::MINUS)) {
        unaryNode->type = blang::UnaryType::unary_minus;
    } else if (match(token, blang::TokenType::NOT)) {
        unaryNode->type = blang::UnaryType::unary_not;
    } else {
        unaryNode->type = blang::UnaryType::none;
        index--;
        unaryNode->rhsUnary = primary(tokens, index);

    }
    return unaryNode;
}

static blang::BinaryOp getBinOpForToken(const blang::Token& tok) {
    switch (tok.type) {
        case blang::TokenType::PLUS: return blang::BinaryOp::Add;
        case blang::TokenType::MINUS: return blang::BinaryOp::Subtract;
        case blang::TokenType::MUL: return blang::BinaryOp::Multiply;
        case blang::TokenType::DIV: return blang::BinaryOp::Divide;
        default: return {};
    }

}


blang::FactorNode* factor(const std::vector<blang::Token>& tokens,  int& index) {
    auto factorNode = new blang::FactorNode();
    factorNode->left = unary(tokens, index);
    if (!factorNode->left) return nullptr;

    // Next we find tail operations (+ or -):
    bool break_tail = false;
    do {
        if (index > tokens.size()-1) {
            break_tail = true;
            break;
        }
        auto token = tokens[index++];
        if (match(token, blang::TokenType::MUL) || match(token, blang::TokenType::DIV)) {
            auto unaryRight = unary(tokens, index);
            // This should not happen, syntax error:
            // op with missing right part of the operator:
            // TODO report syntax error
            if (!unaryRight) return nullptr;
            auto tail = blang::TermTail();
            tail.op = getBinOpForToken(token);
            tail.factor = unaryRight;
            factorNode->term_tails.push_back(tail);

        } else {
            // backtrack
            index--;
            break_tail = true;
        }

    } while (break_tail == false);

    return factorNode;

}


// BNF:
// Term           → Factor { ( "+" | "-" ) Factor }
blang::TermNode* term(const std::vector<blang::Token>& tokens,  int& index) {
    auto termNode = new blang::TermNode();

    termNode->left = factor(tokens, index);
    // TODO actually this is a syntax error, report!!
    if (!termNode->left) return nullptr;

    // Next we find tail operations (+ or -):
    bool breakTermTail = false;
    do {
        if (index > tokens.size()-1) {
            breakTermTail = true;
            break;
        }
        auto token = tokens[index++];
        if (match(token, blang::TokenType::PLUS) || match(token, blang::TokenType::MINUS)) {
            auto factorNodeRight = factor(tokens, index);
            // This should not happen, syntax error:
            // op with missing right part of the operator:
            // TODO report syntax error
            if (!factorNodeRight) return nullptr;
            auto termTail = blang::TermTail();
            termTail.op = getBinOpForToken(token);
            termTail.factor = factorNodeRight;
            termNode->term_tails.push_back(termTail);

        } else {
            breakTermTail = true;
        }

    } while (breakTermTail == false);

    return termNode;

}



blang::AssignmentNode* assignmentStmt(const std::vector<blang::Token>& tokens, int& index) {
    auto token = tokens[index];
    if (!match(token, blang::TokenType::IDENT)) {
        return nullptr;
    }
    auto identNode = new blang::IdentPrimary(token.string_val);
    index++;

    token = tokens[index];
    if (!match(token, blang::TokenType::EQUALS)) {
        return nullptr;
    }
    index++;
    auto expression_node = expressionStmt(tokens, index);
    if (expression_node) {
        return new blang::AssignmentNode(identNode, expression_node);
    }

    return nullptr;

}


blang::StmtNode* stmt(const std::vector<blang::Token>& tokens, int& index) {
    auto assignment = assignmentStmt(tokens, index);
    if (assignment != nullptr) { return assignment; }

    // TODO check for other types of statements

    return nullptr;

}

void stmtList(blang::ProgNode& progNode, const std::vector<blang::Token>& tokens, int& tokenIndex) {
    blang::StmtNode* stmNode = nullptr;
    do {
        // We retrieve another statment or nullptr.
        // The tokenIndex is expected to have moved forward accordingly.
        if (tokenIndex > tokens.size()-1) {
            break;
        }
        stmNode = stmt(tokens, tokenIndex);
        if (stmNode) {
            progNode.stmtList.push_back(stmNode);
            // TODO this is a bit wonky? Why do we need to reduce here?
            // Somewhere upstream the index is incremented 1 too many times?!
            tokenIndex--;
        }

    } while (stmNode != nullptr);


}

blang::AstNode* blang::parse(const std::vector<blang::Token> &tokens) {
    auto programNode = new ProgNode();

    int tokenIndex = 0;
    stmtList(*programNode, tokens, tokenIndex);
    return programNode;

}
