#include "mro.h"
#define PREDICTS M*2

//Co-efficients
const float EWMA_alpha = 0.15;
const float HoltWinter_alpha = 0.1;//0.15
const float HoltWinter_beta = 0.1;

//temp values
float EWMA_Prediction[PREDICTS];
float HoltWinter_Value[PREDICTS];
float HoltWinter_Trend[PREDICTS];
float HoltWinter_Level[PREDICTS];

unsigned char EWMA_Initialized[PREDICTS] = {0};
unsigned char HoltWinter_Initialized[PREDICTS] = {0};

float EWMA_Predictor(int _i, float _value) {
	if (!EWMA_Initialized[_i]) {
		EWMA_Initialized[_i] = 1;
		EWMA_Prediction[_i] = _value;
		return _value;
	}
	return EWMA_Prediction[_i] = EWMA_alpha * _value + ( 1 - EWMA_alpha ) * EWMA_Prediction[_i];
}

float HoltWinter_Predictor(int _i, float _value) {
	if (!HoltWinter_Initialized[_i]) {
		HoltWinter_Initialized[_i] = 1;
		HoltWinter_Value[_i] = HoltWinter_Level[_i] = _value;
		HoltWinter_Trend[_i] = 0.0f;
		return _value;
	}
	HoltWinter_Trend[_i] = HoltWinter_beta * (_value - HoltWinter_Value[_i])  + (1 - HoltWinter_beta) * HoltWinter_Trend[_i];
	HoltWinter_Level[_i] = HoltWinter_alpha * _value + (1 - HoltWinter_alpha) * (HoltWinter_Level[_i] + HoltWinter_Trend[_i]);
	HoltWinter_Value[_i] = _value;
	return HoltWinter_Level[_i] + HoltWinter_Trend[_i];
}
