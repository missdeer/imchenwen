#include "settings.h"
#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtWidgets/QtWidgets>
#include <QtWebEngineWidgets/QtWebEngineWidgets>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(setHomeToCurrentPageButton, SIGNAL(clicked()), this, SLOT(setHomeToCurrentPage()));
    connect(standardFontButton, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(fixedFontButton, SIGNAL(clicked()), this, SLOT(chooseFixedFont()));

    connect(btnSelectPlayer, SIGNAL(clicked()), this, SLOT(onSelectExternalPlayer()));
    connect(btnAddPlayer, SIGNAL(clicked()), this, SLOT(onAddExternalPlayer()));
    connect(btnRemovePlayer, SIGNAL(clicked()), this, SLOT(onRemoveExternalPlayer()));
    connect(btnModifyPlayer, SIGNAL(clicked()), this, SLOT(onModifyExternalPlayer()));
    connect(listPlayer, SIGNAL(currentRowChanged(int)), this, SLOT(onExternalPlayerListCurrentRowChanged(int)));

    loadDefaults();
    loadFromSettings();
}

static QString defaultAcceptLanguage()
{
    const QStringList langs = QLocale().uiLanguages();
    if (langs.isEmpty())
        return QString();
    QString str = langs.first();
    const float qstep = 1.0f / float(langs.count());
    float q = 1.0f - qstep;
    for (int i = 1; i < langs.count(); ++i) {
        str += QStringLiteral(", ") + langs.at(i) + QStringLiteral(";q=") + QString::number(q, 'f', 2);
        q -= qstep;
    }
    return str;
}

void SettingsDialog::loadDefaults()
{
    QWebEngineSettings *defaultSettings = QWebEngineSettings::globalSettings();
    QString standardFontFamily = defaultSettings->fontFamily(QWebEngineSettings::StandardFont);
    int standardFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFontSize);
    standardFont = QFont(standardFontFamily, standardFontSize);
    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));

    QString fixedFontFamily = defaultSettings->fontFamily(QWebEngineSettings::FixedFont);
    int fixedFontSize = defaultSettings->fontSize(QWebEngineSettings::DefaultFixedFontSize);
    fixedFont = QFont(fixedFontFamily, fixedFontSize);
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    downloadsLocation->setText(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));

    enableJavascript->setChecked(defaultSettings->testAttribute(QWebEngineSettings::JavascriptEnabled));
    enablePlugins->setChecked(defaultSettings->testAttribute(QWebEngineSettings::PluginsEnabled));

    enableScrollAnimator->setChecked(defaultSettings->testAttribute(QWebEngineSettings::ScrollAnimatorEnabled));

    persistentDataPath->setText(QWebEngineProfile::defaultProfile()->persistentStoragePath());
    sessionCookiesCombo->setCurrentIndex(QWebEngineProfile::defaultProfile()->persistentCookiesPolicy());
    httpUserAgent->setText(QWebEngineProfile::defaultProfile()->httpUserAgent());
    httpAcceptLanguage->setText(defaultAcceptLanguage());

    if (!defaultSettings->testAttribute(QWebEngineSettings::AutoLoadIconsForPage))
        faviconDownloadMode->setCurrentIndex(0);
    else if (!defaultSettings->testAttribute(QWebEngineSettings::TouchIconsEnabled))
        faviconDownloadMode->setCurrentIndex(1);
    else
        faviconDownloadMode->setCurrentIndex(2);
}

