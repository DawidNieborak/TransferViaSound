#ifndef SENDCOREFUNCTIONALITY_H
#define SENDCOREFUNCTIONALITY_H

#include <QObject>
#include <iostream>
#include <Sound.h>
#include <asciibreaker.h>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

class sendCoreFunctionality
{
public:
    sendCoreFunctionality();

    QString fileName;

public slots:
    void generateSineWave();
};

#endif // SENDCOREFUNCTIONALITY_H
