/*
 * enumeditor.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qinputdialog.h>
#include "desktop/resource.h"
#else
#include "inputdialog.h"
#endif

#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qlayout.h>
#include "database.h"
#include "enumeditor.h"

EnumEditor::EnumEditor(QWidget *parent, const char *name, WFlags f)
  : QDialog(parent, name, TRUE, f), db(0), eeiName("_eeiname"), eeiIndex("_eeiindex"), eecIndex("_eecindex"), eecType("_eectype"), eecOldName("_eecoldname"), eecNewName("_eecnewname")
{
    setCaption(tr("Enum Editor") + " - " + tr("PortaBase"));
    QVBoxLayout *vbox = new QVBoxLayout(this);
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    vbox->setMargin(8);
    vbox->addWidget(new QLabel("<center><b>" + tr("Enum Editor")
                               + "</b></center>", this));
#endif

    QHBox *hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QLabel(tr("Enum Name"), hbox);
    nameBox = new QLineEdit(hbox);

    listBox = new QListBox(this);
    vbox->addWidget(listBox);

    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    QPushButton *addButton = new QPushButton(tr("Add"), hbox);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addOption()));
    QPushButton *editButton = new QPushButton(tr("Edit"), hbox);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editOption()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), hbox);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteOption()));
    QPushButton *upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    QPushButton *downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

#if defined(DESKTOP)
    vbox->addWidget(new QLabel(" ", this));
    hbox = new QHBox(this);
    vbox->addWidget(hbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    vbox->setResizeMode(QLayout::FreeResize);
    setMinimumWidth(parent->width() / 2);
    setMinimumHeight(parent->height());
    setIcon(Resource::loadPixmap("portabase"));
#else
    showMaximized();
#endif
}

EnumEditor::~EnumEditor()
{

}

int EnumEditor::edit(Database *subject, QString enumName)
{
    db = subject;
    originalName = enumName;
    nameBox->setText(enumName);
    if (!enumName.isEmpty()) {
        int enumId = db->getEnumId(enumName);
        QStringList options = db->listEnumOptions(enumId);
        int optionCount = options.count();
        for (int i = 0; i < optionCount; i++) {
            info.Add(eeiName [options[i].utf8()] + eeiIndex [i]);
        }
        updateList();
    }
    int result = exec();
    while (result) {
        if (hasValidName()) {
            if (info.GetSize() > 0) {
                break;
            }
            else {
                QMessageBox::warning(this, tr("PortaBase"),
                                     tr("Must have at least one option"));
                result = exec();
            }
        }
        else {
            result = exec();
        }
    }
    return result;
}

QString EnumEditor::getName()
{
    return nameBox->text();
}

void EnumEditor::addOption()
{
    bool ok = TRUE;
    QString text;
    while (ok) {
#if defined(DESKTOP)
        text = QInputDialog::getText(tr("Add"), tr("Option text"),
                                     QLineEdit::Normal, QString::null,
                                     &ok, this);
#else
        text = InputDialog::getText(tr("Add"), tr("Option text"),
                                    QString::null, &ok, this);
#endif
        if (ok) {
            if (isValidOption(text)) {
                break;
            }
        }
    }
    if (ok) {
        QCString utf8Text = text.utf8();
        changes.Add(eecIndex [changes.GetSize()] + eecType [ADD_OPTION]
                 + eecOldName [""] + eecNewName [utf8Text]);
        info.Add(eeiName [utf8Text] + eeiIndex [info.GetSize()]);
        updateList();
    }
}

void EnumEditor::editOption()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    QString originalText = listBox->text(selected);
    bool ok = TRUE;
    QString newText;
    while (ok) {
#if defined(DESKTOP)
        newText = QInputDialog::getText(tr("Edit"), tr("Option text"),
                                        QLineEdit::Normal, originalText,
                                        &ok, this);
#else
        newText = InputDialog::getText(tr("Edit"), tr("Option text"),
                                       originalText, &ok, this);
#endif
        if (ok) {
            if (newText == originalText) {
                // clicked ok but left unchanged
                ok = FALSE;
            }
            else if (isValidOption(newText)) {
                break;
            }
        }
    }
    if (ok) {
        QCString utf8NewText = newText.utf8();
        int rowIndex = info.Find(eeiIndex [selected]);
        eeiName (info[rowIndex]) = utf8NewText;
        changes.Add(eecIndex [changes.GetSize()] + eecType [RENAME_OPTION]
                   + eecOldName [originalText.utf8()]
                   + eecNewName [utf8NewText]);
        updateList();
        listBox->setCurrentItem(selected);
    }
}

void EnumEditor::deleteOption()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    if (!originalName.isEmpty() && info.GetSize() == 1) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Must have at least one option"));
        return;
    }
    QString name = listBox->text(selected);
    QStringList options = listCurrentOptions();
    options.remove(name);
    bool ok = TRUE;
    QString replace("");
    if (!originalName.isEmpty()) {
#if defined(DESKTOP)
        replace = QInputDialog::getItem(tr("Delete"),
                                        tr("Replace where used with:"),
                                        options, 0, FALSE, &ok, this);
#else
        replace = InputDialog::getItem(tr("Delete"),
                                       tr("Replace where used with:"),
                                       options, 0, FALSE, &ok, this);
#endif
    }
    if (ok) {
        int rowIndex = info.Find(eeiIndex [selected]);
        info.RemoveAt(rowIndex);
        changes.Add(eecIndex [changes.GetSize()] + eecType [DELETE_OPTION]
                    + eecOldName [name.utf8()] + eecNewName [replace.utf8()]);
        updateList();
    }
}

void EnumEditor::moveUp()
{
    int selected = listBox->currentItem();
    if (selected < 1) {
        return;
    }
    int row1 = info.Find(eeiIndex [selected]);
    int row2 = info.Find(eeiIndex [selected - 1]);
    eeiIndex (info[row1]) = selected - 1;
    eeiIndex (info[row2]) = selected;
    updateList();
    listBox->setCurrentItem(selected - 1);
}

void EnumEditor::moveDown()
{
    int selected = listBox->currentItem();
    int optionCount = info.GetSize();
    if (selected == -1 || selected == optionCount - 1) {
        return;
    }
    int row1 = info.Find(eeiIndex [selected]);
    int row2 = info.Find(eeiIndex [selected + 1]);
    eeiIndex (info[row1]) = selected + 1;
    eeiIndex (info[row2]) = selected;
    updateList();
    listBox->setCurrentItem(selected + 1);
}

void EnumEditor::updateList()
{
    listBox->clear();
    listBox->insertStringList(listCurrentOptions());
}

bool EnumEditor::hasValidName()
{
    QString name = nameBox->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("PortaBase"), tr("No name entered"));
        return FALSE;
    }
    if (name == originalName) {
        // hasn't changed and isn't empty, must be valid
        return TRUE;
    }
    if (name[0] == '_') {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Name must not start with '_'"));
        return FALSE;
    }
    // check for other enums with same name
    QStringList enumNames = db->listEnums();
    if (enumNames.findIndex(name) != -1) {
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
        return FALSE;
    }
    return TRUE;
}

bool EnumEditor::isValidOption(QString option)
{
    if (!option.isEmpty()) {
        if (option[0] == '_') {
            QMessageBox::warning(this, tr("PortaBase"),
                                 tr("Name must not start with '_'"));
            return FALSE;
        }
    }
    // check for other options with same text
    QStringList options = listCurrentOptions();
    if (options.findIndex(option) != -1) {
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
        return FALSE;
    }
    return TRUE;
}

void EnumEditor::applyChanges()
{
    QString enumName = nameBox->text();
    if (originalName == "") {
        db->addEnum(enumName, listCurrentOptions());
        return;
    }
    if (enumName != originalName) {
        db->renameEnum(originalName, enumName);
    }
    c4_View sorted = changes.SortOn(eecIndex);
    int count = changes.GetSize();
    for (int i = 0; i < count; i++) {
        int changeType = eecType (sorted[i]);
        QString oldName = QString::fromUtf8(eecOldName (sorted[i]));
        QString newName = QString::fromUtf8(eecNewName (sorted[i]));
        if (changeType == ADD_OPTION) {
            db->addEnumOption(enumName, newName);
        }
        else if (changeType == RENAME_OPTION) {
            db->renameEnumOption(enumName, oldName, newName);
        }
        else if (changeType == DELETE_OPTION) {
            db->deleteEnumOption(enumName, oldName, newName);
        }
    }
    db->setEnumOptionSequence(enumName, listCurrentOptions());
}

QStringList EnumEditor::listCurrentOptions()
{
    QStringList options;
    c4_View sorted = info.SortOn(eeiIndex);
    int optionCount = sorted.GetSize();
    for (int i = 0; i < optionCount; i++) {
        options.append(QString::fromUtf8(eeiName (sorted[i])));
    }
    return options;
}