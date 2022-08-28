#include "ASCIIBreaker.h"

ASCIIBreaker::ASCIIBreaker(int asciiValue) {
    this->ASCIIValue = asciiValue;
}

int ASCIIBreaker::indexOf(int digit){
    for(int i = 0; i < numbers.size(); i++){
        if(numbers[i] == digit) {

            return i;
        }
    }
    return -1;
}

std::vector<int> ASCIIBreaker::ASCIIToFrequency()
{
    std::vector<int> frequencies(std::to_string(ASCIIValue).length() * 2);
    for (int i = 0; i < frequencies.size(); i++)
    {
        if (i % 2 == 0)
        {
            auto digit = std::to_string(ASCIIValue)[i / 2];

            //convert char into int
            int temp = (int)digit - 48;

            int index = indexOf(temp);

            if (index != -1)
            {
                frequencies[i] = frequency[index];
            }
        }
        else
        {
            frequencies[i] = 9000;
        }
    }
    return frequencies;
}

int ASCIIBreaker::frequencyToNumber(int frequency)
{
   for (int i = 0; i < lowerBound.size(); i++)
   {
       if (frequency >= lowerBound[i] && frequency <= upperBound[i])
       {
           return numbers[i];
       }
   }
   return 0;
}

