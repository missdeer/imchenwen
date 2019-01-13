#include "settings.h"
#include "browser.h"
#include "browserwindow.h"
#include "webview.h"
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtWidgets/QtWidgets>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QNetworkInterface>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(tabWidget, &QTabWidget::currentChanged, this, &SettingsDialog::onTabWidgetCurrentChanged);
    onTabWidgetCurrentChanged(0);
    QToolBar* toolbar = new QToolBar(this);
    this->layout()->replaceWidget(widgetPlaceholder, toolbar);
    toolbar->setIconSize(QSize(40, 40));
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    QAction *actionGeneral = toolbar->addAction(QIcon(":/preferences/general.png"), tr("General"));
    connect(actionGeneral, &QAction::triggered, [&](){tabWidget->setCurrentIndex(0);});
    QAction *actionAppearance = toolbar->addAction(QIcon(":/preferences/appearance.png"), tr("Appearance"));
    connect(actionAppearance, &QAction::triggered, [&](){tabWidget->setCurrentIndex(1);});
    QAction *actionPrivacy = toolbar->addAction(QIcon(":/preferences/privacy.png"), tr("Privacy"));
    connect(actionPrivacy, &QAction::triggered, [&](){tabWidget->setCurrentIndex(2);});
    QAction *actionProxy = toolbar->addAction(QIcon(":/preferences/proxy.png"), tr("Proxy"));
    connect(actionProxy, &QAction::triggered, [&](){tabWidget->setCurrentIndex(3);});
    QAction *actionAdvanced = toolbar->addAction(QIcon(":/preferences/advanced.png"), tr("Advanced"));
    connect(actionAdvanced, &QAction::triggered, [&](){tabWidget->setCurrentIndex(4);});
    QAction *actionStorage = toolbar->addAction(QIcon(":/preferences/storage.png"), tr("Storage"));
    connect(actionStorage, &QAction::triggered, [&](){tabWidget->setCurrentIndex(5);});
    QAction *actionPlayer = toolbar->addAction(QIcon(":/preferences/player.png"), tr("Player"));
    connect(actionPlayer, &QAction::triggered, [&](){tabWidget->setCurrentIndex(6);});
    QAction *actionResolver = toolbar->addAction(QIcon(":/preferences/resolver.png"), tr("Resolver"));
    connect(actionResolver, &QAction::triggered, [&](){tabWidget->setCurrentIndex(7);});
    QAction *actionTV = toolbar->addAction(QIcon(":/preferences/tv.png"), tr("Live TV"));
    connect(actionTV, &QAction::triggered, [&](){tabWidget->setCurrentIndex(8);});
    tabWidget->tabBar()->hide();

    setupLiveTVTable();

    for(auto && address : QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol
                && address != QHostAddress(QHostAddress::LocalHost) // Check if it is local adress
                && address.toString().section( ".",-1,-1 ) != "1") // Check if it is virtual machine
        {
            cbDLNAUseIP->addItem(address.toString());
        }
    }

    connect(setHomeToCurrentPageButton, &QPushButton::clicked, this, &SettingsDialog::onSetHomeToCurrentPage);
    connect(standardFontButton, &QPushButton::clicked, this, &SettingsDialog::onChooseFont);
    connect(fixedFontButton, &QPushButton::clicked, this, &SettingsDialog::onChooseFixedFont);

    connect(btnSelectPlayer, &QPushButton::clicked, this, &SettingsDialog::onSelectExternalPlayer);
    connect(btnAddPlayer, &QPushButton::clicked, this, &SettingsDialog::onAddExternalPlayer);
    connect(btnRemovePlayer, &QPushButton::clicked, this, &SettingsDialog::onRemoveExternalPlayer);
    connect(btnModifyPlayer, &QPushButton::clicked, this, &SettingsDialog::onModifyExternalPlayer);
    connect(listPlayer, &QListWidget::currentRowChanged, this, &SettingsDialog::onExternalPlayerListCurrentRowChanged);

    connect(btnAddLiveTVItem, &QPushButton::clicked, this, &SettingsDialog::onAddLiveTVItem);
    connect(btnRemoveLiveTVItem, &QPushButton::clicked, this, &SettingsDialog::onRemoveLiveTVItem);
    connect(btnModifyLiveTVItem, &QPushButton::clicked, this, &SettingsDialog::onModifyLiveTVItem);
    connect(btnImportLiveTVItems, &QPushButton::clicked, this, &SettingsDialog::onImportLiveTVItems);
    connect(btnExportLiveTVItems, &QPushButton::clicked, this, &SettingsDialog::onExportLiveTVItems);
    connect(tblLiveTV, &QTableWidget::itemSelectionChanged, this, &SettingsDialog::onLiveTVTableItemSelectionChanged);

    connect(btnAddLiveTVSubscription, &QPushButton::clicked, this, &SettingsDialog::onAddLiveTVSubscription);
    connect(btnRemoveLiveTVSubscription, &QPushButton::clicked, this, &SettingsDialog::onRemoveLiveTVSubscription);
    connect(tblLiveTVSubscription, &QTableWidget::itemSelectionChanged, this, &SettingsDialog::onLiveTVSubscriptionTableItemSelectionChanged);

    connect(btnBrowseYouGetPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYouGetPath);
    connect(btnBrowseYKDLPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYKDLPath);
    connect(btnBrowseYoutubeDLPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYoutubeDLPath);
    connect(btnBrowseAnniePath, &QPushButton::clicked, this, &SettingsDialog::onBrowseAnniePath);
    connect(btnBrowseFFmpegPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseFFmpegPath);
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

