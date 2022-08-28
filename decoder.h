#ifndef DECODE_H
#define DECODE_H


class Decoder
{
private:
    char alphaBet[10] = { '0','1','2','3','4','5','6','7','8','9' };
    double lowerBound[10] = {896,1152,1408,1664,1920,2176,2432,2688,2944,3200};
    double upperBound[10] = {1152,1408,1664,1920,2176,2432,2688,2944,3200,3456};
    double frequency;

public:
    Decoder(double frequency);

    char convertFrequencyToArray();

};

#endif // DECODE_H
