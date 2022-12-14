#ifndef AUDIO_H
#define AUDIO_H

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <math.h>
#include <thread>
#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <iomanip>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <math.h>
#include <decoder.h>

class Audio {
public:
    std::string audioFilePath = "/Users/dawid/Music/recorderAudio.wav";

    sf::Clock clock;
    const sf::Int16* samples;
    sf::Uint64 sampleSize;
    unsigned int sampleRate;
    int singleChannelSize;

    bool playedSong = false;
    bool audioChannelSplit = false;
    bool hasAppended = false;
    double maxMag = -std::numeric_limits<double>::infinity();
    int maxInd = -1;

    std::vector< std::complex <double> > leftSamples;
    std::vector<double> sampleFrequencyRanges;
    std::vector< std::vector <double> > frequencyWindowMagnitudes;
    std::vector< double > amplitudeVisualizationVector;
    std::vector<int> results;

    sf::SoundBuffer buffer;
    sf::Sound song;


    public:

        Audio() {


            if (!buffer.loadFromFile(audioFilePath)) {
                std::cout << "Couldn't load buffer" << std::endl;
            }
            else {
                song.setBuffer(buffer);
                sampleRate = buffer.getSampleRate();
                // Original sample (unsplit: the samples holding the left and right channels)
                samples = buffer.getSamples();
                sampleSize = buffer.getSampleCount();
                singleChannelSize = sampleSize / 2;
            }

        }

        bool splitAudioChannel() {

            // Sample arrays for left channel
            leftSamples.resize(singleChannelSize + sampleRate);

            for (int i = 0; i < singleChannelSize; ++i) {
                leftSamples[i] = samples[2 * i];
            }

            // Padding data to audio channel
            for (int i = singleChannelSize; i < leftSamples.size(); ++i) {
                leftSamples[i] = 0;
            }

            // Finds the maximum amplitude in 1/30th of a second in the song
            for (int i = 0; i < leftSamples.size() - 1470; i += 1470) {
                double max = 0;
                for (int t = 0; t < 1470; t++) {
                    double amplitude = leftSamples[i + t].real();
                    if (amplitude >= 0) {
                        if (amplitude > max) {
                            max = (int)amplitude / 100;
                        }
                    }
                }

                if (max > 20) {
                    max = 20;
                }


                amplitudeVisualizationVector.push_back(max);
            }

            std::cout << "Finished splitting audio channels" << std::endl;

            return true;
        }

        void playSong() {

            song.play();

            playedSong = true;
        }

        bool songPlayed() {
            return playedSong;
        }

        double getSongPlayingOffset() {
            return song.getPlayingOffset().asSeconds();
        }

        std::vector<double>& getAmplitudeVisualizationVector() {
            return amplitudeVisualizationVector;
        }


        // Creates frequency ranges for the audio samples
        void getSampleOverFrequency() {

            audioChannelSplit = splitAudioChannel();

            if (audioChannelSplit) {

                clock.restart();

                // Gets a sample from the audio channel to process, samples are the size of the sampleRate
                int sampleWindow = sampleRate / 30;
                int windows_averageOverlapReady_count = 0;

                for (int sampleIndex = 0; sampleIndex < singleChannelSize; sampleIndex += sampleWindow/2) {
                    windows_averageOverlapReady_count++;
                    std::vector< std::complex< double> > leftSampleHannWindowed;

                    for (int i = 0; i < sampleWindow; i++) {
                        std::complex< double > amplitudeHannWindowed = HannFunction(i, sampleWindow) * leftSamples[sampleIndex + i];
                        leftSampleHannWindowed.push_back(amplitudeHannWindowed);

                        if (i == sampleWindow - 1) {
                            if (windows_averageOverlapReady_count == 3) {
                                sampleIndex = (sampleIndex + i) - (sampleWindow / 2);
                            }
                        }
                    }


                    std::vector< std::complex< double> > leftSampleSample_FreqBin = FFT(leftSampleHannWindowed);
                    frequencyWindowMagnitudes.push_back(FFT_Magnitude(leftSampleSample_FreqBin));

                    int maxint = -1;
                    for (int i = 0; i < frequencyWindowMagnitudes.size(); i++)
                    {
                        for (int j = 0; j < frequencyWindowMagnitudes[i].size(); j++)
                        {
                            if(frequencyWindowMagnitudes[i][j] > maxMag) {
                                  maxMag = frequencyWindowMagnitudes[i][j];
                                  maxInd = i;
                              }

                        }

                    }


                    // calculate frequency and decode.
                    auto result = (double)sampleRate * maxInd / (leftSampleSample_FreqBin.size() / 2 );
                    if(result > 60 && result < 2720) {

                        Decoder decode = Decoder(result);
                        auto number = decode.convertFrequencyToArray();
                        results.push_back(number);
                        std::cout << "FREQUENCY/NUMBER: " << result << " " << number << std::endl;
                    }

                }

                std::cout << "SampleOverFrequency Runtime: " << clock.getElapsedTime().asSeconds() << " seconds" << std::endl;
            }

        }


