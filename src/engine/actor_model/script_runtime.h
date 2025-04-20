#pragma once

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <variant>
#include "vm.h"


// ======================
// Basic VM Types
// ======================

struct ScriptDefinition;
using ScriptID = std::string; // Could be "enemy_1.ai", etc.

// Forward declaration
struct ScriptContext;

struct Message {
    std::string function_name;
    std::vector<class Value> args;
    ScriptID sender;
};

// ======================
// Value Type
// ======================

class Value {
public:
    enum class Type {
        INT,
        FLOAT,
        STRING,
        BOOL,
        // Add more types here
    };

    Value() : type(Type::INT), data(0) {}
    Value(int i) : type(Type::INT), data(i) {}
    Value(float f) : type(Type::FLOAT), data(f) {}
    Value(const std::string& s) : type(Type::STRING), data(s) {}
    Value(bool b) : type(Type::BOOL), data(b) {}

    Type type;

    std::variant<int, float, std::string, bool> data;

    int as_int() const    { return std::get<int>(data); }
    float as_float() const{ return std::get<float>(data); }
    const std::string& as_string() const { return std::get<std::string>(data); }
    bool as_bool() const  { return std::get<bool>(data); }
};

// ======================
// Script Host Interface
// ======================

struct ScriptHost {
    virtual ~ScriptHost() = default;

    virtual void send_message(const ScriptID& target, const Message& msg) = 0;
    virtual void delay_message(const ScriptID& target, const Message& msg, int delay_frames) = 0;

    virtual ScriptContext* get_script_context(const ScriptID& id) = 0;

    virtual const ScriptID& current_script_id() const = 0;
};

// ======================
// Function Stub (VM callback)
// ======================

struct Function {
    size_t entry_point; // Bytecode index or label
    int arg_count;
    // Later: support debug names, closures, etc.
};

// This is essentially the runtime state/memory
// of a single script instance.
// This allows the VM to be completely stateless and
// only serve as an agnostic bytecode interpreter, which always completely
// relies on the state of the passed in ScriptContext.
// It even allows advanced features like corouting, pre-emptive partial execution etc.
//
struct ScriptContext {
    ScriptID id;
    std::queue<Message> inbox;
    std::unordered_map<std::string, Function> functions;

    // Per-instance runtime data:
    std::vector<Value> stack;
    std::unordered_map<std::string, Value> locals;

    bool wants_tick = false; // For automatic on_update dispatching

    size_t ip = 0;                 // <— current instruction pointer
    bool is_running = false;      // <— paused but alive?
    const Bytecode* bytecode = nullptr;  // <— what we're running
};


class ScriptRuntime : public ScriptHost {
public:
    ScriptRuntime();

    // -- Host API --
    void send_message(const ScriptID& target, const Message& msg) override ;
    void delay_message(const ScriptID& target, const Message& msg, int delay_frames) override ;
    ScriptContext* get_script_context(const ScriptID& id) override;
    const ScriptID& current_script_id() const override;
    void registerScriptDefinition(const std::string& name, ScriptDefinition& def);

    // -- Script Management --
    ScriptContext& create_script(const ScriptID& id);
    void destroy_script(const ScriptID& id);
    void set_function(const ScriptID& id, const std::string& name, Function fn);

    // -- Main Update Loop --
    void tick_all_scripts() ;

private:
    int current_frame = 0;
    ScriptID current_sender;

    std::unordered_map<ScriptID, ScriptContext> scripts;
    std::unordered_map<int, std::vector<std::pair<ScriptID, Message>>> delayed_messages;

    std::unordered_map<std::string, ScriptDefinition> script_defs;

    void dispatch_delayed_messages();
    void run_script_function(ScriptContext& ctx, const Function& fn, const std::vector<Value>& args);
};

// A compiled script (already in bytecode).
struct ScriptDefinition {
    Bytecode code;
    std::unordered_map<std::string, Function> functions;
};

