/* This file is part of the KDE project
   Copyright (C) 2005,2006 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kexicsvexportwizard.h"
#include "kexicsvwidgets.h"
#include <kexidb/cursor.h>
#include <kexidb/utils.h>
#include <core/KexiMainWindowIface.h>
#include <core/kexiproject.h>
#include <core/kexipartinfo.h>
#include <core/kexipartmanager.h>
#include <core/kexiguimsghandler.h>
#include <kexiutils/utils.h>
#include <widget/kexicharencodingcombobox.h>
#include <widget/KexiFileWidget.h>

#include <QCheckBox>
#include <QGroupBox>
#include <QClipboard>
#include <QTextStream>
#include <QGridLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ksavefile.h>
#include <kglobal.h>
#include <KDialog>

KexiCSVExportWizard::KexiCSVExportWizard(const KexiCSVExport::Options& options,
        QWidget * parent)
        : K3Wizard(parent)
        , m_options(options)
// , m_mode(mode)
// , m_itemId(itemId)
        , m_fileSavePage(0)
        , m_defaultsBtn(0)
        , m_importExportGroup(KGlobal::config()->group("ImportExport"))
        , m_rowCount(-1)
        , m_rowCountDetermined(false)
        , m_cancelled(false)
{
    if (m_options.mode == KexiCSVExport::Clipboard) {
        finishButton()->setText(i18n("Copy"));
        backButton()->hide();
    } else {
        finishButton()->setText(i18n("Export"));
    }
    helpButton()->hide();

    QString infoLblFromText;
    KexiGUIMessageHandler msgh(this);
    m_tableOrQuery = new KexiDB::TableOrQuerySchema(
        KexiMainWindowIface::global()->project()->dbConnection(), m_options.itemId);
    if (m_tableOrQuery->table()) {
        if (m_options.mode == KexiCSVExport::Clipboard) {
            setWindowTitle(i18n("Copy Data From Table to Clipboard"));
            infoLblFromText = i18n("Copying data from table:");
        } else {
            setWindowTitle(i18n("Export Data From Table to CSV File"));
            infoLblFromText = i18n("Exporting data from table:");
        }
    } else if (m_tableOrQuery->query()) {
        if (m_options.mode == KexiCSVExport::Clipboard) {
            setWindowTitle(i18n("Copy Data From Query to Clipboard"));
            infoLblFromText = i18n("Copying data from table:");
        } else {
            setWindowTitle(i18n("Export Data From Query to CSV File"));
            infoLblFromText = i18n("Exporting data from query:");
        }
    } else {
        msgh.showErrorMessage(KexiMainWindowIface::global()->project()->dbConnection(),
                              i18n("Could not open data for exporting."));
        m_cancelled = true;
        return;
    }
    // OK, source data found.

    // Setup pages

    // 1. File Save Page
    if (m_options.mode == KexiCSVExport::File) {
        m_fileSavePage = new KexiFileWidget(
            KUrl("kfiledialog:///CSVImportExport"), //startDir
            KexiFileWidget::Custom | KexiFileWidget::SavingFileBasedDB,
            this);
        m_fileSavePage->setObjectName("m_fileSavePage");
        //m_fileSavePage->setMinimumHeight(kapp->desktop()->availableGeometry().height() / 2);
        m_fileSavePage->setAdditionalFilters(csvMimeTypes().toSet());
        m_fileSavePage->setDefaultExtension("csv");
        m_fileSavePage->setLocationText(
            KexiUtils::stringToFileName(m_tableOrQuery->captionOrName()));
        connect(m_fileSavePage, SIGNAL(rejected()), this, SLOT(reject()));
        addPage(m_fileSavePage, i18n("Enter Name of File You Want to Save Data To"));
    }

    /* 2. Export options
        m_exportOptionsPage
        exportOptionsLyr
            m_infoLblFrom
            m_infoLblTo
            m_showOptionsButton
            m_exportOptionsSection
            exportOptionsSectionLyr
    */
    m_exportOptionsPage = new QWidget(this);
    m_exportOptionsPage->setObjectName("m_exportOptionsPage");

    QGridLayout *exportOptionsLyr = new QGridLayout(m_exportOptionsPage);
    exportOptionsLyr->setObjectName("exportOptionsLyr");

    m_infoLblFrom = new KexiCSVInfoLabel(infoLblFromText, m_exportOptionsPage, true/*showFnameLine*/);
    KexiPart::Info *partInfo = Kexi::partManager().infoForClass(
            QString("org.kexi-project.%1").arg(m_tableOrQuery->table() ? "table" : "query"));
    if (partInfo)
        m_infoLblFrom->setIcon(partInfo->itemIcon());
    m_infoLblFrom->separator()->hide();
    m_infoLblFrom->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    exportOptionsLyr->addWidget(m_infoLblFrom, 0, 0, 1, 2);

    m_infoLblTo = new KexiCSVInfoLabel(
        (m_options.mode == KexiCSVExport::File) ? i18n("To CSV file:") : i18n("To clipboard:"),
        m_exportOptionsPage, true/*showFnameLine*/);

    if (m_options.mode == KexiCSVExport::Clipboard)
        m_infoLblTo->setIcon("edit-paste");

    m_infoLblTo->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    exportOptionsLyr->addWidget(m_infoLblTo, 1, 0, 1, 2);

