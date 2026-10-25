/**
 * Copyright 2022 naracanto <https://naracanto.github.io>.
 *
 * This file is part of QTabelo <https://github.com/beletalabs/qtabelo>.
 *
 * QTabelo is an open source table editor written in C++ using the
 * Qt framework.
 *
 * QTabelo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * QTabelo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QTabelo.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "application_window.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaEnum>
#include <QScreen>
#include <QSettings>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QUrl>

#include "about_dialog.h"
#include "colophon_dialog.h"
#include "confirmation_dialog.h"
#include "document_manager.h"
#include "document_widget.h"
#include "document_window.h"
#include "preferences_dialog.h"
#include "properties_dialog.h"
#include "recent_document_list.h"
#include "table_info_provider.h"

#include <QDockWidget>
#include <QQuickWidget>
#include <QQmlContext>

ApplicationWindow::ApplicationWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_documentManager{new DocumentManager}
    , m_recentDocuments(new RecentDocumentList(this))
{
    setWindowIcon(QIcon(QStringLiteral(":/icons/apps/16/tabelo.svg")));

    m_documentManager->setViewMode(DocumentManager::TabbedView);
    m_documentManager->setDocumentMode(true);
    m_documentManager->setTabsClosable(true);
    m_documentManager->setTabsMovable(true);
    setCentralWidget(m_documentManager);


    // Создаём поставщик данных
    m_tableInfoProvider = new TableInfoProvider(this);

    // Создаём QQuickWidget
    m_infoQuickWidget = new QQuickWidget;
    m_infoQuickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    // Регистрируем объект в QML-контексте (ВАЖНО: до setSource!)
    m_infoQuickWidget->rootContext()->setContextProperty("tableInfo", m_tableInfoProvider);

    // Загружаем QML-файл
    m_infoQuickWidget->setSource(QUrl(QStringLiteral("qrc:/TableInfo.qml")));

    // Создаём док-виджет и размещаем в нём QQuickWidget
    m_infoDock = new QDockWidget(tr("Table Info"), this);
    m_infoDock->setObjectName(QStringLiteral("dockTableInfo"));
    m_infoDock->setWidget(m_infoQuickWidget);
    m_infoDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    addDockWidget(Qt::RightDockWidgetArea, m_infoDock);


    connect(m_documentManager, &DocumentManager::subWindowActivated, this, &ApplicationWindow::documentActivated);

    connect(m_recentDocuments, &RecentDocumentList::listChanged, this, &ApplicationWindow::updateMenuOpenRecent);
    connect(m_recentDocuments, &RecentDocumentList::documentSelected, this, &ApplicationWindow::openDocument);

    setupActions();
    loadSettings();

    m_documentManager->initTabVisible();
    m_documentManager->initTabPosition();
    m_documentManager->initTabAutoHide();

    documentActivated(nullptr);
    documentClosed();
}

ApplicationWindow::~ApplicationWindow()
{

}


void ApplicationWindow::setupActions()
{
    //
    // Application

    m_actionAbout = new QAction(tr("&About %1").arg(QApplication::applicationName()), this);
    m_actionAbout->setObjectName(QStringLiteral("actionAbout"));
    m_actionAbout->setIcon(QIcon(QStringLiteral(":/icons/apps/16/tabelo.svg")));
    m_actionAbout->setIconText(tr("About"));
    m_actionAbout->setToolTip(tr("Brief description of the application"));
    m_actionAbout->setMenuRole(QAction::AboutRole);
    connect(m_actionAbout, &QAction::triggered, this, &ApplicationWindow::slotAbout);

    m_actionColophon = new QAction(tr("&Colophon"), this);
    m_actionColophon->setObjectName(QStringLiteral("actionColophon"));
    m_actionColophon->setIcon(QIcon::fromTheme(QStringLiteral("help-about"), QIcon(QStringLiteral(":/icons/actions/16/help-about.svg"))));
    m_actionColophon->setToolTip(tr("Lengthy description of the application"));
    m_actionColophon->setMenuRole(QAction::ApplicationSpecificRole);
    connect(m_actionColophon, &QAction::triggered, this, &ApplicationWindow::slotColophon);

    m_actionPreferences = new QAction(tr("&Preferences..."), this);
    m_actionPreferences->setObjectName(QStringLiteral("actionPreferences"));
    m_actionPreferences->setIcon(QIcon::fromTheme(QStringLiteral("configure"), QIcon(QStringLiteral(":/icons/actions/16/configure.svg"))));
    m_actionPreferences->setToolTip(tr("Customize the appearance and behavior of the application"));
    m_actionPreferences->setMenuRole(QAction::PreferencesRole);
    connect(m_actionPreferences, &QAction::triggered, this, &ApplicationWindow::slotPreferences);

    m_actionQuit = new QAction(tr("&Quit"), this);
    m_actionQuit->setObjectName(QStringLiteral("actionQuit"));
    m_actionQuit->setIcon(QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/icons/actions/16/application-exit.svg"))));
    m_actionQuit->setShortcut(QKeySequence::Quit);
    m_actionQuit->setToolTip(tr("Quit the application"));
    m_actionQuit->setMenuRole(QAction::QuitRole);
    connect(m_actionQuit, &QAction::triggered, this, &ApplicationWindow::close);
    addAction(m_actionQuit);

    auto *menuApplication = menuBar()->addMenu(tr("&Application"));
    menuApplication->setObjectName(QStringLiteral("menuApplication"));
    menuApplication->addAction(m_actionAbout);
    menuApplication->addAction(m_actionColophon);
    menuApplication->addSeparator();
    menuApplication->addAction(m_actionPreferences);
    menuApplication->addSeparator();
    menuApplication->addAction(m_actionQuit);

    m_toolbarApplication = addToolBar(tr("Application Toolbar"));
    m_toolbarApplication->setObjectName(QStringLiteral("toolbarApplication"));
    m_toolbarApplication->addAction(m_actionAbout);
    m_toolbarApplication->addAction(m_actionPreferences);
    m_toolbarApplication->addSeparator();
    m_toolbarApplication->addAction(m_actionQuit);


    //
    // File

    m_actionNew = new QAction(tr("&New"), this);
    m_actionNew->setObjectName(QStringLiteral("actionNew"));
    m_actionNew->setIcon(QIcon::fromTheme(QStringLiteral("document-new"), QIcon(QStringLiteral(":/icons/actions/16/document-new.svg"))));
    m_actionNew->setShortcut(QKeySequence::New);
    m_actionNew->setToolTip(tr("Create new document"));
    connect(m_actionNew, &QAction::triggered, this, &ApplicationWindow::slotNew);
    addAction(m_actionNew);

    m_actionOpen = new QAction(tr("&Open..."), this);
    m_actionOpen->setObjectName(QStringLiteral("actionOpen"));
    m_actionOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open"), QIcon(QStringLiteral(":/icons/actions/16/document-open.svg"))));
    m_actionOpen->setShortcut(QKeySequence::Open);
    m_actionOpen->setToolTip(tr("Open an existing document"));
    connect(m_actionOpen, &QAction::triggered, this, &ApplicationWindow::slotOpen);
    addAction(m_actionOpen);

    m_actionOpenRecentClear = new QAction(tr("Clear &List"), this);
    m_actionOpenRecentClear->setObjectName(QStringLiteral("actionOpenRecentClear"));
    m_actionOpenRecentClear->setToolTip(tr("Clear document list"));
    connect(m_actionOpenRecentClear, &QAction::triggered, m_recentDocuments, &RecentDocumentList::clear);

    m_actionSave = new QAction(tr("&Save"), this);
    m_actionSave->setObjectName(QStringLiteral("actionSave"));
    m_actionSave->setIcon(QIcon::fromTheme(QStringLiteral("document-save"), QIcon(QStringLiteral(":/icons/actions/16/document-save.svg"))));
    m_actionSave->setShortcut(QKeySequence::Save);
    m_actionSave->setToolTip(tr("Save document"));
    connect(m_actionSave, &QAction::triggered, this, &ApplicationWindow::slotSave);
    addAction(m_actionSave);

    m_actionSaveAs = new QAction(tr("Save &As..."), this);
    m_actionSaveAs->setObjectName(QStringLiteral("actionSaveAs"));
    m_actionSaveAs->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as"), QIcon(QStringLiteral(":/icons/actions/16/document-save-as.svg"))));
    m_actionSaveAs->setShortcut(QKeySequence::SaveAs);
    m_actionSaveAs->setToolTip(tr("Save document under a new name"));
    connect(m_actionSaveAs, &QAction::triggered, this, &ApplicationWindow::slotSaveAs);
    addAction(m_actionSaveAs);

    m_actionSaveCopyAs = new QAction(tr("Save &Copy As..."), this);
    m_actionSaveCopyAs->setObjectName(QStringLiteral("actionSaveCopyAs"));
    m_actionSaveCopyAs->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as"), QIcon(QStringLiteral(":/icons/actions/16/document-save-as.svg"))));
    m_actionSaveCopyAs->setToolTip(tr("Save copy of document under a new name"));
    connect(m_actionSaveCopyAs, &QAction::triggered, this, &ApplicationWindow::slotSaveCopyAs);

    m_actionSaveAll = new QAction(tr("Save A&ll"), this);
    m_actionSaveAll->setObjectName(QStringLiteral("actionSaveAll"));
    m_actionSaveAll->setIcon(QIcon::fromTheme(QStringLiteral("document-save-all"), QIcon(QStringLiteral(":/icons/actions/16/document-save-all.svg"))));
    m_actionSaveAll->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
    m_actionSaveAll->setToolTip(tr("Save all open documents"));
    connect(m_actionSaveAll, &QAction::triggered, this, &ApplicationWindow::slotSaveAll);
    addAction(m_actionSaveAll);

    m_actionCopyPath = new QAction(tr("Cop&y Path"), this);
    m_actionCopyPath->setObjectName(QStringLiteral("actionCopyPath"));
    m_actionCopyPath->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-path"), QIcon(QStringLiteral(":/icons/actions/16/edit-copy-path.svg"))));
    m_actionCopyPath->setToolTip(tr("Copy document path to clipboard"));
    connect(m_actionCopyPath, &QAction::triggered, this, &ApplicationWindow::slotCopyPath);

    m_actionCopyFilename = new QAction(tr("Copy &Filename"), this);
    m_actionCopyFilename->setObjectName(QStringLiteral("actionCopyFilename"));
    m_actionCopyFilename->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy-path"), QIcon(QStringLiteral(":/icons/actions/16/edit-copy-path.svg"))));
    m_actionCopyFilename->setToolTip(tr("Copy document filename to clipboard"));
    connect(m_actionCopyFilename, &QAction::triggered, this, &ApplicationWindow::slotCopyFilename);

    m_actionRenameFilename = new QAction(tr("Re&name..."), this);
    m_actionRenameFilename->setObjectName(QStringLiteral("actionRenameFilename"));
    m_actionRenameFilename->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename"), QIcon(QStringLiteral(":/icons/actions/16/edit-rename.svg"))));
    m_actionRenameFilename->setToolTip(tr("Rename file name of the document"));
    connect(m_actionRenameFilename, &QAction::triggered, this, &ApplicationWindow::slotRenameFilename);

    m_actionProperties = new QAction(tr("Proper&ties"), this);
    m_actionProperties->setObjectName(QStringLiteral("actionProperties"));
    m_actionProperties->setIcon(QIcon::fromTheme(QStringLiteral("document-properties"), QIcon(QStringLiteral(":/icons/actions/16/document-properties.svg"))));
    m_actionProperties->setToolTip(QStringLiteral("Document properties"));
    connect(m_actionProperties, &QAction::triggered, this, &ApplicationWindow::slotProperties);

    m_actionClose = new QAction(tr("&Close"), this);
    m_actionClose->setObjectName(QStringLiteral("actionClose"));
    m_actionClose->setIcon(QIcon::fromTheme(QStringLiteral("document-close"), QIcon(QStringLiteral(":/icons/actions/16/document-close.svg"))));
    m_actionClose->setShortcut(QKeySequence::Close);
    m_actionClose->setToolTip(tr("Close document"));
    connect(m_actionClose, &QAction::triggered, m_documentManager, &DocumentManager::closeActiveSubWindow);
    addAction(m_actionClose);

    m_actionCloseOther = new QAction(tr("Close Ot&her"), this);
    m_actionCloseOther->setObjectName(QStringLiteral("actionCloseOther"));
    m_actionCloseOther->setToolTip(tr("Close other open documents"));
    connect(m_actionCloseOther, &QAction::triggered, this, &ApplicationWindow::slotCloseOther);

    m_actionCloseAll = new QAction(tr("Clos&e All"), this);
    m_actionCloseAll->setObjectName(QStringLiteral("actionCloseAll"));
    m_actionCloseAll->setToolTip(tr("Close all open documents"));
    connect(m_actionCloseAll, &QAction::triggered, this, &ApplicationWindow::slotCloseAll);

    m_menuOpenRecent = new QMenu(tr("Open Recent"), this);
    m_menuOpenRecent->setObjectName(QStringLiteral("menuOpenRecent"));
    m_menuOpenRecent->setIcon(QIcon::fromTheme(QStringLiteral("document-open-recent"), QIcon(QStringLiteral(":/icons/actions/16/document-open-recent.svg"))));
//    m_menuOpenRecent->setToolTip(tr("Open a document which was recently opened"));
    updateMenuOpenRecent();

    auto *menuDocument = menuBar()->addMenu(tr("&Document"));
    menuDocument->setObjectName(QStringLiteral("menuDocument"));
    menuDocument->addAction(m_actionNew);
    menuDocument->addSeparator();
    menuDocument->addAction(m_actionOpen);
    menuDocument->addMenu(m_menuOpenRecent);
    menuDocument->addSeparator();
    menuDocument->addAction(m_actionSave);
    menuDocument->addAction(m_actionSaveAs);
    menuDocument->addAction(m_actionSaveCopyAs);
    menuDocument->addAction(m_actionSaveAll);
    menuDocument->addSeparator();
    menuDocument->addAction(m_actionCopyPath);
    menuDocument->addAction(m_actionCopyFilename);
    menuDocument->addAction(m_actionRenameFilename);
    menuDocument->addSeparator();
    menuDocument->addAction(m_actionProperties);
    menuDocument->addSeparator();
    menuDocument->addAction(m_actionClose);
    menuDocument->addAction(m_actionCloseOther);
    menuDocument->addAction(m_actionCloseAll);

    m_toolbarDocument = addToolBar(tr("Document Toolbar"));
    m_toolbarDocument->setObjectName(QStringLiteral("toolbarDocument"));
    m_toolbarDocument->addAction(m_actionNew);
    m_toolbarDocument->addAction(m_actionOpen);
    m_toolbarDocument->addSeparator();
    m_toolbarDocument->addAction(m_actionSave);
    m_toolbarDocument->addAction(m_actionSaveAs);
    m_toolbarDocument->addSeparator();
    m_toolbarDocument->addAction(m_actionClose);


    //
    // Edit

    auto *menuEdit = menuBar()->addMenu(tr("&Edit"));
    menuEdit->setObjectName(QStringLiteral("menuEdit"));

    m_toolbarEdit = addToolBar(tr("Edit Toolbar"));
    m_toolbarEdit->setObjectName(QStringLiteral("toolbarEdit"));


    //
    // View

    auto *menuView = menuBar()->addMenu(tr("&View"));
    menuView->setObjectName(QStringLiteral("menuView"));

    m_toolbarView = addToolBar(tr("View Toolbar"));
    m_toolbarView->setObjectName(QStringLiteral("toolbarView"));


    //
    // Format

    auto *menuFormat = menuBar()->addMenu(tr("&Format"));
    menuFormat->setObjectName(QStringLiteral("menuFormat"));

    m_toolbarFormat = addToolBar(tr("Format Toolbar"));
    m_toolbarFormat->setObjectName(QStringLiteral("toolbarFormat"));


    //
    // Tools

    auto *menuTools = menuBar()->addMenu(tr("&Tools"));
    menuTools->setObjectName(QStringLiteral("menuTools"));

    m_toolbarTools = addToolBar(tr("Tools Toolbar"));
    m_toolbarTools->setObjectName(QStringLiteral("toolbarTools"));


    //
    // Appearance

    m_actionShowPath = new QAction(tr("Show &Path in Titlebar"), this);
    m_actionShowPath->setObjectName(QStringLiteral("actionShowPath"));
    m_actionShowPath->setCheckable(true);
    m_actionShowPath->setChecked(true);
    m_actionShowPath->setIcon(QIcon::fromTheme(QStringLiteral("show-path"), QIcon(QStringLiteral(":/icons/actions/16/show-path.svg"))));
    m_actionShowPath->setIconText(tr("Path"));
    m_actionShowPath->setToolTip(tr("Show document path in the window caption"));
    connect(m_actionShowPath, &QAction::toggled, this, &ApplicationWindow::updateWindowTitle);

    m_actionShowMenubar = new QAction(tr("Show &Menubar"), this);
    m_actionShowMenubar->setObjectName(QStringLiteral("actionShowMenubar"));
    m_actionShowMenubar->setCheckable(true);
    m_actionShowMenubar->setChecked(true);
    m_actionShowMenubar->setIcon(QIcon::fromTheme(QStringLiteral("show-menubar"), QIcon(QStringLiteral(":/icons/actions/16/show-menubar.svg"))));
    m_actionShowMenubar->setIconText(tr("Menubar"));
    m_actionShowMenubar->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_M));
    m_actionShowMenubar->setToolTip(tr("Show the menubar"));
    connect(m_actionShowMenubar, &QAction::toggled, menuBar(), &QMenuBar::setVisible);
    addAction(m_actionShowMenubar);

    m_actionShowToolbarApplication = new QAction(tr("Show &Application Toolbar"), this);
    m_actionShowToolbarApplication->setObjectName(QStringLiteral("actionShowToolbarApplication"));
    m_actionShowToolbarApplication->setCheckable(true);
    m_actionShowToolbarApplication->setToolTip(tr("Show the Application toolbar"));

    m_actionShowToolbarDocument = new QAction(tr("Show &Document Toolbar"), this);
    m_actionShowToolbarDocument->setObjectName(QStringLiteral("actionShowToolbarDocument"));
    m_actionShowToolbarDocument->setCheckable(true);
    m_actionShowToolbarDocument->setToolTip(tr("Show the Document toolbar"));

    m_actionShowToolbarEdit = new QAction(tr("Show &Edit Toolbar"), this);
    m_actionShowToolbarEdit->setObjectName(QStringLiteral("actionShowToolbarEdit"));
    m_actionShowToolbarEdit->setCheckable(true);
    m_actionShowToolbarEdit->setToolTip(tr("Show the Edit toolbar"));

    m_actionShowToolbarView = new QAction(tr("Show &View Toolbar"), this);
    m_actionShowToolbarView->setObjectName(QStringLiteral("actionShowToolbarView"));
    m_actionShowToolbarView->setCheckable(true);
    m_actionShowToolbarView->setToolTip(tr("Show the View toolbar"));

    m_actionShowToolbarFormat = new QAction(tr("Show &Format Toolbar"), this);
    m_actionShowToolbarFormat->setObjectName(QStringLiteral("actionShowToolbarFormat"));
    m_actionShowToolbarFormat->setCheckable(true);
    m_actionShowToolbarFormat->setToolTip(tr("Show the Format toolbar"));

    m_actionShowToolbarTools = new QAction(tr("Show &Tools Toolbar"), this);
    m_actionShowToolbarTools->setObjectName(QStringLiteral("actionShowToolbarTools"));
    m_actionShowToolbarTools->setCheckable(true);
    m_actionShowToolbarTools->setToolTip(tr("Show the Tools toolbar"));

    m_actionShowToolbarSettings = new QAction(tr("Show &Settings Toolbar"), this);
    m_actionShowToolbarSettings->setObjectName(QStringLiteral("actionShowToolbarSettings"));
    m_actionShowToolbarSettings->setCheckable(true);
    m_actionShowToolbarSettings->setToolTip(tr("Show the Settings toolbar"));

    m_actionShowToolbarHelp = new QAction(tr("Show &Help Toolbar"), this);
    m_actionShowToolbarHelp->setObjectName(QStringLiteral("actionShowToolbarHelp"));
    m_actionShowToolbarHelp->setCheckable(true);
    m_actionShowToolbarHelp->setToolTip(tr("Show the Help toolbar"));

    auto *actionToolButtonStyleIconOnly = new QAction(tr("&Icon Only"), this);
    actionToolButtonStyleIconOnly->setObjectName(QStringLiteral("actionToolButtonStyleIconOnly"));
    actionToolButtonStyleIconOnly->setCheckable(true);
    actionToolButtonStyleIconOnly->setToolTip(tr("Only display the icon"));
    actionToolButtonStyleIconOnly->setData(static_cast<int>(Qt::ToolButtonIconOnly));

    auto *actionToolButtonStyleTextOnly = new QAction(tr("&Text Only"), this);
    actionToolButtonStyleTextOnly->setObjectName(QStringLiteral("actionToolButtonStyleTextOnly"));
    actionToolButtonStyleTextOnly->setCheckable(true);
    actionToolButtonStyleTextOnly->setToolTip(tr("Only display the text"));
    actionToolButtonStyleTextOnly->setData(static_cast<int>(Qt::ToolButtonTextOnly));

    auto *actionToolButtonStyleTextBesideIcon = new QAction(tr("Text &Beside Icon"), this);
    actionToolButtonStyleTextBesideIcon->setObjectName(QStringLiteral("actionToolButtonStyleTextBesideIcon"));
    actionToolButtonStyleTextBesideIcon->setCheckable(true);
    actionToolButtonStyleTextBesideIcon->setToolTip(tr("The text appears beside the icon"));
    actionToolButtonStyleTextBesideIcon->setData(static_cast<int>(Qt::ToolButtonTextBesideIcon));

    auto *actionToolButtonStyleTextUnderIcon = new QAction(tr("Text &Under Icon"), this);
    actionToolButtonStyleTextUnderIcon->setObjectName(QStringLiteral("actionToolButtonStyleTextUnderIcon"));
    actionToolButtonStyleTextUnderIcon->setCheckable(true);
    actionToolButtonStyleTextUnderIcon->setToolTip(tr("The text appears under the icon"));
    actionToolButtonStyleTextUnderIcon->setData(static_cast<int>(Qt::ToolButtonTextUnderIcon));

    auto *actionToolButtonStyleDefault = new QAction(tr("&Default"), this);
    actionToolButtonStyleDefault->setObjectName(QStringLiteral("actionToolButtonStyleDefault"));
    actionToolButtonStyleDefault->setCheckable(true);
    actionToolButtonStyleDefault->setToolTip(tr("Follow the theme style"));
    actionToolButtonStyleDefault->setData(static_cast<int>(Qt::ToolButtonFollowStyle));

    m_actionsToolButtonStyle = new QActionGroup(this);
    m_actionsToolButtonStyle->setObjectName(QStringLiteral("actionsToolButtonStyle"));
    m_actionsToolButtonStyle->addAction(actionToolButtonStyleIconOnly);
    m_actionsToolButtonStyle->addAction(actionToolButtonStyleTextOnly);
    m_actionsToolButtonStyle->addAction(actionToolButtonStyleTextBesideIcon);
    m_actionsToolButtonStyle->addAction(actionToolButtonStyleTextUnderIcon);
    m_actionsToolButtonStyle->addAction(actionToolButtonStyleDefault);
    connect(m_actionsToolButtonStyle, &QActionGroup::triggered, this, &ApplicationWindow::slotToolButtonStyle);

    auto *actionToolButtonSizeSmall = new QAction(tr("&Small (16x16)"), this);
    actionToolButtonSizeSmall->setObjectName(QStringLiteral("actionToolButtonSizeSmall"));
    actionToolButtonSizeSmall->setCheckable(true);
    actionToolButtonSizeSmall->setToolTip(tr("Show icons in small size"));
    actionToolButtonSizeSmall->setData(16);

    auto *actionToolButtonSizeMedium = new QAction(tr("&Medium (22x22)"), this);
    actionToolButtonSizeMedium->setObjectName(QStringLiteral("actionToolButtonSizeMedium"));
    actionToolButtonSizeMedium->setCheckable(true);
    actionToolButtonSizeMedium->setToolTip(tr("Show icons in medium size"));
    actionToolButtonSizeMedium->setData(22);

    auto *actionToolButtonSizeLarge = new QAction(tr("&Large (32x32)"), this);
    actionToolButtonSizeLarge->setObjectName(QStringLiteral("actionToolButtonSizeLarge"));
    actionToolButtonSizeLarge->setCheckable(true);
    actionToolButtonSizeLarge->setToolTip(tr("Show icons in large size"));
    actionToolButtonSizeLarge->setData(32);

    auto *actionToolButtonSizeHuge = new QAction(tr("&Huge (48x48)"), this);
    actionToolButtonSizeHuge->setObjectName(QStringLiteral("actionToolButtonSizeHuge"));
    actionToolButtonSizeHuge->setCheckable(true);
    actionToolButtonSizeHuge->setToolTip(tr("Show icons in huge size"));
    actionToolButtonSizeHuge->setData(48);

    auto *actionToolButtonSizeDefault = new QAction(tr("De&fault"), this);
    actionToolButtonSizeDefault->setObjectName(QStringLiteral("actionToolButtonSizeDefault"));
    actionToolButtonSizeDefault->setCheckable(true);
    actionToolButtonSizeDefault->setToolTip(tr("Show icons in theme size"));
    actionToolButtonSizeDefault->setData(0);

    m_actionsToolButtonSize = new QActionGroup(this);
    m_actionsToolButtonSize->setObjectName(QStringLiteral("actionsToolButtonSize"));
    m_actionsToolButtonSize->addAction(actionToolButtonSizeSmall);
    m_actionsToolButtonSize->addAction(actionToolButtonSizeMedium);
    m_actionsToolButtonSize->addAction(actionToolButtonSizeLarge);
    m_actionsToolButtonSize->addAction(actionToolButtonSizeHuge);
    m_actionsToolButtonSize->addAction(actionToolButtonSizeDefault);
    connect(m_actionsToolButtonSize, &QActionGroup::triggered, this, &ApplicationWindow::slotToolButtonSize);

    m_actionShowDocumentTabs = new QAction(tr("Show &Document Tabs"), this);
    m_actionShowDocumentTabs->setObjectName(QStringLiteral("actionShowDocumentTabs"));
    m_actionShowDocumentTabs->setCheckable(true);
    m_actionShowDocumentTabs->setChecked(true);
    m_actionShowDocumentTabs->setIcon(QIcon::fromTheme(QStringLiteral("show-tabbar"), QIcon(QStringLiteral(":/icons/actions/16/show-tabbar.svg"))));
    m_actionShowDocumentTabs->setIconText(tr("Document Tabs"));
    m_actionShowDocumentTabs->setToolTip(tr("Show the document tabs"));
    connect(m_actionShowDocumentTabs, &QAction::toggled, m_documentManager, &DocumentManager::setTabVisible);
    connect(m_documentManager, &DocumentManager::tabVisibleChanged, m_actionShowDocumentTabs, &QAction::setChecked);

    auto *actionDocumentTabPositionTop = new QAction(tr("&Top"), this);
    actionDocumentTabPositionTop->setObjectName(QStringLiteral("actionDocumentTabPositionTop"));
    actionDocumentTabPositionTop->setCheckable(true);
    actionDocumentTabPositionTop->setToolTip(tr("Show tabs above the documents"));
    actionDocumentTabPositionTop->setData(QTabWidget::North);

    auto *actionDocumentTabPositionBottom = new QAction(tr("&Bottom"), this);
    actionDocumentTabPositionBottom->setObjectName(QStringLiteral("actionDocumentTabPositionBottom"));
    actionDocumentTabPositionBottom->setCheckable(true);
    actionDocumentTabPositionBottom->setToolTip(tr("Show tabs below the documents"));
    actionDocumentTabPositionBottom->setData(QTabWidget::South);

    m_actionsDocumentTabPosition = new QActionGroup(this);
    m_actionsDocumentTabPosition->setObjectName(QStringLiteral("actionsDocumentTabPosition"));
    m_actionsDocumentTabPosition->addAction(actionDocumentTabPositionTop);
    m_actionsDocumentTabPosition->addAction(actionDocumentTabPositionBottom);
    connect(m_actionsDocumentTabPosition, &QActionGroup::triggered, this, &ApplicationWindow::slotDocumentTabPosition);
    connect(m_documentManager, &DocumentManager::tabPositionChanged, this, &ApplicationWindow::updateActionsDocumentTabPosition);

    m_actionDocumentTabAutoHide = new QAction(tr("&Auto Hide"), this);
    m_actionDocumentTabAutoHide->setObjectName(QStringLiteral("actionDocumentTabAutoHide"));
    m_actionDocumentTabAutoHide->setCheckable(true);
    m_actionDocumentTabAutoHide->setToolTip(tr("Tabs are automatically hidden if they contain only 1 document"));
    connect(m_actionDocumentTabAutoHide, &QAction::toggled, m_documentManager, &DocumentManager::setTabAutoHide);
    connect(m_documentManager, &DocumentManager::tabAutoHideChanged, m_actionDocumentTabAutoHide, &QAction::setChecked);

    m_actionShowSheetTabBar = new QAction(tr("Show &Sheet Tabbar"), this);
    m_actionShowSheetTabBar->setObjectName(QStringLiteral("actionShowSheetTabBar"));
    m_actionShowSheetTabBar->setCheckable(true);
    m_actionShowSheetTabBar->setChecked(true);
    m_actionShowSheetTabBar->setIcon(QIcon::fromTheme(QStringLiteral("show-tabbar-bottom"), QIcon(QStringLiteral(":/icons/actions/16/show-tabbar-bottom.svg"))));
    m_actionShowSheetTabBar->setIconText(tr("Sheet Tabbar"));
    m_actionShowSheetTabBar->setToolTip(tr("Show the sheet tabbar"));
    connect(m_actionShowSheetTabBar, &QAction::toggled, this, &ApplicationWindow::slotShowSheetTabBar);

    auto *actionSheetTabBarPositionTop = new QAction(tr("&Top"), this);
    actionSheetTabBarPositionTop->setObjectName(QStringLiteral("actionSheetTabBarPositionTop"));
    actionSheetTabBarPositionTop->setCheckable(true);
    actionSheetTabBarPositionTop->setToolTip(tr("Show tabbar above the sheets"));
    actionSheetTabBarPositionTop->setData(QTabWidget::North);

    auto *actionSheetTabBarPositionBottom = new QAction(tr("&Bottom"), this);
    actionSheetTabBarPositionBottom->setObjectName(QStringLiteral("actionSheetTabBarPositionBottom"));
    actionSheetTabBarPositionBottom->setCheckable(true);
    actionSheetTabBarPositionBottom->setToolTip(tr("Show tabbar below the sheets"));
    actionSheetTabBarPositionBottom->setData(QTabWidget::South);

    m_actionsSheetTabBarPosition = new QActionGroup(this);
    m_actionsSheetTabBarPosition->setObjectName(QStringLiteral("actionsSheetTabBarPosition"));
    m_actionsSheetTabBarPosition->addAction(actionSheetTabBarPositionTop);
    m_actionsSheetTabBarPosition->addAction(actionSheetTabBarPositionBottom);
    connect(m_actionsSheetTabBarPosition, &QActionGroup::triggered, this, &ApplicationWindow::slotSheetTabBarPosition);

    m_actionSheetTabBarAutoHide = new QAction(tr("&Auto Hide"), this);
    m_actionSheetTabBarAutoHide->setObjectName(QStringLiteral("actionSheetTabBarAutoHide"));
    m_actionSheetTabBarAutoHide->setCheckable(true);
    m_actionSheetTabBarAutoHide->setToolTip(tr("Tabbar is automatically hidden if it contains only 1 sheet"));
    connect(m_actionSheetTabBarAutoHide, &QAction::toggled, this, &ApplicationWindow::slotSheetTabBarAutoHide);

    m_actionShowStatusbar = new QAction(tr("Show Stat&usbar"), this);
    m_actionShowStatusbar->setObjectName(QStringLiteral("actionShowStatusbar"));
    m_actionShowStatusbar->setCheckable(true);
    m_actionShowStatusbar->setChecked(true);
    m_actionShowStatusbar->setIcon(QIcon::fromTheme(QStringLiteral("show-statusbar"), QIcon(QStringLiteral(":/icons/actions/16/show-statusbar.svg"))));
    m_actionShowStatusbar->setIconText(tr("Statusbar"));
    m_actionShowStatusbar->setToolTip(tr("Show the statusbar"));
    connect(m_actionShowStatusbar, &QAction::toggled, statusBar(), &QStatusBar::setVisible);

    m_actionFullScreen = new QAction(this);
    m_actionFullScreen->setObjectName(QStringLiteral("actionViewFullScreen"));
    m_actionFullScreen->setCheckable(true);
    m_actionFullScreen->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::Key_F11) << QKeySequence::FullScreen);
    connect(m_actionFullScreen, &QAction::toggled, this, &ApplicationWindow::slotFullScreen);
    addAction(m_actionFullScreen);
    updateActionFullScreen();

    auto *menuToolButtonStyle = new QMenu(tr("Tool Button St&yle"), this);
    menuToolButtonStyle->setObjectName(QStringLiteral("menuToolButtonStyle"));
    menuToolButtonStyle->addSection(tr("Text Position"));
    menuToolButtonStyle->addActions(m_actionsToolButtonStyle->actions());
    menuToolButtonStyle->addSection(tr("Icon Size"));
    menuToolButtonStyle->addActions(m_actionsToolButtonSize->actions());

    auto *menuDocumentTabPosition = new QMenu(tr("Document Tab &Position"), this);
    menuDocumentTabPosition->setObjectName(QStringLiteral("menuDocumentTabPosition"));
    menuDocumentTabPosition->addSection(tr("Position"));
    menuDocumentTabPosition->addActions(m_actionsDocumentTabPosition->actions());
    menuDocumentTabPosition->addSection(tr("Behavior"));
    menuDocumentTabPosition->addAction(m_actionDocumentTabAutoHide);
    connect(m_actionShowDocumentTabs, &QAction::toggled, menuDocumentTabPosition, &QMenu::setEnabled);

    m_menuSheetTabBarPosition = new QMenu(tr("Sheet Tabbar P&osition"), this);
    m_menuSheetTabBarPosition->setObjectName(QStringLiteral("menuSheetTabBarPosition"));
    m_menuSheetTabBarPosition->addSection(tr("Position"));
    m_menuSheetTabBarPosition->addActions(m_actionsSheetTabBarPosition->actions());
    m_menuSheetTabBarPosition->addSection(tr("Behavior"));
    m_menuSheetTabBarPosition->addAction(m_actionSheetTabBarAutoHide);
    connect(m_actionShowSheetTabBar, &QAction::toggled, m_menuSheetTabBarPosition, &QMenu::setEnabled);

    auto *menuSettings = menuBar()->addMenu(tr("&Settings"));
    menuSettings->setObjectName(QStringLiteral("menuSettings"));
    menuSettings->addAction(m_actionShowPath);
    menuSettings->addSeparator();
    menuSettings->addAction(m_actionShowMenubar);
    menuSettings->addSeparator();
    menuSettings->addAction(m_actionShowToolbarApplication);
    menuSettings->addAction(m_actionShowToolbarDocument);
    menuSettings->addAction(m_actionShowToolbarEdit);
    menuSettings->addAction(m_actionShowToolbarView);
    menuSettings->addAction(m_actionShowToolbarFormat);
    menuSettings->addAction(m_actionShowToolbarTools);
    menuSettings->addAction(m_actionShowToolbarSettings);
    menuSettings->addAction(m_actionShowToolbarHelp);
    menuSettings->addMenu(menuToolButtonStyle);
    menuSettings->addSeparator();
    menuSettings->addAction(m_actionShowDocumentTabs);
    menuSettings->addMenu(menuDocumentTabPosition);
    menuSettings->addAction(m_actionShowSheetTabBar);
    menuSettings->addMenu(m_menuSheetTabBarPosition);
    menuSettings->addSeparator();
    menuSettings->addAction(m_actionShowStatusbar);
    menuSettings->addSeparator();
    menuSettings->addAction(m_actionFullScreen);

    m_toolbarSettings = addToolBar(tr("Settings Toolbar"));
    m_toolbarSettings->setObjectName(QStringLiteral("toolbarSettings"));
    m_toolbarSettings->addAction(m_actionShowMenubar);
    m_toolbarSettings->addAction(m_actionShowDocumentTabs);
    m_toolbarSettings->addAction(m_actionShowSheetTabBar);
    m_toolbarSettings->addAction(m_actionShowStatusbar);
    m_toolbarSettings->addSeparator();
    m_toolbarSettings->addAction(m_actionFullScreen);


    //
    // Help

    auto *menuHelp = menuBar()->addMenu(tr("&Help"));
    menuHelp->setObjectName(QStringLiteral("menuHelp"));

    m_toolbarHelp = addToolBar(tr("Help Toolbar"));
    m_toolbarHelp->setObjectName(QStringLiteral("toolbarHelp"));


    // Connect toolbars with the corresponding actions
    connect(m_toolbarApplication, &QToolBar::visibilityChanged, m_actionShowToolbarApplication, &QAction::setChecked);
    connect(m_actionShowToolbarApplication, &QAction::toggled, m_toolbarApplication, &QToolBar::setVisible);
    connect(m_toolbarDocument, &QToolBar::visibilityChanged, m_actionShowToolbarDocument, &QAction::setChecked);
    connect(m_actionShowToolbarDocument, &QAction::toggled, m_toolbarDocument, &QToolBar::setVisible);
    connect(m_toolbarEdit, &QToolBar::visibilityChanged, m_actionShowToolbarEdit, &QAction::setChecked);
    connect(m_actionShowToolbarEdit, &QAction::toggled, m_toolbarEdit, &QToolBar::setVisible);
    connect(m_toolbarView, &QToolBar::visibilityChanged, m_actionShowToolbarView, &QAction::setChecked);
    connect(m_actionShowToolbarView, &QAction::toggled, m_toolbarView, &QToolBar::setVisible);
    connect(m_toolbarFormat, &QToolBar::visibilityChanged, m_actionShowToolbarFormat, &QAction::setChecked);
    connect(m_actionShowToolbarFormat, &QAction::toggled, m_toolbarFormat, &QToolBar::setVisible);
    connect(m_toolbarTools, &QToolBar::visibilityChanged, m_actionShowToolbarTools, &QAction::setChecked);
    connect(m_actionShowToolbarTools, &QAction::toggled, m_toolbarTools, &QToolBar::setVisible);
    connect(m_toolbarSettings, &QToolBar::visibilityChanged, m_actionShowToolbarSettings, &QAction::setChecked);
    connect(m_actionShowToolbarSettings, &QAction::toggled, m_toolbarSettings, &QToolBar::setVisible);
    connect(m_toolbarHelp, &QToolBar::visibilityChanged, m_actionShowToolbarHelp, &QAction::setChecked);
    connect(m_actionShowToolbarHelp, &QAction::toggled, m_toolbarHelp, &QToolBar::setVisible);


    //
    // Statusbar

    statusBar()->showMessage(tr("Ready"), 3000);
}


void ApplicationWindow::updateActionsToolButtonStyle(const Qt::ToolButtonStyle style)
{
    const QList<QAction *> actions = m_actionsToolButtonStyle->actions();
    for (auto *action : actions) {
        if (static_cast<Qt::ToolButtonStyle>(action->data().toInt()) == style) {
            action->trigger();
            break;
        }
    }
}


void ApplicationWindow::updateActionsToolButtonSize(const int pixel)
{
    const QList<QAction *> actions = m_actionsToolButtonSize->actions();
    for (auto *action : actions) {
        if (action->data().toInt() == pixel) {
            action->trigger();
            break;
        }
    }
}


void ApplicationWindow::updateActionsDocumentTabPosition(const QTabWidget::TabPosition position)
{
    const QList<QAction *> actions = m_actionsDocumentTabPosition->actions();
    for (auto *action : actions) {
        if (static_cast<QTabWidget::TabPosition>(action->data().toInt()) == position) {
            action->trigger();
            break;
        }
    }
}


void ApplicationWindow::updateActionsSheetTabBarPosition(const QTabWidget::TabPosition position)
{
    const QList<QAction *> actions = m_actionsSheetTabBarPosition->actions();
    for (auto *action : actions) {
        if (static_cast<QTabWidget::TabPosition>(action->data().toInt()) == position) {
            action->trigger();
            break;
        }
    }
}


void ApplicationWindow::updateActionFullScreen()
{
    if (!m_actionFullScreen->isChecked()) {
        m_actionFullScreen->setText(tr("Full S&creen Mode"));
        m_actionFullScreen->setIcon(QIcon::fromTheme(QStringLiteral("view-fullscreen"), QIcon(QStringLiteral(":/icons/actions/16/view-fullscreen.svg"))));
        m_actionFullScreen->setIconText(tr("Full Screen"));
        m_actionFullScreen->setToolTip(tr("Display the window in full screen"));
    }
    else {
        m_actionFullScreen->setText(tr("Exit Full S&creen Mode"));
        m_actionFullScreen->setIcon(QIcon::fromTheme(QStringLiteral("view-restore"), QIcon(QStringLiteral(":/icons/actions/16/view-restore.svg"))));
        m_actionFullScreen->setIconText(tr("Full Screen"));
        m_actionFullScreen->setToolTip(tr("Exit full screen mode"));
    }
}


void ApplicationWindow::updateMenuOpenRecent()
{
    m_menuOpenRecent->clear();

    if (m_recentDocuments->count()) {
        m_menuOpenRecent->setEnabled(true);
        m_menuOpenRecent->addActions(m_recentDocuments->actions());
        m_menuOpenRecent->addSeparator();
        m_menuOpenRecent->addAction(m_actionOpenRecentClear);
    }
    else {
        m_menuOpenRecent->setEnabled(false);
    }
}


void ApplicationWindow::enableActions(const bool enabled)
{
    m_actionSave->setEnabled(enabled);
    m_actionSaveAs->setEnabled(enabled);
    m_actionSaveCopyAs->setEnabled(enabled);
    m_actionSaveAll->setEnabled(enabled);
    m_actionClose->setEnabled(enabled);
    m_actionCloseAll->setEnabled(enabled);

    m_actionShowSheetTabBar->setEnabled(enabled);
    m_menuSheetTabBarPosition->setEnabled(enabled);
}


void ApplicationWindow::enableActionCloseOther(const bool enabled)
{
    m_actionCloseOther->setEnabled(enabled);
}


void ApplicationWindow::enableUrlActions(const bool enabled)
{
    m_actionCopyPath->setEnabled(enabled);
    m_actionProperties->setEnabled(enabled);
}


void ApplicationWindow::enableFileActions(const bool enabled)
{
    Q_UNUSED(enabled)
}


void ApplicationWindow::enableFilenameActions(const bool enabled)
{
    m_actionCopyFilename->setEnabled(enabled);
    m_actionRenameFilename->setEnabled(enabled);
}


void ApplicationWindow::loadSettings()
{
    QSettings settings;


    //
    // Application properties

    // Geometry
    const auto geometry = settings.value(QStringLiteral("Application/Geometry"), QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    else {
        // Default: Center window
        const auto availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() * 2/3, availableGeometry.height() * 2/3);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    }

    // State
    const auto state = settings.value(QStringLiteral("Application/State"), QByteArray()).toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);

        if (isFullScreen())
            m_actionFullScreen->toggle();
    }
    else {
        // Default: Show/hide toolbars
        m_toolbarApplication->setVisible(true);
        m_toolbarDocument->setVisible(true);
        m_toolbarEdit->setVisible(true);
        m_toolbarView->setVisible(true);
        m_toolbarFormat->setVisible(true);
        m_toolbarTools->setVisible(true);
        m_toolbarSettings->setVisible(false);
        m_toolbarHelp->setVisible(false);
    }

    // Show Path
    bool visible = settings.value(QStringLiteral("Application/ShowPath"), true).toBool();
    if (!visible)
        m_actionShowPath->toggle();

    // Show Menubar
    visible = settings.value(QStringLiteral("Application/ShowMenubar"), true).toBool();
    if (!visible)
        m_actionShowMenubar->toggle();

    // Show Statusbar
    visible = settings.value(QStringLiteral("Application/ShowStatusbar"), true).toBool();
    if (!visible)
        m_actionShowStatusbar->toggle();

    // Tool Button Style
    int value = settings.value(QStringLiteral("Application/ToolButtonStyle"), static_cast<int>(Qt::ToolButtonFollowStyle)).toInt();
    const QMetaEnum styles(QMetaEnum::fromType<Qt::ToolButtonStyle>());
    const auto style = !!styles.valueToKey(value) ? static_cast<Qt::ToolButtonStyle>(value) : Qt::ToolButtonFollowStyle;
    updateActionsToolButtonStyle(style);

    // Tool Button Size
    value = settings.value(QStringLiteral("Application/ToolButtonSize"), 0).toInt();
    const QList<int> pixels = {0, 16, 22, 32, 48};
    const int pixel = pixels.contains(value) ? value : 0;
    updateActionsToolButtonSize(pixel);
}


void ApplicationWindow::saveSettings()
{
    QSettings settings;


    //
    // Application properties

    const auto geometry = saveGeometry();
    settings.setValue(QStringLiteral("Application/Geometry"), geometry);

    const auto state = saveState();
    settings.setValue(QStringLiteral("Application/State"), state);

    bool visible = m_actionShowPath->isChecked();
    settings.setValue(QStringLiteral("Application/ShowPath"), visible);

    visible = m_actionShowMenubar->isChecked();
    settings.setValue(QStringLiteral("Application/ShowMenubar"), visible);

    visible = m_actionShowStatusbar->isChecked();
    settings.setValue(QStringLiteral("Application/ShowStatusbar"), visible);

    int value = m_actionsToolButtonStyle->checkedAction()->data().toInt();
    settings.setValue(QStringLiteral("Application/ToolButtonStyle"), value);

    value = m_actionsToolButtonSize->checkedAction()->data().toInt();
    settings.setValue(QStringLiteral("Application/ToolButtonSize"), value);
}


void ApplicationWindow::closeEvent(QCloseEvent *event)
{
    if (m_documentManager->subWindowCount() >= 1) {

        const QString &title = tr("Quit the application");
        const QString &text = tr("This will close all open documents and quit the application.\n"
                                 "Are you sure you want to continue?");
        const QString &confirmationKey = QStringLiteral("ConfirmQuitApplication");

        if (ConfirmationDialog::warningContinueCancel(this, title, text, confirmationKey) == QMessageBox::Cancel) {
            event->ignore();
            return;
        }

        m_documentManager->closeAllSubWindows();
    }

    m_documentManager->saveSettings();
    m_recentDocuments->saveSettings();
    saveSettings();
    event->accept();
}


//
// Application window
//

void ApplicationWindow::updateWindowModified()
{
    bool modified = false;

    auto *docWindow = qobject_cast<DocumentWindow *>(m_documentManager->activeSubWindow());
    if (docWindow)
        modified = docWindow->isWindowModified();

    setWindowModified(modified);
}


void ApplicationWindow::updateWindowTitle()
{
    QString caption;

    auto *docWindow = qobject_cast<DocumentWindow *>(m_documentManager->activeSubWindow());
    if (docWindow) {
        const bool pathVisible = m_actionShowPath->isChecked();
        caption = tr("%1 [*]").arg(docWindow->windowCaption(pathVisible));
    }

    setWindowTitle(caption);
}


//
// Document
//

DocumentWidget *ApplicationWindow::createDocument()
{
    auto *document = new DocumentWidget;
    auto *docWindow = new DocumentWindow;
    docWindow->setWidget(document);
    m_documentManager->addSubWindow(docWindow);

    // Connections: Tabs
    connect(document, &DocumentWidget::tabBarVisibleChanged, this, &ApplicationWindow::documentTabVisibleChanged);
    connect(document, &DocumentWidget::tabBarPositionChanged, this, &ApplicationWindow::documentTabPositionChanged);
    connect(document, &DocumentWidget::tabBarAutoHideChanged, this, &ApplicationWindow::documentTabAutoHideChanged);
    // Connections: Modified
    connect(document, &DocumentWidget::modifiedChanged, docWindow, &DocumentWindow::documentModifiedChanged);
    connect(document, &DocumentWidget::modifiedChanged, this, &ApplicationWindow::documentModifiedChanged);
    // Connections: Url
    connect(document, &DocumentWidget::urlChanged, docWindow, &DocumentWindow::documentUrlChanged);
    connect(document, &DocumentWidget::urlChanged, this, &ApplicationWindow::documentUrlChanged);
    // Connections: Actions
    connect(docWindow, &DocumentWindow::actionCloseOther, m_documentManager, &DocumentManager::closeOtherSubWindows);
    connect(docWindow, &DocumentWindow::actionCopyPath, document, &DocumentWidget::copyPathToClipboard);
    connect(docWindow, &DocumentWindow::actionCopyFilename, document, &DocumentWidget::copyFilenameToClipboard);
    connect(docWindow, &DocumentWindow::actionRenameFilename, document, &DocumentWidget::renameFilename);
    // Connections: Document count
    connect(docWindow, &DocumentWindow::destroyed, this, &ApplicationWindow::documentClosed);
    connect(this, &ApplicationWindow::documentCountChanged, document, &DocumentWidget::documentCountChanged);
    connect(this, &ApplicationWindow::documentCountChanged, docWindow, &DocumentWindow::documentCountChanged);
    // Connections: Content
    connect(document, &TableDocument::contentChanged, this, [this]() {
        updateTableInfo();
    });

    // Initialize
    document->initTabBarVisible();
    document->initTabBarPosition();
    document->initTabBarAutoHide();
    document->initModified();
    document->initUrl();

    return document;
}


bool ApplicationWindow::openDocument(const QUrl &url)
{
    if (!url.isValid())
        return false;

    QMdiSubWindow *subWindow = m_documentManager->findSubWindow(url);
    if (subWindow) {
        // Given document is already loaded; activate the subwindow
        m_documentManager->setActiveSubWindow(subWindow);
        m_recentDocuments->addUrl(url);
        return true;
    }

    return loadDocument(url);
}


bool ApplicationWindow::loadDocument(const QUrl &url)
{
    DocumentWidget *document = createDocument();
    if (!true) {
        // Given document could not be loaded
        document->close();
        return false;
    }

    document->show();
    document->setUrl(url);
    m_recentDocuments->addUrl(url);

    documentCreated();

    return true;
}


bool ApplicationWindow::saveDocument(DocumentWidget *document, const QUrl &altUrl)
{
    Q_UNUSED(altUrl)

    document->resetModified();

    return true;
}


void ApplicationWindow::updateTableInfo()
{
    DocumentWidget *document = activeDocument();

    if (!document) {
        m_tableInfoProvider->setRowCount(0);
        m_tableInfoProvider->setColumnCount(0);
        m_tableInfoProvider->setModified(false);
        return;
    }

    // Обновляем состояние документа
    m_tableInfoProvider->setModified(document->isModified());

    // Ищем таблицу в иерархии документа
    QTabWidget *tabs = document->findChild<QTabWidget*>();
    if (!tabs) {
        m_tableInfoProvider->setRowCount(0);
        m_tableInfoProvider->setColumnCount(0);
        return;
    }

    // Получаем текущую таблицу
    QTableWidget *table = qobject_cast<QTableWidget*>(tabs->currentWidget());
    if (table) {
        m_tableInfoProvider->setRowCount(table->rowCount());
        m_tableInfoProvider->setColumnCount(table->columnCount());
    } else {
        m_tableInfoProvider->setRowCount(0);
        m_tableInfoProvider->setColumnCount(0);
    }
}



//
//
//

void ApplicationWindow::documentCreated()
{
    const int count = m_documentManager->subWindowCount();

    emit documentCountChanged(count);
    enableActionCloseOther(count >= 2);
}


void ApplicationWindow::documentActivated(QMdiSubWindow *subWindow)
{
    updateWindowModified();
    updateWindowTitle();

    enableActions(hasActiveDocument());
    enableUrlActions(hasActiveDocumentUrl());
    enableFileActions(hasActiveDocumentUrlFile());
    enableFilenameActions(hasActiveDocumentUrlFilename());

    DocumentWidget *document = extractDocument(subWindow);
    if (document) {
        m_actionShowSheetTabBar->setChecked(document->isTabBarVisible());
        updateActionsSheetTabBarPosition(document->tabBarPosition());
        m_actionSheetTabBarAutoHide->setChecked(document->isTabBarAutoHide());
    }
    else {
        m_actionShowSheetTabBar->setChecked(true);
        updateActionsSheetTabBarPosition(QTabWidget::South);
        m_actionSheetTabBarAutoHide->setChecked(true);
    }

    updateTableInfo();
}


void ApplicationWindow::documentTabVisibleChanged(const bool visible)
{
    if (sender() == activeDocument())
        m_actionShowSheetTabBar->setChecked(visible);
}


void ApplicationWindow::documentTabPositionChanged(const QTabWidget::TabPosition position)
{
    if (sender() == activeDocument())
        updateActionsSheetTabBarPosition(position);
}


void ApplicationWindow::documentTabAutoHideChanged(const bool autoHide)
{
    if (sender() == activeDocument())
        m_actionSheetTabBarAutoHide->setChecked(autoHide);
}


void ApplicationWindow::documentModifiedChanged(const bool modified)
{
    Q_UNUSED(modified)

    if (sender() == activeDocument())
        updateWindowModified();
    updateTableInfo();
}


void ApplicationWindow::documentUrlChanged(const QUrl &url)
{
    Q_UNUSED(url)

    if (sender() == activeDocument()) {
        updateWindowTitle();

        enableUrlActions(hasActiveDocumentUrl());
        enableFileActions(hasActiveDocumentUrlFile());
        enableFilenameActions(hasActiveDocumentUrlFilename());
    }
}


void ApplicationWindow::documentClosed()
{
    const int count = m_documentManager->subWindowCount();

    emit documentCountChanged(count);
    enableActionCloseOther(count >= 2);
}


//
//
//

DocumentWidget *ApplicationWindow::extractDocument(const QMdiSubWindow *subWindow) const
{
    if (!subWindow)
        return nullptr;

    return qobject_cast<DocumentWidget *>(subWindow->widget());
}


DocumentWidget *ApplicationWindow::activeDocument() const
{
    return extractDocument(m_documentManager->activeSubWindow());
}


bool ApplicationWindow::hasActiveDocument() const
{
    return !!activeDocument();
}


bool ApplicationWindow::hasActiveDocumentUrl() const
{
    auto *document = activeDocument();
    if (!document)
        return false;

    return !document->url().isEmpty();
}


bool ApplicationWindow::hasActiveDocumentUrlFile() const
{
    auto *document = activeDocument();
    if (!document)
        return false;

    return document->url().isLocalFile();
}


bool ApplicationWindow::hasActiveDocumentUrlFilename() const
{
    auto *document = activeDocument();
    if (!document)
        return false;

    return !document->url().fileName().isEmpty();
}


//
// Action slots
//

void ApplicationWindow::slotAbout()
{
    auto *dialog = new AboutDialog(this);
    dialog->open();
}


void ApplicationWindow::slotColophon()
{
    auto *dialog = new ColophonDialog(this);
    dialog->open();
}


void ApplicationWindow::slotPreferences()
{
    auto *dialog = new PreferencesDialog(this);
    dialog->open();
}


void ApplicationWindow::slotNew()
{
    DocumentWidget *document = createDocument();
    document->show();

    documentCreated();
}


void ApplicationWindow::slotOpen()
{
    const QList<QUrl> urls = QFileDialog::getOpenFileUrls(this, tr("Open Documents"));
    for (const QUrl &url : urls)
        openDocument(url);
}


void ApplicationWindow::slotSave()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    if (!document->url().isEmpty())
        saveDocument(document, QUrl());
    else
        slotSaveAs();
}


void ApplicationWindow::slotSaveAs()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    const QUrl &url = QFileDialog::getSaveFileUrl(this, tr("Save Document"));
    if (!url.isEmpty()) {
        document->setUrl(url);
        saveDocument(document, QUrl());
    }
}


void ApplicationWindow::slotSaveCopyAs()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    const QUrl &url = QFileDialog::getSaveFileUrl(this, tr("Save Copy of Document"));
    if (!url.isEmpty())
        saveDocument(document, url);
}


void ApplicationWindow::slotSaveAll()
{
    const QList<QMdiSubWindow *> subWindows = m_documentManager->subWindowList();
    for (auto *subWindow : subWindows) {

        DocumentWidget *document = extractDocument(subWindow);
        if (!document)
            continue;

        if (!document->url().isEmpty()) {
            saveDocument(document, QUrl());
        }
        else {
            const QUrl &url = QFileDialog::getSaveFileUrl(this, tr("Save Document"));
            if (!url.isEmpty()) {
                document->setUrl(url);
                saveDocument(document, QUrl());
            }
        }
    }
}


void ApplicationWindow::slotCopyPath()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->copyPathToClipboard();
}


void ApplicationWindow::slotCopyFilename()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->copyFilenameToClipboard();
}


void ApplicationWindow::slotRenameFilename()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->renameFilename();
}


void ApplicationWindow::slotProperties()
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    auto *dialog = new PropertiesDialog(document->url(), this);
    dialog->open();
}


void ApplicationWindow::slotCloseOther()
{
    if (m_documentManager->subWindowCount() >= 2) {

        const QString &title = tr("Close all documents beside current one");
        const QString &text = tr("This will close all open documents beside the current one.\n"
                                 "Are you sure you want to continue?");
        const QString &confirmationKey = QStringLiteral("ConfirmCloseOtherDocuments");

        if (ConfirmationDialog::warningContinueCancel(this, title, text, confirmationKey) != QMessageBox::Cancel)
            m_documentManager->closeOtherSubWindows(m_documentManager->activeSubWindow());
    }
}


void ApplicationWindow::slotCloseAll()
{
    if (m_documentManager->subWindowCount() >= 1) {

        const QString &title = tr("Close all documents");
        const QString &text = tr("This will close all open documents.\n"
                                 "Are you sure you want to continue?");
        const QString &confirmationKey = QStringLiteral("ConfirmCloseAllDocuments");

        if (ConfirmationDialog::warningContinueCancel(this, title, text, confirmationKey) != QMessageBox::Cancel)
            m_documentManager->closeAllSubWindows();
    }
}


void ApplicationWindow::slotToolButtonStyle(const QAction *action)
{
    const auto style = static_cast<Qt::ToolButtonStyle>(action->data().toInt());

    m_toolbarApplication->setToolButtonStyle(style);
    m_toolbarDocument->setToolButtonStyle(style);
    m_toolbarEdit->setToolButtonStyle(style);
    m_toolbarView->setToolButtonStyle(style);
    m_toolbarFormat->setToolButtonStyle(style);
    m_toolbarTools->setToolButtonStyle(style);
    m_toolbarSettings->setToolButtonStyle(style);
    m_toolbarHelp->setToolButtonStyle(style);
}


void ApplicationWindow::slotToolButtonSize(const QAction *action)
{
    const int pixel = action->data().toInt();
    const auto size = pixel ? QSize(pixel, pixel) : QSize(-1, -1);

    m_toolbarApplication->setIconSize(size);
    m_toolbarDocument->setIconSize(size);
    m_toolbarEdit->setIconSize(size);
    m_toolbarView->setIconSize(size);
    m_toolbarFormat->setIconSize(size);
    m_toolbarTools->setIconSize(size);
    m_toolbarSettings->setIconSize(size);
    m_toolbarHelp->setIconSize(size);
}


void ApplicationWindow::slotDocumentTabPosition(const QAction *action)
{
    m_documentManager->setTabPosition(static_cast<QTabWidget::TabPosition>(action->data().toInt()));
}


void ApplicationWindow::slotShowSheetTabBar(const bool checked)
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->setTabBarVisible(checked);
}


void ApplicationWindow::slotSheetTabBarPosition(const QAction *action)
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->setTabBarPosition(static_cast<QTabWidget::TabPosition>(action->data().toInt()));
}


void ApplicationWindow::slotSheetTabBarAutoHide(const bool checked)
{
    DocumentWidget *document = activeDocument();
    if (!document)
        return;

    document->setTabBarAutoHide(checked);
}


void ApplicationWindow::slotFullScreen(const bool checked)
{
    updateActionFullScreen();

    if (checked)
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}
