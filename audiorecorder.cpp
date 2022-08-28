/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "audiorecorder.h"
#include "audiolevel.h"
#include <curl/curl.h>
#include <cpr/cpr.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ui_audiorecorder.h"
#include "asciibreaker.h"
#include "Sound.h"
#include "decoder.h"

#include <QAudioDecoder>
#include <QMediaRecorder>
#include <QDir>
#include <QFileDialog>
#include <QMediaRecorder>
#include <QStandardPaths>
#include <qmediadevices.h>
#include <qmediaformat.h>
#include <qaudiodevice.h>
#include <qaudiobuffer.h>
#include <qaudioinput.h>
#include <qimagecapture.h>
#include <QMimeType>

static QList<qreal> getBufferLevels(const QAudioBuffer &buffer);

AudioRecorder::AudioRecorder()
    : ui(new Ui::AudioRecorder)
{
    ui->setupUi(this);

    m_audioRecorder = new QMediaRecorder(this);
    m_captureSession.setRecorder(m_audioRecorder);
    m_captureSession.setAudioInput(new QAudioInput(this));


    //audio devices
    ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    for (auto device: QMediaDevices::audioInputs()) {
        auto name = device.description();
        ui->audioDeviceBox->addItem(name, QVariant::fromValue(device));
    }

    //audio codecs and container formats
    updateFormats();
    connect(ui->audioCodecBox, &QComboBox::currentIndexChanged, this, &AudioRecorder::updateFormats);
    connect(ui->containerBox, &QComboBox::currentIndexChanged, this, &AudioRecorder::updateFormats);

    //sample rate
    ui->sampleRateBox->setRange(m_captureSession.audioInput()->device().minimumSampleRate(),
                                m_captureSession.audioInput()->device().maximumSampleRate());
    ui->sampleRateBox->setValue(qBound(m_captureSession.audioInput()->device().minimumSampleRate(), 44100,
                                       m_captureSession.audioInput()->device().maximumSampleRate()));

    //channels
    ui->channelsBox->addItem(tr("Default"), QVariant(-1));
    ui->channelsBox->addItem(QStringLiteral("1"), QVariant(1));
    ui->channelsBox->addItem(QStringLiteral("2"), QVariant(2));
    ui->channelsBox->addItem(QStringLiteral("4"), QVariant(4));

    //quality
    ui->qualitySlider->setRange(0, int(QImageCapture::VeryHighQuality));
    ui->qualitySlider->setValue(int(QImageCapture::NormalQuality));

    //bit rates:
    ui->bitrateBox->addItem(tr("Default"), QVariant(0));
    ui->bitrateBox->addItem(QStringLiteral("32000"), QVariant(32000));
    ui->bitrateBox->addItem(QStringLiteral("64000"), QVariant(64000));
    ui->bitrateBox->addItem(QStringLiteral("96000"), QVariant(96000));
    ui->bitrateBox->addItem(QStringLiteral("128000"), QVariant(128000));

    connect(m_audioRecorder, &QMediaRecorder::durationChanged, this, &AudioRecorder::updateProgress);
    connect(m_audioRecorder, &QMediaRecorder::recorderStateChanged, this, &AudioRecorder::onStateChanged);
    connect(m_audioRecorder, &QMediaRecorder::errorChanged, this, &AudioRecorder::displayErrorMessage);
}

