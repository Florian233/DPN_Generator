#include "actor_gen.hpp"
#include "file_writer.hpp"
#include <set>
#include <string>
#include <vector>
#include "random.hpp"
#include <tuple>
#include <map>
#include <algorithm>
#include <cassert>

using State = std::string;
using Next_State = std::string;
using Action_Name = std::string;

static inline std::string get_filename_from_classpath(
	std::string class_path)
{
	size_t index = class_path.find_last_of(".");
	return class_path.substr(index + 1);
}

static inline void to_lower_case(std::string& str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void generate_input_actor(std::string class_path) {
	std::string a;

	a.append("actor " + class_path + "() ==> int Y0 : \n\n");
	a.append("\tint count := 0;\n\n");
	a.append("\tact1: action ==> Y0 : [count]\n");
	a.append("\t\tguard count < 10000000\n");
	a.append("\tdo\n");
	a.append("\t\tcount := count + 1;\n");
	a.append("\tend\n");
	a.append("end");

	write_file(true, class_path, a);
}

void generate_output_actor(std::string class_path) {
	std::string a;
	a.append("actor " + class_path + "() int X0 ==> :\n\n");
	a.append("\tuint count := 0;\n\n");
	a.append("\t@native procedure test_exit() end\n\n");
	a.append("\tact1 : action X0 : [x0] ==>\n");
	a.append("\tguard count < 10000000\n");
	a.append("\tdo\n");
	a.append("\t\tcount := count + 1;\n");
	a.append("\tend\n\n");
	a.append("\tact2 : action ==>\n");
	a.append("\tguard count = 10000000\n");
	a.append("\tdo\n");
	a.append("\t\ttest_exit();\n");
	a.append("\t\tcount := count + 1;\n");
	a.append("\tend\n");
	a.append("end");

	write_file(true, class_path, a);
}

void generate_native_code(unsigned produced_tokens) {
	std::string n;
	n.append("#include <windows.h>\n");
	n.append("#include <stdio.h>\n");
	n.append("#include <stdlib.h>\n\n");
	n.append("__declspec(align(4)) static volatile int cnt = 0;\n\n");
	n.append("void test_exit(void)\n");
	n.append("{\n");
	n.append("\tif (InterlockedIncrementAcquire(&cnt) == " + std::to_string(produced_tokens) + ") {\n");
	n.append("\t\texit(0);\n");
	n.append("\t}\n");
	n.append("}");

	write_native_file("native.c", n);
}

void generate_fork_code(
	unsigned produced_tokens,
	std::string class_path)
{
	std::string n;
	std::string name = get_filename_from_classpath(class_path);

	n.append("actor " + name + "() ");
	n.append("int cond");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", int X" + std::to_string(i));
	}
	n.append(" ==> int cond_out");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", int Y" + std::to_string(i));
		n.append(", int Z" + std::to_string(i));
	}
	n.append(" :\n\n");

	n.append("act1 : action cond : [c]");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", X" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append(" ==> cond_out : [c]");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", Y" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append("\nguard c < 1\nend\n\n");

	n.append("act2 : action cond : [c]");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", X" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append(" ==> cond_out : [c]");
	for (unsigned i = 0; i < produced_tokens; ++i) {
		n.append(", Z" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append("\nguard c >= 1\nend\n\n");

	n.append("end");

	write_file(true, name, n);
}

void generate_join_code(
	unsigned consumed_tokens,
	std::string class_path)
{
	std::string n;
	std::string name = get_filename_from_classpath(class_path);

	n.append("actor " + name + "() ");
	n.append("int cond");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		n.append(", int X" + std::to_string(i));
		n.append(", int W" + std::to_string(i));
	}
	n.append(" ==>");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		if (i != 0) {
			n.append(",");
		}
		n.append(" int Y" + std::to_string(i));
	}
	n.append(" :\n\n");

	n.append("act1 : action cond : [c]");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		n.append(", X" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append(" ==>");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		if (i != 0) {
			n.append(",");
		}
		n.append(" Y" + std::to_string(i) + " : [x" + std::to_string(i) + "]");
	}
	n.append("\nguard c < 1\nend\n\n");

	n.append("act2 : action cond : [c]");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		n.append(", W" + std::to_string(i) + " : [w" + std::to_string(i) + "]");
	}
	n.append(" ==>");
	for (unsigned i = 0; i < consumed_tokens; ++i) {
		if (i != 0) {
			n.append(",");
		}
		n.append(" Y" + std::to_string(i) + " : [w" + std::to_string(i) + "]");
	}
	n.append("\nguard c >= 1\nend\n\n");

	n.append("end");


	write_file(true, name, n);
}

