//
// Created by mgrus on 30.12.2023.
//
#include <ostream>
//#include <algorithm>
#include "json.h"

void endNumOrId(std::string& currentId, std::string& currentNum, std::vector<JsonToken>& tokens);
bool strContains(const std::string& str, char test);
bool strContains(const std::string& str, const std::string& test);

std::vector<JsonToken> lexJson(const std::string json) {
    std::vector<JsonToken> tokens;
    std::string currentId = "";
    std::string currentNum = "";
    bool startString = false;
    for (auto c : json) {
        // Eat whitespace
        if (c == ' ' || c == '\n' || c == '\t') {
        }
        else if (c == '{') {
            tokens.push_back({JsonTokenType::CURL_OPEN});
        }
        else if (c == '}') {
            endNumOrId(currentId, currentNum, tokens);
            tokens.push_back({JsonTokenType::CURL_CLOSE});
        }
        else if (c == '[') {
            tokens.push_back({JsonTokenType::SQUARE_OPEN});
        }
        else if (c == ']') {
            endNumOrId(currentId, currentNum, tokens);
            tokens.push_back({JsonTokenType::SQUARE_CLOSE});
        }
        else if (c == '"') {
            if (!currentId.empty()) {
                auto tok = JsonToken{};
                tok.type = JsonTokenType::JID;
                tok.textValue = currentId;
                tokens.push_back(tok);
                currentId.clear();
                startString = false;
            } else {
                startString = true;
            }

            tokens.push_back({JsonTokenType::QUOTE});

        } else if (c == ':' ) {
            if (!currentId.empty()) {
                currentId += c;
            } else {
                tokens.push_back({JsonTokenType::COLON});
            }

        } else if (c == 'e') {
            if (!currentId.empty()) {
                currentId += c;
            } else {
                currentNum += c;
            }
        }
        else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == '/' || c == '+') {
            currentId += c;
        } else if (c == '_') {
            currentId += c;
        } else if (c == '-') {
            if (!currentId.empty()) {
                currentId += c;
            } else {
                currentNum += c;
            }
        } else if (c == '.') {
            if (!currentId.empty()) {
                currentId += c;
            } else {
                currentNum += c;
            }
        } else if (c >= 48 && c <= 57) {
            if (!currentId.empty() || startString) {
                currentId += c;
            } else {
                currentNum += c;
            }
        } else if (c == ',') {
            if (tokens[tokens.size()-1].type != JsonTokenType::QUOTE && (currentId == "true" || currentId == "false" )) {
                endNumOrId(currentId, currentNum, tokens);
                tokens.push_back({JsonTokenType::COMMA});
                continue;
            }
            if (!currentId.empty() || startString) {
                currentId += c;
            } else {
                endNumOrId(currentId, currentNum, tokens);
                tokens.push_back({JsonTokenType::COMMA});
            }
        }
    }

    return tokens;
}

void endNumOrId(std::string& currentId, std::string& currentNum, std::vector<JsonToken>& tokens) {
    if (!currentId.empty()) {
        // Bool or normal id
        if (tokens[tokens.size()-1].type == JsonTokenType::QUOTE) {
            auto tok = JsonToken{};
            tok.type = JsonTokenType::JID;
            tok.textValue = currentId;
            tokens.push_back(tok);
            currentId.clear();
        } else {
            // Assume bool, as we have no closing quote
            auto tok = JsonToken{};
            tok.type = JsonTokenType::JBOOL;
            tok.textValue = currentId;
            // TOOD parse actual bool value
            tokens.push_back(tok);
            currentId.clear();
        }
    }
    else if (!currentNum.empty()) {
        auto tok = JsonToken{};
        tok.type = JsonTokenType::JNUMBER;
        tok.textValue = currentNum;
        if (strContains(tok.textValue, '.')) {
           tok.numValue = std::stof(tok.textValue);
       } else {
           tok.numValue = std::stoi(tok.textValue);
       }
        tokens.push_back(tok);
        currentNum.clear();
    } else {

    }
}