void AudioRecorder::updateProgress(qint64 duration)
{
    if (m_audioRecorder->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    ui->statusbar->showMessage(tr("Recorded %1 sec").arg(duration / 1000));
}

void AudioRecorder::onStateChanged(QMediaRecorder::RecorderState state)
{
    QString statusMessage;

    switch (state) {
    case QMediaRecorder::RecordingState:
        statusMessage = tr("Recording to %1").arg(m_audioRecorder->actualLocation().toString());
        ui->recordButton->setText(tr("Stop"));
        ui->pauseButton->setText(tr("Pause"));
        break;
    case QMediaRecorder::PausedState:
        clearAudioLevels();
        statusMessage = tr("Paused");
        ui->recordButton->setText(tr("Stop"));
        ui->pauseButton->setText(tr("Resume"));
        break;
    case QMediaRecorder::StoppedState:
        clearAudioLevels();
        statusMessage = tr("Stopped");
        ui->recordButton->setText(tr("Record"));
        ui->pauseButton->setText(tr("Pause"));
        break;
    }

    ui->pauseButton->setEnabled(m_audioRecorder->recorderState() != QMediaRecorder::StoppedState);
    if (m_audioRecorder->error() == QMediaRecorder::NoError)
        ui->statusbar->showMessage(statusMessage);
}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

void AudioRecorder::toggleRecord()
{
    QString fileName = "recorderAudio";
    m_audioRecorder->setOutputLocation(fileName);
    if (m_audioRecorder->recorderState() == QMediaRecorder::StoppedState) {
        m_captureSession.audioInput()->setDevice(boxValue(ui->audioDeviceBox).value<QAudioDevice>());

        m_audioRecorder->setMediaFormat(selectedMediaFormat());
        m_audioRecorder->setAudioSampleRate(ui->sampleRateBox->value());
        m_audioRecorder->setAudioBitRate(boxValue(ui->bitrateBox).toInt());
        m_audioRecorder->setAudioChannelCount(boxValue(ui->channelsBox).toInt());
        m_audioRecorder->setQuality(QMediaRecorder::Quality(ui->qualitySlider->value()));
        m_audioRecorder->setEncodingMode(ui->constantQualityRadioButton->isChecked() ?
                                 QMediaRecorder::ConstantQualityEncoding :
                                 QMediaRecorder::ConstantBitRateEncoding);

        m_audioRecorder->record();
    }
    else {
        m_audioRecorder->stop();
    }
}

void AudioRecorder::togglePause()
{
    if (m_audioRecorder->recorderState() != QMediaRecorder::PausedState)
        m_audioRecorder->pause();
    else
        m_audioRecorder->record();
}

void AudioRecorder::setOutputLocation()
{
//    QString fileName = QFileDialog::getSaveFileName();

    QString fileName = "recorderAudio";
    m_audioRecorder->setOutputLocation(fileName);
    m_outputLocationSet = true;
}

void AudioRecorder::displayErrorMessage()
{
    ui->statusbar->showMessage(m_audioRecorder->errorString());
}

void AudioRecorder::updateFormats()
{
    if (m_updatingFormats)
        return;
    m_updatingFormats = true;

    QMediaFormat format;
    if (ui->containerBox->count())
        format.setFileFormat(boxValue(ui->containerBox).value<QMediaFormat::FileFormat>());
    if (ui->audioCodecBox->count())
        format.setAudioCodec(boxValue(ui->audioCodecBox).value<QMediaFormat::AudioCodec>());

    int currentIndex = 0;
    ui->audioCodecBox->clear();
    ui->audioCodecBox->addItem(tr("Default audio codec"), QVariant::fromValue(QMediaFormat::AudioCodec::Unspecified));
    for (auto codec : format.supportedAudioCodecs(QMediaFormat::Encode)) {
        if (codec == format.audioCodec())
            currentIndex = ui->audioCodecBox->count();
        ui->audioCodecBox->addItem(QMediaFormat::audioCodecDescription(codec), QVariant::fromValue(codec));
    }
    ui->audioCodecBox->setCurrentIndex(currentIndex);

    currentIndex = 0;
    ui->containerBox->clear();
    ui->containerBox->addItem(tr("Default file format"), QVariant::fromValue(QMediaFormat::UnspecifiedFormat));
    for (auto container : format.supportedFileFormats(QMediaFormat::Encode)) {
        if (container < QMediaFormat::Mpeg4Audio) // Skip video formats
            continue;
        if (container == format.fileFormat())
            currentIndex = ui->containerBox->count();
        ui->containerBox->addItem(QMediaFormat::fileFormatDescription(container), QVariant::fromValue(container));
    }
    ui->containerBox->setCurrentIndex(currentIndex);

    m_updatingFormats = false;
}

void AudioRecorder::clearAudioLevels()
{
    for (auto m_audioLevel : qAsConst(m_audioLevels))
        m_audioLevel->setLevel(0);
}

QMediaFormat AudioRecorder::selectedMediaFormat() const
{
    QMediaFormat format;
    format.setFileFormat(boxValue(ui->containerBox).value<QMediaFormat::FileFormat>());
    format.setAudioCodec(boxValue(ui->audioCodecBox).value<QMediaFormat::AudioCodec>());
    return format;
}

// returns the audio level for each channel
QList<qreal> getBufferLevels(const QAudioBuffer &buffer)
{
    QList<qreal> values;

    auto format = buffer.format();
    if (!format.isValid())
        return values;

    int channels = buffer.format().channelCount();
    values.fill(0, channels);

    int bytesPerSample = format.bytesPerSample();
    QList<qreal> max_values;
    max_values.fill(0, channels);

    const char *data = buffer.constData<char>();
    for (int i = 0; i < buffer.frameCount(); ++i) {
        for (int j = 0; j < channels; ++j) {
            qreal value = qAbs(format.normalizedSampleValue(data));
            if (value > max_values.at(j))
                max_values[j] = value;
            data += bytesPerSample;
        }
    }

    return max_values;
}

void AudioRecorder::processBuffer(const QAudioBuffer& buffer)
{
    if (m_audioLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(m_audioLevels);
        m_audioLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            AudioLevel *level = new AudioLevel(ui->centralwidget);
            m_audioLevels.append(level);
            ui->levelsLayout->addWidget(level);
        }
    }

    QList<qreal> levels = getBufferLevels(buffer);
    for (int i = 0; i < levels.count(); ++i)
        m_audioLevels.at(i)->setLevel(levels.at(i));
}

