#include "extractor.h"
#include <fftw3.h>
#include <cmath>
#include <iostream>
#include "audioFFT.h"
#include <numbers>
#include <SFML/Audio.hpp>
#include <vector>

#define PI 3.14159265359

extractor::extractor()
{

}


// BLAD JEST NASTEPUJACY fftw POZWALA TYLKO NA FFT ZA POMOCA FFTW_COMPLEX A JA POTRZEBUJE ZROBIC FFT Z SHORT[]

double extractor::extract(int numberOfArray, sf::Int16 data[], int sampleRate) {
    const size_t fftSize = (sizeof(data)/sizeof(*data)) + 24 * (sizeof(data)/sizeof(*data));
    double a[((sizeof(data) / sizeof(*data)) + 24 * (sizeof(data) / sizeof(*data))) * 2];
    std::memcpy(this->applyWindow(data), a, (sizeof(data) / sizeof(*data))); //applyWindow(data)

    std::vector<float> input(fftSize, 0.0f);
    std::vector<float> re(audiofft::AudioFFT::ComplexSize(fftSize));
    std::vector<float> im(audiofft::AudioFFT::ComplexSize(fftSize));
    std::vector<float> output(fftSize);

    audiofft::AudioFFT fft;
//    fft.init(1024);
    fft.init((sizeof(data)/sizeof(*data)));
    fft.fft(input.data(), re.data(), im.data());

    // co dalej?

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