void SettingsDialog::fillLiveTVTable()
{
    for (const auto & tv : m_liveTV)
    {
        int index = tblLiveTV->rowCount();
        tblLiveTV->insertRow(index);
        QTableWidgetItem *name = new QTableWidgetItem(std::get<0>(tv));
        tblLiveTV->setItem(index, 0, name);
        emit tblLiveTV->itemChanged(name);
        QTableWidgetItem *url = new QTableWidgetItem(std::get<1>(tv));
        tblLiveTV->setItem(index, 1, url);
        emit tblLiveTV->itemChanged(url);
    }
}

void SettingsDialog::fillLiveTVSubscriptionTable()
{
    for (const auto & tv : m_liveTVSubscription)
    {
        int index = tblLiveTVSubscription->rowCount();
        tblLiveTVSubscription->insertRow(index);
        QTableWidgetItem *url = new QTableWidgetItem(tv);
        tblLiveTVSubscription->setItem(index, 0, url);
        emit tblLiveTVSubscription->itemChanged(url);
    }
}

void SettingsDialog::fillExternalPlayerTable()
{
    for (const auto& p : m_players)
    {
        listPlayer->addItem(p->name() + "\n" + p->arguments());
    }
}

void SettingsDialog::loadFromSettings()
{
    Config cfg;

    cbDLNAUseIP->setCurrentText(cfg.read<QString>("dlnaUseIP"));

    homeLineEdit->setText(cfg.read<QString>("defaultHome"));

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

    QString downloadDirectory = cfg.read(QLatin1String("downloadDirectory"), downloadsLocation->text());
    downloadsLocation->setText(downloadDirectory);

    openLinksIn->setCurrentIndex(cfg.read<int>(QLatin1String("openLinksIn"), openLinksIn->currentIndex()));

    // Appearance
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

    // Privacy
    int persistentCookiesPolicy = cfg.read<int>(QLatin1String("persistentCookiesPolicy"), sessionCookiesCombo->currentIndex());
    sessionCookiesCombo->setCurrentIndex(persistentCookiesPolicy);

    QString pdataPath = cfg.read(QLatin1String("persistentDataPath"), persistentDataPath->text());
    persistentDataPath->setText(pdataPath);

    // Proxy
    proxySupport->setChecked(cfg.read<bool>(QLatin1String("enableProxy"), false));
    proxyType->setCurrentIndex(cfg.read<int>(QLatin1String("proxyType"), 0) == QNetworkProxy::Socks5Proxy ? 0 : 1);
    proxyHostName->setText(cfg.read<QString>(QLatin1String("proxyHostName")));
    proxyPort->setValue(cfg.read<int>(QLatin1String("proxyPort"), 1080));
    proxyUserName->setText(cfg.read<QString>(QLatin1String("proxyUserName")));
    proxyPassword->setText(cfg.read<QString>(QLatin1String("proxyPassword")));
    cbApplyToResolvers->setChecked(cfg.read<bool>(QLatin1String("applyProxyToResolvers"), true));
    cbProxyScope->setCurrentIndex(cfg.read<int>(QLatin1String("proxyScope"), 0));
    edtGFWList->setText(cfg.read(QLatin1String("gfwList"), QString("https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt")));
    edtChinaDomain->setText(cfg.read(QLatin1String("chinaDomain"), QString("https://cdn.jsdelivr.net/gh/felixonmars/dnsmasq-china-list/accelerated-domains.china.conf")));

    // external player
    cfg.read("externalPlayers", m_players);
    fillExternalPlayerTable();

    // Live TV
    cfg.read("liveTV", m_liveTV);
    fillLiveTVTable();

    // Live TV subscription
    cfg.read("liveTVSubscription", m_liveTVSubscription);
    fillLiveTVSubscriptionTable();

    // resolver
    edtYouGetPath->setText(cfg.read<QString>(QLatin1String("you-get")));
    edtYKDLPath->setText(cfg.read<QString>(QLatin1String("ykdl")));
    edtYoutubeDLPath->setText(cfg.read<QString>(QLatin1String("youtube-dl")));
    edtAnniePath->setText(cfg.read<QString>(QLatin1String("annie")));
    edtFFmpegPath->setText(cfg.read<QString>(QLatin1String("ffmpeg")));
    edtVIPResolverSubscription->setText(cfg.read<QString>(QLatin1String("vipResolvers")));
    edtShortcutSubscription->setText(cfg.read<QString>(QLatin1String("shortcut")));

    // storage
    gbStorageService->setChecked(cfg.read<bool>(QLatin1String("enableStorageService")));
    edtStorageServiceAddress->setText(cfg.read<QString>(QLatin1String("storageServiceAddress")));
    btnEnableStorageTranscoding->setChecked(cfg.read<bool>(QLatin1String("enableStorageTranscoding")));
}

