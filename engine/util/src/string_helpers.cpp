//
// Created by mgrus on 12.06.2025.
//

#include <string>

bool strContains1(const std::string& str, char test) {
    for (auto c : str) {
        if (c == test) return true;
    }

    return false;
}

bool strContains(const std::string &str, const std::string &test) {
    if (test.length() > str.length()) return false;
    if (test.length() == str.length()) return str == test;

    for (int i = 0; i < str.length(); i++) {
        std::string testSequence = "";
        for (int t = 0; t  < test.length(); t++) {
            testSequence += str[t];
        }
        if (testSequence == test) {
            return true;
        }


    }

    return false;
}

