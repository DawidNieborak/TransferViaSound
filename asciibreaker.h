#ifndef ASCIIBREAKER_H
#define ASCIIBREAKER_H

#include <iostream>
#include <vector>


class ASCIIBreaker
{
private:
    std::vector<int> numbers{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> lowerBound{896, 1408, 1920, 2432, 2944, 3456, 3968, 4480, 4992, 5504};
    std::vector<int> upperBound{1152, 1664, 2176, 2688, 3200, 3712, 4224, 4736, 5248, 5760};
    std::vector<int> frequency{1024, 1536, 2048, 2560, 3072, 3584, 4096, 4608, 5120, 5632};
    int ASCIIValue;


    // getters
    std::vector<int> getNumbers() {
        return this->numbers;
    }

    std::vector<int> getLowerBound() {
        return this->lowerBound;
    }

    std::vector<int> getUpperBound() {
        return this->upperBound;
    }

    std::vector<int> getFrequencies() {
        return this->frequency;
    }


    // private functions
    int indexOf(int digit);

public:
    ASCIIBreaker(int asciiValue);


    // functions
    std::vector<int> ASCIIToFrequency ();
    int frequencyToNumber(int frequency);
};




#endif // ASCIIBREAKER_H