bool strContains(const std::string& str, char test) {
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

void debugPrintString(int indents, const std::string& text) {
#ifdef JSON_DEBUG
    for (int i = 0; i < indents; i++) {
        printf("  ");
    }

   printf("%s\n", text.c_str());
#endif
}

void debugPrint(int indents, void* obj, bool isopen, bool isobject) {
#ifdef JSON_DEBUG
    for (int i = 0; i < indents; i++) {
        printf("-");
    }
    std::string prefix = "";
    if (isopen) {
        if (isobject) {
            prefix = "{";
        } else {
            prefix = "[";
        }
    } else {
        if (isobject) {
            prefix = "}";
        } else {
            prefix = "]";
        }
    }
    printf("%s %p\n", prefix.c_str(), (void*)obj);
#endif
}


JsonElement* parseJson(const std::string json) {
    auto tokens = lexJson(json);
    // We can rule out anything else than an object or array at the start
    if (tokens[0].type != JsonTokenType::CURL_OPEN && tokens[0].type != JsonTokenType::SQUARE_OPEN ) {
        printf("json must start with an object or array");
        return nullptr;
    }

    JsonValue* top = nullptr;
    int indents = 0;
    JsonToken lastToken;
    JsonMember* member = nullptr;
    for (auto t : tokens) {
        if (t.type == JsonTokenType::QUOTE) {
            continue;
        }
        if (t.type == JsonTokenType::CURL_OPEN) {
            indents++;
            auto val = new JsonValue();
            val ->valueType = JsonValueType::Object;
            val->objectValue = new JsonObject();
            val->parent = top;
            top = val;
            debugPrint(indents, val, true, true);

            if (member) {
                member->element->value = top;
                if (top->parent->valueType == JsonValueType::Object) {
                    top->parent->objectValue->members.push_back(member);
                    member = nullptr;
                }
            }

        }
        if (t.type == JsonTokenType::CURL_CLOSE) {
            if (top->parent && top->parent->valueType == JsonValueType::Array) {
                auto element = new JsonElement();
                element->value = top;
                top->parent->arrayValue->elements.push_back(element);
            }
            debugPrint(indents, top, false, true);
            indents--;
            if (top->parent) {
                auto parentValue = top->parent;
                top = parentValue;
            }

        }
        if (t.type == JsonTokenType::SQUARE_OPEN) {
            indents++;
            auto val = new JsonValue();
            val ->valueType = JsonValueType::Array;
            val->arrayValue = new JsonArray();
            val->parent = top;
            top = val;
            debugPrint(indents, val, true, false);

            if (member) {
                member->element->value = top;
                if (top->parent->valueType == JsonValueType::Object) {
                    top->parent->objectValue->members.push_back(member);
                    member = nullptr;
                }

            }
        }

        if (t.type == JsonTokenType::SQUARE_CLOSE) {
            debugPrint(indents, top, false, false);
            indents--;
            if (top->parent) {
                auto parentValue = top->parent;
                top = parentValue;
            }
        }

        // Recognising members of objects and elements of arrays
        if (t.type == JsonTokenType::JID) {
            bool isKey = true;
            if (lastToken.type == JsonTokenType::CURL_OPEN) {
                debugPrintString(indents, "memberKey: " + t.textValue);


            }
            if (lastToken.type == JsonTokenType::COLON) {
                debugPrintString(indents, "memberValue: " + t.textValue);
                isKey = false;
                member->element->value = new JsonValue();
                member->element->value->valueType = JsonValueType::String;
                member->element->value->stringValue = t.textValue;
                top->objectValue->members.push_back(member);
                member = nullptr;

            }
            if (lastToken.type == JsonTokenType::SQUARE_OPEN) {
                debugPrintString(indents, "elementValue: " + t.textValue);
                isKey = false;

            }
            if (lastToken.type == JsonTokenType::COMMA) {
                if (top->valueType == JsonValueType::Array) {
                    debugPrintString(indents, "elementValue: " + t.textValue);
                    isKey = false;

                }
                if (top->valueType == JsonValueType::Object) {
                    debugPrintString(indents, "memberKey: " + t.textValue);
                }
            }

            if (isKey) {
                member = new JsonMember();
                member->name = t.textValue;
                member->element = new JsonElement();
            }

        }
        if (t.type == JsonTokenType::JNUMBER) {
            bool isKey = true;
            if (lastToken.type == JsonTokenType::CURL_OPEN) {
                debugPrintString(indents, "memberKey: " + t.textValue);

            }
            if (lastToken.type == JsonTokenType::COLON) {
                debugPrintString(indents, "memberValue: " + std::to_string(t.numValue));
                isKey = false;
                member->element->value = new JsonValue();
                member->element->value->valueType = JsonValueType::Number;
                member->element->value->floatValue = t.numValue;
                top->objectValue->members.push_back(member);
                member = nullptr;
            }
            if (lastToken.type == JsonTokenType::SQUARE_OPEN) {
                debugPrintString(indents, "elementValue: " + std::to_string(t.numValue));
                isKey = false;
                auto element = new JsonElement();
                element->value = new JsonValue();
                element->value->valueType = JsonValueType::Number;
                element->value->floatValue = t.numValue;
                top->arrayValue->elements.push_back(element);
            }
            if (lastToken.type == JsonTokenType::COMMA) {
                if (top->valueType == JsonValueType::Array) {
                    debugPrintString(indents, "elementValue: " + std::to_string(t.numValue));
                    isKey = false;
                    auto element = new JsonElement();
                    element->value = new JsonValue();
                    element->value->valueType = JsonValueType::Number;
                    element->value->floatValue = t.numValue;
                    top->arrayValue->elements.push_back(element);
                }
                if (top->valueType == JsonValueType::Object) {
                    debugPrintString(indents, "memberKey: " + std::to_string(t.numValue));

                }
            }
            if (isKey) {
                member = new JsonMember();
                member->name = t.textValue;
                member->element = new JsonElement();
            }
        }


        lastToken = t;

    }

    auto element = new JsonElement();
    element->value = top;
    return element;

}


bool testJson() {
    std::string testJson1 = R"(
        {
            "foo": "bar",
            "myint" : 12,
            "myfloat": 8.9878,
            "mybool" : true,
            {
                "foo2": "bar2"
            }
        };
    )";

    std::string testJson2 = "["
            + testJson1 + "]";

    auto tokens = lexJson(testJson1);
    bool success = true;
    success = tokens[0].type == JsonTokenType::CURL_OPEN;
    success = tokens[1].type == JsonTokenType::QUOTE;
    success = tokens[2].type == JsonTokenType::JID;
    success = tokens[3].type == JsonTokenType::QUOTE;
    success = tokens[4].type == JsonTokenType::COLON;
    success = tokens[5].type == JsonTokenType::QUOTE;
    success = tokens[6].type == JsonTokenType::JID;
    success = tokens[7].type == JsonTokenType::QUOTE;
    success = tokens[8].type == JsonTokenType::COMMA;
    success = tokens[9].type == JsonTokenType::QUOTE;
    success = tokens[10].type == JsonTokenType::JID;
    success = tokens[11].type == JsonTokenType::QUOTE;
    success = tokens[12].type == JsonTokenType::COLON;
    success = tokens[13].type == JsonTokenType::JNUMBER;
    success = tokens[14].type == JsonTokenType::COMMA;
    success = tokens[15].type == JsonTokenType::QUOTE;
    success = tokens[16].type == JsonTokenType::JID;
    success = tokens[17].type == JsonTokenType::QUOTE;
    success = tokens[18].type == JsonTokenType::COLON;
    success = tokens[19].type == JsonTokenType::JNUMBER;
    success = tokens[20].type == JsonTokenType::COMMA;
    success = tokens[21].type == JsonTokenType::QUOTE;
    success = tokens[22].type == JsonTokenType::JID;
    success = tokens[23].type == JsonTokenType::QUOTE;
    success = tokens[24].type == JsonTokenType::COLON;
    success = tokens[25].type == JsonTokenType::JBOOL;
    success = tokens[26].type == JsonTokenType::COMMA;
    success = tokens[27].type == JsonTokenType::CURL_OPEN;
    success = tokens[28].type == JsonTokenType::QUOTE;
    success = tokens[29].type == JsonTokenType::JID;
    success = tokens[30].type == JsonTokenType::QUOTE;
    success = tokens[31].type == JsonTokenType::COLON;
    success = tokens[32].type == JsonTokenType::QUOTE;
    success = tokens[33].type == JsonTokenType::JID;
    success = tokens[34].type == JsonTokenType::QUOTE;
    success = tokens[35].type == JsonTokenType::CURL_CLOSE;

    auto jsonElement = parseJson(testJson1);
    success = jsonElement->value->valueType == JsonValueType::Object;
    success = jsonElement->value->objectValue->members.size() == 5;
    success = jsonElement->value->objectValue->members[1]->element->value->valueType == JsonValueType::Number;
    success = jsonElement->value->objectValue->members[2]->element->value->valueType == JsonValueType::Number;
    success = jsonElement->value->objectValue->members[2]->element->value->floatValue >= 8.9877 &&
            jsonElement->value->objectValue->members[2]->element->value->floatValue <= 8.9878;

    jsonElement = parseJson(testJson2);
    success = jsonElement->value->valueType == JsonValueType::Array;
    jsonElement = jsonElement->value->arrayValue->elements[0];
    success = jsonElement->value->valueType == JsonValueType::Object;
    success = jsonElement->value->objectValue->members.size() == 5;
    success = jsonElement->value->objectValue->members[1]->element->value->valueType == JsonValueType::Number;
    success = jsonElement->value->objectValue->members[2]->element->value->valueType == JsonValueType::Number;
    success = jsonElement->value->objectValue->members[2]->element->value->floatValue >= 8.9877 &&
              jsonElement->value->objectValue->members[2]->element->value->floatValue <= 8.9878;


    return success;
}


