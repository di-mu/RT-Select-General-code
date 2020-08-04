#include "mro.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	float value;
	int index;
} sortElem;

int compare(const void *p1, const void *p2) {
	const sortElem *elem[2] = {(sortElem *) p1, (sortElem *) p2};
	if (elem[0]->value > elem[1]->value) return 1;
	if (elem[0]->value < elem[1]->value) return -1;
	return 0;
}

void heur(unsigned long *x_opt) {
	int i, mosteff, lesseff, effB;
	float *A = Esw;
	float B[M];
	float y;
	unsigned long assigned = 0;
	sortElem RList1[M], RList2[M];
	memset(x_opt, 0, sizeof(long)*M);
	for (i=0; i<M; i++) {
		B[i] = Prb[i] / TH_safe(i) + Eta[i] * ETX[i];
		RList1[i].value = A[i] / N + B[i];
		RList2[i].value = B[i];
		RList1[i].index = RList2[i].index = i;
	}
	qsort(RList1, M, sizeof(sortElem), compare);
	mosteff = RList1[0].index;
	//effB = B[0] < B[1] ? 0 : 1;
	if (Tsw[mosteff] + N / TH_safe(mosteff) <= D) {
		x_opt[mosteff] = N;
		return;
	}
	qsort(RList2, M, sizeof(sortElem), compare);
	for (i=1; i<M; i++) {
		lesseff = RList1[i].index;
		if (Tsw[lesseff] + N / TH_safe(lesseff) > D) continue;
		effB = RList2[0].index;
		y = (lesseff == effB) ? -1.0f : A[effB] / (B[lesseff] - B[effB]);
		if (y < 0.0f || y > N || Tsw[effB] + y / TH_safe(effB) > D) {
			x_opt[lesseff] = N;
		} else {
			x_opt[effB] = (D - Tsw[effB]) * TH_safe(effB);
			x_opt[lesseff] = N - x_opt[effB];
		}
		return;
	}
	for (i=0; i<M; i++) {
		effB = RList2[i].index;
		x_opt[effB] = (D - Tsw[effB]) * TH_safe(effB);
		if (N - assigned <= x_opt[effB]) {
			x_opt[effB] = N - assigned;
			return;
		}
		assigned += x_opt[effB];
	}
}