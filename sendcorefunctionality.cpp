#include "sendcorefunctionality.h"

sendCoreFunctionality::sendCoreFunctionality()
{

}


void sendCoreFunctionality::generateSineWave() {
    ASCIIBreaker asciiBreaker(this->fileName.toInt());
    auto freq = asciiBreaker.ASCIIToFrequency();

    // TODO debug
    for(int i = 0; i < freq.size(); i ++) {
        auto item = asciiBreaker.frequencyToNumber(freq[i]);

        std::cout << "TUTAJ: " << item << std::endl;
    }
    // =====

    sf::RenderWindow window(sf::VideoMode (200,200), "audio");

    const unsigned short n_frequencies = 8;
    sf::SoundBuffer buffer;
    std::vector<sf::Int16> samples;


    for (int i = 0; i < n_frequencies; ++i) {
        for (int j = 0; j < 44100; j++) {
            samples.push_back(sound::SineWave(j, freq[i], 0.9));
        }
    }

    buffer.loadFromSamples(&samples[0], samples.size(), 1, 44100);

    sf::Sound sound;
    sound.setBuffer(buffer);

    sound.play();
    sound.setLoop(true);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                sound.stop();
                window.close();
            }
        }
    }

    window.display();
}
