#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QMainWindow>
#include <QMediaRecorder>
#include <QMediaCaptureSession>
#include <iostream>
#include <QUrl>

QT_BEGIN_NAMESPACE
namespace Ui { class AudioRecorder; }
class QAudioBuffer;
QT_END_NAMESPACE

class AudioLevel;

class AudioRecorder : public QMainWindow
{
    Q_OBJECT

public:
    AudioRecorder();
    QString filePath;
    QString fileName;


private slots:

    void updateProgress(qint64 pos);

    void displayErrorMessage();

    void on_sendFileMain_clicked();

    void generateSineWave();

    void decode();

    void on_reciveFileMain_clicked();

private:
    QMediaFormat selectedMediaFormat() const;

    Ui::AudioRecorder *ui = nullptr;

    QMediaCaptureSession m_captureSession;
    QMediaRecorder *m_audioRecorder = nullptr;
    QList<AudioLevel*> m_audioLevels;
    bool m_outputLocationSet = false;
};

#endif // AUDIORECORDER_H
