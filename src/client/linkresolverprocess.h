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
    void terminate();
    void start();
signals:
    void data(const QByteArray&);
public slots:

private slots:
    void readStandardOutput();
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
private:
    QProcess m_process;
    QByteArray m_data;
};

#endif // LINKRESOLVERPROCESS_H
