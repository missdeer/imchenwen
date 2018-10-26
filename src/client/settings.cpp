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
    tblLiveTV->setColumnCount(3);
    tblLiveTV->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("URL") << tr("Availability"));
    tblLiveTV->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    tblVIPVideo->setColumnCount(2);
    tblVIPVideo->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("URL"));
    tblVIPVideo->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    connect(setHomeToCurrentPageButton, &QPushButton::clicked, this, &SettingsDialog::setHomeToCurrentPage);
    connect(standardFontButton, &QPushButton::clicked, this, &SettingsDialog::chooseFont);
    connect(fixedFontButton, &QPushButton::clicked, this, &SettingsDialog::chooseFixedFont);

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
    connect(btnCheckLiveTVItems, &QPushButton::clicked, this, &SettingsDialog::onCheckLiveTVItems);
    connect(tblLiveTV, &QTableWidget::itemSelectionChanged, this, &SettingsDialog::onLiveTVTableItemSelectionChanged);

    connect(btnAddVIPVideo, &QPushButton::clicked, this, &SettingsDialog::onAddVIPVideo);
    connect(btnRemoveVIPVideo, &QPushButton::clicked, this, &SettingsDialog::onRemoveVIPVideo);
    connect(btnModifyVIPVideo, &QPushButton::clicked, this, &SettingsDialog::onModifyVIPVideo);
    connect(btnImportVIPVideo, &QPushButton::clicked, this, &SettingsDialog::onImportVIPVideo);
    connect(btnExportVIPVideo, &QPushButton::clicked, this, &SettingsDialog::onExportVIPVideo);
    connect(tblVIPVideo, &QTableWidget::itemSelectionChanged, this, &SettingsDialog::onVIPVideoTableItemSelectionChanged);

    connect(btnBrowseYouGetPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYouGetPath);
    connect(btnBrowseYKDLPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYKDLPath);
    connect(btnBrowseYoutubeDLPath, &QPushButton::clicked, this, &SettingsDialog::onBrowseYoutubeDLPath);
    connect(btnBrowseAnniePath, &QPushButton::clicked, this, &SettingsDialog::onBrowseAnniePath);
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
        tblLiveTV->insertRow(0);
        QTableWidgetItem* name = new QTableWidgetItem(std::get<0>(tv));
        tblLiveTV->setItem(0, 0, name);
        QTableWidgetItem* url = new QTableWidgetItem(std::get<1>(tv));
        tblLiveTV->setItem(0, 1, url);
        QTableWidgetItem* availability = new QTableWidgetItem(std::get<2>(tv));
        tblLiveTV->setItem(0, 2, availability);
    }
}

void SettingsDialog::fillVIPVideoTable()
{
    for (const auto & vv : m_vipVideo)
    {
        tblVIPVideo->insertRow(0);
        QTableWidgetItem* name = new QTableWidgetItem(std::get<0>(vv));
        tblVIPVideo->setItem(0, 0, name);
        QTableWidgetItem* url = new QTableWidgetItem(std::get<1>(vv));
        tblVIPVideo->setItem(0, 1, url);
    }
}

void SettingsDialog::loadFromSettings()
{
    Config cfg;
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
    proxyType->setCurrentIndex(cfg.read<int>(QLatin1String("proxyType"), 0));
    proxyHostName->setText(cfg.read<QString>(QLatin1String("proxyHostName")));
    proxyPort->setValue(cfg.read<int>(QLatin1String("proxyPort"), 1080));
    proxyUserName->setText(cfg.read<QString>(QLatin1String("proxyUserName")));
    proxyPassword->setText(cfg.read<QString>(QLatin1String("proxyPassword")));

    // external player
    cfg.read("externalPlayers", m_players);
    for (const auto& p : m_players)
    {
        listPlayer->addItem(std::get<0>(p) + "\n" + std::get<1>(p));
    }

    // Live TV
    cfg.read("liveTV", m_liveTV);
    fillLiveTVTable();

    // VIP video
    cfg.read("vipVideo", m_vipVideo);
    fillVIPVideoTable();

    // resolver
    edtYouGetPath->setText(cfg.read<QString>(QLatin1String("you-get")));
    edtYKDLPath->setText(cfg.read<QString>(QLatin1String("ykdl")));
    edtYoutubeDLPath->setText(cfg.read<QString>(QLatin1String("youtube-dl")));
    edtAnniePath->setText(cfg.read<QString>(QLatin1String("annie")));
}

