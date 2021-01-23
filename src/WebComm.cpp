#pragma comment (lib, "Qt5Core")

#include "WebComm.h"
#include <iostream>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QEventLoop>
#include <QtCore/QTextCodec>
#include "definitions.h"
#include "teamspeak/public_errors.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <Windows.h>

Communicator::Communicator(QObject* parent) :
    QObject(parent)
{
}

Communicator::~Communicator()
{
    delete messages;
}

void Communicator::setConfigPtr(config* con)
{
    conf = con;
}

void Communicator::setFunctionPtr(TS3Functions* functions)
{
    ts3Functions = functions;
}

void Communicator::sendGreetings() {
    std::ostringstream greetings;
    greetings << "Telegram Watchdog\nwas succesfully initiated!";
    TCHAR infoBuf[100];
    DWORD bufCharCount = 100;
    if (GetComputerName(infoBuf, &bufCharCount)) {
        char szString[100];
        size_t nNumCharConverted;
        wcstombs_s(&nNumCharConverted, szString, 100, infoBuf, 100);
        greetings << "\n   Host System: " << szString;
    }
    sendMessage(greetings.str().c_str());
}

void Communicator::sendMessage(const char* message)
{
    if ((conf->getConfigOption("integratedBot").toBool() || !conf->getConfigOption("floodProtection").toBool()) && last != NULL && std::time(nullptr) - last < 3) return;
    std::ostringstream request;
    if (conf->getConfigOption("integratedBot").toBool()) request << "https://telegrambridgebot.julianimhof.de";
    else request << "https://api.telegram.org/bot" << qPrintable(conf->getConfigOption("botToken").toString().trimmed());

    request << "/sendMessage?parse_mode=HTML&chat_id=" << qPrintable(conf->getConfigOption("chatID").toString().trimmed()) << "&text=" << qPrintable(QUrl::toPercentEncoding(message, "", "lgtamp&"));
    this->startRequest(request.str().c_str());
    last = std::time(nullptr);
}

void Communicator::checkForUpdate(update* upd) {
    manager = new QNetworkAccessManager();
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(UPDATE_URL)));
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() != QNetworkReply::NoError) { ts3Functions->logMessage("TWatchdog: Unable to pull version information.", LogLevel_WARNING, "PLUGIN", NULL); return; }
        std::ostringstream response;
        response << reply->readAll().constData();
        if (!QString::compare(QString(response.str().c_str()).trimmed(), PLUGIN_VERSION)) { ts3Functions->logMessage("TWatchdog: No Update found!", LogLevel_INFO, "PLUGIN", NULL); return; }
        ts3Functions->logMessage("TWatchdog: New version found! Download at https://julianimhof.de/files/_TelegramWatchdog.ts3plugin", LogLevel_INFO, "PLUGIN", NULL);
        upd->setText(PLUGIN_VERSION, response.str().c_str());
        upd->show();
    });
}

void Communicator::startRequest(const char* requestedUrl)
{
    manager = new QNetworkAccessManager();
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(requestedUrl)));
}
