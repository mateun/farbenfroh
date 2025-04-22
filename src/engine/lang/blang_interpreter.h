//
// Created by mgrus on 22.04.2025.
//

#ifndef BLANG_INTERPRETER_H
#define BLANG_INTERPRETER_H

#include <map>
#include <string>
#include <vector>

namespace blang {

struct AstNode;

enum class ValueType {
  FLOAT_VAL,
  INT_VAL,
  STRING_VAL,
  BOOL_VAL
};

struct Value {
  ValueType type;
  float float_val;
  int int_val;
  std::string string_val;
  bool bool_val;

};

struct RuntimeEnv {
  std::map<std::string, Value> variables;
};

void interpret(blang::AstNode* ast, RuntimeEnv* env);

}


#endif //BLANG_INTERPRETER_H
