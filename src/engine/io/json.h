//
// Created by mgrus on 30.12.2023.
//

#ifndef SIMPLE_KING_JSON_H
#define SIMPLE_KING_JSON_H

#include <string>
#include <vector>

enum class JsonTokenType {
    CURL_OPEN,
    CURL_CLOSE,
    QUOTE,
    COLON,
    COMMA,
    DOT,
    SQUARE_OPEN,
    SQUARE_CLOSE,
    JID,
    JTEXT,
    JNUMBER,
    JBOOL,
};
struct JsonToken {
    JsonTokenType type;
    std::string textValue = "";
    float numValue = 0;
    bool boolValue = false;
};

enum class JsonValueType {
    Object,
    Array,
    String,
    Number,
    boolTrue,
    boolFalse,
    nullValue


};

class JsonObject;
class JsonValue;

struct JsonElement {
    JsonValue* value;
};

class JsonArray;

struct JsonValue {
    JsonValue* parent = nullptr;
    JsonValueType valueType;
    int intValue;
    float floatValue;
    bool boolValue;
    std::string stringValue;
    JsonObject* objectValue;
    JsonArray* arrayValue;

};

struct JsonArray {

    std::vector<JsonElement*> elements;
};


struct JsonMember {
    std::string name;
    JsonElement* element;
};


struct JsonObject {
    std::vector<JsonMember*> members;
};


// Get a JsonElement object from a json string
JsonElement* parseJson(const std::string json);

// Query for a given element within a Json hierarchy
JsonElement* queryJson(JsonElement* root, const std::string& query);
JsonValue* findByMemberName(JsonObject* input, const std::string& name);

bool testJson();

#endif //SIMPLE_KING_JSON_H