void SettingsDialog::saveToSettings()
{
    Config cfg;
    cfg.write(QLatin1String("dlnaUseIP"), cbDLNAUseIP->currentText());

    cfg.write(QLatin1String("defaultHome"), homeLineEdit->text());

    cfg.write(QLatin1String("openLinksIn"), openLinksIn->currentIndex());

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

    // Appearance
    cfg.write(QLatin1String("fixedFont"), fixedFont);
    cfg.write(QLatin1String("standardFont"), standardFont);
    cfg.write(QLatin1String("enableJavascript"), enableJavascript->isChecked());
    cfg.write(QLatin1String("enablePlugins"), enablePlugins->isChecked());
    cfg.write(QLatin1String("enableScrollAnimator"), enableScrollAnimator->isChecked());
    cfg.write(QLatin1String("userStyleSheet"), userStyleSheet->toPlainText());
    cfg.write(QLatin1String("httpUserAgent"), httpUserAgent->text());
    QWebEngineProfile::defaultProfile()->setHttpUserAgent(httpUserAgent->text());
    cfg.write(QLatin1String("httpAcceptLanguage"), httpAcceptLanguage->text());
    QWebEngineProfile::defaultProfile()->setHttpAcceptLanguage(httpAcceptLanguage->text());
    cfg.write(QLatin1String("faviconDownloadMode"), faviconDownloadMode->currentIndex());

    //Privacy
    int persistentCookiesPolicy = sessionCookiesCombo->currentIndex();
    cfg.write(QLatin1String("persistentCookiesPolicy"), persistentCookiesPolicy);

    QString pdataPath = persistentDataPath->text();
    cfg.write(QLatin1String("persistentDataPath"), pdataPath);

    // proxy
    cfg.write(QLatin1String("enableProxy"), proxySupport->isChecked());
    cfg.write(QLatin1String("proxyType"), proxyType->currentIndex() == 0 ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy);
    cfg.write(QLatin1String("proxyHostName"), proxyHostName->text());
    cfg.write(QLatin1String("proxyPort"), proxyPort->text());
    cfg.write(QLatin1String("proxyUserName"), proxyUserName->text());
    cfg.write(QLatin1String("proxyPassword"), proxyPassword->text());
    cfg.write(QLatin1String("applyProxyToResolvers"), cbApplyToResolvers->isChecked());
    cfg.write(QLatin1String("proxyScope"), cbProxyScope->currentIndex());
    cfg.write(QLatin1String("gfwList"), edtGFWList->text());
    cfg.write(QLatin1String("chinaDomain"), edtChinaDomain->text());

    // resolver
    cfg.write(QLatin1String("you-get"), edtYouGetPath->text());
    cfg.write(QLatin1String("ykdl"), edtYKDLPath->text());
    cfg.write(QLatin1String("youtube-dl"), edtYoutubeDLPath->text());
    cfg.write(QLatin1String("annie"), edtAnniePath->text());
    cfg.write(QLatin1String("ffmpeg"), edtFFmpegPath->text());
    cfg.write(QLatin1String("vipResolvers"), edtVIPResolverSubscription->text());
    cfg.write(QLatin1String("shortcut"), edtShortcutSubscription->text());

    // external players
    cfg.write("externalPlayers", m_players);

    // live TV
    cfg.write("liveTV", m_liveTV);

    // live TV subscription
    cfg.write("liveTVSubscription", m_liveTVSubscription);

    // stoarge
    cfg.write("enableStorageService", gbStorageService->isChecked());
    cfg.write("storageServiceAddress", edtStorageServiceAddress->text());
    cfg.write("enableStorageTranscoding", btnEnableStorageTranscoding->isChecked());

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

void SettingsDialog::onChooseFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, standardFont, this);
    if ( ok ) {
        standardFont = font;
        standardLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
}

