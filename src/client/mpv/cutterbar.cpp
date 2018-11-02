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
    m_sliderPressed = false;
    connect(ui->startSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->startSlider, SIGNAL(valueChanged(int)), this, SLOT(onStartSliderChanged()));
    connect(ui->startSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->endSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->endSlider, SIGNAL(valueChanged(int)), this, SLOT(onEndSliderChanged()));
    connect(ui->endSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(finished()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(startTask()));

    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
}

CutterBar::~CutterBar()
{
    delete ui;
}

void CutterBar::init(QString filename, int length, int currentPos)
{
    this->m_filename = filename;
    ui->startSlider->setMaximum(length);
    ui->startSlider->setValue(currentPos);
    ui->endSlider->setMaximum(length);
    ui->endSlider->setValue(currentPos + 1);
    m_startPos = currentPos;
    m_endPos = currentPos + 1;
}

void CutterBar::onSliderPressed()
{
    m_sliderPressed = true;
}

void CutterBar::onStartSliderChanged()
{
    m_startPos = m_pos = ui->startSlider->value();
    ui->startPosLabel->setText(secToTime(m_pos));
    //Show preview when the progress is changed by keyboard
    if (isVisible() && !m_sliderPressed)
        emit newFrame(m_pos);
}

void CutterBar::onEndSliderChanged()
{
    m_endPos = m_pos = ui->endSlider->value();
    ui->endPosLabel->setText(secToTime(m_pos));
    if (isVisible() && !m_sliderPressed)
        emit newFrame(m_pos);
}

void CutterBar::onSliderReleased()
{
    m_sliderPressed = false;
    //Show preview when the progress is changed by mouse
    emit newFrame(m_pos);
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

    if (m_startPos >= m_endPos)
    {
        QMessageBox::warning(this,  tr("Error"), tr("Time position is not valid."));
        return;
    }

    QString new_name = QString("%1_clip.%2").arg(m_filename.section('.', 0, -2), m_filename.section('.', -1));
    QStringList args;
    args << "-y" << "-ss" << secToTime(m_startPos) << "-i" << m_filename <<
            "-acodec" << "copy" << "-vcodec" << "copy" << "-t" << secToTime(m_endPos - m_startPos) << new_name;
    ui->okButton->setEnabled(false);
    ui->cancelButton->setEnabled(false);
    m_process->start(ffmpeg, args, QProcess::ReadOnly);
}

void CutterBar::onFinished(int status)
{
    if (status)
        QMessageBox::critical(this, tr("FFMPEG ERROR"), QTextCodec::codecForLocale()->toUnicode(m_process->readAllStandardError()));
    ui->okButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);
    QMessageBox::information(this, tr("Finished"), tr("Finished"));
    emit finished();
}
