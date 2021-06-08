/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.fcitx.littlesun.xml -i widget.h -a keyboardAdaptor
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef KEYBOARDADAPTOR_H
#define KEYBOARDADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "centercontroller.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.fcitx.littlesun.server
 */
class ServerAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.fcitx.littlesun.server")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.fcitx.littlesun.server\">\n"
"    <signal name=\"sendCandidateCharacter\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"str\"/>\n"
"    </signal>\n"
"    <method name=\"initView\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"showView\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"    </method>\n"
"    <method name=\"hideView\">\n"
"      <arg direction=\"out\" type=\"i\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    ServerAdaptor(QObject *parent);
    virtual ~ServerAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    int hideView();
    int initView();
    int showView();
Q_SIGNALS: // SIGNALS
    void sendCandidateCharacter(const QString&);
};

#endif
