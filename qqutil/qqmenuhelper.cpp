/*
 * qqmenuhelper.cpp
 *
 * (c) 2005-2008 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file qqutil/qqmenuhelper.cpp
 * Source file for QQMenuHelper
 */

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include "qqhelpbrowser.h"
#include "qqmenuhelper.h"

#define MAX_RECENT_FILES 5

/**
 * Constructor.
 * @param window The main application window.
 * @param toolbar The main toolbar
 * @param fileDescription The description of the document file type as it is
 *                        to appear in file dialogs
 * @param fileExtension The file extension of the document file type (do not
 *                      include the period)
 */
QQMenuHelper::QQMenuHelper(QMainWindow *window, QToolBar *toolbar,
                           const QString &fileDescription,
                           const QString &fileExtension,
                           bool newFileLaunchesDialog)
 : QObject(window), mainWindow(window), mainToolBar(toolbar),
   description(fileDescription), extension(fileExtension), file(0), help(0)
{
    QChar ellipsis(8230);

    // File menu actions
    QString fileNewText(tr("&New"));
    if (newFileLaunchesDialog) {
      fileNewText += ellipsis;
    }
    fileNewAction = new QAction(QIcon(":/icons/new.png"), fileNewText, window);
    fileNewAction->setStatusTip(tr("Create a new file"));
    fileNewAction->setToolTip(fileNewAction->statusTip());
    fileNewAction->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(emitNewFile()));

    fileOpenAction = new QAction(QIcon(":/icons/open.png"), tr("&Open") + ellipsis, window);
    fileOpenAction->setStatusTip(tr("Open an existing file"));
    fileOpenAction->setToolTip(fileOpenAction->statusTip());
    fileOpenAction->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(emitOpenFile()));

    quitAction = new QAction(QIcon(":/icons/quit.png"), tr("&Quit"), window);
    quitAction->setStatusTip(tr("Quit the application"));
    quitAction->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(quitAction, SIGNAL(triggered()), this, SIGNAL(quit()));

    fileSaveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save"), window);
    fileSaveAction->setStatusTip(tr("Save the current file"));
    fileSaveAction->setToolTip(fileSaveAction->statusTip());
    fileSaveAction->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(fileSaveAction, SIGNAL(triggered()), this, SIGNAL(saveFile()));
    
    recent = new QMenu(tr("Open &Recent"), window);
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        // we'll set the actual paths later; just need actions that stick
        // around after a file is opened
        recentActions[i] = new QAction("", window);
        connect(recentActions[i], SIGNAL(triggered()), this, SLOT(openRecent()));
    }
    
    fileSeparatorAction = new QAction(this);
    fileSeparatorAction->setSeparator(true);

    closeAction = new QAction(QIcon(":/icons/close.png"), tr("&Close"), window);
    closeAction->setStatusTip(tr("Close the current file"));
    closeAction->setShortcut(Qt::CTRL + Qt::Key_W);
    connect(closeAction, SIGNAL(triggered()), this, SIGNAL(closeFile()));
    
    prefsAction = new QAction(tr("&Preferences"), window);
    prefsAction->setStatusTip(tr("View or change %1 settings").arg(qApp->applicationName()));
    prefsAction->setShortcut(Qt::CTRL + Qt::Key_P);
    connect(prefsAction, SIGNAL(triggered()), this, SIGNAL(editPreferences()));
    
#if defined(Q_WS_MAC)
    fileNewAction->setIconVisibleInMenu(false);
    fileOpenAction->setIconVisibleInMenu(false);
    quitAction->setIconVisibleInMenu(false);
    fileSaveAction->setIconVisibleInMenu(false);
    closeAction->setIconVisibleInMenu(false);
    docIcon = QIcon(":/icons/document_small.png");
    modifiedDocIcon = QIcon(darkenPixmap(QPixmap(":/icons/document_small.png")));
#endif
    
    // File menu basic setup
    file = new QMenu(tr("&File"), window);
    recent = new QMenu(tr("Open &Recent"), window);
    file->addAction(fileNewAction);
    file->addAction(fileOpenAction);
    file->addMenu(recent);
    file->addAction(fileSeparatorAction);
    file->addAction(closeAction);
    file->addAction(prefsAction);
    insertionPoint = fileSeparatorAction;
#if !defined(Q_WS_MAC)
    file->addSeparator();
#endif
    file->addAction(quitAction);
    window->menuBar()->addMenu(file);

    // Help menu actions
    QString helpText = tr("Help Contents");
    QString macHelpText = tr("%1 Help").arg(qApp->applicationName());
#if defined(Q_WS_MAC)
    helpText = macHelpText;
