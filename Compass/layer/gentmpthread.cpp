#include "gentmpthread.h"
#include "main.h"
#include "tools.h"
#include "program/eenviron.h"
#include "program/evideo.h"
#include <QBuffer>
#include <QProcess>
#include <QMessageBox>
#include <QPainter>

GenTmpThread::GenTmpThread(ProgItem *progItem, const QString &prog_name, const QString &zip_file, const QString &password) : mProgItem(progItem), prog_name(prog_name), zip_file(zip_file), password(password) {
    connect(this, &QThread::finished, this, &QThread::deleteLater);
}

void GenTmpThread::run() {
    auto srcDir = programsDir() + "/" + prog_name;
    dstDir = srcDir + "_tmp";
    //清空目录
    QDir progsDir(programsDir());
    progsDir.remove(prog_name + "_tmp.zip");
    QDir dstQDir(dstDir);
    if(! dstQDir.exists() || dstQDir.removeRecursively()) {
        int iReTryCount = 0;
        while(!progsDir.mkdir(prog_name + "_tmp")) {
            QThread::msleep(250);
            iReTryCount++;
            if(iReTryCount > 4) break;
        }
    }

    QFile jsonFile(srcDir+"/pro.json");
    if(! jsonFile.open(QIODevice::ReadOnly)) {
        emit onErr("Can't open "+srcDir+"/pro.json");
        return;
    }
    auto data = jsonFile.readAll();
    jsonFile.close();
    QString error;
    auto proJson = JFrom(data, &error).toObj();
    if(! error.isEmpty()) {
        emit onErr("Parse "+srcDir+"/pro.json Error: "+error);
        return;
    }

    //扫描节目, 返回多个节目数组
    auto pageNames = QDir(srcDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    //查询 order 属性, 将最上层的放在转换后 layers 的最前面
    //一个 page.json 对应节目任务中的一个 items 里的 program
    std::vector<JObj> pageJsons;
    for(auto &pageName : pageNames) {
        QFile jsonFile(srcDir+"/"+pageName+"/page.json");
        if(jsonFile.open(QIODevice::ReadOnly)) {
            auto data = jsonFile.readAll();
            jsonFile.close();
            auto pageJson = JFrom(data, &error).toObj();
            if(error.isEmpty()) pageJsons.emplace_back(pageJson);
        }
    }
    std::sort(pageJsons.begin(), pageJsons.end(), [](const JObj &a, const JObj &b) {
        return a["order"].toInt() < b["order"].toInt();
    });
    JArray items;
    for(auto &pageJson : pageJsons) {
        srcPageDir = srcDir + "/" + pageJson["name"].toString();
        items.append(cvtPage(pageJson));
    }
    JObj json;
    json["_type"] = "PlayXixunTask";
    json["task"] = JObj{
        {"name", prog_name},
        {"width", proJson["resolution"]["w"]},
        {"height", proJson["resolution"]["h"]},
        {"insert", proJson["isInsert"]},
        {"partLengths", proJson["splitWidths"]},
        {"isVertical", proJson["isVer"]},
        {"items", items}
    };
    QFile program(dstDir + "/program");
    if(program.open(QFile::WriteOnly)) {
        program.write(JToBytes(json, "\t"));
        program.close();
    }

    //如果是usb更新则生成压缩包，网络发送则不需要
    if(! zip_file.isEmpty()) {
#ifdef Q_OS_WIN
        QStringList args{"a", zip_file, dstDir+"/*"};
        if(! password.isEmpty()) args << "-p"+password;
        QProcess::execute("7z.exe", args);
#else
        QStringList args{"-r", zip_file};
        if(! password.isEmpty()) args << "-P" << password;
        args += QDir(dstDir).entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
        QProcess process;
        process.setWorkingDirectory(dstDir);
        process.start("zip", args);
        process.waitForFinished();
#endif
    }
}

//此处需要把幻灯片中的元素按层顺序排序，再放入layers中，每个元素对一个layer。ewindow中的多个顺序元素为一个层上的时间轴上的素材
JObj GenTmpThread::cvtPage(const JObj &pageJson) {
    auto audios = pageJson("audios").toArray();
    auto sourceRepeat = pageJson["loop"].toBool();
    JArray sources;
    int start = 0;
    for(auto &audio : audios) {
        auto dur = audio["dur"].toInt();
        if(dur==0) continue;
        auto name = audio["name"].toString();
        if(name.isEmpty()) continue;
        auto file = audio["dir"].toString()+"/"+name;
        QFileInfo srcInfo(file);
        if(! srcInfo.isFile()) continue;
        auto id = Tools::fileMd5(file);
        if(id.isEmpty()) continue;
        QFile::copy(file, dstDir+"/"+id);
        JObj source;
        source.insert("_type", "Audio");
        source["id"] = id;
        source["md5"] = id;
        source["timeSpan"] = dur;
        source["playTime"] = start;
        source["vol"] = audio["vol"].toInt();
        source["left"] = -1;
        source["top"] = -1;
        source["width"] = 1;
        source["height"] = 1;
        sources.append(source);
        start += dur;
    }
    JArray layers;
    if(! sources.empty()) layers.append(JObj{{"repeat", sourceRepeat}, {"sources", sources}});

    auto elements = pageJson["elements"].toArray();
    for(const auto &ele : elements) {
        auto type = ele["elementType"].toString();
        auto geometry = ele["geometry"];
        bool isWin = type=="Window";
        sources = isWin ? genSources(QString(), ele["elements"].toArray()) : genSources(type, JArray{ele});
        auto startTime = isWin ? 0 : ele["startTime"].toInt();
        for(auto &ss : sources) {
            auto source = ss.toObj();
            source["left"] = geometry["x"];
            source["top"] = geometry["y"];
            source["width"] = geometry["w"];
            source["height"] = geometry["h"];
            source["rotate"] = ele["rotate"];
            source["opacity"] = ele["opacity"].toDouble(1);
            source["playTime"] = startTime;
            startTime += source["timeSpan"].toInt();
        }
        if(! sources.empty()) {
            JObj layer{{"repeat", sourceRepeat}, {"sources", sources}};
            auto border = ele["border"].toString();
            if(! border.isEmpty()) {
                auto bdSrc = QApplication::applicationDirPath()+"/borders/"+border;
                auto id = Tools::fileMd5(bdSrc);
                QFile::copy(bdSrc, dstDir+"/"+id);
                auto borderSize = ele["borderSize"];
                if(borderSize.isNull()){
                    QImage img(bdSrc);
                    borderSize = JArray{img.width(), img.height()};
                }
                layer["border"] = JObj{
                    {"img", id},
                    {"eff", ele["borderEff"]},
                    {"speed", ele["borderSpeed"]},
                    {"img_size", borderSize},
                    {"geometry", JArray{geometry["x"], geometry["y"], geometry["w"], geometry["h"]}},
                    {"rotate", isWin ? 0 : ele["rotate"]}
                };
            }
            layers.append(layer);
        }
    }

    JArray schedules, plans = pageJson["plans"].toArray();
    auto validDate = pageJson["validDate"];
    bool isValid = validDate["isValid"].toBool();
    if(plans.empty()) {
        if(isValid) {
            JObj schedule;
            schedule["dateType"] = "Range";
            schedule["startDate"] = validDate["start"];
            schedule["endDate"] = validDate["end"];
            schedule["timeType"] = "All";
            schedule["filterType"] = "None";
            schedule["weekFilter"] = JArray();
            schedules.append(schedule);
        }
    } else {
        for(auto &plan : plans) {
            JObj schedule;
            if(isValid) {
                schedule["dateType"] = "Range";
                schedule["startDate"] = validDate["start"];
                schedule["endDate"] = validDate["end"];
            } else schedule["dateType"] = "All";
            schedule["timeType"] = "Range";
            schedule["startTime"] = plan["start"];
            schedule["endTime"] = plan["end"];
            auto weekly = plan["weekly"];
            schedule["weekFilter"] = weekly;
            schedule["filterType"] = weekly.toArray().empty() ? "None" : "Week";
            schedules.append(schedule);
        }
    }
    return JObj{
        {"repeatTimes", pageJson["repeat"]},
        {"waitAudio", pageJson["waitAudio"]},
        {"schedules", schedules},
        {"_program", JObj{
            {"name", pageJson["name"].toString()},
            {"layers", layers}
        }}
    };
}

JArray GenTmpThread::genSources(QString type, const JArray &eles) {
    JArray sources;
    auto needType = type.isEmpty();
    for(const auto &ele : eles) {
        JObj source;
        if(needType) type = ele["elementType"].toString();
        if(type=="Text") source = genText(ele, sources);
        else if(type=="Image"||type=="Photo") source = genImage(ele);
        else if(type=="Video"||type=="Movie") {
            //genProg(ele, dstDir, mProgItem);
            auto widget = ele["widget"];
            if(widget.isNull()) widget = ele;
            auto path = widget["path"].toString();
            auto name = widget["file"].toString();
            auto srcFile = path + "/" + name;
            if(! QFileInfo(srcFile).isFile() && ! QFileInfo(srcFile = srcPageDir + "/" + name).isFile()) continue;
            auto id = Tools::fileMd5(srcFile);
            if(id.isEmpty()) continue;
            QFile::copy(srcFile, dstDir+"/"+id);
            source["_type"] = "Video";
            source["id"] = id;
            source["md5"] = id;
            source["name"] = name;
            source["useSW"] = ele["useSW"];
            auto play = ele["play"];
            source["timeSpan"] = play.isNull() ? ele["duration"].toInt() * ele["playTimes"].toInt() : play["playDuration"].toInt() * play["playTimes"].toInt();
        } else if(type=="Gif") source = convertGif(ele);
        else if(type=="DClock") source = convertDClock(ele);
        else if(type=="AClock") source = convertAClock(ele);
        else if(type=="Temp") source = EEnviron::genProg(ele, dstDir, srcPageDir);
        else if(type=="Web") source = convertWeb(ele);
        else if(type=="Timer") source = convertTimer(ele);
        else if(type=="Timer2") {
            source["_type"] = "Countdown";
            source["time"] = ele["targetTime"];
            source["isUp"] = ele["isUp"];
            source["html"] = ele["html"];
            source["backColor"] = ele["backColor"];
        }
        if(! source.empty()) {
            if(source["timeSpan"].isNull()) source["timeSpan"] = ele["duration"];
            source["entryEffect"] = ele["entryEffect"];
            source["exitEffect"] = ele["exitEffect"];
            if(source["entryEffect"].toStr().isEmpty()) source["entryEffect"] = "None"; //兼容旧播放器
            if(source["exitEffect"].toStr().isEmpty()) source["exitEffect"] = "None"; //兼容旧播放器
            source["entryEffectTimeSpan"] = ele["entryDur"];
            source["exitEffectTimeSpan"] = ele["exitDur"];
            if(ele["hasBlink"].toBool()) source["blink"] = ele["blink"];
            else if(ele["hasBreathe"].toBool()) source["breathe"] = ele["blink"];
            sources.append(source);
        }
    }
    return sources;
}

JObj GenTmpThread::genText(const JValue &ele, JArray &sources) {
    auto widget = ele["widget"];
    if(widget.isNull()) widget = ele;
    auto play = ele["play"];
    QString playMode, direction;
    int speed;
    if(play.isNull()) {
        playMode = ele["playMode"].toString();
        direction = ele["direction"].toString();
        speed = ele["speed"].toInt();
    } else {
        QString playModes[]{"Flip", "Scroll", "Static"};
        playMode = playModes[play["style"].toInt()];
        auto rolling = play["rolling"];
        QString ds[]{"left", "top", "right", "bottom"};
        direction = ds[rolling["rollingStyle"].toInt()];
        speed = 1000/rolling["rollingSpeed"].toInt(33);
    }
    auto filenames = widget["files"];
    auto filePrefix = srcPageDir+"/"+widget["idDir"].toString()+"/";

    auto isScroll = playMode=="Scroll";
    if(isScroll) {
        if(filenames.size()>1) {
            JObj source;
            source["_type"] = "Scroll";
            source["direct"] = direction;
            source["speed"] = speed;
            JArray imgs;
            for(auto &filename : filenames) {
                auto file = filePrefix + filename.toString();
                QFile qFile(file);
                if(! qFile.exists()) continue;
                auto id = Tools::fileMd5(file);
                qFile.copy(dstDir+"/"+id);
                imgs.append(id);
            }
            source["imgs"] = imgs;
            return source;
        } else {
            JObj source;
            source["_type"] = "MultiPng";
            source["playMode"] = playMode;
            JArray arrayPics;
            for(auto &filename : filenames) {
                auto file = filePrefix + filename.toString();
                QFile qFile(file);
                if(! qFile.exists()) continue;
                auto id = Tools::fileMd5(file);
                qFile.copy(dstDir+"/"+id);

                JObj arrayPic;
                arrayPic["id"] = id;
                if(direction=="left") arrayPic["effect"] = "right to left";
                else if(direction=="top") arrayPic["effect"] = "bottom to top";
                else if(direction=="right") arrayPic["effect"] = "left to right";
                else if(direction=="bottom") arrayPic["effect"] = "top to bottom";
                arrayPic["scrollSpeed"] = speed;
                arrayPic["picDuration"] = 0;
                arrayPics.append(arrayPic);
            }
            source["arrayPics"] = arrayPics;
            return source;
        }
    } else {
        auto duration = ele["duration"].toInt();
        for(auto &filename : filenames) {
            auto file = filePrefix + filename.toString();
            QFile qFile(file);
            if(! qFile.exists()) continue;
            auto id = Tools::fileMd5(file);
            qFile.copy(dstDir+"/"+id);

            JObj source;
            source["_type"] = "Image";
            source["id"] = id;
            source["md5"] = id;
            source["timeSpan"] = duration;
            source["entryEffect"] = ele["entryEffect"];
            source["exitEffect"] = ele["exitEffect"];
            source["entryEffectTimeSpan"] = ele["entryDur"];
            source["exitEffectTimeSpan"] = ele["exitDur"];
            if(ele["hasBlink"].toBool()) source["blink"] = ele["blink"];
            else if(ele["hasBreathe"].toBool()) source["breathe"] = ele["blink"];
            sources.append(source);
        }
        return JObj();
    }
}

JObj GenTmpThread::genImage(const JValue &ele) {
    auto widget = ele["widget"];
    auto name = widget.isNull() ? ele["name"].toString() : widget["file"].toString();
    auto srcFile = (widget.isNull() ? ele["dir"] : widget["path"]).toString() + "/" + name;
    QFileInfo srcInfo(srcFile);
    JObj source;
    if(! srcInfo.isFile()) return source;
    QImage img(srcFile);
    auto geometry = ele["geometry"];
    auto width = geometry["w"].toInt();
    auto height = geometry["h"].toInt();
    auto direct = ele["direct"].toStr();
    auto speed = ele["speed"].toInt();
    /*if(mProgItem->maxLen) {
        auto scaled = img.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QImage square(gProgItem->isVer ? gProgItem->mWidth*gProgItem->partLens.size() : gProgItem->maxLen, gProgItem->isVer ? gProgItem->maxLen : gProgItem->mHeight*gProgItem->partLens.size(), QImage::Format_ARGB32);
        square.fill(0);
        QPainter painter(&square);
        QPointF pos(x, y);
        auto end = (int)gProgItem->partLens.size();
        if(gProgItem->isVer) {
            painter.drawImage(pos, scaled, QRectF(0, 0, width, gProgItem->partLens[0]-pos.y()));
            for(int i=1; i<end; i++) {
                pos.ry() -= gProgItem->partLens[i-1];
                pos.rx() += gProgItem->mWidth;
                painter.drawImage(pos, scaled, QRectF(0, 0, width, gProgItem->partLens[i]-pos.y()));
            }
        } else {
            painter.drawImage(pos, scaled, QRectF(0, 0, gProgItem->partLens[0]-pos.x(), height));
            for(int i=1; i<end; i++) {
                pos.rx() -= gProgItem->partLens[i-1];
                pos.ry() += gProgItem->mHeight;
                painter.drawImage(pos, scaled, QRectF(0, 0, gProgItem->partLens[i]-pos.x(), height));
            }

        }
        QBuffer buf;
        square.save(&buf, "PNG");
        QCryptographicHash cryptoHash(QCryptographicHash::Md5);
        cryptoHash.addData(buf.data());
        auto md5 = QString::fromLatin1(cryptoHash.result().toHex());
        QFile file(dstDir+"/"+md5);
        if(! file.open(QFile::WriteOnly)) return source;
        file.write(buf.data());
        file.close();
        source["id"] = md5;
        source["md5"] = md5;
    } else */
    auto isScroll = ! direct.isEmpty() && speed > 0;
    QString md5;
    if((isScroll && (img.width() != width || img.height() != height)) || (img.width() > width*2 && img.height() > height*2)) {
        QBuffer buf;
        img.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(&buf, "PNG");
        QCryptographicHash cryptoHash(QCryptographicHash::Md5);
        cryptoHash.addData(buf.data());
        md5 = QString::fromLatin1(cryptoHash.result().toHex());
        QFile file(dstDir+"/"+md5);
        if(! file.open(QFile::WriteOnly)) return source;
        file.write(buf.data());
        file.close();
    } else {
        md5 = Tools::fileMd5(srcFile);
        if(md5.isEmpty()) return source;
        QFile::copy(srcFile, dstDir+"/"+md5);
    }
    if(isScroll) {
        source["_type"] = "Scroll";
        source["direct"] = direct;
        source["speed"] = speed;
        source["imgs"] = JArray{md5};
    } else {
        source["_type"] = "Image";
        source["id"] = md5;
        source["md5"] = md5;
    }
    auto play = ele["play"];
    source["timeSpan"] = play.isNull() ? ele["duration"] : play["playDuration"];
    return source;
}
//转换图片
JObj GenTmpThread::convertGif(const JValue &json) {
    auto widget = json["widget"];
    auto path = widget["path"].toString();
    auto name = widget["file"].toString();
    QString srcFile = path + "/" + name;
    QFileInfo srcInfo(srcFile);
    if(! srcInfo.isFile()) return JObj();
    auto id = Tools::fileMd5(srcFile);
    if(id.isEmpty()) return JObj();
    QFile::copy(srcFile, dstDir+"/"+id);
    JObj oRes;
    oRes["_type"] = "Image";
    oRes["id"] = id;
    oRes["md5"] = id;
    oRes["fileExt"] = srcInfo.suffix().toLower();
    auto play = json["play"];
    oRes["timeSpan"] = (play.isNull() ? json["duration"] : play["playDuration"]).toInt() * play["playTimes"].toInt(1);
    return oRes;
}
JObj GenTmpThread::convertDClock(const JValue &json){
    JObj source;
    source["_type"] = "DigitalClockNew";
    source["name"] = "DigitalClockNew";
    auto widget = json["widget"];
    QFont font;
    QColor color;
    if(widget.isNull()) {
        widget = json;
        source["year"] = json["hasYear"];
        source["month"] = json["hasMonth"];
        source["day"] = json["hasDay"];
        source["hour"] = json["hasHour"];
        source["min"] = json["hasMin"];
        source["sec"] = json["hasSec"];
        source["weekly"] = json["hasWeek"];
        source["fullYear"] = json["isFullYear"];
        source["hour12"] = json["is12Hour"];
        source["AmPm"] = json["hasAmPm"];
        source["multiline"] = json["isMultiline"];
        color = json["color"].toString("#ffff0000");
        font.setFamily(json["font"].toString());
        font.setPixelSize(json["fontSize"].toInt());
        font.setBold(json["fontBold"].toBool());
        font.setItalic(json["fontItalic"].toBool());
        font.setUnderline(json["fontUnderline"].toBool());
    } else {
        source["year"] = widget["year"];
        source["month"] = widget["month"];
        source["day"] = widget["day"];
        source["hour"] = widget["hour"];
        source["min"] = widget["min"];
        source["sec"] = widget["sec"];
        source["weekly"] = widget["weekly"];
        source["fullYear"] = widget["fullYear"];
        source["hour12"] = widget["12Hour"];
        source["AmPm"] = widget["AmPm"];
        source["multiline"] = widget["multiline"];
        auto ft = widget["font"];
        color = Tools::int2Color(ft["color"].toInt());
        font.setFamily(ft["family"].toString());
        font.setPixelSize(ft["size"].toInt());
        font.setBold(ft["bold"].toBool());
        font.setItalic(ft["italics"].toBool());
        font.setUnderline(ft["underline"].toBool());
    }
    source["timeZone"] = widget["timeZone"];
    source["timezone"] = 8;//兼容旧播放器
    source["dateStyle"] = widget["dateStyle"];
    source["timeStyle"] = widget["timeStyle"];

    font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    QFontMetrics metric(font);
    source["spaceWidth"] = metric.horizontalAdvance(" ");
    auto lineHeight = json["isMultiline"].toBool() ? json["lineHeight"].toInt() : 0;
    JArray imgs;
    for(int i=0; i<=9; i++) Tools::saveImg2(dstDir, metric, font, color, imgs, QString::number(i), QString::number(i), lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("MON"), "MON", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("TUE"), "TUE", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("WED"), "WED", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("THU"), "THU", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("FRI"), "FRI", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("SAT"), "SAT", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("SUN"), "SUN", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("AM"), "AM", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, tr("PM"), "PM", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, "年", "YEAR", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, "月", "MONTH", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, "日", "DAY", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, ":", "maohao", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, "/", "xiegang", lineHeight);
    Tools::saveImg2(dstDir, metric, font, color, imgs, "-", "hengxian", lineHeight);
    source["arrayPics"] = imgs;
    return source;
}
JObj GenTmpThread::convertAClock(const JValue &json) {
    auto widget = json["widget"];
    if(widget.isNull()) widget = json;
    auto srcFile = srcPageDir + "/" + widget["selfCreateDialName"].toString();
    if(! QFileInfo::exists(srcFile)) {
        srcFile = srcPageDir + "/" + widget["name"].toString();
        if(! QFileInfo::exists(srcFile)) return JObj();
    }
    QString id = Tools::fileMd5(srcFile);
    QFile(srcFile).copy(dstDir+"/"+id);
    JObj oRes;
    oRes["_type"] = "AnalogClock";
    oRes["id"] = id;
    oRes["md5"] = id;
    oRes["shade"] = 0;//表盘形状
    oRes["opacity"] = 1;//透明度
    oRes["showBg"] = false;//是否显示背景色
    oRes["bgColor"] = 0;
    oRes["showHourScale"] = false;//是否显示时针
    auto color = widget["hourMarkColor"];
    oRes["scaleHourColor"] = color.isStr() ? color : Tools::int2Color(color.toInt()).name();
    color = widget["minMarkColor"];
    oRes["scaleMinColor"] = color.isStr() ? color : Tools::int2Color(color.toInt()).name();
    color = widget["hourHandColor"];
    oRes["pinHourColor"] = color.isStr() ? color : Tools::int2Color(color.toInt()).name();
    color = widget["minHandColor"];
    oRes["pinMinColor"] = color.isStr() ? color : Tools::int2Color(color.toInt()).name();
    color = widget["secHandColor"];
    oRes["pinSecColor"] = color.isStr() ? color : Tools::int2Color(color.toInt()).name();
    oRes["pinHourLen"] = widget["hhLen"].toInt();
    oRes["pinMinLen"] = widget["mhLen"].toInt();
    oRes["pinSecLen"] = widget["shLen"].toInt();
    oRes["pinHourWidth"] = widget["hhWidth"].toInt();
    oRes["pinMinWidth"] = widget["mhWidth"].toInt();
    oRes["pinSecWidth"] = widget["shWidth"].toInt();
    oRes["showMinScale"] = false;
    oRes["scaleStyle"] = 0;
    oRes["showScaleNum"] = false;
    oRes["pinStyle"] = 1;
    oRes["showSecond"] = widget["showSecHand"];
    oRes["timeZone"] = widget["timeZone"];
    return oRes;
}

JObj GenTmpThread::convertWeb(const JValue &res) {
    JObj dst;
    dst["_type"] = "WebURL";
    dst["name"] = "WebURL";
    dst["url"] = res["url"];
    dst["zoom"] = res["zoom"];
    dst["refreshSec"] = res["refreshSec"];
    dst["offX"] = res["offX"];
    dst["offY"] = res["offY"];
    dst["scaleX"] = res["scaleX"].toDouble(100)/100;
    dst["scaleY"] = res["scaleY"].toDouble(100)/100;
    return dst;
}
JObj GenTmpThread::convertTimer(const JValue &json) {
    JObj src;
    src["_type"] = "Timer";
    src["name"] = "Timer";
    src["targetTime"] = json["targetTime"];
    src["isDown"] = json["isDown"];
    src["hasDay"] = json["hasDay"];
    src["hasHour"] = json["hasHour"];
    src["hasMin"] = json["hasMin"];
    src["hasSec"] = json["hasSec"];
    auto isMultiline = json["isMultiline"].toBool();
    src["isMultiline"] = isMultiline;
    auto text = json["text"].toString();
    src["text"] = text;
    QFont font(json["font"].toString());
    font.setPixelSize(json["fontSize"].toInt());
    font.setBold(json["fontBold"].toBool());
    font.setItalic(json["fontItalic"].toBool());
    font.setUnderline(json["fontUnderline"].toBool());
    src["font"] = font.family();
    src["fontSize"] = font.pixelSize();
    src["fontBold"] = font.bold();
    src["fontItalic"] = font.italic();
    src["fontUnderline"] = font.underline();
    auto textColor = json["textColor"].toString();
    src["textColor"] = textColor;
    src["backColor"] = json["backColor"];
    font.setStyleStrategy(gTextAntialiasing ? QFont::PreferAntialias : QFont::NoAntialias);
    QFontMetrics metric(font);
    src["spaceWidth"] = metric.horizontalAdvance(" ");
    QColor color(textColor);
    JObj imgs;
    for(int i=0; i<=9; i++) Tools::saveImg(dstDir, metric, font, color, imgs, QString::number(i), QString::number(i));
    Tools::saveImg(dstDir, metric, font, color, imgs, tr("day"), "day");
    Tools::saveImg(dstDir, metric, font, color, imgs, tr("hour"), "hour");
    Tools::saveImg(dstDir, metric, font, color, imgs, tr("min"), "min");
    Tools::saveImg(dstDir, metric, font, color, imgs, tr("sec"), "sec");
    if(! text.isEmpty()) {
        QSize size;
        if(isMultiline) {
            auto innerW = json["innerW"].toInt();
            auto innerH = json["innerH"].toInt();
            auto rect = metric.boundingRect(0, 0, innerW, innerH, Qt::AlignCenter | Qt::TextWordWrap, text);
            size = {qMin(rect.width(), innerW), qMin(rect.height(), innerH)};
        } else size = {metric.horizontalAdvance(text), metric.lineSpacing()};
        QImage img(size, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        {
            QPainter painter(&img);
            painter.setFont(font);
            painter.setPen(color);
            painter.drawText(QRectF(0, 0, img.width(), img.height()), text, QTextOption(Qt::AlignCenter));
        }
        QByteArray data;
        QBuffer buffer(&data);
        buffer.open(QIODevice::WriteOnly);
        if(img.save(&buffer, "PNG")) {
            QCryptographicHash cryptoHash(QCryptographicHash::Md5);
            cryptoHash.addData(data);
            auto md5 = QString::fromLatin1(cryptoHash.result().toHex());
            QFile file(dstDir+"/"+md5);
            if(file.open(QFile::WriteOnly)) {
                file.write(data);
                file.close();
                imgs.insert("text", md5);
            } else emit onErr("convertTimer file.open false");
        } else emit onErr("convertTimer img.save false");
    }
    src["imgs"] = imgs;
    return src;
}
