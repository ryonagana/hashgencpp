#ifndef BIN_CHECK_CPP
#define BIN_CHECK_CPP
#include <iostream>

#include <QtCore>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

namespace BinaryCheck {
    bool checkFileExistsOn_Linux(const QString name);
    bool checkFileExistsOn_Win32(const QString name);


    QStringList checkFilesOn_Linux();
    QStringList checkFilesOn_Win32();
} // namespace BinaryCheck

#endif // BIN_CHECK_CPP
