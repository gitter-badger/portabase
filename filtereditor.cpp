/*
 * filtereditor.cpp
 *
 * (c) 2002 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <qhbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qvbox.h>
#include "database.h"
#include "condition.h"
#include "conditioneditor.h"
#include "filter.h"
#include "filtereditor.h"

FilterEditor::FilterEditor(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), db(0), filter(0)
{
    setCaption(tr("Filter Editor") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);

    QHBox *hbox = new QHBox(vbox);
    new QLabel(tr("Filter Name"), hbox);
    nameBox = new QLineEdit(hbox);

    listBox = new QListBox(vbox);

    hbox = new QHBox(vbox);
    QPushButton *addButton = new QPushButton(tr("Add"), hbox);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addCondition()));
    QPushButton *editButton = new QPushButton(tr("Edit"), hbox);
    connect(editButton, SIGNAL(clicked()), this, SLOT(editCondition()));
    QPushButton *deleteButton = new QPushButton(tr("Delete"), hbox);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteCondition()));
    QPushButton *upButton = new QPushButton(tr("Up"), hbox);
    connect(upButton, SIGNAL(clicked()), this, SLOT(moveUp()));
    QPushButton *downButton = new QPushButton(tr("Down"), hbox);
    connect(downButton, SIGNAL(clicked()), this, SLOT(moveDown()));

    showMaximized();
}

FilterEditor::~FilterEditor()
{
    if (filter) {
        delete filter;
    }
}

int FilterEditor::edit(Database *subject, QString filterName)
{
    db = subject;
    originalName = filterName;
    nameBox->setText(filterName);
    if (filter) {
        delete filter;
    }
    conditionEditor = new ConditionEditor(db, this);
    filter = new Filter(db, filterName);
    updateList();
    int result = exec();
    while (result) {
        if (hasValidName()) {
            break;
        }
        else {
            result = exec();
        }
    }
    delete conditionEditor;
    return result;
}

QString FilterEditor::getName()
{
    return nameBox->text();
}

void FilterEditor::addCondition()
{
    Condition *condition = new Condition(db);
    if (conditionEditor->edit(condition)) {
        conditionEditor->applyChanges(condition);
        filter->addCondition(condition);
        updateList();
    }
    else {
        delete condition;
    }
}

void FilterEditor::editCondition()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    Condition *condition = filter->getCondition(selected);
    if (conditionEditor->edit(condition)) {
        conditionEditor->applyChanges(condition);
        updateList();
        listBox->setCurrentItem(selected);
    }
}

void FilterEditor::deleteCondition()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    filter->deleteCondition(selected);
    updateList();
}

void FilterEditor::moveUp()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    if (filter->moveConditionUp(selected)) {
        updateList();
        listBox->setCurrentItem(listBox->item(selected - 1));
    }
}

void FilterEditor::moveDown()
{
    int selected = listBox->currentItem();
    if (selected == -1) {
        return;
    }
    if (filter->moveConditionDown(selected)) {
        updateList();
        listBox->setCurrentItem(selected + 1);
    }
}

void FilterEditor::updateList()
{
    listBox->clear();
    int count = filter->getConditionCount();
    for (int i = 0; i < count; i++) {
        listBox->insertItem(filter->getCondition(i)->getDescription());
    }
}

bool FilterEditor::hasValidName()
{
    QString name = nameBox->text();
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("No name entered"));
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
    if (name.find(':') != -1) {
        QMessageBox::warning(this, tr("PortaBase"),
                             tr("Name must not contain ':'"));
        return FALSE;
    }
    // check for other filters with same name
    QStringList filterNames = db->listFilters();
    if (filterNames.findIndex(name) != -1) {
        QMessageBox::warning(this, tr("PortaBase"), tr("Duplicate name"));
        return FALSE;
    }
    return TRUE;
}

void FilterEditor::applyChanges()
{
    db->deleteFilter(originalName);
    filter->setName(nameBox->text());
    db->addFilter(filter);
}

void FilterEditor::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
}
