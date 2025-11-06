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
				nn = net->nl[id];
				nn->id = id;
				nn->outp = 0;
			} else {
				nn = net->nl[id];
			}

			nn->type = tok;
			if (net->counts.contains(tok)) {
				net->counts[tok]++;
			} else {
				net->counts[tok] = 1;
			}

			//printf("adding net with id %d, type %s, and fanin ", id, tok);
			
			//read fanin
			while ((tok = strtok_r(NULL, delimit, &saveptr)) != NULL) {
				int inid;
				sscanf(tok, "%d", &inid);
				nn->fanin.push_back(inid);
				net_t *inn;
				if (!net->nl.contains(inid)) {
					net->nl[inid] = new net_t;
					inn = net->nl[inid];
					inn->id = inid;
					inn->outp = 0;
				} else {
					inn = net->nl[inid];
				}

				inn->fanout.push_back(id);
				//printf("%d", inid);
			}
			//printf("\n");

		} else { //input/output statement
			
			if (!strncmp(tok, "OUTPUT", 5)) {
				tok = strtok_r(NULL, delimit, &saveptr);
				sscanf(tok, "%d", &id);
				net_t *outn;
				if (!net->nl.contains(id)) {
					net->nl[id] = new net_t;
					outn = net->nl[id];
					outn->id = id;
				} else {
					outn = net->nl[id];
				}

				outn->outp = 1;
				if (net->counts.contains("OUTP")) {
					net->counts["OUTP"]++;
				} else {
					net->counts["OUTP"] = 1;
				}

			} else if (!strncmp(tok, "INPUT", 5)) {
				tok = strtok_r(NULL, delimit, &saveptr);
				sscanf(tok, "%d", &id);
				net_t *inn;
				if (!net->nl.contains(id)) {
					net->nl[id] = new net_t;
					inn = net->nl[id];
					inn->id = id;
				} else {
					inn = net->nl[id];
				}

				inn->outp = 0;
				inn->type = "INP";
				if (net->counts.contains("INP")) {
					net->counts["INP"]++;
				} else {
					net->counts["INP"] = 1;
				}

			}
		}
				
	}
}

void fprint_net(FILE *f, net_t *n)
{
	fprintf(f, "%s-%d", n->type.c_str(), n->id);
}

void print_netlist(netlist_t *netl, FILE *output)
{
	//print counts
	fprintf(output, "%d primary inputs\n", netl->counts["INP"]);
	fprintf(output, "%d primary outputs\n", netl->counts["OUTP"]);
	for (const auto & [gate, count] : netl->counts) {
		if (gate != "OUTP" && gate != "INP") {
			fprintf(output, "%d %s gates\n", count, gate.c_str());
		}
	}
	//print fanout
	fprintf(output, "Fanout...\n");
	for (const auto & [id, net] : netl->nl) {
		if (net->type == "INP") {
			continue;
		}
		fprint_net(output, net);
		fprintf(output, ": ");
		if (net->outp) {
			fprintf(output, "OUTP");
		} else {
			for (int i = 0; i < net->fanout.size(); ++i) {
				if (i) {
					fprintf(output, ", ");
				}
				int outid = net->fanout[i];
				if (netl->nl.contains(outid)) {
					fprint_net(output, netl->nl[outid]);
				}
			}
		}
		fprintf(output, "\n");
	}

	//print fanin
	fprintf(output, "Fanin...\n");
	for (const auto & [id, net] : netl->nl) {
		if (net->type == "INP") {
			continue;
		}
		fprint_net(output, net);
		fprintf(output, ": ");
		
		for (int i = 0; i < net->fanin.size(); ++i) {
			if (i) {
				fprintf(output, ", ");
			}
			int outid = net->fanin[i];
			if (netl->nl.contains(outid)) {
				fprint_net(output, netl->nl[outid]);
			}
		}
		
		fprintf(output, "\n");
	}
}

void free_netlist(netlist_t *nl)
{
	for (const auto & [id, net] : nl->nl) {
		delete net;
	}
}
