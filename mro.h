#ifndef __MRO_H__
#define __MRO_H__

#include <stdio.h>
#include <stdint.h>

//#define DEBUG 0
//#define SIMULATION 1
//#define RANDOM 0
#define M 5
#if M == 2
#define N_IR 1
#define N_ER 1
#define N_SOLVER 2
#endif
#if M == 5
#define N_IR 2
#define N_ER 2
#define N_SOLVER 3
#endif
#define TH_safe(i) (0.9f * TH[i])

#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) 
#endif

enum prediction { TH_PREDICT, ETX_PREDICT };

extern long N, CAP;
extern float D, Esw[M], Tsw[M], TH[M], Prb[M], Eta[M], ETX[M];

extern void ilp_bf (unsigned long *);
extern void glp (unsigned long *);
extern void heur (unsigned long *);
extern void greenbag (unsigned long *);
extern unsigned long us_stopwatch (uint8_t, uint8_t);
extern void ms_sleep (unsigned long);
extern float EWMA_Predictor (int, float);
extern float HoltWinter_Predictor (int, float);

#endif