        // Windowing function
        std::complex< double> HannFunction(int n, int N) {
            std::complex< double> hann = pow((sin((M_PI * n) / N)), 2);
            return hann;
        }

        // Cool-Tukey FFT algorithim
        std::vector< std::complex< double> > FFT(std::vector< std::complex <double> > &samples) {
            int N = samples.size();

            if (N == 1) {
                return samples;
            }

            int new_N = N / 2;

            // Splits samples for recursion
            std::vector< std::complex< double> > evenSamples(new_N, 0);
            std::vector< std::complex< double> > oddSamples(new_N, 0);

            for (int i = 0; i < new_N; ++i) {
                evenSamples[i] = samples[i * 2];
                oddSamples[i] = samples[(i * 2) + 1];
            }

            // Recursion breaks down the samples
            std::vector< std::complex< double> > Feven(new_N, 0);
            Feven = FFT(evenSamples);
            std::vector< std::complex< double> > Fodd(new_N, 0);
            Fodd = FFT(oddSamples);

            // Post recursion DFT is calculated
            std::vector< std::complex< double> > freqBins(N, 0);

            for (int k = 0; k < N / 2; ++k) {
                std::complex<double> cmplxExponential = std::polar(1.0, -2*M_PI*k/N)*Fodd[k];
                freqBins[k] = Feven[k] + cmplxExponential;
                freqBins[k+N/2] = Feven[k] - cmplxExponential;
            }

            return freqBins;

        }


        // Calculates the magnitude from freq bins
        std::vector<double> FFT_Magnitude(std::vector< std::complex< double> > complexVector) {
            int samplingFrequency = complexVector.size();

            std::vector< std::complex< double> >::const_iterator first = complexVector.begin();
            std::vector< std::complex< double> >::const_iterator last = complexVector.end();
//            std::vector< std::complex< double> >::const_iterator last = complexVector.begin() + (complexVector.size() / 2);
            std::vector< std::complex< double> > complexVector_NyquistLimited(first, last);

            std::vector<double> frequencyMagnitude(64);

            double magnitude_sum = 0;
            double max = 0;

            for (int frequency = 0; frequency < complexVector_NyquistLimited.size() / 2; frequency++) {

                double magnitude_scaled;
                double magnitude_scaled_avg;

                // Calculates magnitude from freq bin
                double real = complexVector[frequency].real() * 2 / samplingFrequency;
                double imag = complexVector[frequency].imag() * 2 / samplingFrequency;
                double magnitude = sqrt(pow(real,2) + pow(imag, 2));

                if (magnitude == 0.0) {
                    magnitude = 1;
                }


                magnitude_sum += magnitude;

                // Two seperate bools for setting freq ranges to give priority to freq ranges below 1khz
                bool addLowFreqRangeValue = ((frequency % 11) == 0);

                // Sets the vector values to contain an average magnitude in a specific frequency range
                if (frequency > 0) {
                    if (frequency <= 704) {
                        if (addLowFreqRangeValue) {
                            magnitude_scaled_avg = magnitude_sum / 11.0;
                            magnitude_sum = 0;
                            frequencyMagnitude[(frequency / 11) - 1] = magnitude_scaled_avg;
                            if (magnitude_scaled_avg > max) {
                                max = magnitude_scaled_avg;
                            }



                        }
                    }
                }

            }

            return frequencyMagnitude;
        }



};

#endif // AUDIO_H