static std::string generate_instruction(
	std::vector<std::string>& vars,
	std::vector<std::string>& ro_vars)
{
	assert(vars.size() > 0);
	unsigned index1 = rand_in_range(0, static_cast<unsigned>(vars.size()) - 1);
	unsigned index2 = rand_in_range(0, static_cast<unsigned>(vars.size()) - 1);
	unsigned index3 = rand_in_range(0, static_cast<unsigned>(vars.size()) - 1);

	std::string op;
	std::string lval = vars.at(index1);
	std::string arg1 = vars.at(index2);
	std::string arg2;

	/* can emit + - / * with const and var */
	unsigned inst = rand_in_range(0, 3);
	if (inst == 0) {
		/* + */
		op = " + ";
	}
	else if (inst == 1) {
		/* - */
		op = " - ";
	}
	else if (inst == 2) {
		/* * */
		op = " * ";
	}
	else {
		assert(inst == 3);
		/* / */
		op = " / ";
	}
	if (rand_bool() && !vars.empty()) {
		arg1 = vars.at(index2);
	}
	else if (!ro_vars.empty()) {
		unsigned indexr = rand_in_range(0, static_cast<unsigned>(ro_vars.size()) - 1);
		arg1 = ro_vars.at(indexr);
	}
	else {
		arg1 = std::to_string(rand_in_range(0, 33));
	}

	bool arg2_const = false;
	if (rand_bool() && (!vars.empty() || !ro_vars.empty())) {
		if (rand_bool() && !ro_vars.empty()) {
			unsigned indexr = rand_in_range(0, static_cast<unsigned>(ro_vars.size()) - 1);
			arg2 = ro_vars.at(indexr);
		}
		else {
			arg2 = vars.at(index3);
		}
	}
	else {
		arg2_const = true;
		arg2 = std::to_string(rand_in_range(1, 55));
	}

	if (!arg2_const && (inst == 3)) {
		/* Avoid division by zero */
		arg2 = "(" + arg2 + " + 1)";
	}

	return lval + " = " + arg1 + op + arg2 + ";\n";
}

