#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <string.h>
#include <queue>
#include <math.h>
#include <stack>

#include "netlist.h"
#include "lut.h"

#define BUFS 1024

double interpolate(double y, double x, double *idx1, double *idx2, double *table, int id)
{
	//fprintf(stderr, "interpolating for slew=%lf, cap=%lf\n", y, x);
	int xdx = 0;
	int ydx = 0;
	while (y > idx1[ydx] && ydx < 7) {
		ydx++;
	}

	while (x > idx2[xdx] && xdx < 7) {
		xdx++;
	}

	if (xdx == 7) {
		xdx = 6;
		fprintf(stderr, "Load capacitance %lf out of bounds (high) for gate %d\n", x, id); 
	}
	if (ydx == 7) {
		ydx = 6;
		fprintf(stderr, "Input slew %lf out of bounds (high) for gate %d\n", y, id); 
	}

	if (xdx == 0) {
		xdx = 1;
		fprintf(stderr, "Load capacitance %lf out of bounds (low) for gate %d\n", x, id); 
	}
	if (ydx == 0) {
		ydx = 1;
		fprintf(stderr, "Input slew %lf out of bounds (low) for gate %d\n", y, id); 
	}

	double y1 = idx1[ydx - 1], y2 = idx1[ydx], x1 = idx2[xdx - 1], x2 = idx2[xdx];
	double v11 = table[7 * (ydx - 1) + (xdx - 1)],
		v12 = table[7 * (ydx - 1) + (xdx)],
		v21 = table[7 * (ydx) + (xdx - 1)],
		v22 = table[7 * (ydx) + (xdx)];
	
	
	double v = (v11 * (x2 - x) * (y2 - y) + v12 * (x - x1) * (y2 - y) + v21 * (x2 - x) * (y - y1) + v22 * (x - x1) * (y - y1)) / ((x2 - x1) * (y2 - y1));

	//fprintf(stderr, "%lf, %lf, %lf\n %lf %lf %lf\n %lf %lf %lf\n", 0.0, x1, x2, y1, v11, v12, y2, v21, v22);
	//fprintf(stderr, "v= %lf\n", v);
	return v;
}





void calc_out(net_t *net, double cap_out, lut_t *lut)
{
	double *delay_lut = lut->delays[net->type];
	double *slew_lut = lut->slews[net->type];
	double *idx1 = lut->tau_in[net->type];
	double *idx2 = lut->cload[net->type];

	if (delay_lut == NULL || slew_lut == NULL || idx1 == NULL || idx2 == NULL) {
		fprintf(stderr, "lut not found for type %s\n", net->type.c_str());
		exit(1);
	}

	double max_d = 0;
	int d_idx = 0;

	double mod = net->arr_in.size() > 2 ? net->arr_in.size() / 2 : 1;
	
	for (int i = 0; i < net->arr_in.size(); i++) {
		//fprintf(stderr, "interpolating delays, i = %d, sizeof tau_in %d\n", i, net->tau_in.size());
		double part = mod * interpolate(net->tau_in[i], cap_out, idx1, idx2, delay_lut, net->id);
		
		double d = net->arr_in[i] + part;
		net->delays.push_back(part);
		if (d > max_d) {
			max_d = d;
			d_idx = i;
		}
	}

	//fprintf(stderr, "interpolating slews\n");
	double slew_out = mod * interpolate(net->tau_in[d_idx], cap_out, idx1, idx2, slew_lut, net->id);


	net->tau_out = slew_out;
	net->arr_out = max_d;
}


double all_delays(netlist_t *net, lut_t *lut)
{
	std::queue<int> q;
	for (int id : net->inputs) {
		q.push(id);
	}

	while (!q.empty()) {
		int id = q.front();
		q.pop();
		net_t *n = net->nl[id];
		//fprintf(stderr, "dequeued node %d, type %s\n", id, n->type.c_str());

		if (!(n->processed)) {
			for (int i = 0; i < n->fanin.size(); ++i) {
				n->tau_in.push_back(net->nl[n->fanin[i]]->tau_out);
				n->arr_in.push_back(net->nl[n->fanin[i]]->arr_out);
			}

			double out_cap = 0;
			for (int id : n->fanout) {
				out_cap += lut->caps[net->nl[id]->type];
			}
			if (n->outp) {
				out_cap += 4 * lut->caps["INV"];
			}

			calc_out(n, out_cap, lut);
			n->processed = 1;
			n->in_count = 0;
		}

		//fprintf(stderr, "arrival time == %lf\n", n->arr_out * 1000);

		for (int oid : n->fanout) {
			net_t *no = net->nl[oid];
			no->in_count++;
			if (no->in_count == no->fanin.size()) {
				q.push(oid);
				//fprintf(stderr, "enqueued %d\n", oid);
			}
		}		
	}

	double max = 0;
	for (int id : net->outputs) {
		net_t *n = net->nl[id];
		if(n->arr_out > max)
			max = n->arr_out;
	}
	return max;
}

