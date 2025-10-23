#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <stdio.h>
//TODO replace cpp STL data structures

typedef struct
{
	int id;
	std::string type;
	std::vector<int> fanin;
	std::vector<int> fanout;
} net_t;

typedef struct
{
	std::unordered_map<int, net_t *> nl;
} netlist_t;



void build_netlist(netlist_t *net, FILE *input);