static std::string generate_cond_block(
	std::vector<std::string>& vars,
	std::vector<std::string>& ro_vars,
	unsigned instructions,
	std::string prefix)
{
	std::string cond;
	std::string b1;
	std::string b2;

	Config* c = c->getInstance();
	assert(instructions > 0);
	assert(instructions <= c->get_max_inst());
	assert(vars.size() > 0);

	if ((rand_bool() && !vars.empty()) || ro_vars.empty()) {
		cond = vars.at(rand_in_range(0, static_cast<unsigned>(vars.size()) - 1));
	}
	else {
		cond = ro_vars.at(rand_in_range(0, static_cast<unsigned>(ro_vars.size()) - 1));
	}

	if (rand_bool()) {
		cond.append(" < ");
	}
	else {
		cond.append(" > ");
	}

	if (rand_bool()) {
		if ((rand_bool() && !vars.empty()) || ro_vars.empty()) {
			cond.append(vars.at(rand_in_range(0, static_cast<unsigned>(vars.size()) - 1)));
		}
		else {
			cond.append(ro_vars.at(rand_in_range(0, static_cast<unsigned>(ro_vars.size()) - 1)));
		}
	}
	else {
		cond.append(std::to_string(rand_in_range(0, 21)));
	}

	--instructions;

	for (unsigned i = instructions; i > 0;) {
		if ((i > 5) && rand_bool_dist(10)) {
			unsigned bound = i / 3;
			unsigned inst = rand_in_range(2, bound);
			b1.append(generate_cond_block(vars, ro_vars, inst, prefix + "\t"));
			i -= inst;
		}
		else {
			b1.append(prefix + "\t" + generate_instruction(vars, ro_vars));
			--i;
		}
	}

	for (unsigned i = instructions; i > 0;) {
		if ((i > 5) && rand_bool_dist(10)) {
			unsigned bound = i / 3;
			unsigned inst = rand_in_range(2, bound);
			b2.append(generate_cond_block(vars, ro_vars, inst, prefix + "\t"));
			i -= inst;
		}
		else {
			b2.append(prefix + "\t" + generate_instruction(vars, ro_vars));
			--i;
		}
	}

	std::string n;
	n = prefix + "if (" + cond + ") then\n";
	n.append(b1);
	if (!b2.empty()) {
		n.append(prefix + "else\n");
		n.append(b2);
	}
	n.append(prefix + "end\n");

	return n;
}

static std::string generate_loop(
	std::vector<std::string>& vars,
	std::vector<std::string> ro_vars,
	unsigned instructions,
	std::string prefix)
{
	Config* c = c->getInstance();
	assert(instructions > 0);
	assert(instructions <= c->get_max_inst());
	assert(vars.size() > 0);

	std::string cond;
	std::string block;

	cond = "int x_i in 0 .. " + std::to_string(rand_in_range(1, 11));
	ro_vars.push_back("x_i");

	--instructions;

	for (unsigned i = instructions; i > 0;) {
		if (rand_bool() && (i > 1)) {
			unsigned inst = rand_in_range(2, i);
			block.append(generate_cond_block(vars, ro_vars, inst, prefix + "\t"));
			i -= inst;
		}
		else {
			block.append(prefix + "\t" + generate_instruction(vars, ro_vars));
			--i;
		}
	}

	std::string n;
	n = prefix + "foreach " + cond + " do\n";
	n.append(block);
	n.append(prefix + "end\n");

	return n;
}

