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
    QUrl musicFilePath;
    QString filePath;
    QString fileName;


public slots:
    void processBuffer(const QAudioBuffer&);

private slots:
    void togglePause();
    void toggleRecord();

    void onStateChanged(QMediaRecorder::RecorderState);
    void updateProgress(qint64 pos);
    void displayErrorMessage();

    void on_pushButton_clicked();

    void on_file_clicked();

    void on_sendButton_clicked();

    void on_createWav_clicked();

    void on_decodeFile_clicked();

    void on_listenBtn_clicked();

private:
    void clearAudioLevels();
    QMediaFormat selectedMediaFormat() const;

    Ui::AudioRecorder *ui = nullptr;

    QMediaCaptureSession m_captureSession;
    QMediaRecorder *m_audioRecorder = nullptr;
    QList<AudioLevel*> m_audioLevels;
    bool m_outputLocationSet = false;
    double window[];
};

#endif // AUDIORECORDER_H