void SettingsDialog::loadFromSettings()
{
    Config cfg;
    cfg.beginGroup(QLatin1String("MainWindow"));
    homeLineEdit->setText(cfg.read("home", QString(BrowserWindow::defaultHome)));
    cfg.endGroup();

    cfg.beginGroup(QLatin1String("history"));
    int historyExpire = cfg.read<int>("historyExpire");
    int idx = 0;
    switch (historyExpire) {
    case 1: idx = 0; break;
    case 7: idx = 1; break;
    case 14: idx = 2; break;
    case 30: idx = 3; break;
    case 365: idx = 4; break;
    case -1: idx = 5; break;
    default:
        idx = 5;
    }
    expireHistory->setCurrentIndex(idx);
    cfg.endGroup();

    QString downloadDirectory = cfg.read(QLatin1String("downloadDirectory"), downloadsLocation->text());
    downloadsLocation->setText(downloadDirectory);

    cfg.beginGroup(QLatin1String("general"));
    openLinksIn->setCurrentIndex(cfg.read<int>(QLatin1String("openLinksIn"), openLinksIn->currentIndex()));
    cfg.endGroup();

    // Appearance
    cfg.beginGroup(QLatin1String("websettings"));
    fixedFont = qvariant_cast<QFont>(cfg.read(QLatin1String("fixedFont"), QVariant(fixedFont)));
    standardFont = qvariant_cast<QFont>(cfg.read(QLatin1String("standardFont"), QVariant(standardFont)));

    standardLabel->setText(QString(QLatin1String("%1 %2")).arg(standardFont.family()).arg(standardFont.pointSize()));
    fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(fixedFont.family()).arg(fixedFont.pointSize()));

    enableJavascript->setChecked(cfg.read<bool>(QLatin1String("enableJavascript"), enableJavascript->isChecked()));
    enablePlugins->setChecked(cfg.read<bool>(QLatin1String("enablePlugins"), enablePlugins->isChecked()));
    userStyleSheet->setPlainText(cfg.read<QString>(QLatin1String("userStyleSheet")));
    enableScrollAnimator->setChecked(cfg.read<bool>(QLatin1String("enableScrollAnimator"), enableScrollAnimator->isChecked()));
    httpUserAgent->setText(cfg.read(QLatin1String("httpUserAgent"), httpUserAgent->text()));
    httpAcceptLanguage->setText(cfg.read(QLatin1String("httpAcceptLanguage"), httpAcceptLanguage->text()));
    faviconDownloadMode->setCurrentIndex(cfg.read<int>(QLatin1String("faviconDownloadMode"), faviconDownloadMode->currentIndex()));
    cfg.endGroup();

    // Privacy
    cfg.beginGroup(QLatin1String("cookies"));

    int persistentCookiesPolicy = cfg.read<int>(QLatin1String("persistentCookiesPolicy"), sessionCookiesCombo->currentIndex());
    sessionCookiesCombo->setCurrentIndex(persistentCookiesPolicy);

    QString pdataPath = cfg.read(QLatin1String("persistentDataPath"), persistentDataPath->text());
    persistentDataPath->setText(pdataPath);

    cfg.endGroup();

    // Proxy
    cfg.beginGroup(QLatin1String("proxy"));
    proxySupport->setChecked(cfg.read<bool>(QLatin1String("enabled"), false));
    proxyType->setCurrentIndex(cfg.read<int>(QLatin1String("type"), 0));
    proxyHostName->setText(cfg.read<QString>(QLatin1String("hostName")));
    proxyPort->setValue(cfg.read<int>(QLatin1String("port"), 1080));
    proxyUserName->setText(cfg.read<QString>(QLatin1String("userName")));
    proxyPassword->setText(cfg.read<QString>(QLatin1String("password")));
    cfg.endGroup();

    // external player
    cfg.read("externalPlayers", m_players);
    for (auto p : m_players)
    {
        listPlayer->addItem(std::get<0>(p) + "\n" + std::get<1>(p));
    }
}

void SettingsDialog::saveToSettings()
{
    Config cfg;
    cfg.beginGroup(QLatin1String("MainWindow"));
    cfg.write(QLatin1String("home"), homeLineEdit->text());
    cfg.endGroup();

    cfg.beginGroup(QLatin1String("general"));
    cfg.write(QLatin1String("openLinksIn"), openLinksIn->currentIndex());
    cfg.endGroup();

    cfg.beginGroup(QLatin1String("history"));
    int historyExpire = expireHistory->currentIndex();
    int idx = -1;
    switch (historyExpire) {
    case 0: idx = 1; break;
    case 1: idx = 7; break;
    case 2: idx = 14; break;
    case 3: idx = 30; break;
    case 4: idx = 365; break;
    case 5: idx = -1; break;
    }
    cfg.write(QLatin1String("historyExpire"), idx);
    cfg.endGroup();

    // Appearance
    cfg.beginGroup(QLatin1String("websettings"));
    cfg.write(QLatin1String("fixedFont"), fixedFont);
    cfg.write(QLatin1String("standardFont"), standardFont);
    cfg.write(QLatin1String("enableJavascript"), enableJavascript->isChecked());
    cfg.write(QLatin1String("enablePlugins"), enablePlugins->isChecked());
    cfg.write(QLatin1String("enableScrollAnimator"), enableScrollAnimator->isChecked());
    cfg.write(QLatin1String("userStyleSheet"), userStyleSheet->toPlainText());
    cfg.write(QLatin1String("httpUserAgent"), httpUserAgent->text());
    cfg.write(QLatin1String("httpAcceptLanguage"), httpAcceptLanguage->text());
    cfg.write(QLatin1String("faviconDownloadMode"), faviconDownloadMode->currentIndex());
    cfg.endGroup();

    //Privacy
    cfg.beginGroup(QLatin1String("cookies"));

    int persistentCookiesPolicy = sessionCookiesCombo->currentIndex();
    cfg.write(QLatin1String("persistentCookiesPolicy"), persistentCookiesPolicy);

    QString pdataPath = persistentDataPath->text();
    cfg.write(QLatin1String("persistentDataPath"), pdataPath);

    cfg.endGroup();

    // proxy
    cfg.beginGroup(QLatin1String("proxy"));
    cfg.write(QLatin1String("enabled"), proxySupport->isChecked());
    cfg.write(QLatin1String("type"), proxyType->currentIndex());
    cfg.write(QLatin1String("hostName"), proxyHostName->text());
    cfg.write(QLatin1String("port"), proxyPort->text());
    cfg.write(QLatin1String("userName"), proxyUserName->text());
    cfg.write(QLatin1String("password"), proxyPassword->text());
    cfg.endGroup();

    Browser::instance().loadSettings();
}

