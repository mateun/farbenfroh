//
// Created by mgrus on 21.04.2025.
//

#include "blang_parser.h"
#include "blang_interpreter.h"
#include <Windows.h>
#include <cassert>
#include <cstdio>
#include <string>





void writeGreen(const std::string& text) {
  // Get the handle to the console
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

  // Set the text color to green (FOREGROUND_GREEN)
  SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

  // Print the text in green
  printf("%s", text.c_str());

  // Reset to the default console color (white on black)
  SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);


}

int main() {

#ifdef BASIC_BLANG_TESTS
  // Assign int value
  std::string src = R"(
    a = 10
  )";
  auto tokens = blang::lex(src);
  assert(tokens.size() == 3);
  assert(tokens[0].type == blang::TokenType::IDENT);
  assert(tokens[1].type == blang::TokenType::EQUALS);
  assert(tokens[2].type == blang::TokenType::NUMBER_INT);

  // Assign float value
  src = R"(
    a = 12.0
  )";
  tokens = blang::lex(src);
  assert(tokens.size() == 3);
  assert(tokens[0].type == blang::TokenType::IDENT);
  assert(tokens[1].type == blang::TokenType::EQUALS);
  assert(tokens[2].type == blang::TokenType::NUMBER_FLOAT);
  assert(tokens[2].float_val == 12.0f);

  // Multiple assignments
  src = R"(
    a = 12.0
    b = 28
  )";
  tokens = blang::lex(src);
  assert(tokens.size() == 6);
  assert(tokens[0].type == blang::TokenType::IDENT);
  assert(tokens[1].type == blang::TokenType::EQUALS);
  assert(tokens[2].type == blang::TokenType::NUMBER_FLOAT);
  assert(tokens[2].float_val == 12.0f);
  assert(tokens[5].type == blang::TokenType::NUMBER_INT);
  assert(tokens[5].int_val == 28);

  // Expressions
  src = R"(
    a = 12.0
    b = 28
    c = a+   b
    d = a -b
    d = a *     b
    d=a/b

  )";
  tokens = blang::lex(src);
  assert(tokens.size() == 26);
  assert(tokens[0].type == blang::TokenType::IDENT);
  assert(tokens[1].type == blang::TokenType::EQUALS);
  assert(tokens[2].type == blang::TokenType::NUMBER_FLOAT);
  assert(tokens[2].float_val == 12.0f);
  assert(tokens[5].type == blang::TokenType::NUMBER_INT);
  assert(tokens[5].int_val == 28);
  assert(tokens[9].type == blang::TokenType::PLUS);
  assert(tokens[14].type == blang::TokenType::MINUS);
  assert(tokens[19].type == blang::TokenType::MUL);
  assert(tokens[24].type == blang::TokenType::DIV);

  // Bracket expressions
  src = R"(
    d = (x *3 / (k + 4)) * (e - 12) + 4 - {8*3}

  )";
  tokens = blang::lex(src);
  assert(tokens.size() == 26);
  assert(tokens[0].type == blang::TokenType::IDENT);
  assert(tokens[1].type == blang::TokenType::EQUALS);
  assert(tokens[2].type == blang::TokenType::BRAC_OPEN);
  assert(tokens[3].type == blang::TokenType::IDENT);
  assert(tokens[4].type == blang::TokenType::MUL);
  assert(tokens[5].type == blang::TokenType::NUMBER_INT);
  assert(tokens[6].type == blang::TokenType::DIV);
  assert(tokens[7].type == blang::TokenType::BRAC_OPEN);
  assert(tokens[8].type == blang::TokenType::IDENT);
  assert(tokens[9].type == blang::TokenType::PLUS);
  assert(tokens[18].type == blang::TokenType::BRAC_CLOSE);
  assert(tokens[22].type == blang::TokenType::CURL_BRAC_OPEN);
  assert(tokens[25].type == blang::TokenType::CURL_BRAC_CLOSE);

  //Parse tests
  src = R"(
    d = 12 + 8
    a = 1.0 + 2.0
  )";
  tokens = blang::lex(src);
  assert(tokens.size() == 10);
  auto rootNode = parse(tokens);
  blang::ProgNode* progNode = dynamic_cast<blang::ProgNode*>(rootNode);
  assert(progNode != nullptr);
  assert(progNode->stmtList.size() == 2);
  auto assignmentStatement = dynamic_cast<blang::AssignmentNode*>(progNode->stmtList[0]);
  assert(assignmentStatement != nullptr);
  auto termNode = dynamic_cast<blang::TermNode*>(assignmentStatement->expr_);
  assert(termNode != nullptr);
  auto exprLhs = dynamic_cast<blang::IntNumPrimary*>(termNode->left->left->rhsUnary);
  assert(exprLhs != nullptr);
  auto exprRhs = (termNode->term_tails);
  assert(exprRhs.size() == 1);
  assert(exprRhs[0].op == blang::BinaryOp::Add);

  auto runtimeEnv = blang::RuntimeEnv();
  blang::interpret(rootNode, &runtimeEnv);
  assert(runtimeEnv.variables["d"].int_val == 20);
  assert(runtimeEnv.variables["a"].float_val == 3.0f);
#endif

  // Func call tests
  auto src = R"(
    func foobar(a, b) {
      return a + b
    }
    x = 28.0
    a = foobar(12.0, x)
  )";

  auto tokens = blang::lex(src);
  assert(tokens.size() == 24);
  auto ast = parse(tokens);
  auto progNode = dynamic_cast<blang::ProgNode*>(ast);
  assert(progNode != nullptr);
  // TODO debug and fix statement recognition, we only get 1 instead of 3 here
  assert(progNode->stmtList.size() == 3);
  auto env = new blang::RuntimeEnv();
  blang::interpret(ast, env);
  assert(env->variables["x"].float_val == 28.0f);
  assert(env->variables["a"].float_val == 40.0f);


  writeGreen("SUCCESS\n");
  return 0;
};
