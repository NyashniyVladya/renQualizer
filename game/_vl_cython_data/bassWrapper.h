/*
@author: Vladya

BASS docs: http://www.un4seen.com/doc
*/


#include <stdlib.h>
#include <math.h>
#include "bass.h"
#include "interpolator.c"

#define BUFFER_LEN_FLAG BASS_DATA_FFT512
const int BUFFER_NUM_FFT_SAMPLES = 512;
const int BUFFER_LEN_FFT_ARRAY = (BUFFER_NUM_FFT_SAMPLES / 2);
const int BUFFER_LEN_IN_SAMPLES = (BUFFER_LEN_FFT_ARRAY * 8);

const DWORD FREQUENCY = 44100;
const double A4_FREQ = 440.;  // International standard ISO16.
const double MIN_DB_VALUE = (-96.);

const double FFT_ARRAY_STEP_IN_HZ = (
    (((double) FREQUENCY) / BUFFER_NUM_FFT_SAMPLES)
);

float *dataBuffer;


int getErrorCode(void) {
    return BASS_ErrorGetCode();
};


double offsetFromA4_to_Hz(const double offset) {
    /*
    Calculates the frequency of the note,
    expressed by the offset in semitones,
    from the note 'A' of the first octave.
    */
    return (A4_FREQ * pow(2., (offset / 12.)));
};

double getInterpolatedPoint(const double xInHz) {
    /*
    :doc: in 'interpolator.c'.
    -1. if error.
    */
    if (!dataBuffer) {
        return (-1.);
    };
    double result;
    if (BUFFER_LEN_FFT_ARRAY <= 20) {
        result = _lagrangePolynomialsInterpolation(
            xInHz,
            dataBuffer,
            BUFFER_LEN_FFT_ARRAY,
            FFT_ARRAY_STEP_IN_HZ
        );
    } else {
        result = _linearInterpolation(
            xInHz,
            dataBuffer,
            BUFFER_LEN_FFT_ARRAY,
            FFT_ARRAY_STEP_IN_HZ
        );
    };
    if (result <= .0) {
        return .0;
    };
    if (result >= 1.) {
        return 1.;
    };
    return result;
};


double getInterpolatedPointInDB(
    const double xInOffsetFromA4,
    const double volume
) {
    /*
    Converts value from the function 'getInterpolatedPoint' to decibel,
    сut values less than 'MIN_DB_VALUE',
    and return value between MIN_DB_VALUE and .0,
    expressed by a value between 0 and 1.
    */
    double xInHz = offsetFromA4_to_Hz(xInOffsetFromA4);
    double point = getInterpolatedPoint(xInHz);
    if (point <= .0) {
        return point;
    };
    point *= volume;
    if (point <= .0) {
        return .0;
    };
    double resultInDb = (log10(point) * 20.);
    if (resultInDb <= MIN_DB_VALUE) {
        return .0;
    };
    return ((resultInDb - MIN_DB_VALUE) / fabs(MIN_DB_VALUE));
};
