#include "audiorecorder.h"
#include "audiolevel.h"
#include <curl/curl.h>
#include <cpr/cpr.h>
#include <json/json.h>

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "ui_audiorecorder.h"
#include "asciibreaker.h"
#include "Sound.h"
#include "decoder.h"
#include "extractor.h"

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

    connect(m_audioRecorder, &QMediaRecorder::durationChanged, this, &AudioRecorder::updateProgress);
    connect(m_audioRecorder, &QMediaRecorder::recorderStateChanged, this, &AudioRecorder::onStateChanged);
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
        m_audioRecorder->setAudioSampleRate(44100);
        m_audioRecorder->setAudioBitRate(128000);
        m_audioRecorder->setAudioChannelCount(1);
        m_audioRecorder->setQuality(QMediaRecorder::VeryHighQuality);
        m_audioRecorder->setEncodingMode(QMediaRecorder::ConstantQualityEncoding);
        // options
        // QMediaRecorder::ConstantQualityEncoding :
        // QMediaRecorder::ConstantBitRateEncoding

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

void AudioRecorder::displayErrorMessage()
{
    ui->statusbar->showMessage(m_audioRecorder->errorString());
}

void AudioRecorder::clearAudioLevels()
{
    for (auto m_audioLevel : qAsConst(m_audioLevels))
        m_audioLevel->setLevel(0);
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

void AudioRecorder::processBuffer(const QAudioBuffer& buffer)
{
    if (m_audioLevels.count() != buffer.format().channelCount()) {
        qDeleteAll(m_audioLevels);
        m_audioLevels.clear();
        for (int i = 0; i < buffer.format().channelCount(); ++i) {
            AudioLevel *level = new AudioLevel(ui->centralwidget);
            m_audioLevels.append(level);
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

    QFileInfo *audio = new QFileInfo("/Users/dawid/Music/recorderAudio.wav");
    sf::InputSoundFile file;
    if (!file.openFromFile(audio->filePath().toStdString()))
        std::cout << "Error" << std::endl;

    std::cout << "duration: " << file.getDuration().asSeconds() << std::endl;
    std::cout << "channels: " << file.getChannelCount() << std::endl;
    std::cout << "sample rate: " << file.getSampleRate() << std::endl;
    std::cout << "sample count: " << file.getSampleCount() << std::endl;
    // Read and process batches of samples until the end of file is reached
    sf::Int16 samples[1024];
    sf::Uint16 count;
    extractor ex;
    short data[1024];
    int i = 0;
    do
    {
        count = file.read(samples, 1024);
        data[i] = samples[i];

        auto frequency = ex.extract((sizeof(data) / sizeof(*data)), data, file.getSampleRate());
        if(frequency > 800) {
            std::cout << frequency << std::endl;
        }

        // process, analyze, play, convert, or whatever
        // you want to do with the samples...
        i++;
    }
    while (count > 0);


}