void SettingsDialog::onChooseFixedFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, fixedFont, this);
    if ( ok ) {
        fixedFont = font;
        fixedLabel->setText(QString(QLatin1String("%1 %2")).arg(font.family()).arg(font.pointSize()));
    }
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
                           [this](PlayerPtr t) { return t->name() == edtPlayerPath->text() && t->arguments() == edtPlayerArguments->text();});
    if (m_players.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This configuration item exists already."), QMessageBox::Ok);
        return;
    }
    PlayerPtr p(new Player(Player::PT_EXTERNAL, edtPlayerPath->text()));
    p->setArguments(edtPlayerArguments->text());
    m_players.push_back(p);
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
    m_players[currentRow]->setName(edtPlayerPath->text());
    m_players[currentRow]->setArguments(edtPlayerArguments->text());
    currentItem->setText(edtPlayerPath->text() + "\n" + edtPlayerArguments->text());
}

void SettingsDialog::onExternalPlayerListCurrentRowChanged(int currentRow)
{
    if (currentRow < 0 && currentRow >= m_players.size())
        return;

    PlayerPtr p = m_players.at(currentRow);
    edtPlayerPath->setText(p->name());
    edtPlayerArguments->setText(p->arguments());
}

void SettingsDialog::onBrowseYouGetPath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select you-get executable"),
                                                QDir(edtYouGetPath->text()).absolutePath(),
                                            #if defined(Q_OS_WIN)
                                                tr("you-get executable (you-get.exe)")
                                            #else
                                                tr("you-get executable (you-get)")
                                            #endif
                                                );
    if (!path.isEmpty())
    {
        edtYouGetPath->setText(path);
    }
}

void SettingsDialog::onBrowseYKDLPath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select ykdl executable"),
                                                QDir(edtYKDLPath->text()).absolutePath(),
                                            #if defined(Q_OS_WIN)
                                                tr("ykdl executable (ykdl.exe)")
                                            #else
                                                tr("ykdl executable (ykdl)")
                                            #endif
                                                );
    if (!path.isEmpty())
    {
        edtYKDLPath->setText(path);
    }
}

void SettingsDialog::onBrowseYoutubeDLPath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select youtube-dl executable"),
                                                QDir(edtYoutubeDLPath->text()).absolutePath(),
                                            #if defined(Q_OS_WIN)
                                                tr("youtube-dl executable (youtube-dl.exe)")
                                            #else
                                                tr("youtube-dl executable (youtube-dl)")
                                            #endif
                                                );
    if (!path.isEmpty())
    {
        edtYoutubeDLPath->setText(path);
    }
}

void SettingsDialog::onBrowseAnniePath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select annie executable"),
                                                QDir(edtAnniePath->text()).absolutePath(),
                                            #if defined(Q_OS_WIN)
                                                tr("annie executable (annie.exe)")
                                            #else
                                                tr("annie executable (annie)")
                                            #endif
                                                );
    if (!path.isEmpty())
    {
        edtAnniePath->setText(path);
    }
}

void SettingsDialog::onBrowseFFmpegPath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select FFmpeg executable"),
                                                QDir(edtFFmpegPath->text()).absolutePath(),
                                            #if defined(Q_OS_WIN)
                                                tr("FFmpeg executable (ffmpeg.exe)")
                                            #else
                                                tr("FFmpeg executable (ffmpeg)")
                                            #endif
                                                );
    if (!path.isEmpty())
    {
        edtFFmpegPath->setText(path);
    }
}

