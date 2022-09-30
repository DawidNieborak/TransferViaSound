#include "audiorecorder.h"
#include <cpr/cpr.h>
#include <json/json.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ui_audiorecorder.h"
#include <cmath>
#include "asciibreaker.h"
#include "Sound.h"
#include "Audio.h"
#include "decoder.h"
#include "cstdlib"

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
    this->setWindowTitle("Transfer files via sound");

    m_audioRecorder = new QMediaRecorder(this);
    m_captureSession.setRecorder(m_audioRecorder);
    m_captureSession.setAudioInput(new QAudioInput(this));


    //audio devices
    ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    for (auto device: QMediaDevices::audioInputs()) {
        auto name = device.description();
        ui->audioDeviceBox->addItem(name, QVariant::fromValue(device));
    }

    connect(m_audioRecorder, &QMediaRecorder::durationChanged, this, &AudioRecorder::updateProgress);
    connect(m_audioRecorder, &QMediaRecorder::errorChanged, this, &AudioRecorder::displayErrorMessage);

    // check status of the server
    Json::Value response;
    Json::Reader reader;
    cpr::Response r = cpr::Get(cpr::Url{"https://zaliczeniebackend.azurewebsites.net/health/ready"});
    reader.parse(r.text, response);

    if(response["status"] == "Healthy") {
        ui->statusBtn->setStyleSheet("QPushButton { background-color: green; border-radius: 25px; }\n");
    } else {
        ui->statusBtn->setStyleSheet("QPushButton { background-color: red; border-radius: 25px; }\n");
        ui->label_2->setText("Application would'nt work because of inaccessibility to the server.");
    }
}

void AudioRecorder::updateProgress(qint64 duration)
{
    if (m_audioRecorder->error() != QMediaRecorder::NoError || duration < 2000)
        return;

    ui->statusbar->showMessage(tr("Recorded %1 sec").arg(duration / 1000));
}

static QVariant boxValue(const QComboBox *box)
{
    int idx = box->currentIndex();
    if (idx == -1)
        return QVariant();

    return box->itemData(idx);
}

void AudioRecorder::displayErrorMessage()
{
    ui->statusbar->showMessage(m_audioRecorder->errorString());
}

QMediaFormat AudioRecorder::selectedMediaFormat() const
{
    QMediaFormat format;

    format.setFileFormat(QMediaFormat::Wave);
    format.setAudioCodec(QMediaFormat::AudioCodec::Wave);
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

void AudioRecorder::on_sendFileMain_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Choose File");


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


   // send file
    ui->statusbar->showMessage("Please wait a little...");
   cpr::Response r = cpr::Post(cpr::Url{"https://zaliczeniebackend.azurewebsites.net/api/files"},
                      cpr::Multipart{{"files", cpr::File{this->filePath.toStdString()}}});


   if(r.status_code == 200) {
        generateSineWave();
   }
   else {
        ui->statusbar->showMessage("Couldn't send file to the server. Please try again.");
   }
}

void AudioRecorder::generateSineWave() {
    ASCIIBreaker asciiBreaker(this->fileName.toInt());
    auto freq = asciiBreaker.ASCIIToFrequency();
    sf::RenderWindow window(sf::VideoMode (500,500), "Close to stop");

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

void AudioRecorder::on_reciveFileMain_clicked()
{
    QString fileName = "recorderAudio";
    m_audioRecorder->setOutputLocation(fileName);
    if (m_audioRecorder->recorderState() == QMediaRecorder::StoppedState) {
        m_captureSession.audioInput()->setDevice(boxValue(ui->audioDeviceBox).value<QAudioDevice>());

        m_audioRecorder->setMediaFormat(selectedMediaFormat());
        m_audioRecorder->setAudioSampleRate(44100);
        m_audioRecorder->setAudioBitRate(128000);
        m_audioRecorder->setAudioChannelCount(1);
        m_audioRecorder->setQuality(QMediaRecorder::VeryHighQuality);
        m_audioRecorder->setEncodingMode(QMediaRecorder::ConstantQualityEncoding);

        auto time = 17;
        time *= CLOCKS_PER_SEC;

        clock_t now = clock();

        m_audioRecorder->record();

        while(clock() - now < time)

        m_audioRecorder->stop();
        decode();


    }
    else {
        m_audioRecorder->stop();
    }

    QFile fileToDelete = QFile("recorderAudio");
    fileToDelete.remove();
}

// generate sfml window and decode;
void AudioRecorder::decode() {
    sf::RenderWindow window(sf::VideoMode(500, 500), "Audio-decoder Close to Stop", sf::Style::Default);
    window.setFramerateLimit(60);

    Audio audio = Audio();
    std::thread frequencyAnalyzationThread(&Audio::getSampleOverFrequency, &audio);

    auto decoded = audio.results;

    std::string s(decoded.begin(), decoded.end());

    cpr::Response r = cpr::Get(cpr::Url{"https://zaliczeniebackend.azurewebsites.net/api/files/"+s});

    if(r.status_code == 200) {
        ui->results->setText(QString::fromStdString(r.text));
    } else {
        ui->statusbar->showMessage("Couldn't get the file.");
    }

}