#endif
    helpAction = new QAction(helpText, window);
    helpAction->setStatusTip(helpText);
    connect(helpAction, SIGNAL(triggered()), this, SLOT(showHelp()));

    QString aboutText = tr("&About %1").arg(qApp->applicationName());
    aboutAction = new QAction(aboutText, window);
    aboutAction->setStatusTip(aboutText);
    connect(aboutAction, SIGNAL(triggered()), this, SIGNAL(aboutApplication()));

    aboutQtAction = new QAction(tr("About &Qt"), window);
    aboutQtAction->setStatusTip(tr("About Qt"));
    connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQt()));
    
    // Help menu setup
    help = new QMenu(tr("&Help"), window);
    help->addAction(helpAction);
#if !defined(Q_WS_MAC)
    // skip this on the mac, since both "About.." actions get moved elsewhere
    help->addSeparator();
#endif
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
    window->menuBar()->addMenu(help);
    
    // toolbar setup
    toolbar->addAction(fileNewAction);
    toolbar->addAction(fileOpenAction);
    toolbar->addAction(fileSaveAction);
}

/**
 * <p>Load an assortment of saved application settings.  Typically called
 * early in the QMainWindow constructor.</p>
 * <ul>
 * <li>Replaces the default application font with the one saved in the
 * application settings (if present).  Doesn't do this on Mac OS X,
 * because it doesn't seem to stick well (keeps getting replaced by the
 * default font again upon certain actions, like the window regaining
 * focus).  The font settings are stored in the "Font" group, in the "Name"
 * and "Size" entries.</li>
 * <li>Loads the list of most recently opened files.  These are stored as
 * "Recent0", "Recent1", etc. in the "Files" group of the application
 * settings.</li>
 * <li>Loads the last directory where the application either opened or saved
 * a document file.  Replaces it with a reasonable default if the directory no
 * longer exists.  Stored in the "Files" group under "LastDir" in the
 * application settings.</li>
 * </ul>
 * @param settings The QSettings object from which to load the settings.
 */
void QQMenuHelper::loadSettings(QSettings *settings)
{
#if !defined(Q_WS_MAC)
    // Load font settings
    QFont currentFont = qApp->font();
    QString family = currentFont.family().toLower();
    int size = currentFont.pointSize();
    family = settings->value("Font/Name", family).toString();
    size = settings->value("Font/Size", size).toInt();
    QFont font(family, size);
    qApp->setFont(font);
    mainWindow->setFont(font);
#endif
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        QString key = QString("Files/Recent%1").arg(i);
        QString path = settings->value(key, "").toString();
        if (!path.isEmpty() && !QFileInfo(path).isDir()) {
            recentFiles.append(path);
        }
    }
}

/**
 * <p>Save an assortment of application settings to the specified QSettings
 * object.</p>
 * <ul>
 * <li>Updates the list of most recently opened files.</li>
 * <li>Updates the last directory where the application either opened or saved
 * a document file</li>
 * </ul>
 * @param settings The QSettings object to be updated.
 */
void QQMenuHelper::saveSettings(QSettings *settings)
{
    // Save the list of recently opened files
    QStringList files(recentFiles);
    // Insert blank entries if necessary
    while (files.count() < MAX_RECENT_FILES) {
        files.append("");
    }
    for (int i = 0; i < MAX_RECENT_FILES; i++) {
        settings->setValue(QString("Files/Recent%1").arg(i), files[i]);
    }
}

/**
 * Get the path to the directory last used to open or save a file.  If it
 * doesn't exist, get the default document directory instead (and make sure
 * this is registered with the application settings).
 *
 * @param settings The QSettings object to look the current value up in
 */
QString QQMenuHelper::getLastDir(QSettings *settings)
{
    QString lastDir = settings->value("Files/LastDir", "").toString();
    if (lastDir.isEmpty() || !QDir(lastDir).exists()) {
        lastDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (lastDir.isEmpty()) {
            lastDir = QDir::homePath();
        }
        settings->setValue("Files/LastDir", lastDir);
    }
    return lastDir;
}

/**
 * Update the menu of most recently opened files.
 */
void QQMenuHelper::updateRecentMenu()
{
    recent->clear();
    int count = recentFiles.count();
    for (int i = 0; i < MAX_RECENT_FILES && i < count; i++) {
        recentActions[i]->setText(recentFiles[i]);
        recent->addAction(recentActions[i]);
    }
}

/**
 * Indicate whether the current document has been edited or not since it was
 * last saved, so that menu items (particularly the "Save" action) and the
 * main window decorations can be updated accordingly.
 * @param y True if the document has been edited, false otherwise.
 */
void QQMenuHelper::setEdited(bool y)
{
    fileSaveAction->setEnabled(y);
    mainWindow->setWindowModified(y);
#if defined(Q_WS_MAC)
    if (y) {
        mainWindow->setWindowIcon(modifiedDocIcon);
    }
    else {
        mainWindow->setWindowIcon(docIcon);
    }
#endif
}

