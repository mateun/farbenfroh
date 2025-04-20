//
// Created by mgrus on 20.04.2025.
//

#ifndef ENGINE_HOST_H
#define ENGINE_HOST_H

#include <string>
#include <unordered_map>
#include <queue>


using ScriptID = std::string;

// The VM visible actor instance:
struct ScriptContext {
    ScriptID id;
    std::unordered_map<std::string, Function> functions; // registered funcs
    std::queue<Message> inbox;
    ValueStack stack;
    Heap heap;

};




// Represents a "kernel" to the VM, where syscalls can be called.
struct ScriptHost {
    virtual void send_message(ScriptID target, const Message& msg) = 0;
    virtual void delay_message(ScriptID target, const Message& msg, int delay) = 0;
    virtual ScriptContext* get_script_context(ScriptID id) = 0;
    virtual const ScriptID& current_script_id() const = 0;
};




#endif //ENGINE_HOST_H
