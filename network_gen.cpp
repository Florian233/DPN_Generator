#include "network_gen.hpp"
#include "config.hpp"
#include "Network.hpp"
#include "actor_gen.hpp"
#include "file_writer.hpp"
#include "random.hpp"
#include <cassert>

static Actor* fork_gen[10];
static Actor* join_gen[10];

static void generate_path(
	Network* net,
	unsigned remaining_actors,
	std::vector<Actor_Instance_Port>& open_ports,
	unsigned outputs,
	unsigned actor_count,
	unsigned max_ports);


static void write_network(
	Network* net)
{
	std::string n =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<XDF name=\"gen\">\n";

	for (auto it = net->get_actor_instances().begin(); it != net->get_actor_instances().end(); ++it) {
		n.append("<Instance id=\""+ (*it)->name +"\">\n");
		n.append("\t<Class name=\""+ (*it)->actor->class_path +"\"/>\n");
		n.append("</Instance>\n");
	}

	for (auto it = net->get_connections().begin(); it != net->get_connections().end(); ++it) {
		n.append("<Connection dst=\""+ it->sink->name +"\" dst-port=\""+ it->sink_port +"\" src=\""+ it->source->name +"\" src-port=\""+ it->source_port +"\"/>\n");
	}

	n.append("</XDF>");

	write_file(false, "gen", n);
}

static bool evaluate_port_criteria(
	unsigned remaining_actors,
	unsigned max_ports,
	unsigned cur_ports_in,
	unsigned cur_ports_out,
	unsigned remaining_open_channels,
	unsigned outputs)
{
	remaining_actors -= 1; /* One is used for the one we are evaluating currently. */
	unsigned open_after = remaining_open_channels + cur_ports_out - cur_ports_in;


	int upper_bound = remaining_actors * max_ports - remaining_actors + open_after;
	int lower_bound = remaining_actors - (int)(remaining_actors * max_ports) + open_after;

	std::cout << "Upper bound: " << upper_bound << " Lower bound: " << lower_bound << " rem: " << remaining_actors << " remo:" << remaining_open_channels << " in: " << cur_ports_in << " out: " << cur_ports_out << " outs: " << outputs << std::endl;

	if (upper_bound < (int)outputs) {
		return false;
	}
	else if (lower_bound > (int)outputs) {
		return false;
	}
	else {
		return true;
	}
}

static void get_valid_port_nums(
	unsigned remaining_actors,
	unsigned max_ports,
	unsigned remaining_open_channels,
	unsigned* in,
	unsigned* out,
	unsigned outputs)
{
	assert(remaining_actors >= 1);
	assert(max_ports >= 1);
	assert(remaining_open_channels >= 1);
	assert(outputs >= 1);

	unsigned in_tmp, out_tmp;
	unsigned range = max_ports < remaining_open_channels ? max_ports : remaining_open_channels;
	in_tmp = rand_in_range(1, range);
	out_tmp = rand_in_range(1, max_ports);

	while (!evaluate_port_criteria(remaining_actors, max_ports, in_tmp, out_tmp, remaining_open_channels, outputs)) {
		in_tmp = rand_in_range(1, range);
		out_tmp = rand_in_range(1, max_ports);
	}

	*in = in_tmp;
	*out = out_tmp;
}

static bool get_parallel_net_conf(
	unsigned remaining_actors,
	unsigned max_ports,
	unsigned remaining_open_channels,
	unsigned* in,
	unsigned* out,
	unsigned* actors,
	unsigned outputs)
{
	unsigned num_a, num_in, num_out;

	if (remaining_actors/4 < 4) {
		return false;
	}
	if (remaining_open_channels < 2) {
		return false;
	}

	num_a = remaining_actors / 4;
	num_a = rand_in_range(4, num_a);
	/* Limit it a little. */
	if (num_a > 30) {
		num_a = 30;
	}

	unsigned range = remaining_open_channels < max_ports ? remaining_open_channels : max_ports;

	do {
		num_in = rand_in_range(2, range);
		num_out = num_in - 1;

		if (!evaluate_port_criteria(num_a, max_ports, num_in, num_out, remaining_open_channels, outputs)) {

			if (rand_bool_dist(4)) {
				--num_a;
			}
		}
		else {
			break;
		}
	} while (num_a >= 4);

	if (num_a < 4) {
		return false;
	}

	*in = num_in;
	*out = num_out;
	*actors = num_a;
	return true;
}

