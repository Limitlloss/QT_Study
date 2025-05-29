#ifndef COPYDIRTHREAD_H
#define COPYDIRTHREAD_H

#include <QThread>

using PathPairList = QList<std::pair<QString, QString>>;

class CopyDirThread : public QThread {
    Q_OBJECT
public:
    CopyDirThread();
    PathPairList paths;
    void run();
    void move();
protected:
    bool copyDir(const QString &fromDir, const QString &toDir, bool coverFileIfExist);
    bool moveDir(const QString &fromDir, const QString &toDir);

    int i = 0;
    int copiedSize = 0;
signals:
    void sigProgress(int, int);
};
#endif // COPYDIRTHREAD_H
