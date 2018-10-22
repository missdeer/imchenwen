TEMPLATE = app
TARGET = imchenwenOtter
QT += core gui multimedia network printsupport qml svg widgets xmlpatterns
CONFIG += c++14
CONFIG -= qtquickcompiler

win32-*g++* {
    QT += webkit webkitwidgets
    DEFINES += OTTER_ENABLE_QTWEBKIT=1
    HEADERS += \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitCookieJar.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitFtpListingNetworkReply.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitHistoryInterface.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitInspector.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitNetworkManager.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPage.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPluginFactory.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPluginWidget.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitWebBackend.h \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitWebWidget.h \
        $$PWD/modules/backends/web/qtwebkit/3rdparty/qtftp/*.h

    SOURCES += \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitCookieJar.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitFtpListingNetworkReply.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitHistoryInterface.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitInspector.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitNetworkManager.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPage.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPluginFactory.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitPluginWidget.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitWebBackend.cpp \
        $$PWD/modules/backends/web/qtwebkit/QtWebKitWebWidget.cpp \
        $$PWD/modules/backends/web/qtwebkit/3rdparty/qtftp/*.cpp

    RESOURCES += $$PWD/modules/backends/web/qtwebkit/QtWebKitResources.qrc
} else {
    QT += webengine webenginecore webenginewidgets
    DEFINES += OTTER_ENABLE_QTWEBENGINE=1
    HEADERS += $$PWD/modules/backends/web/qtwebengine/*.h
    SOURCES += $$PWD/modules/backends/web/qtwebengine/*.cpp
    RESOURCES += $$PWD/modules/backends/web/qtwebengine/QtWebEngineResources.qrc
}

INCLUDEPATH += $$PWD/../3rdparty/columnresizer $$PWD/../3rdparty/mousegestures

HEADERS += $$PWD/core/*.h \
        $$PWD/ui/*.h \
        $$PWD/ui/preferences/*.h \
        $$PWD/modules/backends/passwords/file/*.h \
        $$PWD/modules/importers/html/*.h \
        $$PWD/modules/importers/opera/*.h \
        $$PWD/modules/importers/opml/*.h \
        $$PWD/modules/widgets/action/*.h \
        $$PWD/modules/widgets/address/*.h \
        $$PWD/modules/widgets/bookmark/*.h \
        $$PWD/modules/widgets/configurationOption/*.h \
        $$PWD/modules/widgets/contentBlockingInformation/*.h \
        $$PWD/modules/widgets/errorConsole/*.h \
        $$PWD/modules/widgets/menuButton/*.h \
        $$PWD/modules/widgets/panelChooser/*.h \
        $$PWD/modules/widgets/privateWindowIndicator/*.h \
        $$PWD/modules/widgets/progressInformation/*.h \
        $$PWD/modules/widgets/search/*.h \
        $$PWD/modules/widgets/statusMessage/*.h \
        $$PWD/modules/widgets/transfers/*.h \
        $$PWD/modules/widgets/zoom/*.h \
        $$PWD/modules/windows/addons/*.h \
        $$PWD/modules/windows/bookmarks/*.h \
        $$PWD/modules/windows/cache/*.h \
        $$PWD/modules/windows/configuration/*.h \
        $$PWD/modules/windows/cookies/*.h \
        $$PWD/modules/windows/history/*.h \
        $$PWD/modules/windows/feeds/*.h \
        $$PWD/modules/windows/links/*.h \
        $$PWD/modules/windows/notes/*.h \
        $$PWD/modules/windows/pageInformation/*.h \
        $$PWD/modules/windows/passwords/*.h \
        $$PWD/modules/windows/preferences/*.h \
        $$PWD/modules/windows/tabHistory/*.h \
        $$PWD/modules/windows/transfers/*.h \
        $$PWD/modules/windows/web/*.h \
        $$PWD/modules/windows/windows/*.h \
        $$PWD/../3rdparty/columnresizer/ColumnResizer.h \
        $$PWD/../3rdparty/mousegestures/MouseGestures.h

SOURCES += $$PWD/main.cpp \
        $$PWD/core/ActionExecutor.cpp \
        $$PWD/core/ActionsManager.cpp \
        $$PWD/core/AdblockContentFiltersProfile.cpp \
        $$PWD/core/AddonsManager.cpp \
        $$PWD/core/AddressCompletionModel.cpp \
        $$PWD/core/Application.cpp \
        $$PWD/core/BookmarksImporter.cpp \
        $$PWD/core/BookmarksManager.cpp \
        $$PWD/core/BookmarksModel.cpp \
        $$PWD/core/ContentFiltersManager.cpp \
        $$PWD/core/Console.cpp \
        $$PWD/core/CookieJar.cpp \
        $$PWD/core/FeedParser.cpp \
        $$PWD/core/FeedsManager.cpp \
        $$PWD/core/FeedsModel.cpp \
        $$PWD/core/GesturesManager.cpp \
        $$PWD/core/HandlersManager.cpp \
        $$PWD/core/HistoryManager.cpp \
        $$PWD/core/HistoryModel.cpp \
        $$PWD/core/Importer.cpp \
        $$PWD/core/IniSettings.cpp \
        $$PWD/core/InputInterpreter.cpp \
        $$PWD/core/ItemModel.cpp \
        $$PWD/core/Job.cpp \
        $$PWD/core/JsonSettings.cpp \
        $$PWD/core/LocalListingNetworkReply.cpp \
        $$PWD/core/LongTermTimer.cpp \
        $$PWD/core/Migrator.cpp \
        $$PWD/core/NetworkAutomaticProxy.cpp \
        $$PWD/core/NetworkCache.cpp \
        $$PWD/core/NetworkManager.cpp \
        $$PWD/core/NetworkManagerFactory.cpp \
        $$PWD/core/NetworkProxyFactory.cpp \
        $$PWD/core/NotesManager.cpp \
        $$PWD/core/NotificationsManager.cpp \
        $$PWD/core/PasswordsManager.cpp \
        $$PWD/core/PasswordsStorageBackend.cpp \
        $$PWD/core/PlatformIntegration.cpp \
        $$PWD/core/SearchEnginesManager.cpp \
        $$PWD/core/SearchSuggester.cpp \
        $$PWD/core/SessionModel.cpp \
        $$PWD/core/SessionsManager.cpp \
        $$PWD/core/SettingsManager.cpp \
        $$PWD/core/SpellCheckManager.cpp \
        $$PWD/core/ThemesManager.cpp \
        $$PWD/core/ToolBarsManager.cpp \
        $$PWD/core/TransfersManager.cpp \
        $$PWD/core/UpdateChecker.cpp \
        $$PWD/core/Updater.cpp \
        $$PWD/core/UserScript.cpp \
        $$PWD/core/Utils.cpp \
        $$PWD/core/WebBackend.cpp \
        $$PWD/ui/AcceptCookieDialog.cpp \
        $$PWD/ui/Action.cpp \
        $$PWD/ui/ActionComboBoxWidget.cpp \
        $$PWD/ui/Animation.cpp \
        $$PWD/ui/ApplicationComboBoxWidget.cpp \
        $$PWD/ui/AuthenticationDialog.cpp \
        $$PWD/ui/BookmarkPropertiesDialog.cpp \
        $$PWD/ui/BookmarksComboBoxWidget.cpp \
        $$PWD/ui/BookmarksImporterWidget.cpp \
        $$PWD/ui/CertificateDialog.cpp \
        $$PWD/ui/ClearHistoryDialog.cpp \
        $$PWD/ui/ColorWidget.cpp \
        $$PWD/ui/ComboBoxWidget.cpp \
        $$PWD/ui/ContentsDialog.cpp \
        $$PWD/ui/ContentsWidget.cpp \
        $$PWD/ui/CookiePropertiesDialog.cpp \
        $$PWD/ui/Dialog.cpp \
        $$PWD/ui/FeedPropertiesDialog.cpp \
        $$PWD/ui/FeedsComboBoxWidget.cpp \
        $$PWD/ui/FilePathWidget.cpp \
        $$PWD/ui/IconWidget.cpp \
        $$PWD/ui/ImagePropertiesDialog.cpp \
        $$PWD/ui/ImportDialog.cpp \
        $$PWD/ui/ItemDelegate.cpp \
        $$PWD/ui/ItemViewWidget.cpp \
        $$PWD/ui/LineEditWidget.cpp \
        $$PWD/ui/LocaleDialog.cpp \
        $$PWD/ui/MainWindow.cpp \
        $$PWD/ui/MasterPasswordDialog.cpp \
        $$PWD/ui/Menu.cpp \
        $$PWD/ui/MenuBarWidget.cpp \
        $$PWD/ui/NotificationDialog.cpp \
        $$PWD/ui/OpenAddressDialog.cpp \
        $$PWD/ui/OpenBookmarkDialog.cpp \
        $$PWD/ui/OptionWidget.cpp \
        $$PWD/ui/PreferencesDialog.cpp \
        $$PWD/ui/PreviewWidget.cpp \
        $$PWD/ui/ProgressBarWidget.cpp \
        $$PWD/ui/ProxyModel.cpp \
        $$PWD/ui/ReloadTimeDialog.cpp \
        $$PWD/ui/ReportDialog.cpp \
        $$PWD/ui/ResizerWidget.cpp \
        $$PWD/ui/SaveSessionDialog.cpp \
        $$PWD/ui/SearchEnginePropertiesDialog.cpp \
        $$PWD/ui/SessionsManagerDialog.cpp \
        $$PWD/ui/SidebarWidget.cpp \
        $$PWD/ui/SourceViewerWebWidget.cpp \
        $$PWD/ui/SourceViewerWidget.cpp \
        $$PWD/ui/SplitterWidget.cpp \
        $$PWD/ui/StartupDialog.cpp \
        $$PWD/ui/StatusBarWidget.cpp \
        $$PWD/ui/Style.cpp \
        $$PWD/ui/TabBarWidget.cpp \
        $$PWD/ui/TabSwitcherWidget.cpp \
        $$PWD/ui/TextBrowserWidget.cpp \
        $$PWD/ui/TextEditWidget.cpp \
        $$PWD/ui/TextLabelWidget.cpp \
        $$PWD/ui/ToolBarDialog.cpp \
        $$PWD/ui/ToolBarDropZoneWidget.cpp \
        $$PWD/ui/ToolBarWidget.cpp \
        $$PWD/ui/ToolButtonWidget.cpp \
        $$PWD/ui/TransferDialog.cpp \
        $$PWD/ui/TrayIcon.cpp \
        $$PWD/ui/UpdateCheckerDialog.cpp \
        $$PWD/ui/WebsiteInformationDialog.cpp \
        $$PWD/ui/WebsitePreferencesDialog.cpp \
        $$PWD/ui/WebWidget.cpp \
        $$PWD/ui/WidgetFactory.cpp \
        $$PWD/ui/Window.cpp \
        $$PWD/ui/WorkspaceWidget.cpp \
        $$PWD/ui/preferences/AcceptLanguageDialog.cpp \
        $$PWD/ui/preferences/ContentBlockingDialog.cpp \
        $$PWD/ui/preferences/ContentBlockingProfileDialog.cpp \
        $$PWD/ui/preferences/CookiesExceptionsDialog.cpp \
        $$PWD/ui/preferences/JavaScriptPreferencesDialog.cpp \
        $$PWD/ui/preferences/KeyboardProfileDialog.cpp \
        $$PWD/ui/preferences/MouseProfileDialog.cpp \
        $$PWD/ui/preferences/PreferencesAdvancedPageWidget.cpp \
        $$PWD/ui/preferences/PreferencesContentPageWidget.cpp \
        $$PWD/ui/preferences/PreferencesGeneralPageWidget.cpp \
        $$PWD/ui/preferences/PreferencesPrivacyPageWidget.cpp \
        $$PWD/ui/preferences/PreferencesSearchPageWidget.cpp \
        $$PWD/ui/preferences/ProxyPropertiesDialog.cpp \
        $$PWD/ui/preferences/UserAgentPropertiesDialog.cpp \
        $$PWD/modules/backends/passwords/file/FilePasswordsStorageBackend.cpp \
        $$PWD/modules/importers/html/HtmlBookmarksImporter.cpp \
        $$PWD/modules/importers/opera/OperaBookmarksImporter.cpp \
        $$PWD/modules/importers/opera/OperaNotesImporter.cpp \
        $$PWD/modules/importers/opera/OperaSearchEnginesImporter.cpp \
        $$PWD/modules/importers/opera/OperaSessionImporter.cpp \
        $$PWD/modules/importers/opml/OpmlImporter.cpp \
        $$PWD/modules/importers/opml/OpmlImporterWidget.cpp \
        $$PWD/modules/widgets/action/ActionWidget.cpp \
        $$PWD/modules/widgets/address/AddressWidget.cpp \
        $$PWD/modules/widgets/bookmark/BookmarkWidget.cpp \
        $$PWD/modules/widgets/configurationOption/ConfigurationOptionWidget.cpp \
        $$PWD/modules/widgets/contentBlockingInformation/ContentBlockingInformationWidget.cpp \
        $$PWD/modules/widgets/errorConsole/ErrorConsoleWidget.cpp \
        $$PWD/modules/widgets/menuButton/MenuButtonWidget.cpp \
        $$PWD/modules/widgets/panelChooser/PanelChooserWidget.cpp \
        $$PWD/modules/widgets/privateWindowIndicator/PrivateWindowIndicatorWidget.cpp \
        $$PWD/modules/widgets/progressInformation/ProgressInformationWidget.cpp \
        $$PWD/modules/widgets/search/SearchWidget.cpp \
        $$PWD/modules/widgets/statusMessage/StatusMessageWidget.cpp \
        $$PWD/modules/widgets/transfers/TransfersWidget.cpp \
        $$PWD/modules/widgets/zoom/ZoomWidget.cpp \
        $$PWD/modules/windows/addons/AddonsContentsWidget.cpp \
        $$PWD/modules/windows/bookmarks/BookmarksContentsWidget.cpp \
        $$PWD/modules/windows/cache/CacheContentsWidget.cpp \
        $$PWD/modules/windows/configuration/ConfigurationContentsWidget.cpp \
        $$PWD/modules/windows/cookies/CookiesContentsWidget.cpp \
        $$PWD/modules/windows/history/HistoryContentsWidget.cpp \
        $$PWD/modules/windows/feeds/FeedsContentsWidget.cpp \
        $$PWD/modules/windows/links/LinksContentsWidget.cpp \
        $$PWD/modules/windows/notes/NotesContentsWidget.cpp \
        $$PWD/modules/windows/pageInformation/PageInformationContentsWidget.cpp \
        $$PWD/modules/windows/passwords/PasswordsContentsWidget.cpp \
        $$PWD/modules/windows/preferences/PreferencesContentsWidget.cpp \
        $$PWD/modules/windows/tabHistory/TabHistoryContentsWidget.cpp \
        $$PWD/modules/windows/transfers/TransfersContentsWidget.cpp \
        $$PWD/modules/windows/web/PasswordBarWidget.cpp \
        $$PWD/modules/windows/web/PermissionBarWidget.cpp \
        $$PWD/modules/windows/web/PopupsBarWidget.cpp \
        $$PWD/modules/windows/web/ProgressToolBarWidget.cpp \
        $$PWD/modules/windows/web/SearchBarWidget.cpp \
        $$PWD/modules/windows/web/SelectPasswordDialog.cpp \
        $$PWD/modules/windows/web/StartPageModel.cpp \
        $$PWD/modules/windows/web/StartPagePreferencesDialog.cpp \
        $$PWD/modules/windows/web/StartPageWidget.cpp \
        $$PWD/modules/windows/web/WebContentsWidget.cpp \
        $$PWD/modules/windows/windows/WindowsContentsWidget.cpp \
        $$PWD/../3rdparty/columnresizer/ColumnResizer.cpp \
        $$PWD/../3rdparty/mousegestures/MouseGestures.cpp

FORMS += \
    $$PWD/ui/AcceptCookieDialog.ui \
        $$PWD/ui/AuthenticationDialog.ui \
        $$PWD/ui/BookmarkPropertiesDialog.ui \
        $$PWD/ui/BookmarksImporterWidget.ui \
        $$PWD/ui/CertificateDialog.ui \
        $$PWD/ui/ClearHistoryDialog.ui \
        $$PWD/ui/CookiePropertiesDialog.ui \
        $$PWD/ui/FeedPropertiesDialog.ui \
        $$PWD/ui/ImagePropertiesDialog.ui \
        $$PWD/ui/ImportDialog.ui \
        $$PWD/ui/LocaleDialog.ui \
        $$PWD/ui/MainWindow.ui \
        $$PWD/ui/MasterPasswordDialog.ui \
        $$PWD/ui/OpenAddressDialog.ui \
        $$PWD/ui/OpenBookmarkDialog.ui \
        $$PWD/ui/PreferencesDialog.ui \
        $$PWD/ui/ReloadTimeDialog.ui \
        $$PWD/ui/ReportDialog.ui \
        $$PWD/ui/SaveSessionDialog.ui \
        $$PWD/ui/SearchEnginePropertiesDialog.ui \
        $$PWD/ui/SessionsManagerDialog.ui \
        $$PWD/ui/SidebarWidget.ui \
        $$PWD/ui/StartupDialog.ui \
        $$PWD/ui/ToolBarDialog.ui \
        $$PWD/ui/TransferDialog.ui \
        $$PWD/ui/UpdateCheckerDialog.ui \
        $$PWD/ui/WebsiteInformationDialog.ui \
        $$PWD/ui/WebsitePreferencesDialog.ui \
        $$PWD/ui/preferences/AcceptLanguageDialog.ui \
        $$PWD/ui/preferences/ContentBlockingDialog.ui \
        $$PWD/ui/preferences/ContentBlockingProfileDialog.ui \
        $$PWD/ui/preferences/CookiesExceptionsDialog.ui \
        $$PWD/ui/preferences/JavaScriptPreferencesDialog.ui \
        $$PWD/ui/preferences/KeyboardProfileDialog.ui \
        $$PWD/ui/preferences/MouseProfileDialog.ui \
        $$PWD/ui/preferences/PreferencesAdvancedPageWidget.ui \
        $$PWD/ui/preferences/PreferencesContentPageWidget.ui \
        $$PWD/ui/preferences/PreferencesGeneralPageWidget.ui \
        $$PWD/ui/preferences/PreferencesPrivacyPageWidget.ui \
        $$PWD/ui/preferences/PreferencesSearchPageWidget.ui \
        $$PWD/ui/preferences/ProxyPropertiesDialog.ui \
        $$PWD/ui/preferences/UserAgentPropertiesDialog.ui \
        $$PWD/modules/importers/opml/OpmlImporterWidget.ui \
        $$PWD/modules/widgets/errorConsole/ErrorConsoleWidget.ui \
        $$PWD/modules/windows/addons/AddonsContentsWidget.ui \
        $$PWD/modules/windows/bookmarks/BookmarksContentsWidget.ui \
        $$PWD/modules/windows/cache/CacheContentsWidget.ui \
        $$PWD/modules/windows/configuration/ConfigurationContentsWidget.ui \
        $$PWD/modules/windows/cookies/CookiesContentsWidget.ui \
        $$PWD/modules/windows/feeds/FeedsContentsWidget.ui \
        $$PWD/modules/windows/history/HistoryContentsWidget.ui \
        $$PWD/modules/windows/links/LinksContentsWidget.ui \
        $$PWD/modules/windows/notes/NotesContentsWidget.ui \
        $$PWD/modules/windows/pageInformation/PageInformationContentsWidget.ui \
        $$PWD/modules/windows/passwords/PasswordsContentsWidget.ui \
        $$PWD/modules/windows/preferences/PreferencesContentsWidget.ui \
        $$PWD/modules/windows/tabHistory/TabHistoryContentsWidget.ui \
        $$PWD/modules/windows/transfers/TransfersContentsWidget.ui \
        $$PWD/modules/windows/web/PasswordBarWidget.ui \
        $$PWD/modules/windows/web/PermissionBarWidget.ui \
        $$PWD/modules/windows/web/PopupsBarWidget.ui \
        $$PWD/modules/windows/web/SearchBarWidget.ui \
        $$PWD/modules/windows/web/SelectPasswordDialog.ui \
        $$PWD/modules/windows/web/StartPagePreferencesDialog.ui \
        $$PWD/modules/windows/windows/WindowsContentsWidget.ui

RESOURCES += $$PWD/resources/resources.qrc


RC_FILE = otter-browser.rc

macx: {
    QT += macextras
    ICON = resources/icons/otter-browser.icns
    icon.files += res/imchenwen128.png
    LIBS += -framework Foundation -framework Cocoa
    HEADERS += \
            $$PWD/modules/platforms/mac/*.h
    SOURCES += \
            $$PWD/modules/platforms/mac/MacPlatformIntegration.mm \
            $$PWD/modules/platforms/mac/MacPlatformStyle.cpp
    CONFIG(release, debug|release) : {
        QMAKE_INFO_PLIST = osxInfo.plist
        MACDEPLOYQT = $$[QT_INSTALL_BINS]/macdeployqt

        deploy.commands += $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app\" -appstore-compliant

        deploy_webengine.depends += deploy
        deploy_webengine.commands += # $$MACDEPLOYQT \"$${OUT_PWD}/$${TARGET}.app/Contents/Frameworks/QtWebEngineCore.framework/Helpers/QtWebEngineProcess.app\" -appstore-compliant

        APPCERT = Developer ID Application: Fan Yang (Y73SBCN2CG)
        INSTALLERCERT = 3rd Party Mac Developer Installer: Fan Yang (Y73SBCN2CG)
        BUNDLEID = com.dfordsoft.imchenwen

        codesign.depends += deploy_webengine deploy_parsed
        codesign.commands = codesign -s \"$${APPCERT}\" -v -f --timestamp=none --deep \"$${OUT_PWD}/$${TARGET}.app\"

        makedmg.depends += codesign
        makedmg.commands = hdiutil create -srcfolder \"$${TARGET}.app\" -volname \"$${TARGET}\" -format UDBZ \"$${TARGET}.dmg\" -ov -scrub -stretch 2g

        QMAKE_EXTRA_TARGETS += deploy deploy_webengine codesign makedmg
    }
}

win32: {
        QT += winextras
        DEFINES += _WIN32_WINNT=0x0600 BOOST_ALL_NO_LIB=1
        HEADERS += \
                $$PWD/modules/platforms/windows/*.h
        SOURCES += \
                $$PWD/modules/platforms/windows/WindowsPlatformIntegration.cpp \
                $$PWD/modules/platforms/windows/WindowsPlatformStyle.cpp

        LIBS += -lole32 -lshell32 -ladvapi32 -luser32
        CONFIG(release, debug|release) : {
        win32-*msvc* {
                QMAKE_CXXFLAGS += /Zi
                QMAKE_LFLAGS += /INCREMENTAL:NO /Debug
        }
        WINDEPLOYQT = $$[QT_INSTALL_BINS]/windeployqt.exe
    }
}
