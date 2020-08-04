#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include "mro.h"

void mdIdx(unsigned long idx, int m, int g, unsigned long *x) {
	unsigned long ff = (1 << g) - 1;
	while (m--) {
		x[m] = idx & ff;
		idx >>= g;
	}
}

unsigned long d2c(unsigned long x, unsigned long n, int g) {
	return x * n / ((1 << g) - 1);
}

void ilp_bf(unsigned long *x_opt) {
	long i, bigsize;
	const int G = 8;
	float energy, min_energy = -1.0;
	unsigned int j;
	unsigned long *x = (unsigned long *) malloc(sizeof(long) * M);
	bigsize = pow(1 << G, M - 1);
	for (i=0; i<bigsize; i++) {
		mdIdx(i, M-1, G, x);
		x[M-1] = (1 << G) - 1;
		energy = 0.0;
		for (j=0; j<M; j++) {
			if (j < M-1) x[M-1] -= x[j];
			if (x[j] == 0) continue;
			x[j] = d2c(x[j], N, G);
			if (Tsw[j] + x[j] / TH_safe(j) > D) {
				energy = DBL_MAX;
				break;
			}
			energy += Esw[j] + (Prb[j] / TH_safe(j) + Eta[j] * ETX[j]) * x[j];
		}
		if ((min_energy < 0 && energy < DBL_MAX) || min_energy > energy) {
			min_energy = energy;
			memcpy(x_opt, x, sizeof(long) * M);
		}
	}
	free(x);
}