static std::string generate_action(
	std::string name,
	std::string guard,
	std::string code,
	std::map<std::string, unsigned> inports,
	std::map<std::string, unsigned> outports,
	unsigned num_instructions,
	Actor_Complexity complexity)
{
	Config* c = c->getInstance();
	assert(num_instructions > 0);
	assert(num_instructions <= c->get_max_inst());

	std::vector<std::string> vars;
	std::vector<std::string> ro_vars;
	std::vector<std::string> output_vars;

	std::string n;
	n.append("\t" + name + " : action");
	for (auto it = inports.begin(); it != inports.end(); ++it) {
		if (it != inports.begin()) {
			n.append(",");
		}
		n.append(" " + it->first + " : [");
		std::string x = it->first;
		to_lower_case(x);

		for (unsigned i = 0; i < it->second; ++i) {
			if (i != 0) {
				n.append(", ");
			}
			std::string y = x + "_" + std::to_string(i);
			ro_vars.push_back(y);
			n.append(y);
		}

		n.append("]");
	}

	n.append(" ==>");
	for (auto it = outports.begin(); it != outports.end(); ++it) {
		if (it != outports.begin()) {
			n.append(",");
		}
		n.append(" " + it->first + " : [");
		std::string x = it->first;
		to_lower_case(x);

		for (unsigned i = 0; i < it->second; ++i) {
			if (i != 0) {
				n.append(", ");
			}
			std::string y = x + "_" + std::to_string(i);
			output_vars.push_back(y);
			n.append(y);
		}
		n.append("]");
	}

	n.append("\n");
	if (!guard.empty()) {
		n.append("\tguard\n");
		n.append(guard);
		n.append("\n");
	}

	n.append("\tvar\n");
	for (auto x : output_vars) {
		n.append("\t\tint " + x + ",\n");
	}

	unsigned e = (num_instructions / 7) + 1;
	for (unsigned i = 0; i < e; ++i) {

		n.append("\t\tint l_" + std::to_string(i));
		vars.push_back("l_" + std::to_string(i));
		n.append(" := " + std::to_string(rand_in_range(1, 17)));

		if (i < (e - 1)) {
			n.append(",");
		}
		n.append("\n");
	}

	n.append("\tdo\n");

	for (unsigned i = num_instructions; i > 0;) {
		if (complexity == Actor_Complexity::Cond && rand_bool_dist(i) && !vars.empty() && (i > 3)) {
			unsigned num_inst = (i > 15) ? 15 : i;
			num_inst = rand_in_range(3, num_inst);
			n.append(generate_cond_block(vars, ro_vars, num_inst, "\t\t"));
			i -= num_inst;
		}
		else if (complexity == Actor_Complexity::Loop && rand_bool_dist(i) && !vars.empty() && (i > 3)) {
			unsigned num_inst = (i > 15) ? 15 : i;
			num_inst = rand_in_range(3, num_inst);
			n.append(generate_loop(vars, ro_vars, num_inst, "\t\t"));
			i -= num_inst;
		}
		else {
			n.append("\t\t" + generate_instruction(vars, ro_vars));
			--i;
		}
	}

	for (auto o : output_vars) {
		if (vars.empty()) {
			unsigned index = rand_in_range(0, static_cast<unsigned>(ro_vars.size()) - 1);
			std::string v = ro_vars.at(index);
			n.append("\t\t" + o + " := " + v + ";\n");
		}
		else {
			unsigned index = rand_in_range(0, static_cast<unsigned>(vars.size()) - 1);
			std::string v = vars.at(index);
			n.append("\t\t" + o + " := " + v + ";\n");
		}
	}

	n.append(code);
	n.append("\tend\n\n");

	return n;
}

static std::string generate_init_action(
	std::vector<Port>& out)
{
	std::string n = "";

	std::vector<std::string> channels;
	for (auto o : out) {
		if (o.feedback) {
			channels.push_back(o.name);
		}
	}

	if (!channels.empty()) {

		std::vector<std::string> vars;

		n.append("\tinitialize ==>");

		for (auto c = channels.begin(); c != channels.end(); ++c) {
			if (c != channels.begin()) {
				n.append(",");
			}
			n.append(" " + *c + " : [");
			std::string x = *c;
			to_lower_case(x);
			x.append("_0");
			n.append(x + "]");
			vars.push_back(x);
		}

		n.append("\tvar\n");

		for (auto v : vars) {
			n.append("\t\t" + v + " := " + std::to_string(rand_in_range(1, 5)));
			if (vars.back() != v) {
				n.append(",");
			}
			n.append("\n");
		}

		n.append("\tend\n\n");
	}

	return n;
}

struct action_config {
	std::map<std::string, unsigned> in;
	std::map<std::string, unsigned> out;
	std::string guard;
	std::string code;
};

