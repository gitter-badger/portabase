/*
 * colorbutton.cpp
 *
 * (c) 2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qcolordialog.h>
#else
#include "colordialog.h"
#endif

#include "colorbutton.h"

ColorButton::ColorButton(const QColor &color, QWidget *parent, const char *name)
    : QPushButton(parent, name)
{
    currentColor = new QColor(color);
    background = new QColor(parent->backgroundColor());
    setPalette(QPalette(color, *background));
    connect(this, SIGNAL(clicked()), this, SLOT(launchDialog()));
}

ColorButton::~ColorButton()
{
    delete currentColor;
    delete background;
}

const QColor ColorButton::getColor()
{
    return *currentColor;
}

void ColorButton::setColor(const QColor &color)
{
    delete currentColor;
    currentColor = new QColor(color);
    setPalette(QPalette(color, *background));
}

void ColorButton::launchDialog()
{
#if defined(DESKTOP)
    QColor color = QColorDialog::getColor(*currentColor, this);
#else
    QColor color = ColorDialog::getColor(*currentColor, this);
#endif
    if (color.isValid()) {
        setColor(color);
    }
}