void SettingsDialog::saveToSettings()
{
    Config cfg;
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
    cfg.write(QLatin1String("httpAcceptLanguage"), httpAcceptLanguage->text());
    cfg.write(QLatin1String("faviconDownloadMode"), faviconDownloadMode->currentIndex());

    //Privacy
    int persistentCookiesPolicy = sessionCookiesCombo->currentIndex();
    cfg.write(QLatin1String("persistentCookiesPolicy"), persistentCookiesPolicy);

    QString pdataPath = persistentDataPath->text();
    cfg.write(QLatin1String("persistentDataPath"), pdataPath);

    // proxy
    cfg.write(QLatin1String("enableProxy"), proxySupport->isChecked());
    cfg.write(QLatin1String("proxyType"), proxyType->currentIndex());
    cfg.write(QLatin1String("proxyHostName"), proxyHostName->text());
    cfg.write(QLatin1String("proxyPort"), proxyPort->text());
    cfg.write(QLatin1String("proxyUserName"), proxyUserName->text());
    cfg.write(QLatin1String("proxyPassword"), proxyPassword->text());

    // resolver
    cfg.write(QLatin1String("you-get"), edtYouGetPath->text());
    cfg.write(QLatin1String("ykdl"), edtYKDLPath->text());
    cfg.write(QLatin1String("youtube-dl"), edtYoutubeDLPath->text());
    cfg.write(QLatin1String("annie"), edtAnniePath->text());

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

void SettingsDialog::onBrowseYouGetPath()
{
    QString path = QFileDialog::getOpenFileName(this,
                                 tr("Select you-get executable"),
                                                QString(),
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
                                                QString(),
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
                                                QString(),
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
                                                QString(),
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

void SettingsDialog::onAddLiveTVItem()
{
    if (edtLiveTVName->text().isEmpty() || edtLiveTVURL->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input Live TV name and URL."), QMessageBox::Ok);
        return;
    }
    auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(),
                           [this](const Tuple3& t) { return std::get<0>(t) == edtLiveTVName->text() && std::get<1>(t) == edtLiveTVURL->text();});
    if (m_liveTV.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This Live TV item exists already."), QMessageBox::Ok);
        return;
    }
    m_liveTV.push_back(std::make_tuple(edtLiveTVName->text(), edtLiveTVURL->text(), tr("unknown")));
    Config cfg;
    cfg.write("liveTV", m_liveTV);
    int rowIndex = tblLiveTV->rowCount();
    tblLiveTV->insertRow(rowIndex);
    QTableWidgetItem* name = new QTableWidgetItem(edtLiveTVName->text());
    tblLiveTV->setItem(rowIndex, 0, name);
    QTableWidgetItem* url = new QTableWidgetItem(edtLiveTVURL->text());
    tblLiveTV->setItem(rowIndex, 1, url);
    QTableWidgetItem* availability = new QTableWidgetItem(tr("unknown"));
    tblLiveTV->setItem(rowIndex, 2, availability);
    edtLiveTVName->setText("");
    edtLiveTVURL->setText("");
}

void SettingsDialog::onRemoveLiveTVItem()
{
    auto ranges = tblLiveTV->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_liveTV.removeAt(currentRow);
    Config cfg;
    cfg.write("liveTV", m_liveTV);
    tblLiveTV->removeRow(currentRow);
}

void SettingsDialog::onModifyLiveTVItem()
{
    auto ranges = tblLiveTV->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_liveTV[currentRow] = std::make_tuple(edtLiveTVName->text(), edtLiveTVURL->text(), std::get<2>(m_liveTV[currentRow]));
    auto item = tblLiveTV->takeItem(currentRow, 0);
    item->setText(edtLiveTVName->text());
    item = tblLiveTV->takeItem(currentRow, 1);
    item->setText(edtLiveTVURL->text());
    Config cfg;
    cfg.write("liveTV", m_liveTV);
}

void SettingsDialog::onImportLiveTVItems()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Select Live TV list to import"),
                                                      QString(),
                                                      tr("JSON format (*.json);;Plain text format (*.txt)")
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
        tblLiveTV->clearContents();
        fillLiveTVTable();
        Config cfg;
        cfg.write("liveTV", m_liveTV);
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

void SettingsDialog::onCheckLiveTVItems()
{

}

void SettingsDialog::onLiveTVTableItemSelectionChanged()
{
    edtLiveTVName->setText("");
    edtLiveTVURL->setText("");
    auto ranges = tblLiveTV->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    edtLiveTVName->setText(std::get<0>(m_liveTV[currentRow]));
    edtLiveTVURL->setText(std::get<1>(m_liveTV[currentRow]));
}

void SettingsDialog::onAddVIPVideo()
{
    if (edtVIPVideoName->text().isEmpty() || edtVIPVideoURL->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("Please input VIP video name and URL."), QMessageBox::Ok);
        return;
    }
    auto it = std::find_if(m_vipVideo.begin(), m_vipVideo.end(),
                           [this](const Tuple2& t) { return std::get<0>(t) == edtVIPVideoName->text() && std::get<1>(t) == edtVIPVideoURL->text();});
    if (m_vipVideo.end() != it)
    {
        QMessageBox::warning(this, tr("Duplicated"), tr("This VIP video item exists already."), QMessageBox::Ok);
        return;
    }
    m_vipVideo.push_back(std::make_tuple(edtVIPVideoName->text(), edtVIPVideoURL->text()));
    Config cfg;
    cfg.write("vipVideo", m_vipVideo);
    int rowIndex = tblVIPVideo->rowCount();
    tblVIPVideo->insertRow(rowIndex);
    QTableWidgetItem* name = new QTableWidgetItem(edtVIPVideoName->text());
    tblVIPVideo->setItem(rowIndex, 0, name);
    QTableWidgetItem* url = new QTableWidgetItem(edtVIPVideoURL->text());
    tblVIPVideo->setItem(rowIndex, 1, url);
    edtVIPVideoName->setText("");
    edtVIPVideoURL->setText("");
}

void SettingsDialog::onRemoveVIPVideo()
{
    auto ranges = tblVIPVideo->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_vipVideo.removeAt(currentRow);
    Config cfg;
    cfg.write("vipVideo", m_vipVideo);
    tblVIPVideo->removeRow(currentRow);
}

void SettingsDialog::onModifyVIPVideo()
{
    auto ranges = tblVIPVideo->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    m_vipVideo[currentRow] = std::make_tuple(edtVIPVideoName->text(), edtVIPVideoURL->text());
    auto item = tblVIPVideo->takeItem(currentRow, 0);
    item->setText(edtVIPVideoName->text());
    item = tblVIPVideo->takeItem(currentRow, 1);
    item->setText(edtVIPVideoURL->text());
    Config cfg;
    cfg.write("vipVideo", m_vipVideo);
}

void SettingsDialog::onImportVIPVideo()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,
                                                      tr("Select VIP video list to import"),
                                                      QString(),
                                                      tr("JSON format (*.json);;Plain text format (*.txt)")
                                                      );
    bool changed = false;
    for (const QString& path : paths)
    {
        QFileInfo fi(path);
        if (fi.suffix().toLower() == "json")
        {
            changed |= importVIPVideoAsJSON(path);
        }
        else
        {
            changed |= importVIPVideoAsPlainText(path);
        }
    }
    if (changed)
    {
        tblVIPVideo->clearContents();
        fillVIPVideoTable();
        Config cfg;
        cfg.write("vipVideo", m_vipVideo);
    }
}

