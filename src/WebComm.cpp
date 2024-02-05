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
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

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
    
    #ifdef _WIN32
    TCHAR infoBuf[100];
    DWORD bufCharCount = 100;
    if (GetComputerName(infoBuf, &bufCharCount)) {
        char szString[100];
        size_t nNumCharConverted;
        wcstombs_s(&nNumCharConverted, (char*) szString, 100, (wchar_t*) infoBuf, 100);
        greetings << "\n   Host System: " << szString;
    }
    #endif
    
    sendMessage(greetings.str().c_str());
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void Communicator::sendMessage(const std::string message)
{
    if ((conf->getConfigOption("integratedBot").toBool() || !conf->getConfigOption("floodProtection").toBool()) && last != NULL && std::time(nullptr) - last < 3) return;
    std::ostringstream request;
    if (conf->getConfigOption("integratedBot").toBool()) request << "https://telegrambridgebot.julianimhof.de";
    else request << "https://api.telegram.org/bot" << qPrintable(conf->getConfigOption("botToken").toString().trimmed());
    std::string messageEscaped = ReplaceAll(message, "<", "&lt;");
    messageEscaped = ReplaceAll(messageEscaped, ">", "&gt;");
    request << "/sendMessage?parse_mode=HTML&chat_id=" << qPrintable(conf->getConfigOption("chatID").toString().trimmed()) << "&text=" << qPrintable(QUrl::toPercentEncoding(messageEscaped.c_str(), "", "&"));
    ts3Functions->logMessage(request.str().c_str(), LogLevel_DEBUG, "WebComm", 0);
    this->startRequest(request.str());
    last = std::time(nullptr);
}

void Communicator::startRequest(const std::string requestedUrl)
{
    manager = new QNetworkAccessManager();
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(requestedUrl.c_str())));
}
