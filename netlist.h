#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>

#include "lut.h"

//TODO replace cpp STL data structures

typedef struct
{
	int id;
	int outp;
	std::string type;
	std::vector<int> fanin;
	std::vector<int> fanout;

	double cap_in;

	std::vector<double> tau_in;
	std::vector<double> arr_in;
	std::vector<double> delays;
	double tau_out;
	double arr_out;
	int in_count;
	int processed;

	double required;
	double slack;
} net_t;

typedef struct
{
	std::unordered_map<int, net_t *> nl;
	std::unordered_map<std::string, int> counts;
	std::vector<int> inputs;
	std::vector<int> outputs;
} netlist_t;



void build_netlist(netlist_t *net, FILE *input);
void print_netlist(netlist_t *netl, FILE *output);
void free_netlist(netlist_t *netl);
double all_delays(netlist_t *net, lut_t *lut);
void find_slacks(netlist_t *net, double t_delay);
void print_slacks(netlist_t *net, FILE *f);
void print_critpath(netlist_t *netl, FILE *f);

void fprint_net(FILE *f, net_t *n);
