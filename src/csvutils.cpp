/*
 * csvutils.cpp
 *
 * (c) 2002-2003,2008-2010,2013 by Jeremy Bowman <jmbowman@alum.mit.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/** @file csvutils.cpp
 * Source file for CSVUtils
 */

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegExp>
#include <QTextStream>
#include "csvutils.h"
#include "database.h"
#include "formatting.h"
#include "image/imageutils.h"
#include "view.h"

/**
 * Constructor.
 *
 * @param delimiter The character used to separate fields
 * @param encoding "Latin-1" for that encoding, anything else for UTF-8
 * @param headers True if the file does or should start with a row of column
 *                headers
 * @param lineEnding The character(s) used to designate the end of a record
 *                   (only when writing)
 * @param addNewEnumOptions True if new option values should be automatically
 *                          added to enums when encountered during import
 */
CSVUtils::CSVUtils(QChar delimiter, const QString &encoding, bool headers,
                   const QString &lineEnding, bool addNewEnumOptions)
  : m_textquote('"'), m_add_options(addNewEnumOptions), m_delimiter(delimiter),
    m_encoding(encoding), m_headers(headers), m_line_ending(lineEnding),
    colCount(0), endStringCount(0), headersParsed(false), rowNum(1),
    calcCount(0)
{

}

/**
 * Destructor.
 */
CSVUtils::~CSVUtils()
{
    for (int i = 0; i < calcCount; i++) {
        delete calcs[i];
    }
}

/**
 * Apply the appropriate text encoding to the specified text stream.  Handles
 * any common aliases that Qt itself doesn't recognize (like "Latin-1" instead
 * of "latin1").
 *
 * @param stream The QTextStream whose encoding is to be set
 */
void CSVUtils::applyCodec(QTextStream *stream)
{
    if (m_encoding == "Latin-1") {
        stream->setCodec("latin1");
    }
    else {
        stream->setCodec(m_encoding.toLatin1());
    }
}

/**
 * Export the records which match the currently applied filter to the
 * specified file.  All fields are exported (not just the ones in the current
 * view), in the order in which they appear in the database format definition.
 * The records are listed in the current sorting order.
 *
 * @param filename The path at which to create the file
 * @param db The database containing the data to export to CSV
 * @param view The database view representing the current filter and sorting
 */
void CSVUtils::writeFile(const QString &filename, Database *db, View *view)
{
    QFile f(filename);
    f.open(QFile::WriteOnly);
    QTextStream output(&f);
    applyCodec(&output);
    if (m_headers) {
        output << encodeRow(db->listColumns());
    }
    int size = view->totalRowCount();
    ImageUtils utils;
    utils.setExportPaths(filename);
    for (int i = 0; i < size; i++) {
      QStringList row = db->getRow(view->getId(i), &utils);
        output << encodeRow(row);
    }
    f.close();
}

/**
 * Parse records from the specified CSV file into the given database.
 *
 * @param filename The file to be parsed
 * @param db The database to load the data into
 * @return Empty if no error occurred.  Otherwise, an error message optionally
 *         followed by the text of the record imported that triggered that
 *         error
 */
