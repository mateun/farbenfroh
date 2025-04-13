//
// Created by mgrus on 13.04.2025.
//

#include "command_parser.h"

#include <algorithm>
#include <string>

static TokType getTokenType(const std::string& word) {

}


std::vector<Token> tokenize(const std::string &input) {
    std::vector<Token> tokens;

    // First pass - split on whitespace:
    // Find the actual words of our sentence.
    std::string word = "";
    std::vector<std::string> words;
    {
        for (auto c : input) {
            // Skip whitespace and finish word:
            if ((c == ' ' || c == '\n') && !word.empty()) {
                words.push_back(word);
                word.clear();
            } else {
                word += c;
            }
        }
        words.push_back(word);
    }

    // Second pass:
    // Go through the words and transform into tokens.
    // If this fails, we report an error:
    std::vector<std::string> verbs = {"create", "delete", "modify"};
    std::vector<std::string> nouns = {"game", "level", "object", "template"};
    std::vector<std::string> navigation_commands = {"cd", "ls", "copy", "paste", "mv" };
    for (auto word : words) {
        Token token;
        // Check all known verbs
        if (std::ranges::contains(verbs, word)) {
            token.tokType = TokType::verb;
        }
        else if (std::ranges::contains(nouns, word)) {
            token.tokType = TokType::noun;
        }
        else if (std::ranges::contains(navigation_commands, word)) {
            token.tokType = TokType::navigation_cmd;
        } else {
            // We could not find a good category,
            // so we declare it an id:
            token.tokType = TokType::id;
        }
        token.string_value = word;
        tokens.push_back(token);

    }

    return tokens;
}

static bool verb(Token token) {
    if (token.tokType == TokType::verb) {
        return true;
    }
    return false;

}

static bool noun(Token token) {
    if (token.tokType == TokType::noun) {
        return true;
    }
    return false;

}

static bool navigation(Token token) {
    if (token.tokType == TokType::navigation_cmd) {
        return true;
    }
    return false;
}

static bool action_command(Token token) {
    switch (token.tokType) {
        case TokType::verb: return verb(token);
        case TokType::noun: return noun(token);

        default: return false;
    }

}

/**
    Command language grammar:
    -------------------------

    command         ::= action_command | navigation_command

    action_command  ::= verb noun [argument_list]

    verb            ::= "create" | "delete" | "copy" | "modify"
    noun            ::= "game" | "level" | "object" | "resource"

    argument_list   ::= argument {argument}
    argument        ::= parameter | identifier
    parameter       ::= identifier "=" value
    identifier      ::= letter { letter | digit | "_" }
    value           ::= string | number

    navigation_command ::= nav_verb path
    nav_verb       ::= "cd" | "ls" | "cp"
    path           ::= "/" [path_element] { "/" [path_element] }
    path_element   ::= identifier

 */
int parse(const std::vector<Token>& tokens) {

    // Try action-command sequence:
    auto isaction = action_command(tokens[0]);

    // Try navigation command:
    if (!isaction) {
        auto isnavi = navigation(tokens[0]);
        if (!isnavi) {
            // return error
            return 1;
        }
    }

    // We did not return early with an error so we assume the parsing was successful.
    // TODO besides returning, maybe we should also build the actual transmitting command here?!
    return 0;
}

