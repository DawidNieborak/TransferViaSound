#ifndef DECODE_H
#define DECODE_H


class Decoder
{
private:
    char alphaBet[10] = { '0','1','2','3', '4', '5', '6', '7', '8', '9'};
//    double lowerBound[10] = {0,71,141,212,283,354,425,496,567,638};
//    double upperBound[10] = {70,140,211,282,353,424,495,566,637,704};

    // to dziala
    double lowerBound[10] = {60,2040,2121,2181,2241,2301,2361,2421,2481,2621};
    double upperBound[10] = {1999,2120,2180,2240,2300,2360,2420,2480,2620, 2720};
//    double lowerBound[10] {896, 1152, 1664, 2176, 2688, 3200, 3712, 4224, 4736, 5248};
//    double upperBound[10] {1152, 1664, 2176, 2688, 3200, 3712, 4224, 4736, 5248, 5760};

    double frequency;

public:
    Decoder(double frequency);

    char convertFrequencyToArray();

};

#endif // DECODE_H
