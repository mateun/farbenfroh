//
// Created by mgrus on 22.04.2025.
//

#include "blang_interpreter.h"
#include "blang_parser.h"

static void processIf(blang::IfStmtNode* ifNode) {

}

static void processAssignment(blang::AssignmentNode* stmt, blang::RuntimeEnv* env) {
    auto identLeft = stmt->id_;
    auto exp = dynamic_cast<blang::TermNode*>(stmt->expr_);
    // TODO runtime error!
    if (exp == nullptr) return;

    auto factorLeft = exp->left;
    // TODO expand factorLeft downward as much as possible.
    float result = 0;
    for (auto t :  exp->term_tails) {
        if (t.op == blang::BinaryOp::Add) {
            result = t.factor->left;
        }
    }

}

static void processStmtNode(std::vector<blang::StmtNode *>::value_type stmt, blang::RuntimeEnv* env) {
    auto assignment = dynamic_cast<blang::AssignmentNode*>(stmt);
    if (assignment) processAssignment(assignment, env); return;

    auto ifStatement = dynamic_cast<blang::IfStmtNode*>(stmt);
    if (ifStatement) processIf(ifStatement); return;

}


/**
 * Walking the AST top down left right.
 * @param ast The AST to interpret.
 */
void blang::interpret(blang::AstNode * ast, RuntimeEnv env) {
    auto programNode = dynamic_cast<blang::ProgNode*>(ast);

    for (auto stmt : programNode->stmtList) {
        processStmtNode(stmt, env);
    }


}
