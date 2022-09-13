#ifndef DECODE_H
#define DECODE_H


class Decoder
{
private:
    char alphaBet[10] = { '0','1','2','3', '4', '5', '6', '7', '8', '9'};
    double lowerBound[10] = {60,2040,2121,2181,2241,2301,2361,2421,2481,2621};
    double upperBound[10] = {1999,2120,2180,2240,2300,2360,2420,2480,2620, 2720};


    double frequency;

public:
    Decoder(double frequency);

    char convertFrequencyToArray();

};

#endif // DECODE_H
