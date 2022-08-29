#include "bin_check.h"

bool BinaryCheck::checkFileExistsOn_Linux(const QString name)
{
    QString system_path = QString::fromUtf8(qgetenv("PATH"));
    QStringList file_path = system_path.split(":");


    for(const auto &p : qAsConst(file_path)){
        QStringList absolute_path;
        absolute_path << p << "/" << name;
        QFileInfo info(absolute_path.join(""));


        if(!info.exists() || p.count() == 0 ) continue;

        if(info.isExecutable() || info.isSymLink()){
            return true;
        }

    }


    return false;

}

QStringList BinaryCheck::checkFilesOn_Linux()
{
    QStringList missing;
    if(!BinaryCheck::checkFileExistsOn_Linux("md5sum")){
        missing << "md5sum";
    }

    if(!BinaryCheck::checkFileExistsOn_Linux("sha256sum")){
        missing << "sha256sum";
    }

    if(!BinaryCheck::checkFileExistsOn_Linux("sha512sum")){
        missing << "sha512sum";
    }


    return missing;
}

bool BinaryCheck::checkFileExistsOn_Win32(const QString name)
{
    QString win_path = QString::fromUtf8(qgetenv("windir"));
    QFileInfo info;
    QStringList abspath;
    abspath << win_path << "\\" << "system32" << "\\" << name;
    info.setFile(abspath.join(""));

    if(!info.exists()){
        return false;
    }

    if(!info.isExecutable()){
        return false;
    }

    return true;
}

QStringList BinaryCheck::checkFilesOn_Win32()
{
    QStringList missing;
    if(!checkFileExistsOn_Win32("certUtil.exe")){
        missing << "certUtil";
    }

    return missing;
}
