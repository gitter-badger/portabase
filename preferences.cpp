/*
 * preferences.cpp
 *
 * (c) 2002-2003 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#if defined(DESKTOP)
#include <qhbox.h>
#include <qpushbutton.h>
#include "desktop/config.h"
#include "desktop/resource.h"
#else
#include <qpe/config.h>
#endif

#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qvbox.h>
#include "preferences.h"

Preferences::Preferences(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f)
{
    setCaption(tr("Preferences") + " - " + tr("PortaBase"));
    vbox = new QVBox(this);
    vbox->resize(size());
#if defined(Q_WS_WIN)
    setSizeGripEnabled(TRUE);
    new QLabel("<center><b>" + tr("Preferences") + "</b></center>", vbox);
#endif
#if defined(DESKTOP)
    sizeFactor = 1;
#else
    sizeFactor = 10;
#endif

    QGroupBox *fontGroup = new QGroupBox(2, Qt::Horizontal, tr("Font"), vbox);
    new QLabel(tr("Name"), fontGroup);
    fontName = new QComboBox(FALSE, fontGroup);
    fonts = fontdb.families();
    fontName->insertStringList(fonts);
    new QLabel(tr("Size"), fontGroup);
    fontSize = new QComboBox(FALSE, fontGroup);
    connect(fontName, SIGNAL(activated(int)), this, SLOT(updateSizes(int)));
    QFont currentFont = qApp->font();
    int fontCount = fonts.count();
    QString family = currentFont.family().lower();
    int index = -1;
    int i;
    for (i = 0; i < fontCount; i++) {
        if (family == fonts[i].lower()) {
            index = i;
        }
    }
    // Windows defaults to a font not in QFontDatabase ???
    if (index == -1) {
        fontName->insertItem(currentFont.family());
        index = fontName->count() - 1;
    }
    fontName->setCurrentItem(index);
    updateSizes(index);
    int size = currentFont.pointSize();
    index = sizes.findIndex(size * sizeFactor);
    if (index > -1) {
        fontSize->setCurrentItem(index);
    }
    connect(fontSize, SIGNAL(activated(int)), this, SLOT(updateSample(int)));
    new QLabel(tr("Sample"), fontGroup);
    sample = new QLabel(tr("Sample text"), fontGroup);

    QGroupBox *generalGroup = new QGroupBox(1, Qt::Horizontal, tr("General"),
                                            vbox);
    confirmDeletions = new QCheckBox(tr("Confirm deletions"), generalGroup);
    Config conf("portabase");
    conf.setGroup("General");
    confirmDeletions->setChecked(conf.readBoolEntry("ConfirmDeletions"));
    booleanToggle = new QCheckBox(tr("Allow checkbox edit in data viewer"),
                                  generalGroup);
    booleanToggle->setChecked(conf.readBoolEntry("BooleanToggle"));
    showSeconds = new QCheckBox(tr("Show seconds for times"), generalGroup);
    showSeconds->setChecked(conf.readBoolEntry("ShowSeconds"));
    QHBox *hbox = new QHBox(generalGroup);
    noteWrap = new QCheckBox(tr("Wrap Notes"), hbox);
    noteWrap->setChecked(conf.readBoolEntry("NoteWrap", TRUE));
    wrapType = new QComboBox(hbox);
    wrapType->insertItem(tr("at whitespace"));
    wrapType->insertItem(tr("anywhere"));
    if (conf.readBoolEntry("WrapAnywhere")) {
        wrapType->setCurrentItem(1);
    }
    wrapType->setEnabled(noteWrap->isChecked());
    connect(noteWrap, SIGNAL(toggled(bool)), wrapType, SLOT(setEnabled(bool)));

#if defined(DESKTOP)
    QGroupBox *dateGroup = new QGroupBox(2, Qt::Horizontal,
                                         tr("Date and Time"), vbox);
    Config config("qpe");
    config.setGroup("Date");
    new QLabel(tr("Date format"), dateGroup);
    PBDateFormat df(QChar(config.readEntry("Separator", "/")[0]),
	    (PBDateFormat::Order)config.readNumEntry("ShortOrder",
                                                  PBDateFormat::DayMonthYear),
	    (PBDateFormat::Order)config.readNumEntry("LongOrder",
                                                  PBDateFormat::DayMonthYear));
    dateFormatCombo = new QComboBox(dateGroup);
    int currentdf = 0;
    date_formats[0] = PBDateFormat('/', PBDateFormat::MonthDayYear);
    date_formats[1] = PBDateFormat('.', PBDateFormat::DayMonthYear);
    date_formats[2] = PBDateFormat('-', PBDateFormat::YearMonthDay, 
	    PBDateFormat::DayMonthYear);
    date_formats[3] = PBDateFormat('/', PBDateFormat::DayMonthYear);
    for (i = 0; i < 4; i++) {
        dateFormatCombo->insertItem(date_formats[i].toNumberString());
        if (df == date_formats[i]) {
	    currentdf = i;
        }
    }
    dateFormatCombo->setCurrentItem(currentdf);

    config.setGroup("Time");
    new QLabel(tr("Time format"), dateGroup);
    ampmCombo = new QComboBox(dateGroup);
    ampmCombo->insertItem(tr("24 hour"), 0);
    ampmCombo->insertItem(tr("12 hour"), 1);
    int show12hr = config.readBoolEntry("AMPM", TRUE) ? 1 : 0;
    ampmCombo->setCurrentItem(show12hr);

    new QLabel(tr("Weeks start on"), dateGroup);
    weekStartCombo = new QComboBox(dateGroup);
    weekStartCombo->insertItem(tr("Sunday"), 0);
    weekStartCombo->insertItem(tr("Monday"), 1);
    int startMonday =  config.readBoolEntry("MONDAY") ? 1 : 0;
    weekStartCombo->setCurrentItem( startMonday );

    hbox = new QHBox(vbox);
    new QWidget(hbox);
    QPushButton *okButton = new QPushButton(tr("OK"), hbox);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    new QWidget(hbox);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"), hbox);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    new QWidget(hbox);
    setIcon(Resource::loadPixmap("portabase"));
#else
    new QWidget(vbox);
    showMaximized();
#endif
}

Preferences::~Preferences()
{

}

void Preferences::updateSizes(int selection)
{
    int currentSize = 0;
    if (fontSize->count() > 0) {
        currentSize = sizes[fontSize->currentItem()];
    }
    sizes = fontdb.pointSizes(fontName->text(selection));
    int count = sizes.count();
    if (count == 0) {
        QFont currentFont = qApp->font();
        int size = currentFont.pointSize();
        sizes.append(size);
        count = 1;
    }
    fontSize->clear();
    int newIndex = 0;
    for (int i = 0; i < count; i++) {
        fontSize->insertItem(QString::number(sizes[i] / sizeFactor));
        if (sizes[i] <= currentSize) {
            newIndex = i;
        }
    }
    fontSize->setCurrentItem(newIndex);
    if (currentSize > 0) {
        updateSample(newIndex);
    }
}

void Preferences::updateSample(int sizeSelection)
{
    QString name = fontName->currentText();
    int size = sizes[sizeSelection] / sizeFactor;
    QFont font(name, size);
    sample->setFont(font);
}

QFont Preferences::applyChanges()
{
#if defined(DESKTOP)
    Config config("qpe");
    config.setGroup("Date");
    PBDateFormat df = date_formats[dateFormatCombo->currentItem()];
    config.writeEntry("Separator", QString(df.separator()));
    config.writeEntry("ShortOrder", df.shortOrder());
    config.writeEntry("LongOrder", df.longOrder());
    config.setGroup("Time");
    config.writeEntry("AMPM", ampmCombo->currentItem());
    config.writeEntry("MONDAY", weekStartCombo->currentItem());
#endif

    Config conf("portabase");
    conf.setGroup("General");
    conf.writeEntry("ConfirmDeletions", confirmDeletions->isChecked());
    conf.writeEntry("BooleanToggle", booleanToggle->isChecked());
    conf.writeEntry("ShowSeconds", showSeconds->isChecked());
    conf.writeEntry("NoteWrap", noteWrap->isChecked());
    conf.writeEntry("WrapAnywhere", wrapType->currentItem() == 1);
    conf.setGroup("Font");
    QString name = fontName->currentText();
    int size = sizes[fontSize->currentItem()] / sizeFactor;
    conf.writeEntry("Name", name);
    conf.writeEntry("Size", size);
    QFont font(name, size);
    qApp->setFont(font);
    return font;
}

void Preferences::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    vbox->resize(size());
}