//    QWidget *stretchWidget = new QWidget();
//    exportOptionsLyr->addWidget(stretchWidget, 2, 0, 2, 2);
    exportOptionsLyr->setRowStretch(2, 1);

    m_showOptionsButton = new KPushButton(KGuiItem(i18n("Show Options >>"), "configure"));
    m_showOptionsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_showOptionsButton, SIGNAL(clicked()), this, SLOT(slotShowOptionsButtonClicked()));
    exportOptionsLyr->addWidget(m_showOptionsButton, 3, 1);

    // -<options section>
    m_exportOptionsSection = new QGroupBox(""/*i18n("Options")*/);
    m_exportOptionsSection->setObjectName("m_exportOptionsSection");
    m_exportOptionsSection->setAlignment(Qt::Vertical);
    m_exportOptionsSection->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    exportOptionsLyr->addWidget(m_exportOptionsSection, 4, 0, 1, 2);
#ifdef __GNUC__
#warning TODO    exportOptionsLyr->setRowStretch();
#else
#pragma WARNING( TODO exportOptionsLyr->setRowStretch(); )
#endif

    QGridLayout *exportOptionsSectionLyr = new QGridLayout;
    exportOptionsLyr->setObjectName("exportOptionsLyr");
    m_exportOptionsSection->setLayout(exportOptionsSectionLyr);

    // -delimiter
    QLabel *delimiterLabel = new QLabel(i18n("Delimiter:"));
    exportOptionsSectionLyr->addWidget(delimiterLabel, 0, 0);

    m_delimiterWidget = new KexiCSVDelimiterWidget(false /* !lineEditOnBottom*/);
    m_delimiterWidget->setDelimiter(defaultDelimiter());
    delimiterLabel->setBuddy(m_delimiterWidget);
    exportOptionsSectionLyr->addWidget(m_delimiterWidget, 0, 1);

    // -text quote
    QLabel *textQuoteLabel = new QLabel(i18n("Text quote:"));
    exportOptionsSectionLyr->addWidget(textQuoteLabel, 1, 0);

    QWidget *textQuoteWidget = new QWidget;
    QHBoxLayout *textQuoteLyr = new QHBoxLayout(textQuoteWidget);

    m_textQuote = new KexiCSVTextQuoteComboBox(textQuoteWidget);
    m_textQuote->setTextQuote(defaultTextQuote());
    textQuoteLabel->setBuddy(m_textQuote);
    textQuoteLyr->addWidget(m_textQuote);
    textQuoteLyr->addStretch(0);

    exportOptionsSectionLyr->addWidget(textQuoteWidget, 1, 1);

    // - character encoding
    QLabel *characterEncodingLabel = new QLabel(i18n("Text encoding:"));
    exportOptionsSectionLyr->addWidget(characterEncodingLabel, 2, 0);

    m_characterEncodingCombo = new KexiCharacterEncodingComboBox();
    m_characterEncodingCombo->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    characterEncodingLabel->setBuddy(m_characterEncodingCombo);
    exportOptionsSectionLyr->addWidget(m_characterEncodingCombo, 2, 1);

    // - checkboxes
    m_addColumnNamesCheckBox = new QCheckBox(i18n("Add column names as the first row"));
    m_addColumnNamesCheckBox->setChecked(true);
    exportOptionsSectionLyr->addWidget(m_addColumnNamesCheckBox, 3, 1);

    m_alwaysUseCheckBox = new QCheckBox(
        m_options.mode == KexiCSVExport::Clipboard ?
          i18n("Always use above options for copying")
        : i18n("Always use above options for exporting"));
    exportOptionsLyr->addWidget(m_alwaysUseCheckBox, 5, 1);

    m_exportOptionsSection->hide();
    m_alwaysUseCheckBox->hide();
    // -</options section>

    addPage(m_exportOptionsPage, m_options.mode == KexiCSVExport::Clipboard ? i18n("Copying") : i18n("Exporting"));
    setFinishEnabled(m_exportOptionsPage, true);

    // load settings
    if (m_options.mode != KexiCSVExport::Clipboard
            && readBoolEntry("ShowOptionsInCSVExportDialog", false)) {
        show();
        slotShowOptionsButtonClicked();
    }
    if (readBoolEntry("StoreOptionsForCSVExportDialog", false)) {
        // load defaults:
        m_alwaysUseCheckBox->setChecked(true);
        QString s = readEntry("DefaultDelimiterForExportingCSVFiles", defaultDelimiter());
        if (!s.isEmpty())
            m_delimiterWidget->setDelimiter(s);
        s = readEntry("DefaultTextQuoteForExportingCSVFiles", defaultTextQuote());
        m_textQuote->setTextQuote(s); //will be invaliudated here, so not a problem
        s = readEntry("DefaultEncodingForExportingCSVFiles");
        if (!s.isEmpty())
            m_characterEncodingCombo->setSelectedEncoding(s);
        m_addColumnNamesCheckBox->setChecked(
            readBoolEntry("AddColumnNamesForExportingCSVFiles", true));
    }

    // -keep widths equal on page #2:
    int width = qMax(m_infoLblFrom->leftLabel()->sizeHint().width(),
                     m_infoLblTo->leftLabel()->sizeHint().width());
    m_infoLblFrom->leftLabel()->setFixedWidth(width);
    m_infoLblTo->leftLabel()->setFixedWidth(width);

    updateGeometry();
}

