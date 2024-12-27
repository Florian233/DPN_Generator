#pragma once

#include <vector>
#include <string>
#include "config.hpp"

struct Port {
	std::string name;
	unsigned num_tokens;
	bool feedback;
};


struct Actor {
	/* Generation parameters */
	unsigned num_instructions;
	Actor_Complexity complexity;
	std::vector<Port> inports;
	std::vector<Port> outports;
	bool FSM;
	unsigned num_states;
	unsigned num_actions;
	bool state_vars;
	bool priorities;
	bool dynamic;
	unsigned min_tokenrate;
	unsigned max_tokenrate;
	bool complex_guards;
	bool join;
	bool fork;

	/* Network parameters */
	std::string class_path;
};

struct Actor_instance {
	std::string name;
	Actor* actor;
};

struct Actor_Instance_Port {
	Port port;
	Actor_instance* inst;
};

struct Connection {
	Actor_instance* source;
	std::string source_port;
	Actor_instance* sink;
	std::string sink_port;
};

class Network {
private:
	std::vector<Actor*> actors;
	std::vector<Actor_instance*> instances;
	std::vector<Connection> connections;

public:

	Network() {};

	void add_actor(Actor* a) {
		actors.push_back(a);
	}

	std::vector<Actor*>& get_actors(void) {
		return actors;
	}

	void add_actor_instance(Actor_instance* a) {
		instances.push_back(a);
	}

	std::vector<Actor_instance*>& get_actor_instances(void) {
		return instances;
	}

	void add_connection(Connection c) {
		connections.push_back(c);
	}

	std::vector<Connection>& get_connections(void) {
		return connections;
	}
};