void SettingsDialog::onAddLiveTVSubscription()
{
    if (edtLiveTVSubscription->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input live TV subscription URL."), QMessageBox::Ok);
        return;
    }
    if (m_liveTVSubscription.contains(edtLiveTVSubscription->text(), Qt::CaseInsensitive))
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This live TV subscription exists already."), QMessageBox::Ok);
        return;
    }
    m_liveTVSubscription.push_back(edtLiveTVSubscription->text());
    int rowIndex = tblLiveTVSubscription->rowCount();
    tblLiveTVSubscription->insertRow(rowIndex);
    QTableWidgetItem *url = new QTableWidgetItem(edtLiveTVSubscription->text());
    tblLiveTVSubscription->setItem(rowIndex, 0, url);
    emit tblLiveTVSubscription->itemChanged(url);
    edtLiveTVSubscription->setText("");
}

void SettingsDialog::onRemoveLiveTVSubscription()
{
    auto ranges = tblLiveTVSubscription->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_liveTVSubscription.removeAt(currentRow);
    tblLiveTVSubscription->removeRow(currentRow);
}

void SettingsDialog::onLiveTVSubscriptionTableItemSelectionChanged()
{
    edtLiveTVSubscription->setText("");
    auto items = tblLiveTVSubscription->selectedItems();
    if (items.isEmpty())
        return;
    edtLiveTVSubscription->setText(items[0]->text());
}

void SettingsDialog::onAddLiveTVItem()
{
    if (edtLiveTVName->text().isEmpty() || edtLiveTVURL->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input Live TV name and URL."), QMessageBox::Ok);
        return;
    }
    auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(),
                           [this](const Tuple2& t) {
        return std::get<0>(t) == edtLiveTVName->text() && std::get<1>(t) == edtLiveTVURL->text() ;
    });
    if (m_liveTV.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This Live TV item exists already."), QMessageBox::Ok);
        return;
    }
    m_liveTV.push_back(std::make_tuple(edtLiveTVName->text(), edtLiveTVURL->text()));

    int rowIndex = tblLiveTV->rowCount();
    tblLiveTV->insertRow(rowIndex);
    QTableWidgetItem *name = new QTableWidgetItem(edtLiveTVName->text());
    tblLiveTV->setItem(rowIndex, 0, name);
    emit tblLiveTV->itemChanged(name);
    QTableWidgetItem *url = new QTableWidgetItem(edtLiveTVURL->text());
    tblLiveTV->setItem(rowIndex, 1, url);
    emit tblLiveTV->itemChanged(url);

    edtLiveTVName->setText("");
    edtLiveTVURL->setText("");
}

void SettingsDialog::onRemoveLiveTVItem()
{
    auto ranges = tblLiveTV->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    qDebug() << __FUNCTION__ << currentRow;
    m_liveTV.removeAt(currentRow);
    tblLiveTV->removeRow(currentRow);
}

void SettingsDialog::onModifyLiveTVItem()
{
    auto ranges = tblLiveTV->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_liveTV[currentRow] = std::make_tuple(edtLiveTVName->text(), edtLiveTVURL->text());

    auto items = tblLiveTV->selectedItems();
    items[0]->setText(edtLiveTVName->text());
    emit tblLiveTV->itemChanged(items[0]);
    items[1]->setText(edtLiveTVURL->text());
    emit tblLiveTV->itemChanged(items[1]);
}

void SettingsDialog::onImportLiveTVItems()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Select Live TV list to import"),
                                                      QString(),
                                                      tr("Supported formats (*.json *.txt);;JSON format (*.json);;Plain text format (*.txt)")
                                                      );
    bool changed = false;
    for (const QString& path : paths)
    {
        QFileInfo fi(path);
        if (fi.suffix().toLower() == "json")
        {
            changed |= importLiveTVAsJSON(path);
        }
        else
        {
            changed |= importLiveTVAsPlainText(path);
        }
    }
    if (changed)
    {
        tblLiveTV->clear();
        setupLiveTVTable();
        fillLiveTVTable();
    }
}

void SettingsDialog::onExportLiveTVItems()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Live TV list to"),
                                                    QString(),
                                                    tr("JSON format (*.json);;Plain text format (*.txt)")
                                                    );
    QFileInfo fi(fileName);
    if (fi.suffix().toLower() == "json")
    {
        exportLiveTVAsJSON(fileName);
    }
    else
    {
        exportLiveTVAsPlainText(fileName);
    }
}