void SettingsDialog::accept()
{
    saveToSettings();
    QDialog::accept();
}

void SettingsDialog::setCurrentPage(int index)
{
    tabWidget->setCurrentIndex(index);
}

void SettingsDialog::showCookies()
{
}

void SettingsDialog::showExceptions()
{
}

void SettingsDialog::chooseFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, standardFont, this);
    if ( ok ) {
        standardFont = font;
        standardLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
}

void SettingsDialog::chooseFixedFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fixedFont, this);
    if ( ok ) {
        fixedFont = font;
        fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
}

void SettingsDialog::on_httpUserAgent_editingFinished()
{
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(httpUserAgent->text());
}

void SettingsDialog::on_httpAcceptLanguage_editingFinished()
{
    QWebEngineProfile::defaultProfile()->setHttpAcceptLanguage(httpAcceptLanguage->text());
}

void SettingsDialog::onSelectExternalPlayer()
{
    QString mediaPlayerPath = QFileDialog::getOpenFileName(this,
                                 tr("Select media player executable"));
    if (!mediaPlayerPath.isEmpty())
    {
        edtPlayerPath->setText(mediaPlayerPath);
        edtPlayerArguments->setText("");
    }
}

void SettingsDialog::onAddExternalPlayer()
{
    if (edtPlayerPath->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input player path."), QMessageBox::Ok);
        return;
    }
    auto it = std::find_if(m_players.begin(), m_players.end(),
                           [this](const Tuple2& t) { return std::get<0>(t) == edtPlayerPath->text() && std::get<1>(t) == edtPlayerArguments->text();});
    if (m_players.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This configuration item exists already."), QMessageBox::Ok);
        return;
    }
    m_players.push_back(std::make_tuple(edtPlayerPath->text(), edtPlayerArguments->text()));
    Config cfg;
    cfg.write("externalPlayers", m_players);
    listPlayer->addItem(edtPlayerPath->text() + "\n" + edtPlayerArguments->text());
    edtPlayerPath->setText("");
    edtPlayerArguments->setText("");
}

void SettingsDialog::onRemoveExternalPlayer()
{
    QListWidgetItem *currentItem = listPlayer->currentItem();
    if (!currentItem)
    {
        QMessageBox::warning(this, tr("Error"), tr("Please select an item in list to be removed."), QMessageBox::Ok);
        return;
    }
    int currentRow = listPlayer->currentRow();
    m_players.removeAt(currentRow);
    Config cfg;
    cfg.write("externalPlayers", m_players);
    currentItem = listPlayer->takeItem(currentRow);
    delete currentItem;
}

void SettingsDialog::onModifyExternalPlayer()
{
    QListWidgetItem *currentItem = listPlayer->currentItem();
    if (!currentItem)
    {
        return;
    }
    int currentRow = listPlayer->currentRow();
    m_players[currentRow] = std::make_tuple(edtPlayerPath->text(), edtPlayerArguments->text());
    currentItem->setText(edtPlayerPath->text() + "\n" + edtPlayerArguments->text());
    Config cfg;
    cfg.write("externalPlayers", m_players);
}

void SettingsDialog::onExternalPlayerListCurrentRowChanged(int currentRow)
{
    if (currentRow < 0 && currentRow >= m_players.size())
        return;

    const Tuple2& p = m_players.at(currentRow);
    edtPlayerPath->setText(std::get<0>(p));
    edtPlayerArguments->setText(std::get<1>(p));
}

void SettingsDialog::setHomeToCurrentPage()
{
    BrowserWindow *mw = static_cast<BrowserWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
        homeLineEdit->setText(webView->url().toString());
}
