#ifndef QJSON_H
#define QJSON_H

#include "cpp.h"
#include "QtCore/qhashfunctions.h"
#include <QDataStream>
#include <QTextStream>

class JValue;
using JObj = LinkedMap<QString, JValue>;
using JArray = Vector<JValue>;

QDebug operator<<(QDebug debug, const JValue &val);

class JValue {
public:
    int64_t data = 0;
    enum Type {
        Null, Long, Ulong, Double, Bool, Obj, Array, Str
    };
    Type type = Null;

    JValue(Type = Null) {}
    JValue(bool b) : type(Bool) {data = b;}
    JValue(int n) : type(Long) {data = n;}
    JValue(qint64 n) : type(Long) {data = n;}
    JValue(quint64 n) : type(Ulong) {*(quint64*) &data = n;}
    JValue(double d) : type(Double) {*(double*) &data = d;}
    JValue(const JObj &o) : type(Obj) {new (&data) JObj(o);}
    JValue(const JArray &a) : type(Array) {new (&data) JArray(a);}
    JValue(JObj &&o) : type(Obj) {new (&data) JObj(std::move(o));}
    JValue(JArray &&a) : type(Array) {new (&data) JArray(std::move(a));}
    JValue(const QString &s) : type(Str) {*(SharedData<QString>**) &data = new SharedData<QString>{1, s};}
    JValue(QString &&s) : type(Str) {*(SharedData<QString>**) &data = new SharedData<QString>{1, std::move(s)};}
    JValue(const char *s) : JValue(QString::fromUtf8(s)) {}
    ~JValue() {
        if(type < Obj) return;
        else if(type==Obj) ((JObj*) &data)->~JObj();
        else if(type==Array) ((JArray*) &data)->~JArray();
        else if(type==Str) {
            auto ptr = *(SharedData<QString>**) &data;
            if(ptr->cnt > 1) ptr->cnt--;
            else delete ptr;
        }
    }

    JValue(const JValue &other) {
        type = other.type;
        if(type==Obj) new (&data) JObj(*(JObj*) &other.data);
        else if(type==Array) new (&data) JArray(*(JArray*) &other.data);
        else {
            data = other.data;
            if(type==Str) (*(SharedData<QString>**) &data)->cnt++;
        }
    }
    JValue &operator=(const JValue &other) {
        this->~JValue();
        new (this) JValue(other);
        return *this;
    }
    JValue(JValue &&other) noexcept : data(other.data), type(other.type) {
        other.data = 0;
        other.type = Null;
    }
    JValue &operator=(JValue &&other) noexcept {
        std::swap(data, other.data);
        std::swap(type, other.type);
        return *this;
    }

    bool isNull() const {return type==Null;}
    bool isStr() const {return type==Str;}
    bool isObj() const {return type==Obj;}
    bool isArray() const {return type==Array;}

    bool toBool(bool def = false) const {
        return type==Null ? def : data;
    }
    int toInt(int def = 0) const {
        return toLong(def);
    }
    qint64 toLong(qint64 def = 0) const {
        if(type==Long || type==Bool) return data;
        if(type==Double) return *(double*) &data;
        if(type==Ulong) return *(quint64*) &data;
        return def;
    }
    quint64 toULong(quint64 def = 0) const {
        if(type==Ulong) return *(quint64*) &data;
        if(type==Long || type==Bool) return data;
        if(type==Double) return *(double*) &data;
        return def;
    }
    double toDouble(double def = 0) const {
        if(type==Double) return *(double*) &data;
        if(type==Long || type==Bool) return data;
        if(type==Ulong) return *(quint64*) &data;
        return def;
    }
    QString toStr(const QString &def = "") const {
        if(type==Str) return (*(SharedData<QString>**) &data)->data;
        if(type==Long) return QString::number(data);
        if(type==Double) return QString::number(*(double*) &data);
        if(type==Bool) return data ? "true" : "false";
        if(type==Ulong) return QString::number(*(quint64*) &data);
        return def;
    }
    QString toString(const QString &def = "") const {
        return toStr(def);
    }
    JObj toObj() const {
        if(type==Obj) return *(JObj*) &data;
        return JObj();
    }
    JArray toArray() const {
        if(type==Array) return *(JArray*) &data;
        return JArray();
    }

