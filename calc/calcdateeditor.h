/*
 * calcdateeditor.h
 *
 * (c) 2003-2004 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef CALCDATEEDITOR_H
#define CALCDATEEDITOR_H

#include <qstringlist.h>
#include "../pbdialog.h"

class CalcNode;
class DateWidget;
class QButtonGroup;
class QComboBox;

class CalcDateEditor: public PBDialog
{
    Q_OBJECT
public:
    CalcDateEditor(const QStringList &colNames, int *colTypes,
                   QWidget *parent = 0, const char *name = 0);
    ~CalcDateEditor();

    CalcNode *createNode();
    void setNode(CalcNode *node);
    void reset();
    void updateNode(CalcNode *node);

private:
    QButtonGroup *group;
    QComboBox *columnList;
    DateWidget *dateWidget;
};

#endif