void find_slacks(netlist_t *net, double t_delay)
{
	std::queue<int> q;
	for (int id : net->outputs) {
		net_t *n = net->nl[id];
		n->required = 1.1 * t_delay;
		q.push(id);
	}

	while (!q.empty()) {
		int id = q.front();
		q.pop();
		net_t *n = net->nl[id];
		//fprintf(stderr, "dequeued node %d, type = %s\n", id, n->type.c_str());

		n->slack = n->required - n->arr_out;

		//fprintf(stderr, "slack == %lf\n", n->slack * 1000);

		for (int i = 0; i < n->fanin.size(); ++i) {
			net_t *no = net->nl[n->fanin[i]];
			no->in_count++;
			double tr = n->required - n->delays[i];
			//printf("node %d, %lf - %lf = %lf\n", no->id, n->required, n->delays[i], tr);
			if (tr < no->required) {
				no->required = tr;
			}
			if (no->in_count == no->fanout.size()) {
				q.push(n->fanin[i]);
				//fprintf(stderr, "enqueued %d\n", n->fanin[i]);
			}
		}		
	}
}

void print_slacks(netlist_t *netl, FILE *f)
{
	for (const auto & [id, net] : netl->nl) {
		fprintf(f, "%s-n%d: %.2lf ps\n", net->type.c_str(), id, net->slack * 1000);
	}
}

void print_critpath(netlist_t *netl, FILE *f)
{
	std::stack<net_t *> path;
	std::vector<int> *test = &(netl->outputs);
	net_t *crit;
	do {
		crit = NULL;
		for (int id : (*test)) {
			net_t *n = netl->nl[id];
			if (crit == NULL || n->slack < crit->slack) {
				crit = n;
			}
		}
		path.push(crit);
		test = &(crit->fanin);
		//printf("pushed node %d\n", crit->id);
	} while (crit->fanin.size() > 0);

	while (!path.empty()) {
		net_t *n = path.top();
		path.pop();
		fprintf(f, "%s-n%d", n->type.c_str(), n->id);
		if(!path.empty()) {
			fprintf(f, ", ");
		} else {
			fprintf(f, "\n");
		}
	}
}

void fix_names(char *tok)
{
	char *s = tok;
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}
}

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
			fix_names(tok);
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
				nn->processed = 0;
				nn->in_count = 0;
				nn->required = INFINITY;
			} else {
				nn = net->nl[id];
			}

			nn->type = tok;
			int is_dff = !strncmp(tok, "DFF", 4);
			if (!is_dff) {
				if (net->counts.contains(tok)) {
					net->counts[tok]++;
				} else {
					net->counts[tok] = 1;
				}
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
					inn->processed = 0;
					inn->in_count = 0;
					inn->required = INFINITY;
				} else {
					inn = net->nl[inid];
				}
				if (!is_dff) {
					inn->fanout.push_back(id);
				}
				//printf("%d", inid);
			}

			//if its a DFF, we want to split it
			if (is_dff) {
				//it should only have one fanin, which should be set as an output
				net_t *din = net->nl[nn->fanin[0]];
				din->outp = 1;

				if (net->counts.contains("OUTP"))
					net->counts["OUTP"]++;
				else
					net->counts["OUTP"] = 1;

				net->outputs.push_back(din->id);

				//the net should be reclassified as an input
				nn->type = "INP";
				if (net->counts.contains("INP"))
					net->counts["INP"]++;
				else
					net->counts["INP"] = 1;
				nn->arr_out = 0;
				nn->tau_out = 0.002;
				nn->processed = 1;
				net->inputs.push_back(nn->id);
				nn->fanin.clear();

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
					outn->processed = 0;
					outn->in_count = 0;
					outn->required = INFINITY;
				} else {
					outn = net->nl[id];
				}

				outn->outp = 1;
				if (net->counts.contains("OUTP")) {
					net->counts["OUTP"]++;
				} else {
					net->counts["OUTP"] = 1;
				}
				net->outputs.push_back(id);

			} else if (!strncmp(tok, "INPUT", 5)) {
				tok = strtok_r(NULL, delimit, &saveptr);
				sscanf(tok, "%d", &id);
				net_t *inn;
				if (!net->nl.contains(id)) {
					net->nl[id] = new net_t;
					inn = net->nl[id];
					inn->id = id;
					inn->in_count = 0;
					inn->required = INFINITY;
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
				inn->arr_out = 0;
				inn->tau_out = 0.002;
				inn->processed = 1;
				net->inputs.push_back(id);

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
			fprintf(output, "OUTP; ");
		}
		
		for (int i = 0; i < net->fanout.size(); ++i) {
			if (i) {
				fprintf(output, ", ");
			}
			int outid = net->fanout[i];
			if (netl->nl.contains(outid)) {
				fprint_net(output, netl->nl[outid]);
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