void SettingsDialog::onLiveTVTableItemSelectionChanged()
{
    edtLiveTVName->setText("");
    edtLiveTVURL->setText("");

    auto items = tblLiveTV->selectedItems();
    if (items.isEmpty())
        return;
    edtLiveTVName->setText(items[0]->text());
    edtLiveTVURL->setText(items[1]->text());
}

void SettingsDialog::onTabWidgetCurrentChanged(int index)
{
    QStringList titles = {
        tr("Settings - General"),
        tr("Settings - Appearance"),
        tr("Settings - Privacy"),
        tr("Settings - Proxy"),
        tr("Settings - Advanced"),
        tr("Settings - Storage"),
        tr("Settings - Player"),
        tr("Settings - Resolver"),
        tr("Settings - Live TV"),
    };
    if (index >= 0 && index < titles.length())
        setWindowTitle(titles[index]);
    else
        setWindowTitle(tr("Settings"));
}

bool SettingsDialog::importLiveTVAsJSON(const QString &path)
{
    QFile f(path);
    int c = m_liveTV.length();
    if (f.open(QIODevice::ReadOnly))
    {
        auto d = f.readAll();
        f.close();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(d, &error);
        if (error.error != QJsonParseError::NoError)
        {
            QMessageBox::warning(this, tr("Error on importing live TV list as JSON"), error.errorString(), QMessageBox::Ok);
            return false;
        }
        if (doc.isObject())
        {
            auto rootObj = doc.object();
            auto titleObj = rootObj["title"];

            QString title = tr("unknown");
            if (!titleObj.isString())
                title = titleObj.toString();
            auto channels = rootObj["channels"];
            if (!channels.isArray())
                return false;
            auto arr = channels.toArray();
            for (auto a : arr)
            {
                if (!a.isObject())
                    continue;
                auto o = a.toObject();
                if (!o["name"].isString() || o["name"].toString().isEmpty() || !o["url"].isString() || o["url"].toString().isEmpty())
                    continue;
                QString category = title;
                auto vv = std::make_tuple(o["name"].toString(), o["url"].toString());
                auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(), [&vv](const Tuple2& v){
                    return std::get<0>(v) == std::get<0>(vv) && std::get<1>(v) == std::get<1>(vv);
                });
                if (m_liveTV.end() == it)
                    m_liveTV.push_back(vv);
            }
        }
    }
    return m_liveTV.length() != c;
}

bool SettingsDialog::importLiveTVAsPlainText(const QString &path)
{
    QFile f(path);
    int c = m_liveTV.length();
    if (f.open(QIODevice::ReadOnly))
    {
        auto d = f.readAll();
        f.close();
        auto lines = d.split('\n');
        for (const auto& line : lines)
        {
            auto ele = line.trimmed().split(' ');
            if (ele.length() <2 )
            continue;

            auto vv = std::make_tuple(QString(ele[0]), QString(ele[1]));
            auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(), [&vv](const Tuple2& v){
                return std::get<0>(v) == std::get<0>(vv) && std::get<1>(v) == std::get<1>(vv) ;
            });
            if (m_liveTV.end() == it)
                m_liveTV.push_back(vv);
        }
    }
    return m_liveTV.length() != c;
}

void SettingsDialog::exportLiveTVAsJSON(const QString &path)
{
    QJsonDocument doc = QJsonDocument::fromJson("{\"title\": \"imchenwen exported custom live TV list\", \"channels\":[]}");
    auto a = doc.object();
    auto channels = a["channels"].toArray();
    for (const auto& vv : m_liveTV)
    {
        QJsonObject o;
        o.insert("name", std::get<0>(vv));
        o.insert("url", std::get<1>(vv));
        channels.append(o);
    }
    auto d = doc.toJson(QJsonDocument::Indented);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(d);
        f.close();
    }
}

void SettingsDialog::exportLiveTVAsPlainText(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        for (const auto& tv : m_liveTV)
        {
            f.write(std::get<0>(tv).toUtf8() + " " + std::get<1>(tv).toUtf8() + "\n");
        }
        f.close();
    }
}

void SettingsDialog::onSetHomeToCurrentPage()
{
    BrowserWindow *mw = static_cast<BrowserWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
        homeLineEdit->setText(webView->url().toString());
}

void SettingsDialog::setupLiveTVTable()
{
    tblLiveTV->setColumnCount(2);
    tblLiveTV->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("URL"));
    tblLiveTV->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    tblLiveTVSubscription->setColumnCount(1);
    tblLiveTVSubscription->setHorizontalHeaderLabels(QStringList() << tr("URL"));
    tblLiveTVSubscription->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}