KexiCSVExportWizard::~KexiCSVExportWizard()
{
    delete m_tableOrQuery;
}

bool KexiCSVExportWizard::cancelled() const
{
    return m_cancelled;
}

void KexiCSVExportWizard::showPage(QWidget * page)
{
    if (page == m_fileSavePage) {
        m_fileSavePage->setFocus();
    } else if (page == m_exportOptionsPage) {
        if (m_options.mode == KexiCSVExport::File)
            m_infoLblTo->setFileName(m_fileSavePage->highlightedFile());
        QString text = m_tableOrQuery->captionOrName();
        if (!m_rowCountDetermined) {
            //do this costly operation only once
            m_rowCount = KexiDB::rowCount(*m_tableOrQuery);
            m_rowCountDetermined = true;
        }
        int columns = KexiDB::fieldCount(*m_tableOrQuery);
        text += "\n";
        if (m_rowCount > 0)
            text += i18n("(rows: %1, columns: %2)", m_rowCount, columns);
        else
            text += i18n("(columns: %1)", columns);
        m_infoLblFrom->setLabelText(text);
        //QFontMetrics fm(m_infoLblFrom->fileNameLabel()->font());
        //m_infoLblFrom->fileNameLabel()->setFixedHeight(fm.height() * 2 + fm.lineSpacing());
        if (m_defaultsBtn)
            m_defaultsBtn->show();
    }

    if (page != m_exportOptionsPage) {
        if (m_defaultsBtn)
            m_defaultsBtn->hide();
    }

    K3Wizard::showPage(page);
}

void KexiCSVExportWizard::next()
{
    if (currentPage() == m_fileSavePage) {
        if (!m_fileSavePage->checkSelectedFile()) {
            return;
	}
	kDebug() << "selectedFile:" << m_fileSavePage->selectedFile();
    kDebug() << "selectedUrl:" << m_fileSavePage->selectedUrl();
    kDebug() << "highlightedFile:" << m_fileSavePage->highlightedFile();
        K3Wizard::next();
        finishButton()->setFocus();
        return;
    }
    K3Wizard::next();
}

void KexiCSVExportWizard::done(int result)
{
    kDebug() << result;
    if (QDialog::Accepted == result) {
        if (m_fileSavePage) {
	    kDebug() << m_fileSavePage->highlightedFile();
            m_options.fileName = m_fileSavePage->highlightedFile();
	}
        m_options.delimiter = m_delimiterWidget->delimiter();
        m_options.textQuote = m_textQuote->textQuote();
        m_options.addColumnNames = m_addColumnNamesCheckBox->isChecked();
        if (!KexiCSVExport::exportData(*m_tableOrQuery, m_options))
            return;
    } else if (QDialog::Rejected == result) {
        //nothing to do
    }

    //store options
    if (m_options.mode != KexiCSVExport::Clipboard)
        writeEntry("ShowOptionsInCSVExportDialog", m_exportOptionsSection->isVisible());
    const bool store = m_alwaysUseCheckBox->isChecked();
    writeEntry("StoreOptionsForCSVExportDialog", store);
    // only save if an option differs from default

    if (store && m_delimiterWidget->delimiter() != defaultDelimiter())
        writeEntry("DefaultDelimiterForExportingCSVFiles", m_delimiterWidget->delimiter());
    else
        deleteEntry("DefaultDelimiterForExportingCSVFiles");
    if (store && m_textQuote->textQuote() != defaultTextQuote())
        writeEntry("DefaultTextQuoteForExportingCSVFiles", m_textQuote->textQuote());
    else
        deleteEntry("DefaultTextQuoteForExportingCSVFiles");
    if (store && !m_characterEncodingCombo->defaultEncodingSelected())
        writeEntry(
            "DefaultEncodingForExportingCSVFiles", m_characterEncodingCombo->selectedEncoding());
    else
        deleteEntry("DefaultEncodingForExportingCSVFiles");
    if (store && !m_addColumnNamesCheckBox->isChecked())
        writeEntry(
            "AddColumnNamesForExportingCSVFiles", m_addColumnNamesCheckBox->isChecked());
    else
        deleteEntry("AddColumnNamesForExportingCSVFiles");

    K3Wizard::done(result);
}