/**
 * Show the application's main help file.  This should be a resource named
 * help/[applicationName].html (i.e. "help/MyApp.html").
 */
void QQMenuHelper::showHelp()
{
    QQHelpBrowser helpBrowser(QString("qrc:/help/html/%1.html").arg(qApp->applicationName()), mainWindow);
    helpBrowser.exec();
}

/**
 * Display a dialog window containing information about the Qt library.
 */
void QQMenuHelper::aboutQt()
{
    QMessageBox::aboutQt(mainWindow, qApp->applicationName());
}

/**
 * <p>Adjust the "File" menu and toolbar contents as appropriate for the file
 * selection screen.  Sets the "File" menu to have the following content:</p>
 * <ul>
 * <li>New</li>
 * <li>Open</li>
 * <li>Open Recent <i>(submenu)</i></li>
 * <li>(items added using addToFileMenu() go here)</li>
 * <li>Preferences</li>
 * <li><i>(separator)</i></li>
 * <li>Quit</li>
 * </ul>
 * <p>Also shows the "New" and "Open" buttons on the toolbar (and on the Mac,
 * makes sure no document icon is shown in the titlebar).</p>
 */
void QQMenuHelper::startFileSelectorMenu()
{
    // update action visibility
    fileNewAction->setVisible(true);
    fileOpenAction->setVisible(true);
    updateRecentMenu();
    recent->menuAction()->setVisible(true);
    fileSaveAction->setVisible(false);
    fileSeparatorAction->setVisible(false);
    closeAction->setVisible(false);

    // enable and disable actions as appropriate for the current state
    fileNewAction->setEnabled(true);
    fileOpenAction->setEnabled(true);
    fileSaveAction->setEnabled(false);
    closeAction->setEnabled(false);
    
#if defined(Q_WS_MAC)
    mainWindow->setWindowIcon(QIcon());
#endif
}

/**
 * <p>Adjust the "File" menu and toolbar contents as appropriate for the
 * document screen.  Sets the "File" menu to have the following content:</p>
 * <ul>
 * <li>Save</li>
 * <li>(items added using addToFileMenu() go here)</li>
 * <li><i>(separator)</i></li>
 * <li>Close</li>
 * <li>Preferences</li>
 * <li><i>(separator)</i></li>
 * <li>Quit</li>
 * </ul>
 * <p>Also shows the "Save" button on the toolbar.  If creating
 * a new file which isn't automatically saved, follow this with a call
 * to setEdited(true).  On the Mac, also shows a document icon in the
 * titlebar.</p>
 */
void QQMenuHelper::startDocumentFileMenu()
{
    // update action visibility
    fileSaveAction->setVisible(true);
    fileSeparatorAction->setVisible(true);
    closeAction->setVisible(true);
    fileNewAction->setVisible(false);
    fileOpenAction->setVisible(false);
    recent->menuAction()->setVisible(false);

    // enable and disable actions as appropriate for the current state
    fileNewAction->setEnabled(false);
    fileOpenAction->setEnabled(false);
    fileSaveAction->setEnabled(false);
    closeAction->setEnabled(true);

#if defined(Q_WS_MAC)
    mainWindow->setWindowIcon(docIcon);
#endif
}

/**
 * Return the current "File" menu.  Usually called between the methods
 * which start and finish creating the menu, in order to add custom actions
 * and separators to it.
 * @return The current "File" menu.
 */
QMenu *QQMenuHelper::fileMenu()
{
    return file;
}

/**
 * Return the current "Help" menu.  Sometimes needed in order to change its
 * font, etc.
 * @return The current "Help" menu.
 */
QMenu *QQMenuHelper::helpMenu()
{
    return help;
}

/**
 * Add the specified action to the "File" menu just before the standard
 * footer items for the current mode (either "Preferences" or the separator
 * before "Close").
 *
 * @param action The action to be added to the File menu
 */
void QQMenuHelper::addToFileMenu(QAction *action)
{
    file->insertAction(insertionPoint, action);
}

/**
 * Return the file save action.  Occasionally needed externally, for example
 * if providing the option to remove it from the toolbar.
 * @return The file save action
 */
QAction *QQMenuHelper::saveAction()
{
    return fileSaveAction;
}

/**
 * Determine if the file has been changed, and if so ask the user if they
 * would like to save the changes.  Emit the saveFile() signal if
 * appropriate.  Often called in an overridden QMainWindow::closeEvent();
 * if both the closeFile() and quit() signals are connected to the window's
 * close() slot, then both of these actions and a pressing of the window's
 * "X" button can all be handled in that method.
 */
void QQMenuHelper::saveChangesPrompt()
{
    if (fileSaveAction->isEnabled()) {
        int choice = QMessageBox::warning(mainWindow, qApp->applicationName(),
                                          tr("Save changes?"),
                                          tr("Yes"), tr("No"));
        if (choice == 0) {
            emit saveFile();
        }
    }
}