static Actor_Complexity get_complexity(
	Actor_Complexity max)
{
	unsigned top = 0;
	if (max == Actor_Complexity::Simple) {
		return Actor_Complexity::Simple;
	}
	else if (max == Actor_Complexity::Cond) {
		top = 1;
	}
	else if (max == Actor_Complexity::Loop) {
		top = 2;
	}
	else {
		/* Doesn't exist.... */
		assert(0);
	}
	unsigned rand = rand_in_range(0, top);

	if (rand == 0) {
		return Actor_Complexity::Simple;
	}
	else if (rand == 1) {
		return Actor_Complexity::Cond;
	}
	else {
		assert(rand == 2);
		return Actor_Complexity::Loop;
	}
}

static void set_default_tokenrate(
	Network *net,
	std::string actor,
	std::string port)
{
	for (auto it = net->get_actors().begin(); it != net->get_actors().end(); ++it) {
		if ((*it)->class_path == actor) {
			for (auto p = (*it)->outports.begin(); p != (*it)->outports.end(); ++p) {
				if (p->name == port) {
					p->num_tokens = 1;
					return;
				}
			}
			assert(0);
		}
	}
	assert(0);
}


static void generate_parallel_network(
	unsigned num_actors,
	unsigned actor_count,
	std::vector<Actor_Instance_Port> inputs,
	unsigned outputs,
	Network* net,
	std::vector<Actor_Instance_Port>& open_ports)
{
	Config* c = c->getInstance();
	std::vector<Actor_Instance_Port> left;
	std::vector<Actor_Instance_Port> right;

	num_actors -= 2; // for join and fork

	unsigned remaining_left = num_actors / 2;
	unsigned remaining_right = num_actors - remaining_left;

	Actor_instance* fork;
	Actor_instance* join;

	assert(inputs.size() < 10);
	assert(outputs < 10);
	assert(inputs.size() == outputs + 1);

	if (fork_gen[outputs] == nullptr) {
		Actor* a = new Actor();
		a->dynamic = true;
		a->fork = true;
		a->class_path = "cal.fork" + std::to_string(outputs);
		fork_gen[outputs] = a;
	}

	fork = new Actor_instance();
	fork->actor = fork_gen[outputs];
	fork->name = "fork_inst_" + std::to_string(actor_count);
	++actor_count;
	net->add_actor_instance(fork);

	int count = -1;
	for (auto it = inputs.begin(); it != inputs.end(); ++it) {
		Connection con;
		con.sink = fork;
		if (count == -1) {
			con.sink_port = "cond";
		}
		else {
			con.sink_port = "X" + std::to_string(count);
		}
		con.source =it->inst ;
		con.source_port = it->port.name;

		net->add_connection(con);

		if (count != -1) {
			Actor_Instance_Port aip_left;
			aip_left.inst = fork;
			aip_left.port.feedback = false;
			aip_left.port.name = "Y" + std::to_string(count);
			aip_left.port.num_tokens = 1;

			left.push_back(aip_left);

			Actor_Instance_Port aip_right;
			aip_right.inst = fork;
			aip_right.port.feedback = false;
			aip_right.port.name = "Z" + std::to_string(count);
			aip_right.port.num_tokens = 1;

			right.push_back(aip_right);
		}

		++count;
	}

	assert(count == outputs);
	std::cout << "Generate if" << std::endl;
	generate_path(net, remaining_left, left, outputs, actor_count, c->get_max_ports());
	actor_count += remaining_left;
	std::cout << "Generate else" << std::endl;
	generate_path(net, remaining_right, right, outputs, actor_count, c->get_max_ports());
	actor_count += remaining_right;


	if (join_gen[outputs] == nullptr) {
		Actor* a = new Actor();
		a->dynamic = true;
		a->join = true;
		a->class_path = "cal.join" + std::to_string(outputs);
		join_gen[outputs] = a;
	}
	join = new Actor_instance();
	join->actor = join_gen[outputs];
	join->name = "join_inst_" + std::to_string(actor_count);
	net->add_actor_instance(join);

	count = 0;
	for (auto it = left.begin(); it != left.end(); ++it) {
		Actor_Instance_Port aip = *it;
		Connection con;

		assert(aip.port.num_tokens == 1);

		con.source = aip.inst;
		con.source_port = aip.port.name;
		con.sink = join;
		con.sink_port = "X" + std::to_string(count);

		net->add_connection(con);

		++count;
	}
	assert(count == outputs);

	count = 0;
	for (auto it = right.begin(); it != right.end(); ++it) {
		Actor_Instance_Port aip = *it;
		Connection con;

		assert(aip.port.num_tokens == 1);

		con.source = aip.inst;
		con.source_port = aip.port.name;
		con.sink = join;
		con.sink_port = "W" + std::to_string(count);

		net->add_connection(con);

		++count;
	}
	assert(count == outputs);

	Connection con;
	con.sink = join;
	con.source = fork;
	con.source_port = "cond_out";
	con.sink_port = "cond";

	for (unsigned i = 0; i < outputs; ++i) {
		Actor_Instance_Port aip;
		aip.inst = join;
		aip.port.feedback = false;
		aip.port.name = "Y" + std::to_string(i);
		aip.port.num_tokens = 1;
		open_ports.push_back(aip);
	}
}

