#pragma once 

#include <filesystem>
#include "config.hpp"
#include <string>
#include <fstream>
#include <iostream>

static inline void write_file(
	bool cal,
	std::string filename,
	std::string content)
{
	Config* c = c->getInstance();

	std::filesystem::path p{ c->get_output_dir() };

	if (cal) {
		p /= "cal";
		p /= filename + ".cal";
	}
	else {
		p /= "xdf";
		p /= filename + ".xdf";
	}

	std::ofstream out{ p };
	if (out.fail()) {
		std::cout << "Cannot open file " << p.string() << std::endl;
		exit(1);
	}

	out << content;
	out.close();
}

static inline void write_native_file(
	std::string filename,
	std::string content)
{
	Config* c = c->getInstance();

	std::filesystem::path p{ c->get_output_dir() };
	p /= filename;

	std::ofstream out{ p };
	if (out.fail()) {
		std::cout << "Cannot open file " << p.string() << std::endl;
		exit(1);
	}

	out << content;
	out.close();
}