#include "worker.h"
#include <QDebug>


ProcessWorker::ProcessWorker() : QRunnable()
{
    arglist.clear();
    this->type = HashType::MD5;
}

ProcessWorker::ProcessWorker(const QString &command, const QStringList &args, HashType type) : QRunnable()
{
    arglist.clear();
    this->command = command;
    this->arglist = args;
    this->type = type;


    this->setAutoDelete(true);




}

ProcessSignals::ProcessSignals() : QObject()
{

}



void ProcessWorker::run()
{
    QProcess proc;
    proc.setProgram(this->command);
    proc.setArguments(this->arglist);
    proc.start();





    if(!proc.waitForFinished(60000)){
        qDebug() << proc.errorString();
        proc.kill();
        proc.waitForFinished(1);

        emit this->sig.progress(1);
        emit this->sig.result(this->type, QStringList() << "hash generation Timed Out");
        return;
    }


    QString output = proc.readAllStandardOutput();



    emit this->sig.progress(1);
    emit this->sig.result(this->type, output.split("\r\n"));

    qDebug() << output;

    proc.kill();
    proc.waitForFinished(1);



}