QStringList CSVUtils::parseFile(const QString &filename, Database *db)
{
    initialize(db, filename);
    headersParsed = false;
    QFile f(filename);
    QStringList returnVal;
    if (!f.open(QFile::ReadOnly)) {
        returnVal.append(QObject::tr("Unable to open file"));
        return returnVal;
    }
    QTextStream input(&f);
    applyCodec(&input);
    rowNum = 1;
    message = "";
    enum { S_START, S_QUOTED_FIELD, S_MAYBE_END_OF_QUOTED_FIELD,
           S_END_OF_QUOTED_FIELD, S_MAYBE_NORMAL_FIELD,
           S_NORMAL_FIELD } state = S_START;
    QChar x;
    rowString = "";
    row.clear();
    QString field = "";
    // store IDs of added rows; if there's an error, delete them
    addedIds.clear();
    bool crLast = false;
    while (!input.atEnd()) {
        input >> x; // read one char

        if (x == '\r') {
            // treat as '\n', and watch for a following real '\n'
            crLast = true;
            x = '\n';
        }
        else if (crLast && x == '\n') {
            crLast = false;
            continue;
        }
        else {
            crLast = false;
        }
        rowString += x;
        switch (state)
        {
        case S_START :
            if (x == m_textquote && m_delimiter != '\t') {
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter) {
                row.append("");
            }
            else if (x == '\n') {
                if (row.count() == 0) {
                    // blank line, ignore it
                    continue;
                }
                else {
                    // row ended on a delimiter (empty cell)
                    row.append("");
                    field = "";
                    if (!addRow(db)) {
                        break;
                    }
                }
            }
            else {
                field += x;
                state = S_MAYBE_NORMAL_FIELD;
            }
            break;
        case S_QUOTED_FIELD :
            if (x == m_textquote) {
                state = S_MAYBE_END_OF_QUOTED_FIELD;
            }
            else {
                field += x;
            }
            break;
        case S_MAYBE_END_OF_QUOTED_FIELD :
            if (x == m_textquote) {
                field += x;
                state = S_QUOTED_FIELD;
            }
            else if (x == m_delimiter || x == '\n') {
                row.append(field);
                field = "";
                if (x == '\n') {
                    if (!addRow(db)) {
                        break;
                    }
                }
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_END_OF_QUOTED_FIELD :
            if (x == m_delimiter || x == '\n') {
                row.append(field);
                field = "";
                if (x == '\n') {
                    if (!addRow(db)) {
                        break;
                    }
                }
                state = S_START;
            }
            else {
                state = S_END_OF_QUOTED_FIELD;
            }
            break;
        case S_MAYBE_NORMAL_FIELD :
            if (x == m_textquote && m_delimiter != '\t') {
                field = "";
                state = S_QUOTED_FIELD;
                break;
            }
        case S_NORMAL_FIELD :
            if (x == m_delimiter) {
                row.append(field);
                field = "";
                state = S_START;
            }
            else if (x == '\n') {
                row.append(field);
                field = "";
                if (!addRow(db)) {
                    break;
                }
            }
            else {
                field += x;
            }
        }
        if (!message.isEmpty()) {
            break;
        }
    }
    if (message.isEmpty() && row.count() > 0) {
        // last line doesn't end with '\n'
        row.append(field);
        addRow(db);
    }
    if (!message.isEmpty()) {
        // an error was encountered; delete any rows that were added
        int count = addedIds.count();
        for (int i = count - 1; i > -1; i--) {
            db->deleteRow(addedIds[i]);
        }
        returnVal.append(message);
        returnVal.append(rowString);
    }
    f.close();
    return returnVal;
}

/**
 * Convert the provided row into a text line suitable for inclusion in a
 * CSV file.
 *
 * @param row A row of database fields in text form
 * @return A CSV record
 */
QString CSVUtils::encodeRow(QStringList row)
{
    QString result;
    int count = (int)row.count();
    for (int i = 0; i < count; i++) {
        result += encodeCell(row[i]);
        if (i < count - 1) {
            result += m_delimiter;
        }
    }
    return result + m_line_ending;
}

/**
 * Encode the given string into text usable as a CSV field.  This is
 * typically the same as the input value unless it contains newlines,
 * the quoting character, or the field delimiting character.
 *
 * @param content The text to be converted
 * @return A CSV field string
 */
QString CSVUtils::encodeCell(QString content)
{
    if (m_delimiter == '\t') {
        // tsv files just don't allow tabs in fields
        return content.replace("\t", "");
    }
    if (content.contains('"') == 0 && content.contains(m_delimiter) == 0
        && content.contains('\n') == 0 && content.contains('\r') == 0) {
        return content;
    }
    QString result("\"");
    result += content.replace(QRegExp("\""), "\"\"");
    return result + "\"";
}

/**
 * Prepare to import content into the specified database from the named file.
 *
 * @param db The database to import data into
 * @param filename Path to the file to be imported from
 */
void CSVUtils::initialize(Database *db, const QString &filename)
{
    colNames = db->listColumns();
    types = db->listTypes();
    colCount = colNames.count();
    endStringCount = 0;
    int i;
    for (i = colCount - 1; i > -1; i--) {
        int type = types[i];
        if (type != STRING && type != NOTE) {
            break;
        }
        endStringCount++;
    }
    for (i = 0; i < calcCount; i++) {
        delete calcs[i];
    }
    calcs.clear();
    calcDecimals.clear();
    for (i = 0; i < colCount; i++) {
        if (types[i] == CALC) {
            int decimals = 2;
            calcs.append(db->loadCalc(colNames[i], &decimals));
            calcDecimals.append(decimals);
        }
    }
    calcCount = calcs.count();
    QFileInfo info(filename);
    db->setImportBasePath(info.absolutePath() + "/");
}

/**
 * Add the last parsed row of CSV data to the specified database.
 *
 * @param db The database to import data into
 * @return True if successful, false if an error occurred
 */
bool CSVUtils::addRow(Database *db)
{
    if (m_headers && !headersParsed) {
        headersParsed = true;
        return true;
    }
    int countDiff = colCount - row.count();
    int i;
    if (countDiff > 0 && countDiff <= endStringCount) {
        // last columns may have been blank in Excel or whatever...
        for (i = 0; i < countDiff; i++) {
            row.append("");
        }
    }
    while (countDiff < 0) {
        // are the extra columns empty and ignorable?
        if (row.last() == "") {
            row.removeLast();
        }
        countDiff++;
    }
    int rowId = 0;
    if (calcCount > 0) {
        // calculate the derived values in the added row
        int calcIndex = 0;
        for (i = 0; i < colCount; i++) {
            if (types[i] == CALC) {
                double value = calcs[calcIndex]->value(row, colNames);
                int decimals = calcDecimals[calcIndex];
                row[i] = Formatting::formatDouble(value, decimals);
                calcIndex++;
            }
        }
    }
    message = db->addRow(row, &rowId, false, true, m_add_options);
    if (!message.isEmpty()) {
        message = QObject::tr("Error in row %1").arg(rowNum) + "\n" + message;
        return false;
    }
    addedIds.append(rowId);
    row.clear();
    rowString = "";
    rowNum++;
    return true;
}