static std::vector<action_config> generate_static_action_configs(
	unsigned num_actions,
	std::vector<Port>& in,
	std::vector<Port>& out,
	bool complex_guard,
	std::vector<std::string> globals)
{
	std::vector<action_config> result;

	if (num_actions == 0) {
		return result;
	}

	std::map<std::string, unsigned> inp;
	std::map<std::string, unsigned> outp;

	for (auto x = in.begin(); x != in.end(); ++x) {
		inp[x->name] = x->num_tokens;
	}

	for (auto x = out.begin(); x != out.end(); ++x) {
		outp[x->name] = x->num_tokens;
	}

	unsigned guard_factors = static_cast<unsigned>(log2(num_actions));

	std::map<std::string, bool> global_var_update;
	std::map<std::string, bool> input_used;

	std::vector<std::pair<std::string, int>> guard_factors_list;

	if (guard_factors > 0) {

		for (unsigned i = 0; i < guard_factors; ++i) {
			if ((rand_bool() || !complex_guard || (input_used.size() == in.size())) && !globals.empty() && (global_var_update.size() < globals.size())) {
				std::string f = globals.at(rand_in_range(0, static_cast<unsigned>(globals.size()) - 1));
				while (global_var_update.contains(f)) {
					f = globals.at(rand_in_range(0, static_cast<unsigned>(globals.size()) - 1));
				}
				guard_factors_list.push_back(std::make_pair(f, rand_in_range(1, 5)));
				global_var_update[f] = false;
			}
			else if (complex_guard && !in.empty() && (input_used.size() < in.size())) {
				std::string f = in.at(rand_in_range(0, static_cast<unsigned>(in.size()) - 1)).name;
				while (input_used.contains(f)) {
					f = in.at(rand_in_range(0, static_cast<unsigned>(in.size()) - 1)).name;
				}
				to_lower_case(f);
				f.append("_0");
				guard_factors_list.push_back(std::make_pair(f, rand_in_range(1, 5)));
				input_used[f] = false;
			}
			else {
				assert((!complex_guard && (globals.size() < guard_factors)) || (complex_guard && ((globals.size() + in.size()) < guard_factors)));
			}
		}
	}

	for (unsigned i = 0; i < num_actions; ++i) {
		std::string code = "";
		std::string guard = "";

		action_config a;
		a.in = inp;
		a.out = outp;

		if (!guard_factors_list.empty()) {

			for (unsigned j = 0; j < guard_factors_list.size(); ++j) {
				std::string c = guard_factors_list.at(j).first;
				bool s;
				if (!guard.empty()) {
					guard.append(" && ");
				}
				if (i & (1 << j)) {
					guard.append(c + " < " + std::to_string(guard_factors_list.at(j).second));
					s = true;
				}
				else {
					guard.append(c + " >= " + std::to_string(guard_factors_list.at(j).second));
					s = false;
				}

				if (global_var_update.contains(c)) {
					code.append("\t\t" + c + " := (" + c + " + 1)");

					if (!s) {
						code.append(" % " + std::to_string(guard_factors_list.at(j).second + rand_in_range(0, 11)));
					}

					code.append(";\n");
				}
			}
		}

		a.code = code;
		a.guard = guard;

		result.push_back(a);
	}
	return result;
}