void SettingsDialog::onExportVIPVideo()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Live tv list to"),
                                                    QString(),
                                                    tr("JSON format (*.json);;Plain text format (*.txt)")
                                                    );
    QFileInfo fi(fileName);
    if (fi.suffix().toLower() == "json")
    {
        exportVIPVideoAsJSON(fileName);
    }
    else
    {
        exportVIPVideoAsPlainText(fileName);
    }
}

void SettingsDialog::onVIPVideoTableItemSelectionChanged()
{
    edtVIPVideoName->setText("");
    edtVIPVideoURL->setText("");
    auto ranges = tblVIPVideo->selectedRanges();
    if (ranges.isEmpty())
        return;
    int currentRow = ranges.begin()->topRow();
    edtVIPVideoName->setText(std::get<0>(m_vipVideo[currentRow]));
    edtVIPVideoURL->setText(std::get<1>(m_vipVideo[currentRow]));
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
        if (doc.isArray())
        {
            auto arr = doc.array();
            for (auto a : arr)
            {
                if (!a.isObject())
                    continue;
                auto o = a.toObject();
                if (!o["name"].isString() || o["name"].toString().isEmpty() || !o["url"].isString() || o["url"].toString().isEmpty())
                    continue;

                auto vv = std::make_tuple(o["name"].toString(), o["url"].toString(), tr("unknown"));
                auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(), [&vv](const Tuple3& v){
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
            auto ele = line.split(' ');
            if (ele.length() != 2)
            continue;

            auto vv = std::make_tuple(QString(ele[0]), QString(ele[1]), tr("unknown"));
            auto it = std::find_if(m_liveTV.begin(), m_liveTV.end(), [&vv](const Tuple3& v){
                return std::get<0>(v) == std::get<0>(vv) && std::get<1>(v) == std::get<1>(vv);
            });
            if (m_liveTV.end() == it)
                m_liveTV.push_back(vv);
        }
    }
    return m_liveTV.length() != c;
}

void SettingsDialog::exportLiveTVAsJSON(const QString &path)
{
    QJsonDocument doc = QJsonDocument::fromJson("[]");
    QJsonArray a = doc.array();
    for (const auto& vv : m_liveTV)
    {
        QJsonObject o;
        o.insert("name", std::get<0>(vv));
        o.insert("url", std::get<1>(vv));
        a.append(o);
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
            f.write(std::get<0>(tv).toUtf8() + " " + std::get<1>(tv).toUtf8());
        }
        f.close();
    }
}

