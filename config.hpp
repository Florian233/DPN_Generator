#pragma once

#include <filesystem>

enum Actor_Complexity
{
    Simple,
    Cond,
    Loop
};

class Config {
    static Config* instance;

    unsigned num_nodes = 0;
    bool dynamic = false;
    bool cond_flow_dynamic = false;
    unsigned num_inputs = 0;
    unsigned num_outputs = 0;
    unsigned min_inst = 1;
    unsigned max_inst = 200;
    Actor_Complexity complexity = Actor_Complexity::Simple;
    bool feedback_loops = false;
    unsigned max_ports = 0;
    bool fsm = false;
    bool priorities = false;
    bool state_vars = false;
    unsigned max_tokenrate = 0;
    unsigned max_feedback_loops = 10;

    bool uneq_tokenrates = false;

    std::filesystem::path output_dir;


public:
    static Config* getInstance() {
        if (!instance) {
            instance = new Config;
        }
        return instance;
    }

    void set_num_nodes(unsigned n) {
        num_nodes = n;
    }
    unsigned get_num_nodes(void) {
        return num_nodes;
    }

    void set_dynamic(void) {
        dynamic = true;
    }
    bool get_dynamic(void) {
        return dynamic;
    }

    void set_num_inputs(unsigned i) {
        num_inputs = i;
    }
    unsigned get_num_inputs(void) {
        return num_inputs;
    }

    void set_num_outputs(unsigned o) {
        num_outputs = o;
    }
    unsigned get_num_outputs(void) {
        return num_outputs;
    }

    void set_min_inst(unsigned m) {
        min_inst = m;
    }
    unsigned get_min_inst(void) {
        return min_inst;
    }

    void set_max_inst(unsigned m) {
        max_inst = m;
    }
    unsigned get_max_inst(void) {
        return max_inst;
    }

    void set_complexity(Actor_Complexity c) {
        complexity = c;
    }
    Actor_Complexity get_complexity(void) {
        return complexity;
    }

    void set_feedback_loops(void) {
        feedback_loops = true;
    }
    bool get_feedback_loops(void) {
        return feedback_loops;
    }

    void set_max_ports(unsigned m) {
        max_ports = m;
    }
    unsigned get_max_ports(void) {
        return max_ports;
    }

    void set_output_dir(std::filesystem::path p) {
        output_dir = p;
    }
    std::filesystem::path get_output_dir(void) {
        return output_dir;
    }

    void set_fsm(bool f) {
        fsm = f;
    }
    bool get_fsm(void) {
        return fsm;
    }

    void set_priorities(bool p) {
        priorities = p;
    }
    bool get_priorities(void) {
        return priorities;
    }

    void set_statevars(bool s) {
        state_vars = s;
    }
    bool get_statevars(void) {
        return state_vars;
    }

    void set_max_tokenrate(unsigned mt) {
        max_tokenrate = mt;
    }
    unsigned get_max_tokenrate(void) {
        return max_tokenrate;
    }

    void set_cond_flow_dynamic(void) {
        cond_flow_dynamic = true;
    }
    bool get_cond_flow_dynamic(void) {
        return cond_flow_dynamic;
    }

    void set_num_feedbackcycles(unsigned f) {
        max_feedback_loops = f;
    }
    unsigned get_num_feedbackcycles(void) {
        return max_feedback_loops;
    }

    void set_uneq_tokenrates(void) {
        uneq_tokenrates = true;
    }
    bool get_uneq_tokenrates(void) {
        return uneq_tokenrates;
    }
};