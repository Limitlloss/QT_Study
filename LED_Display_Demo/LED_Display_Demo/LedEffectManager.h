#ifndef LEDEFFECTMANAGER_H
#define LEDEFFECTMANAGER_H

#include <QObject>
#include <QTimer>
#include <QColor>

class LedEffectManager : public QObject {
    Q_OBJECT

public:
    explicit LedEffectManager(QObject* parent = nullptr);
    void start();
    void stop();

signals:
    void colorChanged(const QColor& color);

private:
    QTimer timer;
    int hue = 0;

private slots:
    void updateColor();
};

#endif // LEDEFFECTMANAGER_H
