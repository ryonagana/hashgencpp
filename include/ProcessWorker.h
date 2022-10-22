#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H

#include <QtCore>
#include <QRunnable>
#include <QObject>
#include <QMutex>
enum HashType {
    MD5,
    SHA256,
    SHA512
};


class ProcessSignals : public QObject {
    Q_OBJECT
public:
    ProcessSignals();



signals:
    void progress(int i);
    void result(int type, QStringList result);
};



class ProcessWorker : public QRunnable
{

public:
    ProcessWorker();
    ProcessWorker(const QString &command, const QStringList &args, HashType type);
    void setSize(qint64 _val) { size = _val; }
    qint64 getSize() { return size; } ;
    ProcessSignals& getSignals() { return sig; }

protected:
    void run();


private:
    QString command;
    QStringList arglist;
    HashType type;
    ProcessSignals sig;
    qint64 size;





};




#endif // PROCESSWORKER_H
