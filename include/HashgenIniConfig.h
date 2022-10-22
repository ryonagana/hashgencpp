#ifndef HASHGENINICONFIG_H
#define HASHGENINICONFIG_H
#include <QtCore>
#include <QSettings>

class HashgenIniConfig {
public:
    HashgenIniConfig(){}
    HashgenIniConfig(const QString& filepath);
    bool loadFile(const QString& filepath);
    void defaultConfig();


    void getWindirPath(QSettings &settings, const QString &file);
    void getLinuxPath(QSettings &settings, const QString &keyname, const QString &file);
private:
    QSettings settings;
    QString filepath;
};


#endif // HASHGENINICONFIG_H
