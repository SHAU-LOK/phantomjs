/*
  This file is part of the chromessJS project from Ofi Labs.

  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>
  Copyright (C) 2011 Ivan De Marino <ivan.de.marino@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "chromess.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QtWebKitWidgets/QWebPage>
#include <QDebug>
#include <QMetaObject>
#include <QMetaProperty>
#include <QStandardPaths>

#include "consts.h"
#include "terminal.h"
#include "utils.h"
#include "webpage.h"
#include "webserver.h"
#include "repl.h"
#include "system.h"
#include "callback.h"
#include "cookiejar.h"
#include "childprocess.h"

static chromess* chromessInstance = NULL;

// private:
chromess::chromess(QObject* parent)
    : QObject(parent)
    , m_terminated(false)
    , m_returnValue(0)
    , m_filesystem(0)
    , m_system(0)
    , m_childprocess(0)
{
    QStringList args = QApplication::arguments();

    // Prepare the configuration object based on the command line arguments.
    // Because this object will be used by other classes, it needs to be ready ASAP.
    m_config.init(&args);
    // Apply debug configuration as early as possible
    Utils::printDebugMessages = m_config.printDebugMessages();
}

void chromess::init()
{
    if (m_config.helpFlag()) {
        Terminal::instance()->cout(QString("%1").arg(m_config.helpText()));
        Terminal::instance()->cout("Any of the options that accept boolean values ('true'/'false') can also accept 'yes'/'no'.");
        Terminal::instance()->cout("");
        Terminal::instance()->cout("Without any argument, chromessJS will launch in interactive mode (REPL).");
        Terminal::instance()->cout("");
        Terminal::instance()->cout("Documentation can be found at the web site, http://chromessjs.org.");
        Terminal::instance()->cout("");
        m_terminated = true;
        return;
    }

    if (m_config.versionFlag()) {
        m_terminated = true;
        Terminal::instance()->cout(QString("%1").arg(chromessJS_VERSION_STRING));
        return;
    }

    if (!m_config.unknownOption().isEmpty()) {
        Terminal::instance()->cerr(m_config.unknownOption());
        m_terminated = true;
        return;
    }

    // Initialize the CookieJar
    m_defaultCookieJar = new CookieJar(m_config.cookiesFile());

    QWebSettings::setOfflineWebApplicationCachePath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (m_config.offlineStoragePath().isEmpty()) {
        QWebSettings::setOfflineStoragePath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    } else {
        QWebSettings::setOfflineStoragePath(m_config.offlineStoragePath());
    }
    if (m_config.offlineStorageDefaultQuota() > 0) {
        QWebSettings::setOfflineStorageDefaultQuota(m_config.offlineStorageDefaultQuota());
    }

    m_page = new WebPage(this, QUrl::fromLocalFile(m_config.scriptFile()));
    m_page->setCookieJar(m_defaultCookieJar);
    m_pages.append(m_page);

    // Set up proxy if required
    QString proxyType = m_config.proxyType();
    if (proxyType != "none") {
        setProxy(m_config.proxyHost(), m_config.proxyPort(), proxyType, m_config.proxyAuthUser(), m_config.proxyAuthPass());
    }

    // Set output encoding
    Terminal::instance()->setEncoding(m_config.outputEncoding());

    // Set script file encoding
    m_scriptFileEnc.setEncoding(m_config.scriptEncoding());

    connect(m_page, SIGNAL(javaScriptConsoleMessageSent(QString)),
            SLOT(printConsoleMessage(QString)));
    connect(m_page, SIGNAL(initialized()),
            SLOT(onInitialized()));

    m_defaultPageSettings[PAGE_SETTINGS_LOAD_IMAGES] = QVariant::fromValue(m_config.autoLoadImages());
    m_defaultPageSettings[PAGE_SETTINGS_JS_ENABLED] = QVariant::fromValue(true);
    m_defaultPageSettings[PAGE_SETTINGS_XSS_AUDITING] = QVariant::fromValue(false);
    m_defaultPageSettings[PAGE_SETTINGS_USER_AGENT] = QVariant::fromValue(m_page->userAgent());
    m_defaultPageSettings[PAGE_SETTINGS_LOCAL_ACCESS_REMOTE] = QVariant::fromValue(m_config.localToRemoteUrlAccessEnabled());
    m_defaultPageSettings[PAGE_SETTINGS_WEB_SECURITY_ENABLED] = QVariant::fromValue(m_config.webSecurityEnabled());
    m_defaultPageSettings[PAGE_SETTINGS_JS_CAN_OPEN_WINDOWS] = QVariant::fromValue(m_config.javascriptCanOpenWindows());
    m_defaultPageSettings[PAGE_SETTINGS_JS_CAN_CLOSE_WINDOWS] = QVariant::fromValue(m_config.javascriptCanCloseWindows());
    m_page->applySettings(m_defaultPageSettings);

    setLibraryPath(QFileInfo(m_config.scriptFile()).dir().absolutePath());
}

// public:
chromess* chromess::instance()
{
    if (NULL == chromessInstance) {
        chromessInstance = new chromess();
        chromessInstance->init();
    }
    return chromessInstance;
}

chromess::~chromess()
{
    // Nothing to do: cleanup is handled by QObject relationships
}

QVariantMap chromess::defaultPageSettings() const
{
    return m_defaultPageSettings;
}

QString chromess::outputEncoding() const
{
    return Terminal::instance()->getEncoding();
}

void chromess::setOutputEncoding(const QString& encoding)
{
    Terminal::instance()->setEncoding(encoding);
}

bool chromess::execute()
{
    if (m_terminated) {
        return false;
    }

#ifndef QT_NO_DEBUG_OUTPUT
    qDebug() << "chromess - execute: Configuration";
    const QMetaObject* configMetaObj = m_config.metaObject();
    for (int i = 0, ilen = configMetaObj->propertyCount(); i < ilen; ++i) {
        qDebug() << "    " << i << configMetaObj->property(i).name() << ":" << m_config.property(configMetaObj->property(i).name()).toString();
    }

    qDebug() << "chromess - execute: Script & Arguments";
    qDebug() << "    " << "script:" << m_config.scriptFile();
    QStringList args = m_config.scriptArgs();
    for (int i = 0, ilen = args.length(); i < ilen; ++i) {
        qDebug() << "    " << i << "arg:" << args.at(i);
    }
#endif

    if (m_config.isWebdriverMode()) {                                   // Remote WebDriver mode requested
        qDebug() << "chromess - execute: Starting Remote WebDriver mode";

        if (!Utils::injectJsInFrame(":/ghostdriver/main.js", QString(), m_scriptFileEnc, QDir::currentPath(), m_page->mainFrame(), true)) {
            m_returnValue = -1;
            return false;
        }
    } else if (m_config.scriptFile().isEmpty()) {                       // REPL mode requested
        qDebug() << "chromess - execute: Starting REPL mode";

        // REPL is only valid for javascript
        const QString& scriptLanguage = m_config.scriptLanguage();
        if (scriptLanguage != "javascript" && !scriptLanguage.isNull()) {
            QString errMessage = QString("Unsupported language: %1").arg(scriptLanguage);
            Terminal::instance()->cerr(errMessage);
            qWarning("%s", qPrintable(errMessage));
            return false;
        }

        // Create the REPL: it will launch itself, no need to store this variable.
        REPL::getInstance(m_page->mainFrame(), this);
    } else {                                                            // Load the User Script
        qDebug() << "chromess - execute: Starting normal mode";

        if (m_config.debug()) {
            // Debug enabled
            int originalPort = m_config.remoteDebugPort();
            m_config.setRemoteDebugPort(m_page->showInspector(m_config.remoteDebugPort()));
            if (m_config.remoteDebugPort() == 0) {
                qWarning() << "Can't bind remote debugging server to the port" << originalPort;
            }
            if (!Utils::loadJSForDebug(m_config.scriptFile(), m_config.scriptLanguage(), m_scriptFileEnc, QDir::currentPath(), m_page->mainFrame(), m_config.remoteDebugAutorun())) {
                m_returnValue = -1;
                return false;
            }
        } else {
            if (!Utils::injectJsInFrame(m_config.scriptFile(), m_config.scriptLanguage(), m_scriptFileEnc, QDir::currentPath(), m_page->mainFrame(), true)) {
                m_returnValue = -1;
                return false;
            }
        }
    }

    return !m_terminated;
}

int chromess::returnValue() const
{
    return m_returnValue;
}

QString chromess::libraryPath() const
{
    return m_page->libraryPath();
}

void chromess::setLibraryPath(const QString& libraryPath)
{
    m_page->setLibraryPath(libraryPath);
}

QVariantMap chromess::version() const
{
    QVariantMap result;
    result["major"] = chromessJS_VERSION_MAJOR;
    result["minor"] = chromessJS_VERSION_MINOR;
    result["patch"] = chromessJS_VERSION_PATCH;
    return result;
}

QObject* chromess::page() const
{
    return m_page;
}

Config* chromess::config()
{
    return &m_config;
}

bool chromess::printDebugMessages() const
{
    return m_config.printDebugMessages();
}

bool chromess::areCookiesEnabled() const
{
    return m_defaultCookieJar->isEnabled();
}

void chromess::setCookiesEnabled(const bool value)
{
    if (value) {
        m_defaultCookieJar->enable();
    } else {
        m_defaultCookieJar->disable();
    }
}

bool chromess::webdriverMode() const
{
    return m_config.isWebdriverMode();
}

// public slots:
QObject* chromess::createCookieJar(const QString& filePath)
{
    return new CookieJar(filePath, this);
}

QObject* chromess::createWebPage()
{
    WebPage* page = new WebPage(this);
    page->setCookieJar(m_defaultCookieJar);

    // Store pointer to the page for later cleanup
    m_pages.append(page);
    // Apply default settings to the page
    page->applySettings(m_defaultPageSettings);

    // Show web-inspector if in debug mode
    if (m_config.debug()) {
        page->showInspector(m_config.remoteDebugPort());
    }

    return page;
}

QObject* chromess::createWebServer()
{
    WebServer* server = new WebServer(this);
    m_servers.append(server);
    return server;
}

QObject* chromess::createFilesystem()
{
    if (!m_filesystem) {
        m_filesystem = new FileSystem(this);
    }

    return m_filesystem;
}

QObject* chromess::createSystem()
{
    if (!m_system) {
        m_system = new System(this);

        QStringList systemArgs;
        systemArgs += m_config.scriptFile();
        systemArgs += m_config.scriptArgs();
        m_system->setArgs(systemArgs);
    }

    return m_system;
}

QObject* chromess::_createChildProcess()
{
    if (!m_childprocess) {
        m_childprocess = new ChildProcess(this);
    }

    return m_childprocess;
}

QObject* chromess::createCallback()
{
    return new Callback(this);
}

void chromess::loadModule(const QString& moduleSource, const QString& filename)
{
    if (m_terminated) {
        return;
    }

    QString scriptSource =
        "(function(require, exports, module) {\n" +
        moduleSource +
        "\n}.call({}," +
        "require.cache['" + filename + "']._getRequire()," +
        "require.cache['" + filename + "'].exports," +
        "require.cache['" + filename + "']" +
        "));";
    m_page->mainFrame()->evaluateJavaScript(scriptSource, QString(JAVASCRIPT_SOURCE_PLATFORM_URL).arg(QFileInfo(filename).fileName()));
}

bool chromess::injectJs(const QString& jsFilePath)
{
    QString pre = "";
    qDebug() << "chromess - injectJs:" << jsFilePath;

    // If in Remote Webdriver Mode, we need to manipulate the PATH, to point it to a resource in `ghostdriver.qrc`
    if (webdriverMode()) {
        pre = ":/ghostdriver/";
        qDebug() << "chromess - injectJs: prepending" << pre;
    }

    if (m_terminated) {
        return false;
    }

    return Utils::injectJsInFrame(pre + jsFilePath, libraryPath(), m_page->mainFrame());
}

void chromess::setProxy(const QString& ip, const qint64& port, const QString& proxyType, const QString& user, const QString& password)
{
    qDebug() << "Set " << proxyType << " proxy to: " << ip << ":" << port;
    if (ip.isEmpty()) {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    } else {
        QNetworkProxy::ProxyType networkProxyType = QNetworkProxy::HttpProxy;

        if (proxyType == "socks5") {
            networkProxyType = QNetworkProxy::Socks5Proxy;
        }
        // Checking for passed proxy user and password
        if (!user.isEmpty() && !password.isEmpty()) {
            QNetworkProxy proxy(networkProxyType, ip, port, user, password);
            QNetworkProxy::setApplicationProxy(proxy);
        } else {
            QNetworkProxy proxy(networkProxyType, ip, port);
            QNetworkProxy::setApplicationProxy(proxy);
        }
    }
}

QString chromess::proxy()
{
    QNetworkProxy proxy = QNetworkProxy::applicationProxy();
    if (proxy.hostName().isEmpty()) {
        return NULL;
    }
    return proxy.hostName() + ":" + QString::number(proxy.port());
}

int chromess::remoteDebugPort() const
{
    return m_config.remoteDebugPort();
}

void chromess::exit(int code)
{
    if (m_config.debug()) {
        Terminal::instance()->cout("chromess::exit() called but not quitting in debug mode.");
    } else {
        doExit(code);
    }
}

void chromess::debugExit(int code)
{
    doExit(code);
}

QString chromess::resolveRelativeUrl(QString url, QString base)
{
    QUrl u = QUrl::fromEncoded(url.toLatin1());
    QUrl b = QUrl::fromEncoded(base.toLatin1());

    return b.resolved(u).toEncoded();
}

QString chromess::fullyDecodeUrl(QString url)
{
    return QUrl::fromEncoded(url.toLatin1()).toDisplayString();
}

// private slots:
void chromess::printConsoleMessage(const QString& message)
{
    Terminal::instance()->cout(message);
}

void chromess::onInitialized()
{
    // Add 'chromess' object to the global scope
    m_page->mainFrame()->addToJavaScriptWindowObject("chromess", this);

    // Bootstrap the chromessJS scope
    m_page->mainFrame()->evaluateJavaScript(
        Utils::readResourceFileUtf8(":/bootstrap.js"),
        QString(JAVASCRIPT_SOURCE_PLATFORM_URL).arg("bootstrap.js")
    );
}

bool chromess::setCookies(const QVariantList& cookies)
{
    // Delete all the cookies from the CookieJar
    m_defaultCookieJar->clearCookies();
    // Add a new set of cookies
    return m_defaultCookieJar->addCookiesFromMap(cookies);
}

QVariantList chromess::cookies() const
{
    // Return all the Cookies in the CookieJar, as a list of Maps (aka JSON in JS space)
    return m_defaultCookieJar->cookiesToMap();
}

bool chromess::addCookie(const QVariantMap& cookie)
{
    return m_defaultCookieJar->addCookieFromMap(cookie);
}

bool chromess::deleteCookie(const QString& cookieName)
{
    if (!cookieName.isEmpty()) {
        return m_defaultCookieJar->deleteCookie(cookieName);
    }
    return false;
}

void chromess::clearCookies()
{
    m_defaultCookieJar->clearCookies();
}


// private:
void chromess::doExit(int code)
{
    emit aboutToExit(code);
    m_terminated = true;
    m_returnValue = code;

    // Iterate in reverse order so the first page is the last one scheduled for deletion.
    // The first page is the root object, which will be invalidated when it is deleted.
    // This causes an assertion to go off in BridgeJSC.cpp Instance::createRuntimeObject.
    QListIterator<QPointer<WebPage> > i(m_pages);
    i.toBack();
    while (i.hasPrevious()) {
        const QPointer<WebPage> page = i.previous();

        if (!page) {
            continue;
        }

        // stop processing of JavaScript code by loading a blank page
        page->mainFrame()->setUrl(QUrl(QStringLiteral("about:blank")));
        // delay deletion into the event loop, direct deletion can trigger crashes
        page->deleteLater();
    }
    m_pages.clear();
    m_page = 0;
    QApplication::instance()->exit(code);
}
