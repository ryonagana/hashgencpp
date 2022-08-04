#include "runcommand.h"
#include <QDebug>

RunCommand::RunCommand() : QRunnable()
{
    arglist.clear();
    this->type = HashType::MD5;
}

RunCommand::RunCommand(const QString &command, const QStringList &args, HashType type) : QRunnable()
{
    arglist.clear();
    this->command = command;
    this->arglist = args;
    this->type = type;


}

void RunCommand::run()
{
  QProcess proc;
  proc.setProgram(this->command);
  proc.setArguments(this->arglist);
  proc.start();

  proc.waitForFinished();

  output = proc.readAllStandardOutput();

  qDebug() << "Command:" << this->command << " " << this->arglist << "\n";
  qDebug() << output.toUtf8() << "\n";

  proc.closeWriteChannel();



}