enum class QueryTokenType {
    Slash,
    Star,
    DoubleStar,
    ID,
    SQ_OPEN,
    SQ_CLOSE,
    INDEX
};

struct QueryToken {
    QueryTokenType type;
    std::string stringValue;
    int intValue;
};

std::vector<QueryToken> lexQuery(const std::string& query) {
    std::vector<QueryToken> tokens;
    char lastChar = ' ';
    std::string starBuffer = "";
    std::string id="";
    for (auto c : query) {
        if (c == '/') {

            if (isalpha(lastChar)) {
                // End the current id
                tokens.push_back({QueryTokenType::ID, id});
                id.clear();
            }
            if (lastChar == '*') {
                if (starBuffer == "**") {
                    tokens.push_back({QueryTokenType::DoubleStar});
                } else {
                    tokens.push_back({QueryTokenType::Star});
                }
                starBuffer.clear();
            }
            tokens.push_back({QueryTokenType::Slash});

        }
        else if (c == '*') {
            starBuffer += c;
        }
        else if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) {
            id+=c;

        } else if (c >= 48 && c <= 57) {
            id += c;
        }
        else if (c == '[') {
            tokens.push_back({QueryTokenType::ID, id});
            id.clear();
            tokens.push_back({QueryTokenType::SQ_OPEN});
        } else if ( c== ']') {
            int num = std::stoi(id);
            id.clear();
            tokens.push_back({QueryTokenType::INDEX, "", num});
            tokens.push_back({QueryTokenType::SQ_CLOSE});
        }

        lastChar = c;
    }

    // Check for leftover tokens if the last char is not a slash
    if (lastChar != '/') {
        //Check for solitary star
        if (lastChar == '*') {
            tokens.push_back({QueryTokenType::Star});
        }
        if (isalpha(lastChar)) {
            tokens.push_back({QueryTokenType::ID, id});
        }
    }



    return tokens;
}



JsonElement* queryJson(JsonElement* root, const std::string& query) {
    auto toks = lexQuery(query);

    JsonElement* currentElem = nullptr;
    for (auto c : toks) {
        if (c.type == QueryTokenType::Slash) {
            if (!currentElem) {
                currentElem = root;
            }
        } else if (c.type == QueryTokenType::ID) {
            for (auto m : currentElem->value->objectValue->members) {
                if (m->name == c.stringValue) {
                    currentElem = m->element;
                    break;
                }
            }
        } else if (c.type == QueryTokenType::INDEX) {
            currentElem = currentElem->value->arrayValue->elements[c.intValue];

        }
    }
    return currentElem;
}

JsonValue *findByMemberName(JsonObject *input, const std::string &name) {
    auto ifit = std::find_if(input->members.begin(), input->members.end(), [&name](const JsonMember* arg) {
        return arg->name == name; });
    if (ifit != input->members.end()) {
        return (*ifit)->element->value;
    } else {
        return nullptr;
    }
}
