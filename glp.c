#include "mro.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void glp(unsigned long *x_opt) {
	float *A = Esw;
	float B[M];
	char strline[128];
	char *str;
	int i;
	FILE *fp = fopen("input.lp", "w");
	fprintf(fp, "Minimize\n  obj: ");
	for (i=0; i<M; i++) {
		B[i] = Prb[i] / TH_safe(i) + Eta[i] * ETX[i];
		fprintf(fp, "%f y%d + %f x%d", A[i], i+1, B[i], i+1);
		if (i < M-1) fprintf(fp, " + ");
	}
	fprintf(fp, "\nSubject To\n  c1: ");
	for (i=0; i<M; i++) {
		fprintf(fp, "x%d", i+1);
		if (i < M-1) fprintf(fp, " + ");
	}
	fprintf(fp, " = %ld\n", N);
	for (i=0; i<M; i++) {
		fprintf(fp, "  c%d: x%d - %ld y%d <= 0\n", i+2, i+1, N, i+1);
	}
	fprintf(fp, "Bounds\n");
	for (i=0; i<M; i++) {
		fprintf(fp, "  0 <= x%d <= %ld\n", i+1, (long)((D - Tsw[i]) * TH_safe(i)));
	}
	fprintf(fp, "Integer\n");
	for (i=0; i<M; i++) fprintf(fp, "  x%d\n", i+1);
	fprintf(fp, "Binary\n");
	for (i=0; i<M; i++) fprintf(fp, "  y%d\n", i+1);
	fprintf(fp, "End");
	fclose(fp);
	while (access ("input.lp", F_OK) == -1) usleep (1000);
	system ("glpsol --cpxlp input.lp -o output.lp > /dev/null");
	while (access ("output.lp", F_OK) == -1) usleep (1000);
	fp = fopen("output.lp", "r");
	do {
		if (!fgets(strline, sizeof(strline), fp)) break;
		str = strstr(strline, "1 y1");
	} while (!str);
	for (i=0; i<M; i++) {
		fgets(strline, sizeof(strline), fp);
		x_opt[i] = atoi(strstr(strline, "*") + 1);
		fgets(strline, sizeof(strline), fp);
	}
	fclose (fp);
	remove ("input.lp");
	remove ("output.lp");
}