#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <iostream>

class extractor
{
public:
    extractor();

    double extract(int numberOfArray, short data[], int sampleRate);

private:
    void buildHammWindow(int size);
    double* applyWindow(short input[]);

    //var
    double window[];
};

#endif // EXTRACTOR_H
