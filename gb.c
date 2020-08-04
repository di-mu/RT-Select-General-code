#include "mro.h"
#include <stdlib.h>
#include <string.h>

int min_index(float *value, int length) {
	int i, min_idx = 0;
	float min = value[0];
	for (i=1; i<length; i++) {
		if (value[i] < min) {
			min = value[i];
			min_idx = i;
		}
	}
	return min_idx;
}

void greenbag(unsigned long *x_opt) {
	int i, mosteff;
	float *A = Esw;
	float B[M], AvgEng[M], TH_adj[M] = {0.0f};
	float TH_sum = 0.0f;
	memset(x_opt, 0, sizeof(long)*M);
	for (i=0; i<M; i++) {
		B[i] = Prb[i] / TH_safe(i) + Eta[i] * ETX[i];
		AvgEng[i] = A[i] / N + B[i];
	}
	mosteff = min_index(AvgEng, M);
	if (Tsw[mosteff] + N / TH_safe(mosteff) <= D) {
		x_opt[mosteff] = N;
		return;
	}
	for (i=0; i<M; i++) {
		if (D < Tsw[i]) continue;
		TH_adj[i] = TH_safe(i) * (D - Tsw[i]) / D;
		TH_sum += TH_adj[i];
	}
	for (i=0; i<M; i++) x_opt[i] = N * (TH_adj[i] / TH_sum);
}