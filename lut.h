#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <stdio.h>

typedef struct
{
	std::vector<std::string> names; //necessary?
	
	std::unordered_map<std::string, double *> delays;
	std::unordered_map<std::string, double *> slews;

	double *tau_in;
	double *cload;
} lut_t;

void build_lut(lut_t *lut, FILE *input);
void free_lut(lut_t *lut);
void fprint_lut(FILE *f, lut_t *lut, int snd);
