//
// Created by mgrus on 20.04.2025.
//

#ifndef VM_H
#define VM_H

#include <cstdint>
#include <vector>
#include <string>

enum class OpCode : uint8_t {
    PUSH_INT,
    PUSH_STRING,
    SEND,
    RETURN,
};

using Bytecode = std::vector<uint8_t>;



class Value;
class ScriptHost;
class ScriptContext;
class Function;

int read_int(const Bytecode& code, size_t& ip);
std::string read_string(const Bytecode& code, size_t& ip);

class VM {
public:
    VM(ScriptHost* host, const Bytecode& bytecode);

    void run(ScriptContext& ctx, const Function& fn, const std::vector<Value>& args);

private:
    ScriptHost* host;
    const Bytecode& bytecode;
    std::vector<Value> stack;
    size_t ip = 0;
};




#endif //VM_H
