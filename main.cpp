#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>

#include "config.hpp"
#include "network_gen.hpp"
#include "random.hpp"

Config* Config::instance = 0;

void parse_command_line_input(int argc, char* argv[]) {
	Config* c = c->getInstance();

	c->set_min_inst(1);
	c->set_max_ports(4);
	c->set_max_inst(100);
	c->set_num_inputs(5);
	c->set_num_outputs(5);
	c->set_num_nodes(50);
	c->set_complexity(Actor_Complexity::Cond);
	c->set_max_tokenrate(3);

	bool output_set{ false };
	bool fsm = true;
	bool prios = true;
	bool statevars = true;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			std::cout << "\nUsage: %s [options]\n"
				"\nCommon arguments:\n"
				"	-h                 Print this message.\n"
				"	-w <directory>     Specify the directory to write the output to.\n"
				"   -m <number>        Maximum number of instructions per actor, default 100.\n"
				"   -n <number>        Maximum number of actors, default 50.\n"
				"   -i <number>        Number of Input nodes, default 5\n"
				"   -o <number>        Number of Output nodes, default 5.\n"
				"   -f                 Allows feedback loops.\n"
				"   -d                 Allow dynamic nodes.\n"
				"   -dd                Allow dynamic tokenflow with parallel paths (fork-join generation)."
				"   -c <complexity>    Complexity of the generated actors, simple|if|loop, default if.\n"
				"   -z                 Disable FSM generation.\n"
				"   -u                 Disable priority generation.\n"
				"   -q                 Disable State Variables.\n"
				"   -t <num>           Set max tokenrate.\n"
				"   -l <num>           Set max number of feedback cycles.\n"
				"   -p <num>           Set max number of ports of the actors.\n"
				"   -e                 Allow token consumption rates that are not equal of the production rates (experimental)"
				<< std::endl;

			exit(0);
		}
		else if (strcmp(argv[i], "-w") == 0) {
			c->set_output_dir(std::filesystem::path(argv[++i]));
			output_set = true;
		}
		else if (strcmp(argv[i], "-i") == 0) {
			c->set_num_inputs(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-o") == 0) {
			c->set_num_outputs(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-m") == 0) {
			c->set_max_inst(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-t") == 0) {
			c->set_max_tokenrate(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-l") == 0) {
			c->set_num_feedbackcycles(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-p") == 0) {
			c->set_max_ports(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-d") == 0) {
			c->set_dynamic();
		}
		else if (strcmp(argv[i], "-dd") == 0) {
			c->set_cond_flow_dynamic();
		}
		else if (strcmp(argv[i], "-z") == 0) {
			fsm = false;
		}
		else if (strcmp(argv[i], "-u") == 0) {
			prios = false;
		}
		else if (strcmp(argv[i], "-q") == 0) {
			statevars = false;
		}
		else if (strcmp(argv[i], "-e") == 0) {
			c->set_uneq_tokenrates();
		}
		else if (strcmp(argv[i], "-n") == 0) {
			c->set_num_nodes(static_cast<unsigned int>(atoi(argv[++i])));
		}
		else if (strcmp(argv[i], "-f") == 0) {
			c->set_feedback_loops();
		}
		else if (strcmp(argv[i], "-c") == 0) {
			std::string strat{ argv[++i] };
			if (strat == "simple") {
				c->set_complexity(Actor_Complexity::Simple);
			}
			else if (strat == "if") {
				c->set_complexity(Actor_Complexity::Cond);
			}
			else if (strat == "loop") {
				c->set_complexity(Actor_Complexity::Loop);
			}
			else {
				std::cout << "Cannot detect actor complexity." << std::endl;
				exit(1);
			}
		}
		else {
			std::cout << "Error:Unknown input " << argv[i] << std::endl;
			exit(1);
		}
	}

	if (!output_set) {
		std::cout << "Output directory not set...exiting." << std::endl;
		exit(1);
	}

	c->set_fsm(fsm);
	c->set_priorities(prios);
	c->set_statevars(statevars);
}

int main(int argc, char* argv[]) {

	Config* c = c->getInstance();

	std::cout << "Generator starting up." << std::endl;

	parse_command_line_input(argc, argv);

	std::cout << "Output directory: " << c->get_output_dir() << std::endl;
	std::cout << "Number of actors: " << c->get_num_nodes() << " Inputs: " << c->get_num_inputs() << " outputs: " << c->get_num_outputs() << std::endl;
	std::cout << "Actor complexity: ";
	switch (c->get_complexity()) {
	case Actor_Complexity::Cond:
		std::cout << "if";
		break;
	case Actor_Complexity::Simple:
		std::cout << "simple";
		break;
	case Actor_Complexity::Loop:
		std::cout << "loop";
		break;
	}
	std::cout << " number of instructions: min: " << c->get_min_inst() << " max: " << c->get_max_inst() << std::endl;
	std::cout << "Maximum number of ports per actors: " << c->get_max_ports() << std::endl;
	if (c->get_dynamic()) {
		std::cout << "Dynamic actors allowed." << std::endl;
	}
	if (c->get_cond_flow_dynamic()) {
		std::cout << "Dynamic token flow with parallel paths allowed." << std::endl;
	}
	if (c->get_feedback_loops()) {
		std::cout << "Feedback loops allowed." << std::endl;
	}
	if (c->get_fsm()) {
		std::cout << "FSM enabled." << std::endl;
	}
	if (c->get_priorities()) {
		std::cout << "Priorities enabled." << std::endl;
	}
	if (c->get_statevars()) {
		std::cout << "State variables enabled." << std::endl;
	}

	if ((c->get_num_nodes() < (c->get_num_inputs() + c->get_num_outputs())) || 
		((c->get_num_nodes() == (c->get_num_inputs() + c->get_num_outputs()))) && (c->get_num_inputs() != c->get_num_outputs()))
	{
		/* Not possible, cannot be generated */
		std::cout << "Number of actors must be larger than number of inputs and outputs." << std::endl;
		return 1;
	}

	/* generate output dirs */
	std::filesystem::path p = c->get_output_dir();
	std::filesystem::create_directory(p);
	std::filesystem::path cp = p;
	cp /= "cal";
	std::filesystem::create_directory(cp);
	std::filesystem::create_directory(p /= "xdf");

	rand_set_time_seed();

	return generate_network();
}