    const JValue operator[](const QString &key) const {
        return type==Obj ? (*(const JObj*) &data)[key] : JValue();
    }
    const JValue operator[](uint64_t i) const {
        return type==Array ? (*(const JArray*) &data)[i] : JValue();
    }

    JValue &ref(const QString &key) {
        return (*(JObj*) &data)[key];
    }
    JValue &ref(uint64_t i) {
        return (*(JArray*) &data)[i];
    }

    JArray::iterator begin() const noexcept {
        return type==Array ? ((const JArray*) &data)->begin() : JArray::iterator();
    }
    JArray::iterator end() const noexcept {
        return type==Array ? ((const JArray*) &data)->end() : JArray::iterator();
    }
    bool empty() const noexcept {
        if(type==Array) return ((const JArray*) &data)->empty();
        else if(type==Obj) return ((const JObj*) &data)->empty();
        return 0;
    }
    size_t size() const noexcept {
        if(type==Array) return ((const JArray*) &data)->size();
        else if(type==Obj) return ((const JObj*) &data)->size();
        return 0;
    }

    bool operator==(const JValue &other) const {
        if(type==other.type) {
            if(data==other.data) return true;
            if(type==Null) return true;
            if(type<=Double) return false;
            if(type==Bool) return ((bool)data)==(bool)other.data;
            if(type==Str) return (*(SharedData<QString>**) &data)->data==(*(SharedData<QString>**) &other.data)->data;
            if(type==Obj) return *(JObj*) &data == *(JObj*) &other.data;
            if(type==Array) return *(JArray*) &data == *(JArray*) &other.data;
        } else {
            if(type>Double || other.type>Double || type==Null || other.type==Null) return false;
            if(type==Double || other.type==Double) return toDouble()==other.toDouble();
            if(type==Long) return data==other.toLong();
            else return toLong()==other.data;
        }
        return false;
    }
    bool operator!=(const JValue &other) const {
        return ! (*this==other);
    }
private:
    JValue(const void *) = delete; // avoid implicit conversions from char * to bool
};

class JParser {
public:
    JParser(QDataStream &in) : in(in) {}

    JValue read() {
        in >> ch;
        if(ch==0) throw "Unexpected end-of-input";
        if(ch==0xEF && (readOne()!=0xBB || readOne()!=0xBF)) throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting an UTF-8 BOM";
        if(ch<33) skipSpace();
        return readValue();
    }
protected:
    JValue readValue();
    QString readStr();
    void skipSpace();
    unsigned char readOne() {
        in >> ch;
        return ch;
    }

    QDataStream &in;
    unsigned char ch{0}, bk{0};
};

inline JValue JFrom(const QByteArray &json, QString *err = 0) {
    QDataStream in(json);
    try {
        return JParser(in).read();
    } catch (QString anerr) {
        if(err) *err = anerr;
    } catch (const char *anerr) {
        if(err) *err = anerr;
    } catch (...) {
        if(err) *err = "unknow error";
    }
    return JValue();
}
inline JValue JFrom(QIODevice *device, QString *err = 0) {
    QDataStream in(device);
    try {
        return JParser(in).read();
    } catch (QString anerr) {
        if(err) *err = anerr;
    } catch (const char *anerr) {
        if(err) *err = anerr;
    } catch (...) {
        if(err) *err = "unknow error";
    }
    return JValue();
}

class JOut {
public:
    JOut(QTextStream &out, QString indent = "") : out(out), indent(indent) {
#if(QT_VERSION_MAJOR < 6)
    out.setCodec("UTF-8");
#endif
    }

    void write(const JValue &value);
    void writeStr(const QString &str);
    void writeMap(const JObj &map);
    void writeList(const JArray &objs);
protected:
    QTextStream &out;
    QString indent;
    int cnt{0};
};

inline QString JToStr(const JValue &value, QString indent = "") {
    QString json;
    QTextStream out(&json);
    JOut(out, indent).write(value);
    return json;
}
inline QByteArray JToBytes(const JValue &value, QString indent = "") {
    QByteArray json;
    QTextStream out(&json);
    JOut(out, indent).write(value);
    return json;
}
inline QTextStream::Status JWrite(const JValue &value, QIODevice *device, QString indent = "") {
    QTextStream out(device);
    JOut(out, indent).write(value);
    return out.status();
}

#endif // QJSON_H