static bool check_satisfiability(
	unsigned remaining_actors,
	unsigned inputs,
	unsigned max_ports,
	unsigned outputs)
{
	for (unsigned i = 1; i < max_ports; ++i) {
		for (unsigned j = 1; j < max_ports; ++j) {
			if (evaluate_port_criteria(remaining_actors, max_ports, i, j, inputs, outputs)) {
				return true;
			}
		}
	}

	return false;
}

static unsigned check_parallel_network(
	unsigned remaining_actors,
	std::vector<Actor_Instance_Port>& open_ports,
	Network *net,
	unsigned actor_count,
	unsigned outputs,
	unsigned open_feedbacks)
{
	Config* c = c->getInstance();
	unsigned actors = 0;

	if (c->get_dynamic() && rand_bool_dist(20)) {
		unsigned in, out;
		bool d = get_parallel_net_conf(remaining_actors, c->get_max_ports(), (unsigned)open_ports.size() - open_feedbacks, &in, &out, &actors, outputs);

		if (d) {
			remaining_actors -= actors;
			std::vector<Actor_Instance_Port> inp;
			for (unsigned i = 0; i < in; ++i) {
				unsigned range = (unsigned)open_ports.size() / 3;
				auto src = open_ports.begin() + rand_in_range(0, range);
				inp.push_back(*src);
				open_ports.erase(src);
				if (src->port.num_tokens != 1) {
					set_default_tokenrate(net, src->inst->actor->class_path, src->port.name);
				}
			}
			generate_parallel_network(actors, actor_count, inp, out, net, open_ports);
		}
	}
	return actors;
}

