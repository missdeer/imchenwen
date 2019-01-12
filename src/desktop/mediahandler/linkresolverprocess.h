#ifndef LINKRESOLVERPROCESS_H
#define LINKRESOLVERPROCESS_H

#include <QObject>
#include <QProcess>

class LinkResolverProcess : public QObject
{
    Q_OBJECT
public:
    explicit LinkResolverProcess(QObject *parent = nullptr);
    void setProgram(const QString& program);
    void setArguments(const QStringList& arguments);
    void start();
    void setTimeout(int timeout);

signals:
    void done(const QByteArray&);
public slots:
    void stop();
private slots:
    void onReadStandardOutput();
    void onFinished(int, QProcess::ExitStatus);
private:
    QProcess m_process;
    QByteArray m_data;
    int m_timeout;
};

#endif // LINKRESOLVERPROCESS_H
