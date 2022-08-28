#include "decoder.h"

Decoder::Decoder(double frequency)
{
    this->frequency = frequency;
}

char Decoder::convertFrequencyToArray() {
    int alphaIndex = 0;
    for(int i = 0;i < sizeof(this->lowerBound);i++) {
        if(this->frequency >= this->lowerBound[i] && this->frequency <= this->upperBound[i]){
            alphaIndex = i;
            break;
        } else {
            alphaIndex = -1;
        }
    }
    if(alphaIndex == -1) {
        return '-';
    } else {
        return alphaBet[alphaIndex];
    }
}
