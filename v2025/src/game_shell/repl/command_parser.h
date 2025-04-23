//
// Created by mgrus on 13.04.2025.
//

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

struct Repl_Message;

enum class TokType {
  verb,
  noun,
  number,
  id,
  parameter,
  key,
  string_value,
  float_value,
  int_value,
  navigation_cmd
};

struct Token {
  TokType tokType;
  std::string string_value;
  int int_value;
  float float_value;

};

std::vector<Token> tokenize(const std::string& input);

int parse(const std::vector<Token>& tokens);

void processSpecialKey(UINT vkCode);
void processKeystroke(char c);

#endif //LEXER_H
