#ifndef RUNCOMMAND_H
#define RUNCOMMAND_H

#include <QtCore>
#include <QRunnable>


enum class HashType : int {
    MD5,
    SHA256,
    SHA512
};


class RunCommand : public QRunnable
{
public:
    RunCommand();
    RunCommand(const QString &command, const QStringList &args, HashType type);
    virtual void run();

signals:
    void progress(int i);
    void result(HashType type, QString result);

private:
    QString command;
    QStringList arglist;
    HashType type;
    QString output;
};

#endif // RUNCOMMAND_H
