//
// Created by mgrus on 22.04.2025.
//

#include "blang_interpreter.h"

#include <functional>
#include <stdexcept>

#include "blang_parser.h"

static void processIf(blang::IfStmtNode* ifNode) {

}

static void processAssignment(blang::AssignmentNode* stmt, blang::RuntimeEnv* env) {
    auto identLeft = stmt->id_;
    auto exp = dynamic_cast<blang::TermNode*>(stmt->expr_);
    // TODO runtime error!
    if (exp == nullptr) return;

    auto result = exp->reduce();
    env->variables[identLeft->ident] = result;

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
void blang::interpret(blang::AstNode * ast, RuntimeEnv* env) {
    auto programNode = dynamic_cast<blang::ProgNode*>(ast);

    for (auto stmt : programNode->stmtList) {
        processStmtNode(stmt, env);
    }


}

template <typename T>
static blang::Value apply_arithmetic(const blang::Value& lhs, const blang::Value& rhs, blang::ValueType type, T op) {
    if (lhs.type != type || rhs.type != type) {
        throw std::runtime_error("Type mismatch in arithmetic operation");
    }

    if (type == blang::ValueType::INT_VAL) {
        return blang::Value{type, 0.0f, op(lhs.int_val, rhs.int_val)};
    }
    if (type == blang::ValueType::FLOAT_VAL) {
        return blang::Value{type, op(lhs.float_val, rhs.float_val)};
    }

    throw std::runtime_error("Unsupported type for arithmetic operation");
}



blang::Value blang::Value::negate() const {
    if (type == ValueType::INT_VAL) {
        return Value{ValueType::INT_VAL, 0.0f, -this->int_val};
    }
    if (type == ValueType::FLOAT_VAL) {
        return Value{ValueType::INT_VAL, -this->float_val, 0};
    }
    throw std::runtime_error("Unsupported type for negation operation");
}

blang::Value blang::Value::operatornot() const {
    if (type == ValueType::BOOL_VAL) {
        auto result = Value{ValueType::BOOL_VAL};
        result.bool_val = !this->bool_val;
        return result;
    }
    throw std::runtime_error("Unsupported type for NOT operation");
}


blang::Value blang::Value::operator+(const Value &rhs) const {
    if (type == ValueType::STRING_VAL && rhs.type == ValueType::STRING_VAL) {
        return Value{ValueType::STRING_VAL, 0.0f, 0, string_val + rhs.string_val};
    }
    return apply_arithmetic(*this, rhs, type, std::plus{});

}

blang::Value blang::Value::operator-(const Value &rhs) const {
    return apply_arithmetic(*this, rhs, type, std::minus{});
}

blang::Value blang::Value::operator*(const Value &rhs) const {
    return apply_arithmetic(*this, rhs, type, std::multiplies{});
}

blang::Value blang::Value::operator/(const Value &rhs) const {
    return apply_arithmetic(*this, rhs, type, std::divides<>{});
}


blang::Value blang::TermNode::reduce() {
    auto value_left = left->reduce();
    for (auto tt : term_tails) {
        if (tt.op == BinaryOp::Add) {
            value_left = value_left + tt.factor->reduce();
        }
        else if (tt.op == BinaryOp::Subtract) {
            value_left = value_left - tt.factor->reduce();
        }
    }

    return value_left;

}

// BNF:
// Factor         → Unary  { ( "*" | "/" | "%" ) Unary }
blang::Value blang::FactorNode::reduce() {
    auto lhs = left->reduce();
    for (auto ft: this->factor_tails) {
        if (ft.op == BinaryOp::Multiply) {
            lhs = lhs * ft.unary->reduce();
        }
        else if (ft.op == BinaryOp::Subtract) {
            lhs = lhs - ft.unary->reduce();
        }
    }

    return lhs;
}

blang::Value blang::UnaryNode::reduce() {

    if (!rhsUnary) {
        return {};

    }
    auto rhs = this->rhsUnary->reduce();

    if (type == UnaryType::unary_minus) {
        return rhs.negate();
    }
    if (type == UnaryType::unary_not) {
        return rhs.operatornot();
    }

    return rhs;

}

blang::Value blang::FloatNumPrimary::reduce() {
    return {ValueType::FLOAT_VAL, num};
}

blang::Value blang::IntNumPrimary::reduce() {
    return {ValueType::INT_VAL, 0.0f, num};
}

