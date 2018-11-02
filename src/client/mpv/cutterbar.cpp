#include "cutterbar.h"
#include "ui_cutterbar.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QTextCodec>


static QString secToTime(int second, bool use_format = false)
{
    static QString format = "<span style=\" font-size:14pt; font-weight:600;color:#00ff00;\">%1:%2:%3</span>";
    QString  hour = QString::number(second / 3600);
    QString min = QString::number((second % 3600) / 60);
    QString sec = QString::number(second % 60);
    if (min.length() == 1)
        min.prepend('0');
    if (sec.length() == 1)
        sec.prepend('0');
    if (use_format)
        return format.arg(hour, min, sec);
    else
        return QString("%1:%2:%3").arg(hour, min, sec);
}

CutterBar::CutterBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutterBar)
{
    ui->setupUi(this);
    slider_pressed = false;
    connect(ui->startSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->startSlider, SIGNAL(valueChanged(int)), this, SLOT(onStartSliderChanged()));
    connect(ui->startSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->endSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->endSlider, SIGNAL(valueChanged(int)), this, SLOT(onEndSliderChanged()));
    connect(ui->endSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(finished()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(startTask()));

    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
}

CutterBar::~CutterBar()
{
    delete ui;
}

void CutterBar::init(QString filename, int length, int currentPos)
{
    this->filename = filename;
    ui->startSlider->setMaximum(length);
    ui->startSlider->setValue(currentPos);
    ui->endSlider->setMaximum(length);
    ui->endSlider->setValue(currentPos + 1);
    startPos = currentPos;
    endPos = currentPos + 1;
}

void CutterBar::onSliderPressed()
{
    slider_pressed = true;
}

void CutterBar::onStartSliderChanged()
{
    startPos = pos = ui->startSlider->value();
    ui->startPosLabel->setText(secToTime(pos));
    //Show preview when the progress is changed by keyboard
    if (isVisible() && !slider_pressed)
        emit newFrame(pos);
}

void CutterBar::onEndSliderChanged()
{
    endPos = pos = ui->endSlider->value();
    ui->endPosLabel->setText(secToTime(pos));
    if (isVisible() && !slider_pressed)
        emit newFrame(pos);
}

void CutterBar::onSliderReleased()
{
    slider_pressed = false;
    //Show preview when the progress is changed by mouse
    emit newFrame(pos);
}

void CutterBar::startTask()
{
    QString ffmpeg = "ffmpeg";
    // Check whether ffmpeg is installed
    if (ffmpeg.isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("FFMPEG is not installed. Please download it from") +
                             "\n    http://johnvansickle.com/ffmpeg/\n" +
                            tr("and place file \"ffmpeg\" into ~/.moonplayer/ or /usr/share/moonplayer/"));
        return;
    }

    if (startPos >= endPos)
    {
        QMessageBox::warning(this,  tr("Error"), tr("Time position is not valid."));
        return;
    }

    QString new_name = QString("%1_clip.%2").arg(filename.section('.', 0, -2), filename.section('.', -1));
    QStringList args;
    args << "-y" << "-ss" << secToTime(startPos) << "-i" << filename <<
            "-acodec" << "copy" << "-vcodec" << "copy" << "-t" << secToTime(endPos - startPos) << new_name;
    ui->okButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);
    process->start(ffmpeg, args, QProcess::ReadOnly);
}

void CutterBar::onFinished(int status)
{
    if (status)
        QMessageBox::critical(this, tr("FFMPEG ERROR"), QTextCodec::codecForLocale()->toUnicode(process->readAllStandardError()));
    ui->okButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);
    QMessageBox::information(this, tr("Finished"), tr("Finished"));
    emit finished();
}
