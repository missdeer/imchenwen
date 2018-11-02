#ifndef CUTTERBAR_H
#define CUTTERBAR_H

#include <QWidget>

namespace Ui {
class CutterBar;
}
class QProcess;
class QTimer;

class CutterBar : public QWidget
{
    Q_OBJECT

public:
    explicit CutterBar(QWidget *parent = nullptr);
    ~CutterBar();
    void init(QString m_filename, int length, int currentPos);

signals:
    void newFrame(int m_pos);
    void finished(void);

private:
    Ui::CutterBar *ui;
    QString m_filename;
    int m_pos;
    int m_startPos;
    int m_endPos;
    bool m_sliderPressed;
    QProcess *m_process;

private slots:
    void onStartSliderChanged(void);
    void onEndSliderChanged(void);
    void onSliderPressed(void);
    void onSliderReleased(void);
    void startTask(void);
    void onFinished(int status);
};

QString secToTime(int second, bool use_format = false);

#endif // CUTTERBAR_H
