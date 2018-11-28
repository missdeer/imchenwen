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
    void stop();
    void start();
signals:
    void done(const QByteArray&);
public slots:

private slots:
    void onReadStandardOutput();
    void onFinished(int, QProcess::ExitStatus);
private:
    QProcess m_process;
    QByteArray m_data;
};

#endif // LINKRESOLVERPROCESS_H
