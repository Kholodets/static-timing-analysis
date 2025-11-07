/*
 * EE 5301 Static Timing Analysis Project
 * Phase 1
 * isc and lib parser
 * Lexi MacLean
 */

//includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "netlist.h"
#include "lut.h"

//constant definitions
#define READ_CKT 0
#define READ_SLEWS 1
#define READ_DELAYS 2
#define TRAVERSE 3

int main(int argc, char **argv)
{
	--argc, ++argv; //first arg is binary name
	
	int mode;

	//parse arguments
	if (argc && !strcmp(*argv, "read_ckt")) {
		mode = READ_CKT;
		--argc, ++argv;
	} else if (argc && !strcmp(*argv, "read_nldm")) {
		--argc, ++argv;
		if (argc && !strcmp(*argv, "delays")) {
			mode = READ_DELAYS;
		} else if (argc && !strcmp(*argv, "slews")) {
			mode = READ_SLEWS;
		} else {
			fprintf(stderr, "Improper usage of read_nldm\n");
			return 1;
		}
		--argc, ++argv;
	} else {
		mode = TRAVERSE;
	}

	if (!argc) {
		fprintf(stderr, "no input file specified\n");
		return 1;
	}

	FILE *input = fopen(*argv, "r");
	--argc, ++argv;
	if (input == NULL) {
		perror("Failed to open input file");
		return 1;
	}

	if (mode == READ_CKT) {
		FILE *details = fopen("ckt_details.txt", "w");
		if (details == NULL) {
			perror("fopen ckt_details.txt");
			exit(1);
		}
		netlist_t netl;
		clock_t start = clock();
		build_netlist(&netl, input);
		clock_t fin = clock();
		//fprintf(stderr, "parsing took %f ms\n", ((float) (fin - start)) / (CLOCKS_PER_SEC / 1000.0));
		print_netlist(&netl, details);
		free_netlist(&netl);
		fclose(details);
	}

	if (mode == READ_SLEWS || mode == READ_DELAYS) {
		FILE *details = fopen(mode == READ_SLEWS ? "slew_LUT.txt" : "delay_LUT.txt", "w");
		if (details == NULL) {
			perror("fopen _lut.txt");
			exit(1);
		}
		lut_t lut;
		build_lut(&lut, input);
		fprint_lut(details, &lut, mode == READ_SLEWS ? 1 : 0);
		free_lut(&lut);
		fclose(details);
	}

	if (mode == TRAVERSE) {
		FILE *ckt = fopen(*argv, "r");
		--argc, ++argv;
		if (ckt == NULL) {
			perror("failed to open ckt file");
		}
		FILE *traversal = fopen("ckt_traversal.txt", "w");
		if (traversal == NULL) {
			perror("fopen ckt_traversal.txt");
			exit(1);
		}
		netlist_t netl;
		lut_t lut;
		build_lut(&lut, input);
		build_netlist(&netl, ckt);
		
		double max_delay = all_delays(&netl, &lut);
		fprintf(traversal, "Circuit delay: %.2lf ps\n", max_delay * 1000);
		find_slacks(&netl, max_delay);
		fprintf(traversal, "\nGate slacks:\n");
		print_slacks(&netl, traversal);
		fprintf(traversal, "\nCritical path:\n");
		print_critpath(&netl, traversal);

		free_lut(&lut);
		free_netlist(&netl);
		fclose(traversal);
		fclose(ckt);

	}


	if (fclose(input)) {
		perror("fclose");
		return 1;
	}

	return 0;
}
