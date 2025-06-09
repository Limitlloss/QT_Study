#include "qjson.h"
#include "qdebug.h"

QDebug operator<<(QDebug debug, const JValue &val) {
    auto old = debug.autoInsertSpaces();
    debug.noquote().nospace() << JToBytes(val, "\t");
    debug.setAutoInsertSpaces(old);
    return debug;
}

JValue JParser::readValue() {
    if(ch=='{') {
        JObj obj;
        while(true) {
            do skipSpace(); //ch有三种可能
            while(ch==',');
            if(ch=='}') return obj;
            QString key;
            if(ch=='"') key = readStr();
            else if(ch!='n' || readOne()!='u' || readOne()!='l' || readOne()!='l') throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting double-quote to start field name or null";
            skipSpace();
            if(ch!=':') throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting a colon to separate field name and value";
            skipSpace();
            obj.insert(key, readValue());
            skipSpace(); //ch有两种可能
            if(ch=='}') return obj;
            if(ch!=',') throw QString("Unexpected char ")+(char)ch+"' (code "+QString::number(ch)+"): was expecting } to end Object or comma to separate Object entries";
        }
    }
    if(ch=='[')  {
        JArray list;
        while(true) {
            do skipSpace(); //ch有三种可能
            while(ch==',');
            if(ch==']') return list;
            list->push_back(readValue());
            skipSpace(); //ch有两种可能
            if(ch==']') return list;
            if(ch!=',') throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting ] to end Array or comma to separate Array entries";
        }
    }
    if(ch=='"') return readStr();
    if((ch>='0' && ch<='9') || ch=='-') {
        QByteArray buf;
        buf += ch;
        bool isInt = true;
        while(readOne()) {
            if((ch>'*' && ch<':' && ch!=',' && ch!='/') || ch=='e' || ch=='E') {
                buf.append(ch);
                if(isInt && ch=='.') isInt = false;
            } else {
                bk = ch;
                break;
            }
        }
        bool ok;
        if(isInt) {
            auto num = buf.toLongLong(&ok);
            if(! ok) throw "Illegal number: "+buf;
            return num;
        } else {
            auto num = buf.toDouble(&ok);
            if(! ok) throw "Illegal number: "+buf;
            return num;
        }
    }
    if(ch=='n') {
        if(readOne()=='u' && readOne()=='l' && readOne()=='l') return JValue();
        else throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting null";
    }
    if(ch=='t') {
        if(readOne()=='r' && readOne()=='u' && readOne()=='e') return true;
        else throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting true";
    }
    if(ch=='f') {
        if(readOne()=='a' && readOne()=='l' && readOne()=='s' && readOne()=='e') return false;
        else throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting false";
    }
    throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+"): was expecting {}, [], \"string\", number, null, true or false";
}
inline int toHex(unsigned char ch) {
    if(ch<'0') return -1;
    if(ch<='9') return ch-'0';
    if(ch<'A') return -1;
    if(ch<='F') return ch-55;
    if(ch<'a') return -1;
    if(ch<='f') return ch-87;
    return -1;
}
QString JParser::readStr() {
    QByteArray buf;
    while(readOne() != '"') {
        if(ch==0) throw "Unexpected end-of-input: was expecting closing quote for string";
        if(ch=='\\') {
            if(readOne()==0) throw "Unexpected end-of-input in char escape sequence";
            if(ch=='"' || ch=='\\' || ch=='/') buf.append(ch);
            else if(ch=='n') buf.append('\n');
            else if(ch=='r') buf.append('\r');
            else if(ch=='t') buf.append('\t');
            else if(ch=='f') buf.append('\f');
            else if(ch=='b') buf.append('\b');
            else if(ch=='u') {
                uint hex = 0;
                for(int i=3; i>=0; i--) {
                    if(readOne()==0) throw "Unexpected end-of-input in char escape sequence";
                    auto h = toHex(ch);
                    if(h==-1) throw "Illegal hex-digits in char escape sequence: \\u"+(hex==0?"":QString::number(hex, 16)+(char)ch);
                    hex |= h<<(i<<2);
                }
                buf.append(QString(QChar(hex)).toUtf8());
            } else throw QString("Unrecognized char-escape ")+(char)ch+" (code "+QString::number(ch)+") after '\\'";
        } else buf.append(ch);
    }
    return QString::fromUtf8(buf);
}
void JParser::skipSpace() {
    if(bk) {
        bk = 0;
        if(ch>=33) return;
    }
    do {
        in >> ch;
        if(ch==0) throw "Unexpected end-of-input";
    } while(ch < 33); // || ch==65279
    if(ch=='/') {
        in >> ch;
        if(ch=='/') {  //skipComment
            do {
                in >> ch;
                if(ch==0) throw "Unexpected end-of-input";
            } while(ch!='\n' && ch!='\r');
            skipSpace();
            return;
        }
        if(ch=='*') {  //skipMultiComment
            int last;
            do {
                last = ch;
                in >> ch;
                if(ch==0) throw "Unexpected end-of-input";
            } while(ch!='/' || last!='*');
            skipSpace();
            return;
        }
        throw QString("Unexpected char ")+(char)ch+" (code "+QString::number(ch)+")";
    }
}

void JOut::write(const JValue &value) {
    if(value.type==JValue::Null) out << "null";
    else if(value.type==JValue::Str) writeStr(value.toStr());
    else if(value.type==JValue::Obj) writeMap(value.toObj());
    else if(value.type==JValue::Array) writeList(value.toArray());
    else out << value.toStr();
    out.flush();
}
void JOut::writeStr(const QString &str) {
    out << '"';
    QChar ch;
    for(int i=0; i<str.length(); i++) {
        ch = str[i];
        if(ch=='"'||ch=='\\') out<<'\\'<<ch;
        else if(ch < ' ') {
            if(ch=='\n') out<<"\\n";
            else if(ch=='\r') out<<"\\r";
            else if(ch=='\t') out<<"\\t";
            else if(ch=='\f') out<<"\\f";
            else if(ch=='\b') out<<"\\b";
            else {
                out<<"\\u00";
                auto aa = QString::number(ch.unicode(), 16);
                if(aa.size()==1) out<<'0';
                out<<aa;
            }
        } else out << ch;
    }
    out << '"';
}
void JOut::writeMap(const JObj &map) {
    out << '{';
    if(! map.empty()) {
        if(indent.size()) {
            out << '\n';
            cnt++;
            for(int c=0; c<cnt; c++) out << indent;
        }
        bool addSep = false;
        for(const auto &pair : map) {
            if(addSep) {
                out << ',';
                if(indent.size()) {
                    out << '\n';
                    for(int c=0; c<cnt; c++) out << indent;
                }
            }
            else addSep = true;
            out << '"' << pair.first << "\":";
            if(indent.size()) out << ' ';
            write(pair.second);
        }
        if(indent.size()) {
            out << '\n';
            cnt--;
            for(int c=0; c<cnt; c++) out << indent;
        }
    }
    out << '}';
}
void JOut::writeList(const JArray &vals) {
    out << '[';
    auto iter = vals.begin();
    auto end = vals.end();
    if(iter!=end) write(*iter++);
    while(iter!=end) {
        out << ',';
        if(indent.size()) out << ' ';
        write(*iter++);
    }
    out << ']';
}