static std::vector<action_config> generate_dynamic_action_configs(
	unsigned num_actions,
	std::vector<Port>& in,
	std::vector<Port>& out,
	bool complex_guard,
	std::vector<std::string> globals,
	bool fsm)
{
	std::vector<action_config> result;

	if (num_actions == 0) {
		return result;
	}

	if (fsm || rand_bool_dist(5) || (num_actions < 3)) {
		result = generate_static_action_configs(num_actions, in, out, complex_guard, globals);

		for (auto it = result.begin(); it != result.end(); ++it) {
			for (auto in_it = it->in.begin(); in_it != it->in.end(); ++in_it) {
				if (rand_bool() && (in_it->second > 1)) {
					in_it->second = in_it->second / 2;
				}
				else {
					if (in_it->second != 1) {
						in_it->second -= rand_in_range(0, in_it->second - 1);
					}
				}
			}
			for (auto out_it = it->out.begin(); out_it != it->out.end(); ++out_it) {
				if (rand_bool() && (out_it->second > 1)) {
					out_it->second = out_it->second / 2;
				}
				else {
					if (out_it->second != 1) {
						out_it->second -= rand_in_range(0, out_it->second - 1);
					}
				}
			}
		}

		return result;
	}

	std::vector<Port> guard_factors;
#if 0
	//This cannot be used, token values are not predictable and therefore the scheduling for dynamic actions with different tokenrates cannot depend on them. We need homogenous output rates!
	for (auto x : in) {
		if (x.num_tokens > 1) {
			guard_factors.push_back(x);
		}
	}
#endif

	/* Actions are generated in pairs of two that cover together all channels. */
	while (num_actions >= 2 && (!guard_factors.empty() || !globals.empty())) {
		std::string factor;
		bool p = false;
		Port port;
		if (!guard_factors.empty() && !globals.empty()) {
			if (rand_bool()) {
				p = true;
				port = guard_factors.at(rand_in_range(0, static_cast<unsigned>(guard_factors.size()) - 1));
				factor = port.name;
				to_lower_case(factor);
				factor.append("_0");
			}
			else {
				factor = globals.at(rand_in_range(0, static_cast<unsigned>(globals.size()) - 1));
			}
		}
		else if (!guard_factors.empty()) {
			p = true;
			port = guard_factors.at(rand_in_range(0, static_cast<unsigned>(guard_factors.size()) - 1));
			factor = port.name;
			to_lower_case(factor);
			factor.append("_0");
		}
		else if (!globals.empty()) {
			factor = globals.at(rand_in_range(0, static_cast<unsigned>(globals.size()) - 1));
		}
		else {
			/* This cannot happen. */
			assert(0);
		}

		std::map<std::string, unsigned> inp1;
		std::map<std::string, unsigned> inp2;
		std::map<std::string, unsigned> outp1;
		std::map<std::string, unsigned> outp2;

		std::string guard1, guard2;
		std::string code1, code2;

		for (auto it = in.begin(); it != in.end(); ++it) {
			if (p && (port.name == it->name)) {
				unsigned t = rand_in_range(1, it->num_tokens - 1);
				inp1[it->name] = t;
				inp2[it->name] = it->num_tokens - t;
			}
			else {
				if (it->num_tokens > 1 && rand_bool()) {
					unsigned t = rand_in_range(1, it->num_tokens - 1);
					inp1[it->name] = t;
					inp2[it->name] = it->num_tokens - t;
				}
				else {
					if (rand_bool()) {
						inp1[it->name] = it->num_tokens;
					}
					else {
						inp2[it->name] = it->num_tokens;
					}
				}
			}
		}
		for (auto it = out.begin(); it != out.end(); ++it) {
			if (it->num_tokens > 1 && rand_bool()) {
				unsigned t = rand_in_range(1, it->num_tokens - 1);
				outp1[it->name] = t;
				outp2[it->name] = it->num_tokens - t;
			}
			else {
				if (rand_bool()) {
					outp1[it->name] = it->num_tokens;
				}
				else {
					outp2[it->name] = it->num_tokens;
				}
			}
		}

		unsigned bound = rand_in_range(5, 13);
		guard1 = factor + " < " + std::to_string(bound);
		guard2 = factor + " >= " + std::to_string(bound);
		if (!p) {
			/* Must be based on state var, we must update it */
			code1 = "\t\t" + factor + " := " + factor + " + 1;\n";
			code2 = "\t\t" + factor + " := (" + factor + " + 1) % " + std::to_string(bound + rand_in_range(4, 18)) + "; \n";
		}

		action_config one;
		action_config two;

		one.in = inp1;
		one.out = outp1;
		one.code = code1;
		one.guard = guard1;

		two.in = inp2;
		two.out = outp2;
		two.code = code2;
		two.guard = guard2;

		result.push_back(one);
		result.push_back(two);

		num_actions -= 2;
	}

	/* Remaining possibly 1 action is like static, or all actions if not state vars. */
	auto x = generate_static_action_configs(num_actions, in, out, complex_guard, globals);

	for (auto a : x) {
		result.push_back(a);
	}

	return result;
}

