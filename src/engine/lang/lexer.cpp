//
// Created by mgrus on 21.04.2025.
//

#include "blang_parser.h"

bool is_key_word(const std::string& word, blang::TokenType &out_keywordType) {

    if (blang::keyword_map.contains(word)) {
        out_keywordType = blang::keyword_map.at(word);
        return true;
    }

    return false;
}

static void word_ident_keyword_process(std::string& currentWord, blang::Token& currentToken, std::vector<blang::Token>& tokens) {
    blang::TokenType keywordType;
    if (is_key_word(currentWord, keywordType)) {
        currentToken.type = keywordType;
    } else {
        currentToken.type = blang::TokenType::IDENT;
        currentToken.string_val = currentWord;
        tokens.push_back(currentToken);
        currentWord.clear();
    }
}

static void finish_num_word(std::string& numberWord, blang::Token& currentToken, std::vector<blang::Token>& tokens) {
    float num = atof(numberWord.c_str());
    currentToken.type = blang::TokenType::NUMBER;
    currentToken.float_val = num;
    tokens.push_back(currentToken);
    numberWord.clear();
}

static void process_brackets(std::string& currentWord, blang::Token& currentToken, std::vector<blang::Token>& tokens, char c) {
    currentToken.type = blang::keyword_map.at(std::string(1, c));
    tokens.push_back(currentToken);
    currentWord.clear();

}


 std::vector<blang::Token> blang::lex(const std::string& sourceCode) {

     std::vector<blang::Token> tokens;
     std::string currentWord = "";
     float currentNumber = 0.0f;
     std::string numberWord = "";

     for (auto c : sourceCode) {

         Token currentToken;

         // Whitespace:
         // Generally ignores & closing the current word.
         // Check if keyword, otherwise classify as identifier.
         if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
             if (currentWord.empty() && numberWord.empty()) {
                 continue;
             }
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
                 continue;
             }
             if (!numberWord.empty()) {
                 finish_num_word(numberWord, currentToken, tokens);
                 continue;
             }

         }

         if (c == '=') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }

             currentToken.type = TokenType::EQUALS;
             tokens.push_back(currentToken);
             continue;

         }

         if (c == '+') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }
             currentToken.type = TokenType::PLUS;
             tokens.push_back(currentToken);
             continue;
         }

         if (c == '-') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }
             if (!numberWord.empty()) {
                 finish_num_word(numberWord, currentToken, tokens);
             }
             currentToken.type = TokenType::MINUS;
             tokens.push_back(currentToken);
             continue;
         }

         if (c == '*') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }
             currentToken.type = TokenType::MUL;
             tokens.push_back(currentToken);
             continue;
         }

         if (c == '/') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }
             currentToken.type = TokenType::DIV;
             tokens.push_back(currentToken);
             continue;
         }

         // The dot can be part of a word/identifier.
         // Or, make the numbers decimal point:
         if (c == '.') {
             if (!currentWord.empty()) {
                 currentWord += c;
                 continue;
             }
             if (!numberWord.empty()) {
                 numberWord += c;
             }
         }

         if (c == '(' || c== ')' || c == '[' || c == ']' || c == '{' || c == '}') {
             if (!currentWord.empty()) {
                 word_ident_keyword_process(currentWord, currentToken, tokens);
             }
             if (!numberWord.empty()) {
                 finish_num_word(numberWord, currentToken, tokens);
             }

             process_brackets(currentWord, currentToken, tokens, c);
             continue;

         }

         // If this is a char, add it to the current word.
         // This is always safe.
         if (isalpha(c)) {
             currentWord += c;
             continue;
         }

         // We encountered a digt.
         // This can mean a few things:
         // If we are in a word, we concatenate it to this word. e.g. foo3 and we are good.
         if (isdigit(c)) {
             // We have a word, just concat the digit:
             if (!currentWord.empty()) {
                 currentWord += c;
                 continue;
             }
             // Otherwise add it to the current number.
             numberWord += c;

         }

     }

     return tokens;
 }