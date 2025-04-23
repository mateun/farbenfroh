//
// Created by mgrus on 20.04.2025.
//
#include "vm.h"
#include "script_runtime.h"

VM::VM(ScriptHost* host, const Bytecode& bytecode)
       : host(host), bytecode(bytecode) {}

void VM::run(ScriptContext& ctx, const Function& fn, const std::vector<Value>& args) {
    ip = fn.entry_point;
    stack = args; // use args as initial stack

    while (ip < bytecode.size()) {
        OpCode op = static_cast<OpCode>(bytecode[ip++]);
        switch (op) {
            case OpCode::PUSH_INT: {
                int val = read_int(bytecode, ip);
                stack.emplace_back(val);
                break;
            }
            case OpCode::PUSH_STRING: {
                std::string str = read_string(bytecode, ip);
                stack.emplace_back(str);
                break;
            }
            case OpCode::SEND: {
                std::string func = read_string(bytecode, ip);
                int argc = bytecode[ip++];

                std::vector<Value> args(argc);
                for (int i = argc - 1; i >= 0; --i)
                    args[i] = stack.back(), stack.pop_back();

                std::string target = stack.back().as_string(); stack.pop_back();

                Message msg { .function_name = func, .args = args, .sender = ctx.id };
                host->send_message(target, msg);

                break;
            }
            case OpCode::RETURN: {
                return;
            }
            default:
                printf("Unknown opcode: %d\n", static_cast<int>(op));
            return;
        }
    }
}

int read_int(const Bytecode& code, size_t& ip) {
    int val = (code[ip] << 24) | (code[ip + 1] << 16) | (code[ip + 2] << 8) | code[ip + 3];
    ip += 4;
    return val;
}

std::string read_string(const Bytecode& code, size_t& ip) {
    int len = code[ip++];
    std::string str(code.begin() + ip, code.begin() + ip + len);
    ip += len;
    return str;
}