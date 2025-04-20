//
// Created by mgrus on 20.04.2025.
//

#pragma once

#include "script_runtime.h"

#include "vm.h"

ScriptRuntime::ScriptRuntime() {}

    // -- Host API --
    void ScriptRuntime::send_message(const ScriptID& target, const Message& msg)  {
        auto* ctx = get_script_context(target);
        if (ctx) {
            ctx->inbox.push(msg);
        }
    }

    void ScriptRuntime::delay_message(const ScriptID& target, const Message& msg, int delay_frames)  {
        delayed_messages[current_frame + delay_frames].emplace_back(target, msg);
    }

    ScriptContext* ScriptRuntime::get_script_context(const ScriptID& id)  {
        auto it = scripts.find(id);
        return it != scripts.end() ? &it->second : nullptr;
    }

    const ScriptID& ScriptRuntime::current_script_id() const  {
        return current_sender;
    }

void ScriptRuntime::registerScriptDefinition(const std::string &name, ScriptDefinition &def) {
    script_defs[name] = def;
}

// -- Script Management --
    ScriptContext& ScriptRuntime::create_script(const ScriptID& id) {
        ScriptContext ctx;
        ctx.id = id;
        auto script_name = id.substr(0, id.find_last_of('.'));
        ctx.functions = script_defs[script_name].functions;
        return scripts.emplace(id, std::move(ctx)).first->second;
    }

    void ScriptRuntime::destroy_script(const ScriptID& id) {
        scripts.erase(id);
    }

    void ScriptRuntime::set_function(const ScriptID& id, const std::string& name, Function fn) {
        scripts[id].functions[name] = fn;
    }

    // -- Main Update Loop --
    void ScriptRuntime::tick_all_scripts() {
        ++current_frame;
        dispatch_delayed_messages();

        for (auto& [id, script] : scripts) {
            if (script.inbox.empty()) continue;

            while (!script.inbox.empty()) {
                Message msg = script.inbox.front(); script.inbox.pop();
                auto fn_it = script.functions.find(msg.function_name);
                if (fn_it == script.functions.end()) continue;

                current_sender = msg.sender;

                // Stub — we’ll wire in VM execution later
                run_script_function(script, fn_it->second, msg.args);

                current_sender.clear();
            }
        }
    }

    void ScriptRuntime::dispatch_delayed_messages() {
        auto it = delayed_messages.find(current_frame);
        if (it != delayed_messages.end()) {
            for (const auto& [target, msg] : it->second) {
                send_message(target, msg);
            }
            delayed_messages.erase(it);
        }
    }

    void ScriptRuntime::run_script_function(ScriptContext& ctx, const Function& fn, const std::vector<Value>& args) {
        VM vm(this, *ctx.bytecode);
        vm.run(ctx, fn, args);
        //printf("[Script %s] Running %s(%zu args)\n", ctx.id.c_str(), fn.entry_point ? "user_fn" : "???", args.size());
    }

