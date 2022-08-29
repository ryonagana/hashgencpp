#include "hashgen_ini.h"

HashgenIniConfig::HashgenIniConfig(const QString &filepath)
{
    this->loadFile(filepath);
}

bool HashgenIniConfig::loadFile(const QString &filepath)
{
    QFileInfo info(filepath);

    if(!info.exists()){
        this->defaultConfig();
    }



    this->filepath = filepath;
    if(this->settings.status() == QSettings::Status::AccessError){
        return false;
    }


    return true;
}

void HashgenIniConfig::defaultConfig()
{
    QSettings tmp(QDir::currentPath() + filepath, QSettings::Format::IniFormat);
    tmp.beginGroup("path");

#ifdef Q_OS_WIN
    getWindirPath(tmp, "certutil.exe");
#endif
   tmp.endGroup();


   qDebug() << tmp.allKeys();

   for(const auto &key : tmp.allKeys()){

       qDebug() << tmp.value(key);

   }


   tmp.sync();


    return;
}

void HashgenIniConfig::getWindirPath(QSettings& settings, const QString &file)
{
    QString windir = QString::fromUtf8(qgetenv("windir"));
    settings.setValue("certutil_path", windir + "\\" + "system32" + "\\" +file);
    return;
}

void HashgenIniConfig::getLinuxPath(QSettings *settings, const QString& keyname, const QString &file)
{
    QString path = QString::fromUtf8(qgetenv("PATH"));
    QStringList lst = path.split(":");

    for(const auto& p : qAsConst(lst)){
        QStringList abspath = QStringList() << p << "//" << file;
        settings->setValue(keyname, p);

    }
}
