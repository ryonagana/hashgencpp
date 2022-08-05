#ifndef WORKER_H
#define WORKER_H

#include <QtCore>
#include <QRunnable>
#include <QObject>

enum HashType {
    MD5,
    SHA256,
    SHA512
};


class ProcessSignals : public QObject{
    Q_OBJECT

signals:
    void progress(int i);
    void result(int type, QStringList result);
};



class ProcessWorker : public QRunnable
{
public:
    ProcessWorker();
    ProcessWorker(const QString &command, const QStringList &args, HashType type);

    ProcessSignals& getSignals() { return sig; }

protected:
    void run();



private:
    QString command;
    QStringList arglist;
    HashType type;
    QString output;
    ProcessSignals sig;





};




#endif // WORKER_H
