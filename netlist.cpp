#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <string.h>

#include "netlist.h"

#define BUFS 1024

void build_netlist(netlist_t *net, FILE *input)
{
	char buf[BUFS];
	char delimit[] = "=(), \t\r\n\v\f";
	char *saveptr;
	while(fgets(buf, BUFS, input) != NULL) {
		char *tok = strtok_r(buf, delimit, &saveptr);
		if (tok == NULL) { // line is empty
			continue;
		}

		int id;

		if (sscanf(tok, "%d", &id)) { // = assign statement
			tok = strtok_r(NULL, delimit, &saveptr);
			if (tok == NULL) {
				printf("malformed input, skipping line\n");
				continue;
			}


			net_t *nn;
			if (!net->nl.contains(id)) {
				net->nl[id] = new net_t;
			}
			nn = net->nl[id];
			nn->id = id;
			nn->type = tok;

			//printf("adding net with id %d, type %s, and fanin ", id, tok);
			
			//read fanin
			while ((tok = strtok_r(NULL, delimit, &saveptr)) != NULL) {
				int inid;
				sscanf(tok, "%d", &inid);
				nn->fanin.push_back(inid);
				net_t *inn;
				if (!net->nl.contains(inid)) {
					net->nl[id] = new net_t;
					inn = net->nl[id];
					inn->id = inid;
				} else {
					inn = net->nl[id];
				}

				inn->fanout.push_back(id);
				//printf("%d", inid);
			}
			//printf("\n");

		} else { //input/output statement
			
			std::string ntype = tok;
			tok = strtok_r(NULL, delimit, &saveptr);
			sscanf(tok, "%d", &id);

			net_t *nn;
			if (!net->nl.contains(id)) {
				net->nl[id] = new net_t;
			}

			nn = net->nl[id];
			nn->id = id;
			nn->type = ntype;
		}
				
	}
}

void print_fanin(net_t *n)
{
	