void AudioRecorder::on_pushButton_clicked()
{
    cpr::Response r = cpr::Get(cpr::Url{"https://zaliczeniebackend.azurewebsites.net/api/files"});
    std::cout << r.text << std::endl;
}


void AudioRecorder::on_file_clicked()
{
    QString filename= QFileDialog::getOpenFileName(this, "Choose File");


   if(filename.isEmpty())
       return;


   int number = rand() % 90000000 + 10000000;
   std::cout << "OUTPUY NUMBER: " << number << std::endl;

   QFileInfo file(filename);
   QFile fileToCopy(filename);

   // todo: delete files from copied folder in order to prevent build failure
   fileToCopy.setFileName(QString::fromStdString(std::to_string(number)) + "." + file.completeSuffix());



   QFileInfo newFileInfo(fileToCopy.fileName());
   this->filePath = newFileInfo.filePath();
   this->fileName = fileToCopy.fileName();
   QFile::copy(file.absoluteFilePath(), this->filePath);
   std::cout << "FILEPATH: " << this->filePath.toStdString() << std::endl;
   std::cout << "FILE NAME: " << this->fileName.toStdString() << std::endl;
}


void AudioRecorder::on_sendButton_clicked()
{
    // send file
    cpr::Response r = cpr::Post(cpr::Url{"https://zaliczeniebackend.azurewebsites.net/api/files"},
                       cpr::Multipart{{"files", cpr::File{this->filePath.toStdString()}}});

    std::cout << r.status_code << std::endl;

    // DEBUG:
    std::cout << r.text << std::endl;
}


void AudioRecorder::on_createWav_clicked()
{
    //    ASCIIBreaker asciiBreaker(this->fileName.toInt());
        // debug


        ASCIIBreaker asciiBreaker(31412551);
        auto freq = asciiBreaker.ASCIIToFrequency();

        // debug
        for(int i = 0; i < freq.size(); i ++) {
            auto item = asciiBreaker.frequencyToNumber(freq[i]);

            std::cout << "TUTAJ: " << item << std::endl;
        }


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


void AudioRecorder::on_decodeFile_clicked()
{
    return;
//     QFileInfo *audio = new QFileInfo("/Users/dawid/Music/recorderAudio.wav");
//    sf::InputSoundFile file;
//    if (!file.openFromFile(audio->filePath().toStdString()))
//        std::cout << "Error" << std::endl;

//    std::cout << "duration: " << file.getDuration().asSeconds() << std::endl;
//    std::cout << "channels: " << file.getChannelCount() << std::endl;
//    std::cout << "sample rate: " << file.getSampleRate() << std::endl;
//    std::cout << "sample count: " << file.getSampleCount() << std::endl;

//    // Read and process batches of samples until the end of file is reached
//    sf::Int16 samples[1024];
//    sf::Uint64 count;
//    do
//    {
//        count = file.read(samples, 1024);

//        std::cout << count << std::endl;
//        // process, analyze, play, convert, or whatever
//        // you want to do with the samples...
//    }
//    while (count > 0);

}
