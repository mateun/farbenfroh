//
// Created by mgrus on 21.04.2025.
//

#include <algorithm>
#include <stdexcept>

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
blang::FuncCallNode* funcCallStmt(const std::vector<blang::Token>& tokens, int& index);
blang::StmtNode* stmt(const std::vector<blang::Token>& tokens, int& index);
bool eat(const std::vector<blang::Token>& tokens, int& index, blang::TokenType referenceType);

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

    if (match(token, blang::TokenType::IDENT)) {
        // Try a funccall?!
        index--;
        auto func_call = funcCallStmt(tokens, index);
        if (func_call) {
            return func_call;
        }
        // We just leave it as a standalone identifier then:
        return new blang::IdentPrimary(token.string_val);
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

blang::ReturnStatementNode* returnStatement(const std::vector<blang::Token>& tokens, int& index) {
    auto token = tokens[index];
    if (!match(tokens[index], blang::TokenType::RETURN)) {
        return nullptr;
    }
    index++;
    auto expr = expressionStmt(tokens, index);
    auto rsn = new blang::ReturnStatementNode();
    rsn->expr = expr;
    return rsn;

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

bool eat(const std::vector<blang::Token>& tokens, int& index, blang::TokenType referenceType) {

    auto token = tokens[index];
    if (!match(token, referenceType)) {
        return false;
    }
    index++;
    return true;
}

blang::FunctionDeclNode* funcDeclStmt(const std::vector<blang::Token>& tokens, int& index) {
    auto token = tokens[index];
    if (!match(token, blang::TokenType::FUNC)) {
        return nullptr;
    }
    index++;
    token = tokens[index];
    if (!match(token, blang::TokenType::IDENT)) {
        index--;
        return nullptr;
    }
    auto func_decl_node = new blang::FunctionDeclNode();
    func_decl_node->func_name = new blang::IdentPrimary(token.string_val);

    // Eat the openen bracket
    index++;
    token = tokens[index];
    if (!match(token, blang::TokenType::BRAC_OPEN)) {
        return nullptr;
    }

    // Function parameters
    bool params_left =true;
    do {
        index++;
        token = tokens[index];
        if (!match(token, blang::TokenType::IDENT)) {
            break;
        }
        func_decl_node->params.push_back(new blang::IdentPrimary(token.string_val));
        index++;
        if (!match(tokens[index], blang::TokenType::COMMA)) {
            index--;
            break;
        }

    } while (params_left);


    index++;
    token = tokens[index];
    if (!match(token, blang::TokenType::BRAC_CLOSE)) {
        return nullptr;
    }

    // Function body
    index++;
    token = tokens[index];
    if (!match(token, blang::TokenType::CURL_BRAC_OPEN)) {
        return nullptr;
    }
    index++;

    // Body statements
    bool stmts_left =true;
    do {
        auto stmtNode = stmt(tokens, index);
        if (!stmtNode) {
            break;
        }

        func_decl_node->body_stmts.push_back(stmtNode);
    } while (stmts_left);

    //index++;
    token = tokens[index];
    if (!match(token, blang::TokenType::CURL_BRAC_CLOSE)) {
        return nullptr;
    }

    return func_decl_node;

}

blang::FuncCallNode* funcCallStmt(const std::vector<blang::Token>& tokens, int& index) {

    auto func_call_node = new blang::FuncCallNode();
    auto token = tokens[index++];
    if (!match(token, blang::TokenType::IDENT)) {
        return nullptr;
    }
    func_call_node->func_name = new blang::IdentPrimary(token.string_val);

    token = tokens[index++];
    if (!match(token, blang::TokenType::BRAC_OPEN)) {
        return nullptr;
    }

    bool args_left = true;
    do {
        auto expr = expressionStmt(tokens, index);
        // Is this a parameterless function?
        if (!expr) {
            args_left = false;
            break;
        }
        func_call_node->args.push_back(expr);
        // Now either we consume a comma or we are done
        index--;
        token = tokens[index];
        if (!match(token, blang::TokenType::COMMA)) {
            args_left = false;
            //index++;
            break;
        }
        // Eat the comma in any case
        index++;


    } while (args_left);

    token = tokens[index++];
    if (!match(token, blang::TokenType::BRAC_CLOSE)) {
        // TODO syntax error?
        return nullptr;
    }

    return func_call_node;

}


blang::StmtNode* stmt(const std::vector<blang::Token>& tokens, int& index) {
    auto indexOriginal = index;
    auto assignment = assignmentStmt(tokens, index);
    if (assignment) { return assignment; }

    auto returnStmtNode = returnStatement(tokens, index);
    if (returnStmtNode) return returnStmtNode;

    auto funcCall = funcCallStmt(tokens, indexOriginal);
    if (funcCall) { return funcCall; }

    auto funcDecl = funcDeclStmt(tokens, index);
    if (funcDecl) { return funcDecl; }

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