static std::string generate_actions(
	bool dynamic,
	unsigned num_actions,
	std::vector<Port>& in,
	std::vector<Port>& out,
	bool fsm,
	std::vector<std::tuple<State, Action_Name, Next_State>>& fsm_def,
	unsigned num_states,
	bool prios,
	std::vector<std::pair<Action_Name, Action_Name>>& prio_def,
	unsigned max_instructions,
	Actor_Complexity complexity,
	bool complex_guard,
	std::vector<std::string> globals)
{
	std::string n = generate_init_action(out);
	Config* c = c->getInstance();

	std::vector<std::string> action_names;
	unsigned action_count = 0;

	if (fsm) {
		bool use_empty = false;
		std::string empty_action = "";
		bool cyclic = rand_bool();
		if ((num_states > num_actions) || (prios && (num_states == num_actions))) {
			/* We don't have sufficient number of actions to have distinct actions for each state, so we need an empty guard action. */
			use_empty = true;
			empty_action = "act0";
			++action_count;
			/* only static is sufficient here, the result is the same anyhow. */
			auto v = generate_static_action_configs(1, in, out, complex_guard, globals);
			assert(v.size() == 1);
			assert(v.front().guard == "");
			action_names.push_back(empty_action);
			n.append(generate_action(empty_action, "", v.front().code, v.front().in, v.front().out, max_instructions, complexity));
		}

		for (unsigned i = 0; i < num_states; ++i) {
			unsigned this_num = (num_actions - action_count) / (num_states - i);

			std::vector<action_config> v;

			if (dynamic) {
				v = generate_dynamic_action_configs(this_num, in, out, complex_guard, globals, true);
			}
			else {
				v = generate_static_action_configs(this_num, in, out, complex_guard, globals);
			}

			unsigned first = true;

			std::vector<std::string> actions_in_state;

			for (auto x : v) {
				std::string name = "act" + std::to_string(action_count);
				++action_count;
				unsigned inst;
				if (out.size() > max_instructions) {
					inst = max_instructions;
				}
				else {
					inst = rand_in_range(static_cast<unsigned>(out.size()), max_instructions);
				}
				assert(inst > 0 && inst <= c->get_max_inst());
				action_names.push_back(name);
				n.append(generate_action(name, x.guard, x.code, x.in, x.out, inst, complexity));

				if (prios && !first) {
					std::string pn = "act" + std::to_string(action_count - 1);
					prio_def.push_back(std::make_pair(name, pn));
				}
				else if (prios && use_empty && (this_num == 1)) {
					prio_def.push_back(std::make_pair(name, empty_action));
					actions_in_state.push_back(empty_action);
				}

				actions_in_state.push_back(name);

				first = false;
			}

			if (actions_in_state.empty()) {
				actions_in_state.push_back(empty_action);
			}
			
			/* Define FSM */
			std::string state = "state" + std::to_string(i);
			std::string next_cyle = "state" + std::to_string((i + 1) % num_states);

			if (cyclic) {
				for (auto x : actions_in_state) {
					fsm_def.push_back(std::make_tuple(state, x, next_cyle));
				}
			}
			else {
				/* Yes, this can the the same as next_cycle ... bad luck, this is random anyhow. */
				std::string next = "state" + std::to_string(rand_in_range(0, num_states - 1));

				for (auto it = actions_in_state.begin(); it != actions_in_state.end(); ++it) {
					if (it == actions_in_state.begin()) {
						fsm_def.push_back(std::make_tuple(state, *it, next_cyle));
					}
					else {
						if (rand_bool()) {
							fsm_def.push_back(std::make_tuple(state, *it, next_cyle));
						}
						else {
							fsm_def.push_back(std::make_tuple(state, *it, next));
						}
					}
				}
 			}
		}

	}
	else {
		std::vector<action_config> a;
		if (dynamic) {
			a = generate_dynamic_action_configs(num_actions, in, out, complex_guard, globals, false);
		}
		else {
			a = generate_static_action_configs(num_actions, in, out, complex_guard, globals);
		}

		for (auto x : a) {
			std::string name = "act" + std::to_string(action_count);
			++action_count;
			unsigned inst;
			if (out.size() > max_instructions) {
				inst = max_instructions;
			}
			else {
				inst = rand_in_range(static_cast<unsigned>(out.size()), max_instructions);
			}
			assert(inst > 0 && inst <= c->get_max_inst());
			action_names.push_back(name);
			n.append(generate_action(name, x.guard, x.code, x.in, x.out, inst, complexity));

			if (prios && action_count > 1) {
				std::string pn = "act" + std::to_string(action_count - 1);
				prio_def.push_back(std::make_pair(name, pn));
			}
		}
	}

	return n;
}