static void define_new_actor(
	unsigned actor_count,
	Network* net,
	unsigned remaining_actors,
	std::vector<Actor_Instance_Port>& open_ports,
	unsigned max_ports,
	bool allow_feedback,
	std::vector<Actor_Instance_Port>& feedbacks,
	unsigned outputs,
	unsigned max_tokenrate)
{
	Config* c = c->getInstance();
	Actor* a = new Actor();
	net->add_actor(a);

	Actor_instance* inst = new Actor_instance();
	inst->actor = a;
	inst->name = "actor_" + std::to_string(actor_count) + "_inst";

	unsigned in, out;
	get_valid_port_nums(remaining_actors, max_ports, (unsigned)(open_ports.size() - feedbacks.size()), &in, &out, outputs);

	std::cout << "Defining actor with in: " << in << " and out: " << out << std::endl;

	bool feedback_set = false;
	for (unsigned i = 0; i < in; ++i) {
		Actor_Instance_Port aip;
		aip.inst = inst;
		Port p;
		p.name = "X" + std::to_string(i);

		if (allow_feedback && !feedback_set && (feedbacks.size() < 4) && (remaining_actors > feedbacks.size()) && rand_bool_dist(c->get_num_nodes() - remaining_actors) && (in > 1)) {

			std::cout << " + Adding feedback loop inport: " << feedbacks.size() << std::endl;

			p.num_tokens = 1;
			p.feedback = true;

			a->inports.push_back(p);

			aip.port = p;

			feedbacks.push_back(aip);

			feedback_set = true;
		}
		else {
			unsigned range = (unsigned)open_ports.size() / 3;
			unsigned offset = rand_in_range(0, range);

			auto src = open_ports.begin() + offset;

			assert(src->port.num_tokens >= 1 && src->port.num_tokens <= max_tokenrate);

			p.num_tokens = src->port.num_tokens;
			p.feedback = false;

			a->inports.push_back(p);

			aip.port = p;
			Connection con;
			con.sink = inst;
			con.sink_port = p.name;
			con.source = src->inst;
			con.source_port = src->port.name;

			net->add_connection(con);
			open_ports.erase(src);
		}
	}

	bool feedback_send = false;
	for (unsigned i = 0; i < out; ++i) {
		Actor_Instance_Port aip;
		aip.inst = inst;
		Port p;
		p.name = "Y" + std::to_string(i);

		if (!feedback_send && !feedbacks.empty() && rand_bool_dist_limit(remaining_actors - static_cast<unsigned>(feedbacks.size()), 25)) {
			std::cout << " - Connecting feedback loop: " << feedbacks.size() << std::endl;
			auto x = feedbacks.begin() + rand_in_range(0, static_cast<unsigned>(feedbacks.size()) - 1);
			p.num_tokens = 1;
			p.feedback = true;
			aip.port = p;
			Connection con;
			con.sink = x->inst;
			con.sink_port = x->port.name;
			con.source = inst;
			con.source_port = p.name;
			net->add_connection(con);
			feedbacks.erase(x);

			feedback_send = true;
		}
		else {
			p.num_tokens = rand_in_range(1, c->get_max_tokenrate());
			p.feedback = false;
			aip.port = p;
			open_ports.push_back(aip);
		}

		a->outports.push_back(p);
	}

	a->num_instructions = rand_in_range(c->get_min_inst(), c->get_max_inst());
	a->num_actions = rand_in_range(1, 4);
	a->complex_guards = true;

	if (c->get_fsm()) {
		a->FSM = rand_bool();
		if (a->FSM) {
			a->num_states = rand_in_range(2, 4);
			if (a->num_states > a->num_actions) {
				a->num_actions = a->num_states;
			}
		}
	}
	else {
		a->FSM = false;
	}

	if (c->get_priorities()) {
		a->priorities = rand_bool();
	}
	else {
		a->priorities = false;
	}

	if (c->get_statevars()) {
		a->state_vars = rand_bool();
	}
	else {
		a->state_vars = false;
	}

	if (c->get_dynamic()) {
		a->dynamic = rand_bool();
	}
	else {
		a->dynamic = false;
	}

	a->complexity = get_complexity(c->get_complexity());

	a->fork = false;
	a->join = false;
	a->class_path = "cal.actor_" + std::to_string(actor_count);

	net->add_actor_instance(inst);
}

static void generate_path(
	Network* net,
	unsigned remaining_actors,
	std::vector<Actor_Instance_Port>& open_ports,
	unsigned outputs,
	unsigned actor_count,
	unsigned max_ports)
{
	std::cout << "Path generation." << std::endl;

	while (remaining_actors > 0) {

		unsigned pn = check_parallel_network(remaining_actors, open_ports, net, actor_count, outputs, 0);
		if (pn != 0) {
			remaining_actors -= pn;
			actor_count += pn;
			continue;
		}

		std::vector<Actor_Instance_Port> dummy;
		define_new_actor(actor_count, net, remaining_actors, open_ports, max_ports, false, dummy, outputs, 1);

		++actor_count;
		--remaining_actors;
	}
	std::cout << "Path generation done." << std::endl;
}