/**
 * Start the process of creating a new file.  Asks the user where to create
 * it and what to name it.  If this is completed successfully, emit the
 * newFile() signal.
 */
void QQMenuHelper::emitNewFile()
{
    QString filename = createNewFile(description, extension);
    if (filename.isEmpty()) {
        return;
    }
    emit(newFile(filename));
}

/**
 * Launch a file dialog which allows the user to create a new file of the
 * specified type.  Used by <code>emitNewFile()</code> to create new native
 * document files, but can also be used to create files for exporting data,
 * etc.
 *
 * @param fileDescription The description of the file type as it is to appear
 *                        in the file dialog (null for the application default)
 * @param fileExtension The file extension of the file type (do not include
 *                      the period, use null for the application default)
 * @return The path of the file to be created (or an empty string if none)
 */
QString QQMenuHelper::createNewFile(const QString &fileDescription,
                      const QString &fileExtension)
{
    QString desc = fileDescription.isNull() ? description : fileDescription;
    QString ext = fileExtension.isNull() ? extension : fileExtension;
    QString filter = QString("%1 (*.%2)").arg(desc).arg(ext);
    QSettings settings;
    QString lastDir = getLastDir(&settings);
    QString filename = QFileDialog::getSaveFileName(mainWindow,
                           tr("Choose a filename to save under"), lastDir,
                           filter);
    if (filename.isEmpty()) {
        return "";
    }
    if (!filename.endsWith(ext, Qt::CaseInsensitive)) {
        filename += QString(".%1").arg(ext);
    }
    if (QFile::exists(filename)) {
        int overwrite = QMessageBox::warning(mainWindow,
                             qApp->applicationName(),
                             tr("File already exists; overwrite it?"),
                             tr("Yes"), tr("No"), QString::null, 1);
        if (overwrite == 1) {
            return "";
        }
    }
    QFileInfo info(filename);
    settings.setValue("Files/LastDir", info.absolutePath());
    // Add it to the recent files list if it's a native document file
    if (fileExtension == extension) {
        QString absPath = info.absoluteFilePath();
        if (!recentFiles.contains(absPath)) {
            recentFiles.prepend(info.absoluteFilePath());
            if (recentFiles.count() > MAX_RECENT_FILES) {
                recentFiles.removeLast();
            }
        }
    }
    return filename;
}

/**
 * Handle a selection of the "File" menu's "Open" action by emitting
 * openFile() with an appropriate argument.  If no file has been selected,
 * return without doing anything.
 */
void QQMenuHelper::emitOpenFile()
{
    QSettings settings;
    QString lastDir = getLastDir(&settings);
    QString filter = QString("%1 (*.%2)").arg(description).arg(extension);
    QString filename = QFileDialog::getOpenFileName(mainWindow,
                           tr("Choose a file"), lastDir, filter);
    if (filename.isEmpty()) {
        return;
    }
    QFileInfo info(filename);
    settings.setValue("Files/LastDir", info.absolutePath());
    QString absPath = info.absoluteFilePath();
    if (!recentFiles.contains(absPath)) {
        recentFiles.prepend(absPath);
        if (recentFiles.count() > MAX_RECENT_FILES) {
            recentFiles.removeLast();
        }
    }
    emit openFile(filename);
}

/**
 * Start the process of opening a file selected from the recent files menu.
 * Emit openFile() if the file still exists, otherwise display an error
 * message.
 * @param action The action representing the selected menu item.
 */
void QQMenuHelper::openRecent()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    QString path = action->text();
    if (!QFile::exists(path)) {
        QMessageBox::warning(mainWindow, qApp->applicationName(),
                             tr("File does not exist"));
        return;
    }
    if (QFileInfo(path).isDir()) {
        QMessageBox::warning(mainWindow, qApp->applicationName(),
                             tr("The selected item is a directory"));
    }
    emit openFile(path);
}

/**
 * Get a darker version of the provided pixmap.  Used on Mac OS X to show via
 * the titlebar icon that the current document has been modified.
 *
 * @param pixmap The pixmap to be darkened
 * @return The darkened version of the pixmap
 */
QPixmap QQMenuHelper::darkenPixmap(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int imgh = img.height();
    int imgw = img.width();
    int h, s, v, a;
    QRgb pixel;
    for (int y = 0; y < imgh; ++y) {
        for (int x = 0; x < imgw; ++x) {
            pixel = img.pixel(x, y);
            a = qAlpha(pixel);
            QColor hsvColor(pixel);
            hsvColor.getHsv(&h, &s, &v);
            s = qMin(100, s * 2);
            v = v / 2;
            hsvColor.setHsv(h, s, v);
            pixel = hsvColor.rgb();
            img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
        }
    }
    return QPixmap::fromImage(img);
}
