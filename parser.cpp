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
		fprintf(stderr, "improper usage\n");
		return 1;
	}

	if (!argc) {
		fprintf(stderr, "no input file specified\n");
		return 1;
	}

	FILE *input = fopen(*argv, "r");
	if (input == NULL) {
		perror("Failed to open input file");
		return 1;
	}

	if (mode == READ_CKT) {
		netlist_t netl;
		clock_t start = clock();
		build_netlist(&netl, input);
		clock_t fin = clock();
		fprintf(stderr, "parsing took %f ms\n", ((float) (fin - start)) / (CLOCKS_PER_SEC / 1000.0));
		print_netlist(&netl, stdout);
		free_netlist(&netl);
	}

	if (mode == READ_SLEWS || mode == READ_DELAYS) {
		lut_t lut;
		printf("lut time\n");
		build_lut(&lut, input);
		fprint_lut(stdout, &lut, mode == READ_SLEWS ? 1 : 0);
		free_lut(&lut);
	}


	if (fclose(input)) {
		perror("fclose");
		return 1;
	}

	return 0;
}