bool SettingsDialog::importVIPVideoAsJSON(const QString &path)
{
    QFile f(path);
    int c = m_vipVideo.length();
    if (f.open(QIODevice::ReadOnly))
    {
        auto d = f.readAll();
        f.close();
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(d, &error);
        if (error.error != QJsonParseError::NoError)
        {
            QMessageBox::warning(this, tr("Error on importing VIP video list as JSON"), error.errorString(), QMessageBox::Ok);
            return false;
        }
        if (doc.isArray())
        {
            auto arr = doc.array();
            for (auto a : arr)
            {
                if (!a.isObject())
                    continue;
                auto o = a.toObject();
                if (!o["name"].isString() || o["name"].toString().isEmpty() || !o["url"].isString() || o["url"].toString().isEmpty())
                    continue;

                auto vv = std::make_tuple(o["name"].toString(), o["url"].toString());
                auto it = std::find_if(m_vipVideo.begin(), m_vipVideo.end(), [&vv](const Tuple2& v){
                    return std::get<0>(v) == std::get<0>(vv) && std::get<1>(v) == std::get<1>(vv);
                });
                if (m_vipVideo.end() == it)
                    m_vipVideo.push_back(vv);
            }
        }
    }
    return m_vipVideo.length() != c;
}

bool SettingsDialog::importVIPVideoAsPlainText(const QString &path)
{
    QFile f(path);
    int c = m_vipVideo.length();
    if (f.open(QIODevice::ReadOnly))
    {
        auto d = f.readAll();
        f.close();
        auto lines = d.split('\n');
        for (const auto& line : lines)
        {
            auto ele = line.split(' ');
            if (ele.length() != 2)
                continue;
            auto vv = std::make_tuple(QString(ele[0]), QString(ele[1]));
            auto it = std::find_if(m_vipVideo.begin(), m_vipVideo.end(), [&vv](const Tuple2& v){
                return std::get<0>(v) == std::get<0>(vv) && std::get<1>(v) == std::get<1>(vv);
            });
            if (m_vipVideo.end() == it)
                m_vipVideo.push_back(vv);
        }
    }
    return m_liveTV.length() != c;
}

void SettingsDialog::exportVIPVideoAsJSON(const QString &path)
{
    QJsonDocument doc = QJsonDocument::fromJson("[]");
    QJsonArray a = doc.array();
    for (const auto& vv : m_vipVideo)
    {
        QJsonObject o;
        o.insert("name", std::get<0>(vv));
        o.insert("url", std::get<1>(vv));
        a.append(o);
    }
    auto d = doc.toJson(QJsonDocument::Indented);
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(d);
        f.close();
    }
}

void SettingsDialog::exportVIPVideoAsPlainText(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly))
    {
        for (const auto& vv : m_vipVideo)
        {
            f.write(std::get<0>(vv).toUtf8() + " " + std::get<1>(vv).toUtf8());
        }
        f.close();
    }
}

void SettingsDialog::setHomeToCurrentPage()
{
    BrowserWindow *mw = static_cast<BrowserWindow*>(parent());
    WebView *webView = mw->currentTab();
    if (webView)
        homeLineEdit->setText(webView->url().toString());
}
