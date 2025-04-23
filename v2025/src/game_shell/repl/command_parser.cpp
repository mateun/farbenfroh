//
// Created by mgrus on 13.04.2025.
//

#include <Windows.h>
#include "command_parser.h"

#include <algorithm>
#include <string>

extern void openHelpWindow();

// The parser can send type ahead proposals to our repl window so it
// can display it.
extern void updateTypeProp(const std::string& newTypeProp);
extern void clearTypeProp();
extern void updateActualInput(const std::string& newInput);
extern std::string getTypeProp();


static std::string g_currentTypeAheadWord = "";


static std::vector<std::string> verbs = {"create", "delete", "modify", "help"};
static std::vector<std::string> nouns = {"game", "level", "object", "template"};
static std::vector<std::string> navigation_commands = {"cd", "ls", "copy", "paste", "mv" };

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



static bool verb(std::vector<Token> tokens) {
    // We assume this is a verb, so lets see which one it is:
    auto verbValue = tokens[0].string_value;
    if (verbValue == "help") {
        openHelpWindow();
        return true;
    }
    if (verbValue == "create") {
        // TODO whole create universe...
        // expect noun to be next etc.
        // then trigger the creation of the actual type (game, level, object...)
        // parse any further parameters.
        return true;
    }

    return false;

}

static bool noun(std::vector<Token> tokens) {
    if (tokens[0].tokType == TokType::noun) {
        return true;
    }
    return false;

}

static bool navigation(std::vector<Token> tokens) {
    if (tokens[0].tokType == TokType::navigation_cmd) {
        return true;
    }
    return false;
}

static bool action_command(std::vector<Token> tokens) {
    switch (tokens[0].tokType) {
        case TokType::verb: return verb(tokens);
        case TokType::noun: return noun(tokens);

        default: return false;
    }

}

/**
    Command language grammar:
    -------------------------

    command         ::= action_command | navigation_command

    action_command  ::= verb noun [argument_list]

    verb            ::= "create" | "delete" | "copy" | "modify" | "help"
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
    auto isaction = action_command(tokens);

    // Try navigation command:
    if (!isaction) {
        auto isnavi = navigation(tokens);
        if (!isnavi) {
            // return error
            return 1;
        }
    }

    // We did not return early with an error so we assume the parsing was successful.
    // TODO besides returning, maybe we should also build the actual transmitting command here?!
    return 0;
}

/**
 * Handle tab and esc etc.
 * @param vkCode The incoming special key
 *
 */
void processSpecialKey(UINT vkCode) {

    // This tab completion generally works, BUT
    // it always completely discards any words before
    // Need to use the whole line
    if (vkCode == VK_TAB) {
        if (!g_currentTypeAheadWord.empty()) {
            updateActualInput(g_currentTypeAheadWord + getTypeProp());
            g_currentTypeAheadWord.clear();
            clearTypeProp();
        }
    }
}

// Helper for case-insensitive "starts with"
bool startsWith(const std::string &str, const std::string &prefix) {
    if (prefix.size() > str.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin(), [](char a, char b){
        return std::tolower(a) == std::tolower(b);
    });
}

void processKeystroke(char nextChar) {

    g_currentTypeAheadWord += nextChar;

    // Look for a match among known commands.
    auto it = std::find_if(verbs.begin(), verbs.end(),
        [&](const std::string &cmd) {
            return startsWith(cmd, g_currentTypeAheadWord);
        });

    if (it != verbs.end())
    {
        std::string suggestion = it->substr(g_currentTypeAheadWord.size());
        updateTypeProp(suggestion);
    }
    else {
        g_currentTypeAheadWord.clear();
        clearTypeProp();
    }

}

