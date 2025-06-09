#include "copydirthread.h"
#include <QDir>

CopyDirThread::CopyDirThread() {
    connect(this, &CopyDirThread::finished, this, &CopyDirThread::deleteLater);
}

void CopyDirThread::run() {
    for(auto &path : paths) {
        copiedSize = 0;
        copyDir(path.first, path.second, true);
    }
}

bool CopyDirThread::copyDir(const QString &fromDir, const QString &toDir, bool coverIfExist) {
    QDir targetDir(toDir);
    if(! targetDir.exists() && ! targetDir.mkdir(toDir)) return false;
    QFileInfoList fileInfos = QDir(fromDir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for(auto &fileInfo : fileInfos) {
        if(fileInfo.isDir()) {    //< 当为目录时，递归的进行copy
            if(! copyDir(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverIfExist)) return false;
        } else {            //当允许覆盖操作时，将旧文件进行删除操作
            if(coverIfExist && targetDir.exists(fileInfo.fileName())) targetDir.remove(fileInfo.fileName());
            if(! QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) return false;
            else {
                copiedSize += fileInfo.size();
                emit sigProgress(i, copiedSize);
            }
        }
    }
    return true;
}


void CopyDirThread::move() {
    for(auto &path : paths) {
        copiedSize = 0;
        moveDir(path.first, path.second);
    }
}

bool CopyDirThread::moveDir(const QString &fromDir, const QString &toDir) {
    QDir targetDir(toDir);
    if(! targetDir.exists() && ! targetDir.mkdir(toDir)) return false;
    QFileInfoList fileInfos = QDir(fromDir).entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, fileInfos) {
        if(fileInfo.isDir()) {    //< 当为目录时，递归的进行copy
            if(! moveDir(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) return false;
        } else {            //当允许覆盖操作时，将旧文件进行删除操作
            if(! QFile::rename(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) return false;
            else {
                copiedSize += fileInfo.size();
                emit sigProgress(i, copiedSize);
            }
        }
    }
    return true;
}
