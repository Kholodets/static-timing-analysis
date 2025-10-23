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

#include "netlist.h"

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

	netlist_t netl;
	build_netlist(&netl, input);


	if (fclose(input)) {
		perror("fclose");
		return 1;
	}

	return 0;
}
