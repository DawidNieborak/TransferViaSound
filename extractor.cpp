#include "extractor.h"
#include <fftw3.h>
#include <cmath>
#include <iostream>
#include <numbers>

#define REAL 0
#define IMAG 1
#define PI 3.14159265359

extractor::extractor()
{

}


// BLAD JEST NASTEPUJACY fftw POZWALA TYLKO NA FFT ZA POMOCA FFTW_COMPLEX A JA POTRZEBUJE ZROBIC FFT Z SHORT[]

double extractor::extract(int numberOfArray, short data[], int sampleRate) {
//    fftw_complex x[numberOfArray];

//    fftw_complex outputArray[numberOfArray];

//    for(int i = 0; i < numberOfArray; i++) {
//        x[i][REAL] = i + 1;
//        x[i][IMAG] = 0;
//    }


    double a[((sizeof(data) / sizeof(*data)) + 24 * (sizeof(data) / sizeof(*data))) * 2];
    std::memcpy(this->applyWindow(data), a, (sizeof(data) / sizeof(*data))); //applyWindow(data)

//    fftw_plan plan = fftw_plan_dft_1d(numberOfArray, x, outputArray, FFTW_FORWARD, FFTW_ESTIMATE);
//    fftw_execute(plan);

    double maxMag = -std::numeric_limits<double>::infinity();
    int maxInd = -1;

    for(int i = 0; i < (sizeof(a) / sizeof(*a)) / 2; ++i) {
        double re  = a[2*i];
        double im  = a[2*i+1];
        double mag = sqrt(re * re + im * im);

        if(mag > maxMag) {
            maxMag = mag;
            maxInd = i;
        }
    }

    // cleaning
//    fftw_destroy_plan(plan);
//    fftw_cleanup();


    return (double)sampleRate * maxInd / (sizeof(a) / sizeof(*a) / 2);


    // display
//    for(int i = 0; i < numberOfArray; i++) {
//        if(outputArray[i][IMAG] < 0)
//            std::cout << outputArray[i][REAL] << " - " << abs(outputArray[i][IMAG]) << std::endl;
//        else
//            std::cout << outputArray[i][REAL] << " + " << abs(outputArray[i][IMAG]) << std::endl;
//    }
}


void extractor::buildHammWindow(int size) {
    this->window[size];

    for(int i = 0; i < size; ++i) {
        this->window[i] = .54 - .46 * cos(2 * PI * i / (size - 1.0));
    }
}

double* extractor::applyWindow(short input[]) {
    double res[sizeof(input) / sizeof(*input)];

    this->buildHammWindow(sizeof(input) / sizeof(*input));
    for(int i = 0; i < (sizeof(input) / sizeof(*input)); ++i) {
        res[i] = (double)input[i] * this->window[i];
    }
    return res;
}
