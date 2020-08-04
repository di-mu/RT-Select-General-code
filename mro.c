#include "radio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

long N, CAP;
unsigned long *X;
void (*solver[N_SOLVER])(unsigned long *) = {
#if N_SOLVER == 2
	heur, greenbag
#else
	heur, glp, greenbag
#endif
};
char solver_name[N_SOLVER][16] = {
#if N_SOLVER == 2
	"RTS", "GnB"
#else
	"RTS", "GLP", "GnB"
#endif
};

void updateCAP() {
	int i; CAP = 0;
	for (i=0; i<M; i++) if (D > Tsw[i]) CAP += (D - Tsw[i]) * TH_safe(i);
}

void randgen() {
	time_t t; int i;
	srand((unsigned) time(&t));
	for (i=0; i<M; i++) {
		TH[i] = pow(1 + rand() % 100, 2) / 5.0f;
		ETX[i] = (10 + rand() % 10) / 10.0f;
		Esw[i] = pow(1 + rand() % 1000, 2) / 1.0E+6f;
		Tsw[i] = pow(1 + rand() % 30, 2) / 1.0E+3f;
		Prb[i] = pow(1 + rand() % 100, 2) / 1.0E+4f;
		Eta[i] = (1 + rand() % 100) / 1.0E+6f;
		printf("Radio %d: TH=%f, ETX=%f, Esw=%f, Tsw=%f, Prb=%f, Eta=%f\n",
			i+1, TH[i], ETX[i], Esw[i], Tsw[i], Prb[i], Eta[i]);
	}
}

float est_eng(unsigned long *x) {
	float energy = 0.0f; int i;
	for (i=0; i<M; i++) if (x[i] > 0)
		energy += Esw[i] + (Prb[i] / TH[i] + Eta[i] * ETX[i]) * x[i];
	return energy;
}

void run_solver(void (*solver)(unsigned long *)) {
	int i;
#ifdef ONDEV
	us_stopwatch (0, 1);
#endif
	updateCAP();
	if (CAP < N) {
		solver = greenbag;
		printf("(G)");
	}
	us_stopwatch (1, 1);
	solver(X);
	printf("\t%lu\t", us_stopwatch (1, 0));
	for (i=0; i<M; i++) printf ("%lu\t", X[i]);
	fflush(stdout);
#ifdef ONDEV
	uint16_t scs = m_radio_send(X);
	for (i=0; i<M; i++) printf (X[i] > 0 ? "%u," : "-,", (scs>>i)&1);
	printf("\t%d\t", scs==(1<<M)-1); fflush(stdout);
	int32_t us_wait = (int32_t) (D * 1e6) - us_stopwatch (0, 0);
	if (us_wait > 0) ms_sleep (us_wait / 1000);
#endif
	printf ("%.3f\t%d\n", est_eng(X), CAP >= N); fflush(stdout);
}

void run_batch(int solveridx, int periods) {
#ifdef ONDEV
	int i;
	printf ("--- N = %lu --- D = %.2f ---\n", N, D);
	printf ("* * * Solver: %s * * *\n\tus_sol\t", solver_name[solveridx]);
	for (i=0; i<M; i++) printf ("X%d\t", i+1);
	printf ("scs\tSCS\testEng\tCAP>=N\n");
	while (periods--) run_solver (solver[solveridx]);
	ms_sleep (10000);
#else
	printf ("N = %lu\tD = %.2f\t%s", N, D, solver_name[solveridx]);
	run_solver (solver[solveridx]);
#endif
}

int main() {
	int i, j;
#ifdef RANDOM
	randgen();
#endif
#ifdef ONDEV
	radio_init();
	fflush(stdout);
	ms_sleep(10000);
#endif
	X = (unsigned long *) malloc(sizeof(long) * M);
	memset(X, 0, sizeof(long) * M);
#ifdef BIGCOVER
	printf("\t\t\t\t\t");
	for (i=0; i<M; i++) printf("X%d\t", i+1);
	printf("EST-ENG\n");
	for (D=0.80; D<2.795; D+=0.01) {
		for (N=245; N<6617; N+=32) {
			for (i=0; i<N_SOLVER; i++)
				run_batch(i, 1);
		}
	}
#else
// fixed data size
#if M == 2
	N = 184;  //480x480
	for (D=0.6; D<1.05; D+=0.04)
#else
	N = 850; //1280x720
	for (D=0.8; D<1.25; D+=0.04)
#endif
	for (i=0; i<N_SOLVER; i++) run_batch(i, 50);

//fixed deadline
#if M == 2
	D = 0.8;
	N = 245;
	long imgstep = 55; //640x480 ... 1280x720
#else
	D = 1.44;
	N = 850;
	long imgstep = 230; //1280x720 ... 1920x1080
#endif
	for (j=0; j<12; j++) {
		for (i=0; i<N_SOLVER; i++) run_batch(i, 50);
		N += imgstep;
	}
#endif
	free(X);
#ifdef ONDEV
	radio_end();
#endif
	return 0;
}

	//long imgDR[10] = {245, 383, 470, 627, 735, 977, 1148, 1305, 1656, 6617};
	//640x480, 800x600, 1024x576, 1024x768, 1280x720, 1280x960, 1600x900, 1600x1024, 1920x1080, 3840x2160
	//long imgDR[8] = {836, 1148, 1656, 1836, 2938, 3266, 4133, 6617};
	//1366×768, 1600×900, 1920×1080, 1920×1200, 2560×1440, 2560×1600, 2880×1800, 3840×2160
 //1345; //Youtube uses 720p (1280×720) with 1.378 Mbps H.264 coding
			/*if (N<CAP && N+N/2>CAP && !visited_cap) {
				N = CAP*33/50;
				visited_cap = 1;
			}*/