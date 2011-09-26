/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -c MeecastIf -a dbusadaptor.h:dbusadaptor.cpp com.meecast.applet.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef DBUSADAPTOR_H_1317068268
#define DBUSADAPTOR_H_1317068268

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface com.meecast.applet
 */
class MeecastIf: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.meecast.applet")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.meecast.applet\">\n"
"    <method name=\"SetCurrentData\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"station\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"temperature\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"icon\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    MeecastIf(QObject *parent);
    virtual ~MeecastIf();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void SetCurrentData(const QString &station, const QString &temperature, const QString &icon);
Q_SIGNALS: // SIGNALS
};

#endif