int generate_network(void)
{
	Config* c = c->getInstance();
	Network* net = new Network();
	unsigned remaining_actors = c->get_num_nodes() - c->get_num_inputs() - c->get_num_outputs();
	unsigned actor_count = 0;

	if (!check_satisfiability(remaining_actors, c->get_num_inputs(), c->get_max_ports(), c->get_num_outputs())) {
		std::cout << "Satisfiability not given." << std::endl;
		return 1;
	}

	std::cout << "Satisfiability check okay." << std::endl;

	for (unsigned i = 0; i < 10; ++i) {
		fork_gen[i] = nullptr;
		join_gen[i] = nullptr;
	}

	/* Generate input and output actors, and add them to the network. */
	Actor* input_actor = new Actor();
	input_actor->class_path = "cal.src";
	Actor* output_actor = new Actor();
	output_actor->class_path = "cal.sink";
	generate_input_actor("src");
	generate_output_actor("sink");

	net->add_actor(input_actor);
	net->add_actor(output_actor);

	std::vector<Actor_Instance_Port> open_ports;
	std::vector<Actor_Instance_Port> feedbacks;

	for (unsigned i = 0; i < c->get_num_inputs(); ++i) {
		Actor_instance* inst = new Actor_instance();
		inst->name = "in_" + std::to_string(i) + "_inst";
		inst->actor = input_actor;
		net->add_actor_instance(inst);
		Port p;
		p.num_tokens = 1;
		p.feedback = false;
		p.name = "Y0";
		Actor_Instance_Port aip;
		aip.port = p;
		aip.inst = inst;

		open_ports.push_back(aip);
	}

	while (remaining_actors > 0) {
		unsigned pn = check_parallel_network(remaining_actors, open_ports, net, actor_count, c->get_num_outputs(), static_cast<unsigned>(feedbacks.size()));
		if (pn != 0) {
			remaining_actors -= pn;
			actor_count += pn;
			continue;
		}

		define_new_actor(actor_count, net, remaining_actors, open_ports, c->get_max_ports(), c->get_feedback_loops(), feedbacks, c->get_num_outputs(), c->get_max_tokenrate());

		++actor_count;
		--remaining_actors;
	}

	if (!feedbacks.empty()) {
		std::cout << "Not all feedback loops connected. Generation failed." << std::endl;
		return 1;
	}

	if (open_ports.size() > c->get_num_outputs()) {
		std::cout << "Generation failed." << std::endl;
		return 1;
	}

	unsigned tokenrate_out_sum = 0;

	for (unsigned i = 0; i < c->get_num_outputs(); ++i) {
		Actor_instance* inst = new Actor_instance();
		inst->name = "out_" + std::to_string(i) + "_inst";
		inst->actor = output_actor;
		net->add_actor_instance(inst);

		/* Draw also connection */
		Actor_Instance_Port p = open_ports.back();

		Connection c;
		c.source = p.inst;
		c.source_port = p.port.name;
		c.sink = inst;
		c.sink_port = "X0";

		tokenrate_out_sum += p.port.num_tokens;

		net->add_connection(c);

		open_ports.pop_back();
	}

	/* The following could be extended by tokenrate_out_sum, but for now the numbers are hardcoded in the actors. */
	unsigned prod_tokens = c->get_num_outputs();
	generate_native_code(prod_tokens);

	std::cout << "Writing network." << std::endl;

	write_network(net);

	std::cout << "Network written." << std::endl;

	std::cout << "Writing actors." << std::endl;

	for (auto it = net->get_actors().begin(); it != net->get_actors().end(); ++it) {
		if ((*it != output_actor) && (*it != input_actor)) {
			generate_actor(*it);
		}
	}

	for (unsigned i = 0; i < 10; ++i) {
		if (fork_gen[i] != nullptr) {
			generate_fork_code(i, fork_gen[i]->class_path);
		}
		if (join_gen[i] != nullptr) {
			generate_join_code(i, join_gen[i]->class_path);
		}
	}

	std::cout << "Generation done." << std::endl;

	return 0;
}