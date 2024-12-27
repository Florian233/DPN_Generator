#pragma once

#include <vector>
#include <string>
#include "Network.hpp"
#include "config.hpp"

void generate_input_actor(std::string class_path);

void generate_output_actor(std::string class_path);

void generate_native_code(unsigned produced_tokens);

void generate_fork_code(unsigned produced_tokens, std::string class_path);

void generate_join_code(unsigned consumed_tokens, std::string class_path);

void generate_actor(
	Actor *actor);