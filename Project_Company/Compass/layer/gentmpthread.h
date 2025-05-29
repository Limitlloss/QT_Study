#ifndef GENTMPTHREAD_H
#define GENTMPTHREAD_H

#include <QThread>
#include "gutil/qjson.h"

class ProgItem;
class GenTmpThread : public QThread {
    Q_OBJECT
public:
    explicit GenTmpThread(ProgItem *progItem, const QString &prog_name, const QString &zip_file, const QString &password);

    ProgItem *mProgItem;
protected:
    virtual void run() override;

    JObj cvtPage(const JObj &);
    JArray genSources(QString type, const JArray &eles);
    JObj genText(const JValue &json, JArray &);
    JObj genImage(const JValue &json);
    JObj convertGif(const JValue &json);
    JObj convertDClock(const JValue &json);
    JObj convertAClock(const JValue &json);
    JObj convertWeb(const JValue &json);
    JObj convertTimer(const JValue &json);
signals:
    void onErr(QString);
private:
    QString prog_name, dstDir, srcPageDir;
    QString zip_file, password;
    int res_id = 0;
};

#endif // GENTMPTHREAD_H
