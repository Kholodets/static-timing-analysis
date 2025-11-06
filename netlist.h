#pragma once
#include <unordered_map>
#include <flat_map>
#include <vector>
#include <string>
#include <stdio.h>
//TODO replace cpp STL data structures

typedef struct
{
	int id;
	int outp;
	std::string type;
	std::vector<int> fanin;
	std::vector<int> fanout;
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

void fprint_net(FILE *f, net_t *n);
