#ifndef SNIFFER_H
#define SNIFFER_H

#include <QObject>
#include <QProcess>

class Sniffer : public QObject
{
    Q_OBJECT
public:
    explicit Sniffer(QObject *parent = nullptr);

    void sniff(const QString &url);
signals:
    void done(QString);
    void error();
public slots:
    void onReadStandardOutput();
    void onFinished(int exitCode, QProcess::ExitStatus);

private:
    QProcess m_process;
    QByteArray m_data;
};

#endif // SNIFFER_H
