#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lut.h"

#define BUFS 1024

void build_lut(lut_t *lut, FILE *input)
{
	char buf[BUFS];
	char delimit[] = "(),\\ \";{}\t\r\n\v\f";
	//char delimit_digit[] = "1234567890()\\, \";{}\t\r\n\v\f"
	int got_idx = 0;

	while (1) {
		char *tok;
		char *saveptr;
		std::string name;

		char stopwrd[64];
		do {
			if (fgets(buf, BUFS, input) == NULL) {
				//end of file
				return;
			}
			sscanf(buf, "%s", stopwrd);
		} while(strncmp(stopwrd, "cell", 5));
		


		//start of cell
		tok = strtok_r(buf, delimit, &saveptr); // this should say just "cell"	
		tok = strtok_r(NULL, delimit, &saveptr); //this is what we want (the cell name)
		
		name = tok;

		do {
			fgets(buf, BUFS, input);
			if (buf == NULL) {
				exit(1);
			}
			sscanf(buf, "%s", stopwrd);
		} while(strncmp(stopwrd, "capacitance", 12)); // get capacitance number in there'
		
		double cap;
		sscanf(buf, " capacitance : %lf;", &cap);

		do {
			fgets(buf, BUFS, input);
			if (buf == NULL) {
				exit(1);
			}
			sscanf(buf, "%s", stopwrd);
		} while(strncmp(stopwrd, "cell_delay(Timing_7_7)", 24));


		if (!got_idx) {
			double *tau = (double *) malloc(sizeof(double) * 7);
			double *load = (double *) malloc(sizeof(double) * 7);

			
			//read input slews
			fgets(buf, BUFS, input);
			tok = strtok_r(buf, delimit, &saveptr); //discard, this should just say "index_1"
			for (int i = 0; i < 7; ++i) {
				double x;
				tok = strtok_r(NULL, delimit, &saveptr);
				if (sscanf(tok, "%lf", &x)) {
					tau[i] = x;
				} else {
					printf("reading input slews went wrong\n");
					exit(1);
				}
			}

			
			//read output caps
			fgets(buf, BUFS, input);
			tok = strtok_r(buf, delimit, &saveptr); //discard, this should just say "index_2"
			for (int i = 0; i < 7; ++i) {
				double x;
				tok = strtok_r(NULL, delimit, &saveptr);
				if (sscanf(tok, "%lf", &x)) {
					load[i] = x;
				} else {
					printf("reading output caps went wrong\n");
					exit(1);
				}
			}


			lut->tau_in = tau;
			lut->cload = load;

			got_idx = 1;
		} else {
			//discard these two lines if we dont need them
			fgets(buf, BUFS, input);
			fgets(buf, BUFS, input);
		}


		double *delays = (double *) malloc(sizeof(double) * 7 * 7);
		for (int i = 0; i < 7; ++i) {
			fgets(buf, BUFS, input);
			tok = strtok_r(buf, delimit, &saveptr);
			for (int j = 0; j < 7; ++j) {
				if (j || !i)
					tok = strtok_r(NULL, delimit, &saveptr);
				double x;
				if (sscanf(tok, "%lf", &x)) {
					delays[7*i + j] = x;
				} else {
					printf("reading delays went wrong\n");
					exit(1);
				}
			}
		}


		do {
			fgets(buf, BUFS, input);
			if (buf == NULL) {
				exit(1);
			}
			sscanf(buf, "%s", stopwrd);
		} while(strncmp(stopwrd, "output_slew(Timing_7_7)", 22));


		//we'll have already gotten the inputs/outputs, discard two lines
		fgets(buf, BUFS, input);
		fgets(buf, BUFS, input);


		double *slews = (double *) malloc(sizeof(double) * 7 * 7);
		for (int i = 0; i < 7; ++i) {
			fgets(buf, BUFS, input);
			tok = strtok_r(buf, delimit, &saveptr);
			for (int j = 0; j < 7; ++j) {
				if (j || !i)
					tok = strtok_r(NULL, delimit, &saveptr);
				double x;
				if (sscanf(tok, "%lf", &x)) {
					slews[7*i + j] = x;
				} else {
					exit(1);
				}
			}
		}


		lut->names.push_back(name);
		lut->delays[name] = delays;
		lut->slews[name] = slews;


	}
}

void fprint_lut(FILE *f, lut_t *lut, int snd)
{
	//printf("printing :)\n");
	for (std::string name : lut->names) {
		fprintf(f, "cell, %s\n", name.c_str());
		fprintf(f, "input slews: ");
		for (int i = 0; i < 7; ++i) {
			fprintf(f, "%lf%c", lut->tau_in[i], i!=6 ? ',' : '\n');
		}

		fprintf(f, "load cap: ");
		for (int i = 0; i < 7; ++i) {
			fprintf(f, "%lf%c", lut->cload[i], i!=6 ? ',' : '\n');
		}

		fprintf(f, snd ? "slews:\n" : "delays:\n");
		double *to_print = snd ? lut->slews[name] : lut->delays[name];
		for (int i = 0; i < 7; ++i) {
			for (int j = 0; j < 7; ++j) {
				fprintf(f, "%lf%c", to_print[i * 7 + j], i==6 ? ',' : ';');
			}
			fprintf(f, "\n");
		}
	}
}

void free_lut(lut_t *lut)
{
	free(lut->tau_in);
	free(lut->cload);

	for (std::string name : lut->names) {
		free(lut->slews[name]);
		free(lut->delays[name]);
	}
}