static void print_actor_config(
	Actor* actor)
{
	std::cout << "Actor: " << actor->class_path << std::endl;
	std::cout << "Num actions: " << actor->num_actions << ", max instructions: " << actor->num_instructions << " dynamic: " << actor->dynamic << std::endl;
	if (actor->FSM) {
		std::cout << "FSM: " << actor->num_states << std::endl;
	}
	else {
		std::cout << "No FSM." << std::endl;
	}
	for (auto it = actor->inports.begin(); it != actor->inports.end(); ++it) {
		std::cout << "In: " << it->name << " rate: " << it->num_tokens << " feedback: " << it->feedback << std::endl;
	}
	for (auto it = actor->outports.begin(); it != actor->outports.end(); ++it) {
		std::cout << "Out: " << it->name << " rate: " << it->num_tokens << " feedback: " << it->feedback << std::endl;
	}
	std::cout << "Complexity: ";
	switch (actor->complexity) {
	case Actor_Complexity::Cond: std::cout << "Cond"; break;
	case Actor_Complexity::Loop: std::cout << "Loop"; break;
	case Actor_Complexity::Simple: std::cout << "Simple"; break;
	default: std::cout << "Unknown"; break;
	}
	std::cout << std::endl;
}

void generate_actor(
	Actor* actor)
{
	std::string n;
	std::string name = get_filename_from_classpath(actor->class_path);

#ifndef NDEBUG
	print_actor_config(actor);
#endif

	n.append("actor " + name + "()");
	for (auto it = actor->inports.begin(); it != actor->inports.end(); ++it) {
		if (it != actor->inports.begin()) {
			n.append(",");
		}
		n.append(" int " + it->name);
	}
	n.append(" ==>");
	for (auto it = actor->outports.begin(); it != actor->outports.end(); ++it) {
		if (it != actor->outports.begin()) {
			n.append(",");
		}
		n.append(" int " + it->name);
	}
	n.append(" :\n\n");

	std::vector<std::string> global_vars;
	std::vector<std::pair<Action_Name, Action_Name>> prios;
	State start_state = "state0";
	std::vector<std::tuple<State, Action_Name, Next_State>> fsm;

	if (actor->state_vars) {
		unsigned num;
		if (actor->num_instructions < 20) {
			num = 1;
		}
		else {
			num = rand_in_range(1, actor->num_instructions / 10);
		}
		for (unsigned i = 0; i < num; ++i) {
			n.append("\tint gstate_" + std::to_string(i) + " := 0;\n");
			global_vars.push_back("gstate_" + std::to_string(i));
		}
		n.append("\n");
	}

	n.append(generate_actions(actor->dynamic, actor->num_actions, actor->inports, actor->outports, actor->FSM, fsm, actor->num_states,
			                  actor->priorities, prios, actor->num_instructions, actor->complexity, actor->complex_guards, global_vars));

	if (actor->FSM) {
		n.append("\tschedule fsm " + start_state + ":\n");
		for (auto it = fsm.begin(); it != fsm.end(); ++it) {
			n.append("\t\t" + std::get<0>(*it) + "(" + std::get<1>(*it) + ") --> " + std::get<2>(*it) + ";\n");
		}
		n.append("\tend\n\n");
	}

	if (actor->priorities) {
		n.append("\tpriority\n");
		for (auto it = prios.begin(); it != prios.end(); ++it) {
			n.append("\t\t" + it->first + " > " + it->second+";\n");
		}
		n.append("\tend\n\n");
	}

	n.append("end");

	write_file(true, name, n);
}