void KexiCSVExportWizard::slotShowOptionsButtonClicked()
{
    if (m_exportOptionsSection->isVisible()) {
        m_showOptionsButton->setText(i18n("Show Options >>"));
        m_exportOptionsSection->hide();
        m_alwaysUseCheckBox->hide();
        if (m_defaultsBtn)
            m_defaultsBtn->hide();
    } else {
        m_showOptionsButton->setText(i18n("Hide Options <<"));
        m_exportOptionsSection->show();
        m_alwaysUseCheckBox->show();
        if (m_defaultsBtn)
            m_defaultsBtn->show();
    }
}

void KexiCSVExportWizard::layOutButtonRow(QHBoxLayout * layout)
{
    Q3Wizard::layOutButtonRow(layout);

    //find the last sublayout
    QLayout *l = layout->itemAt(layout->count()-1)->layout();
    if (dynamic_cast<QBoxLayout*>(l)) {
        if (!m_defaultsBtn) {
            m_defaultsBtn = new KPushButton(i18n("Defaults"), this);
            QWidget::setTabOrder(backButton(), m_defaultsBtn);
            connect(m_defaultsBtn, SIGNAL(clicked()), this, SLOT(slotDefaultsButtonClicked()));
        }
        if (!m_exportOptionsSection->isVisible())
            m_defaultsBtn->hide();
        dynamic_cast<QBoxLayout*>(l)->insertWidget(0, m_defaultsBtn);
    }
}

void KexiCSVExportWizard::slotDefaultsButtonClicked()
{
    m_delimiterWidget->setDelimiter(defaultDelimiter());
    m_textQuote->setTextQuote(defaultTextQuote());
    m_addColumnNamesCheckBox->setChecked(true);
    m_characterEncodingCombo->selectDefaultEncoding();
}


static QString convertKey(const char *key, KexiCSVExport::Mode mode)
{
    QString _key(QString::fromLatin1(key));
    if (mode == KexiCSVExport::Clipboard) {
        _key.replace("Exporting", "Copying");
        _key.replace("Export", "Copy");
        _key.replace("CSVFiles", "CSVToClipboard");
    }
    return _key;
}

bool KexiCSVExportWizard::readBoolEntry(const char *key, bool defaultValue)
{
    return m_importExportGroup.readEntry(convertKey(key, m_options.mode), defaultValue);
}

QString KexiCSVExportWizard::readEntry(const char *key, const QString& defaultValue)
{
    return m_importExportGroup.readEntry(convertKey(key, m_options.mode), defaultValue);
}

void KexiCSVExportWizard::writeEntry(const char *key, const QString& value)
{
    m_importExportGroup.writeEntry(convertKey(key, m_options.mode), value);
}

void KexiCSVExportWizard::writeEntry(const char *key, bool value)
{
    m_importExportGroup.writeEntry(convertKey(key, m_options.mode), value);
}

void KexiCSVExportWizard::deleteEntry(const char *key)
{
    m_importExportGroup.deleteEntry(convertKey(key, m_options.mode));
}

QString KexiCSVExportWizard::defaultDelimiter() const
{
    if (m_options.mode == KexiCSVExport::Clipboard) {
        if (!m_options.forceDelimiter.isEmpty())
            return m_options.forceDelimiter;
        else
            return KEXICSV_DEFAULT_CLIPBOARD_DELIMITER;
    }
    return KEXICSV_DEFAULT_FILE_DELIMITER;
}

QString KexiCSVExportWizard::defaultTextQuote() const
{
    if (m_options.mode == KexiCSVExport::Clipboard)
        return KEXICSV_DEFAULT_CLIPBOARD_TEXT_QUOTE;
    return KEXICSV_DEFAULT_FILE_TEXT_QUOTE;
}

#include "kexicsvexportwizard.moc"
