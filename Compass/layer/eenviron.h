#ifndef EENVIRON_H
#define EENVIRON_H
#include "ebase.h"
#include "gutil/qgui.h"

struct EnvironItem {
    QString text;
    QString unit;
    QString label;
    bool has = true;
};

class EEnviron : public EBase {
    Q_OBJECT
public:
    static LinkedMap<QString, EnvironItem> genItemMap() {
        return LinkedMap<QString, EnvironItem> {
            {"temperature", {tr("Temperature"), "℃"}},
            {"humidity", {tr("Humidity"), "%"}},
            {"noise", {tr("Noise"), "dB"}},
            {"windSpeed", {tr("Wind Speed"), "m/s"}},
            {"windDirection", {tr("Wind Direction")}},
            {"pm2.5", {"PM2.5", "μg/m³"}},
            {"pm10", {"PM10", "μg/m³"}},
            {"SO2", {"SO₂", "ppb"}},
            {"NO2", {"NO₂", "ppb"}},
            {"CO", {"CO", "ppb"}},
            {"O3", {"O₃", "ppb"}},
            {"pressure", {tr("Pressure"), "hPa"}},
            {"rainfall", {tr("Rainfall"), "mm"}},
            {"radiation", {tr("Radiation"), "W/m²"}},
            {"beam", {tr("Beam"), "lux"}},
            {"CO2", {"CO₂", "ppm"}}
        };
    }
    LinkedMap<QString, EnvironItem> itemMap = genItemMap();
    QString title;
    QColor textColor = Qt::red;
    QColor backColor = Qt::transparent;
    QFont font = qfont("Arial", 12);
    int tempCompen = 0;
    int align = 0;
    int scrollSpeed = 30;
    bool useFahrenheit = false;
    bool isSingleLine = false;

    static JObj genProg(const JValue &, const QString &, const QString &);

    explicit EEnviron(EBase *multiWin = nullptr);
    explicit EEnviron(const JObj &json, EBase *multiWin = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void timerEvent(QTimerEvent *) override;
    int type() const override { return EBase::Environ; }
    QWidget* attrWgt() override;
    bool save(const QString &pRoot) override;
    JObj attrJson() const override;

private:
    void init();
    void drawText(QPainter*, QRectF&);
    void calAttr();
    int scroll_width;
    QString scroll_txt;
    int item_cnt = 0;
    int timer_id = 0;
    int scroll_off = 0;

};

#endif // EENVIRON_H
