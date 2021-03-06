/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestChangeTracking.h"

#include <QtGui>
#include <KDebug>
#include <QtScript>
#include <QtTest>

#include <KoOdfStylesReader.h>
#include <KoStore.h>
#include <KoOdfStylesReader.h>
#include <KoTextLoader.h>
#include <KoTextWriter.h>
#include <KoXmlReader.h>
#include <KoOdfReadStore.h>
#include <KoOdfWriteStore.h>
#include <KTemporaryFile>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoTextShapeDataBase.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <kcomponentdata.h>
#include <KoTextDebug.h>
#include <KoListStyle.h>
#include <KoTableStyle.h>
#include <KoTableCellStyle.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoText.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextSharedLoadingData.h>
#include <KoTextSharedSavingData.h>
#include <KoTextDocument.h>
#include <kstandarddirs.h>

#include <KoGenChanges.h>
#include <KoChangeTracker.h>
#include <KoDeleteChangeMarker.h>
#include <KoChangeTrackerElement.h>

TestChangeTracking::TestChangeTracking()
{
    componentData = new KComponentData("TestLoading");
    componentData->dirs()->addResourceType("styles", "data", "words/styles/");
}

TestChangeTracking::~TestChangeTracking()
{
    delete componentData;
}

void TestChangeTracking::init()
{
}

void TestChangeTracking::cleanup()
{
}

QTextDocument *TestChangeTracking::documentFromOdt(const QString &odt, const QString &changeFormat)
{
    if (!QFile(odt).exists()) {
        qFatal("%s does not exist", qPrintable(odt));
        return 0;
    }

    KoStore *readStore = KoStore::createStore(odt, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        qDebug() << "Parsing error : " << error;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoStyleManager *styleManager = new KoStyleManager(0);
    KoChangeTracker *changeTracker = new KoChangeTracker;
    KoInlineTextObjectManager *inlineTextObjectManager = new KoInlineTextObjectManager;
    if (changeFormat == "DeltaXML")
        changeTracker->setSaveFormat(KoChangeTracker::DELTAXML);
    else
        changeTracker->setSaveFormat(KoChangeTracker::ODF_1_2);


    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store(), *componentData);
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KoTextSharedLoadingData *textSharedLoadingData = new KoTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KOTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    QTextDocument *document = new QTextDocument;
    KoTextDocument(document).setInlineTextObjectManager(inlineTextObjectManager); // required while saving
    KoTextDocument(document).setStyleManager(styleManager);
    KoTextDocument(document).setChangeTracker(changeTracker);

    KoTextLoader loader(shapeLoadingContext);
    QTextCursor cursor(document);
    loader.loadBody(body, cursor);   // now let's load the body from the ODF KoXmlElement.

    delete readStore;
    return document;
}

QString TestChangeTracking::documentToOdt(const QString &testCase, QTextDocument *document)
{
    Q_UNUSED(testCase);

    QString odt("test.odt");
    if (QFile::exists(odt))
        QFile::remove(odt);
    QFile f(odt);
    f.open(QFile::WriteOnly);
    f.close();

    KoStore *store = KoStore::createStore(odt, KoStore::Write, "application/vnd.oasis.opendocument.text", KoStore::Zip);
    KoOdfWriteStore odfWriteStore(store);
    KoXmlWriter *manifestWriter = odfWriteStore.manifestWriter("application/vnd.oasis.opendocument.text");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    if (!store->open("content.xml"))
        return QString();

    KoStoreDevice contentDev(store);
    KoXmlWriter* contentWriter = KoOdfWriteStore::createOasisXmlWriter(&contentDev, "office:document-content");

    // for office:body
    KTemporaryFile contentTmpFile;
    if (!contentTmpFile.open())
        qFatal("Error opening temporary file!");
    KoXmlWriter xmlWriter(&contentTmpFile, 1);

    KoGenStyles mainStyles;
    KoStyleManager *styleMan = KoTextDocument(document).styleManager();
    Q_UNUSED(styleMan);
    KoEmbeddedDocumentSaver embeddedSaver;

    KoGenChanges changes;
    KoShapeSavingContext context(xmlWriter, mainStyles, embeddedSaver);

    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    KoTextSharedSavingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << textSharedData;

    if (!textSharedData) {
        textSharedData = new KoTextSharedSavingData();
        textSharedData->setGenChanges(changes);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }

    // Setup layout and managers just like kotext
    KoTextDocument(document).setInlineTextObjectManager(new KoInlineTextObjectManager()); // required while saving
    KoStyleManager *styleManager = new KoStyleManager(0);
    KoTextDocument(document).setStyleManager(styleManager);
    KoTextDocument(document).setTextEditor(new KoTextEditor(document));

    KoTextWriter writer(context, 0);
    writer.write(document, 0, -1);

    contentTmpFile.close();

    mainStyles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, contentWriter);

    contentWriter->startElement("office:body");
    contentWriter->startElement("office:text");

    changes.saveOdfChanges(contentWriter, false);

    contentWriter->addCompleteElement(&contentTmpFile);

    contentWriter->endElement(); //office text
    contentWriter->endElement(); //office body

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;


    if (!store->close())
        qWarning() << "Failed to close the store";

    mainStyles.saveOdfStylesDotXml(store, manifestWriter);

    odfWriteStore.closeManifestWriter();


    delete store;

    return odt;
}

void TestChangeTracking::testChangeTracking()
{
    QFETCH(QString, testcase);
    QFETCH(QString, changeFormat);
    QString testFileName = testcase;
    testFileName.prepend(QString(FILES_DATA_DIR));

    QTextDocument *originalDocument = documentFromOdt(testFileName, changeFormat);
    QString roundTripFileName = documentToOdt(testcase, originalDocument);

    QVERIFY(verifyContentXml(testFileName, roundTripFileName));
}

void TestChangeTracking::testChangeTracking_data()
{
    QTest::addColumn<QString>("testcase");
    QTest::addColumn<QString>("changeFormat");
    
    // Text unit-test-cases
    QTest::newRow("Simple Text Insertion") << "ChangeTracking/text/simple-addition/simple-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Simple Text Deletion")  << "ChangeTracking/text/simple-deletion/simple-deletion-tracked.odt" << "DeltaXML";
    QTest::newRow("Numbered Paragraph Addition")  << "ChangeTracking/text/simple-numbered-paragraph-addition/simple-numbered-paragraph-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Addition")  << "ChangeTracking/text/simple-paragraph-addition/simple-paragraph-addition-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Deletion")  << "ChangeTracking/text/simple-paragraph-deletion/simple-paragraph-deletion-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-1")  << "ChangeTracking/text/simple-paragraph-split/simple-paragraph-split-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-2")  << "ChangeTracking/text/split-paragraph-with-text-addition/split-para-with-added-text-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-3")  << "ChangeTracking/text/split-paragraph-by-table-addition/para-split-by-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Split-4")  << "ChangeTracking/text/split-paragraph-by-table-and-text/split-para-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Merge")  << "ChangeTracking/text/simple-paragraph-merge/simple-paragraph-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Different Element Merge")  << "ChangeTracking/text/different-element-merge/different-element-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Multiple Paragraph Merge")  << "ChangeTracking/text/multiple-paragraph-merge/multiple-paragraph-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("Merge Across Table")  << "ChangeTracking/text/paragraph-merge-across-table/merge-across-table-tracked.odt" << "DeltaXML";
    
    // List unit-test-cases
    QTest::newRow("List Item Insertion")  << "ChangeTracking/lists/added-list-item/added-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("List Insertion")  << "ChangeTracking/lists/added-list/added-list-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion")  << "ChangeTracking/lists/added-multi-para-list-item/added-multi-para-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion Full")  << "ChangeTracking/lists/added-multi-para-list-item-full/added-multi-para-list-item-full-tracked.odt" << "DeltaXML";
    QTest::newRow("Multi Para List Item Insertion Partial")  << "ChangeTracking/lists/added-multi-para-list-item-partial/added-multi-para-list-item-partial-tracked.odt" << "DeltaXML";
    QTest::newRow("Sub-List Insertion Full")  << "ChangeTracking/lists/added-sublist-full/added-sublist-full-tracked.odt" << "DeltaXML";
    QTest::newRow("Sub-List Insertion Partial")  << "ChangeTracking/lists/added-sublist-partial/added-sublist-partial-tracked.odt" << "DeltaXML";
    QTest::newRow("List Deletion")  << "ChangeTracking/lists/deleted-list/deleted-list-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Deletion")  << "ChangeTracking/lists/deleted-list-item/deleted-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Merge")  << "ChangeTracking/lists/list-item-merge/list-item-merge-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Split")  << "ChangeTracking/lists/list-item-split/list-item-split-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Numbering Restarted")  << "ChangeTracking/attributes/attribute-addition/restarted-numbering-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Numbering Restart Removed")  << "ChangeTracking/attributes/attribute-deletion/restarted-list-numbering-removed-tracked.odt" << "DeltaXML";

    //Table unit-test-cases
    QTest::newRow("Added Table")  << "ChangeTracking/tables/added-table/added-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Table")  << "ChangeTracking/tables/deleted-table/deleted-table-tracked.odt" << "DeltaXML";
    QTest::newRow("Added Row")  << "ChangeTracking/tables/added-row/added-row-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Row")  << "ChangeTracking/tables/deleted-row/deleted-row-tracked.odt" << "DeltaXML";
    QTest::newRow("Added Column")  << "ChangeTracking/tables/added-column/added-column-tracked.odt" << "DeltaXML";
    QTest::newRow("Deleted Column")  << "ChangeTracking/tables/deleted-column/deleted-column-tracked.odt" << "DeltaXML";

    //Text Style Changes
    QTest::newRow("Text Made Bold")  << "ChangeTracking/styling/text-made-bold/text-made-bold-tracked.odt" << "DeltaXML";
    QTest::newRow("Bold Made Normal")  << "ChangeTracking/styling/bold-text-unstyled/bold-text-unstyled-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Style Changes")  << "ChangeTracking/styling/create-new-style-for-para/new-style-for-para-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Style Changes - 2")  << "ChangeTracking/attributes/attribute-modification/paragraph-style-change-tracked.odt" << "DeltaXML";

    //Complex Delete Merges
    QTest::newRow("Paragraph with list-item - 1")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item/paragraph-merge-with-list-item-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph with list-item - 2")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item-2/paragraph-merge-with-list-item-2.odt" << "DeltaXML";
    QTest::newRow("Paragraph with list-item - 3")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-list-item-simple/paragraph-merge-with-list-item-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 1")  << "ChangeTracking/complex-delete-merges/list-item-merge-with-a-succeeding-paragraph/list-item-merge-with-a-succeeding-paragraph-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 2")  << "ChangeTracking/complex-delete-merges/list-item-merge-with-a-succeeding-paragraph-2/list-item-merge-with-a-succeeding-paragraph-2.odt" << "DeltaXML";
    QTest::newRow("List Item with a Paragraph - 3")  << "ChangeTracking/complex-delete-merges/list-item-merge-with-a-succeeding-paragraph-simple/list-item-merge-with-a-succeeding-paragraph-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Item Merges")  << "ChangeTracking/complex-delete-merges/list-item-merge-simple/list-item-merge-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("List Merges")  << "ChangeTracking/complex-delete-merges/list-merges/list-merges-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph with header")  << "ChangeTracking/complex-delete-merges/paragraph-merge-with-header-simple/paragraph-merge-with-header-simple-tracked.odt" << "DeltaXML";
    QTest::newRow("Header with Paragraph")  << "ChangeTracking/complex-delete-merges/header-merge-with-paragrah-simple/header-merge-with-paragrah-simple-tracked.odt" << "DeltaXML";
    
    //Other tests
    //QTest::newRow("Others-1")  << "ChangeTracking/other/michiels-deletion-sample/delete-text-across-siblings-tracked.odt" << "DeltaXML";
    QTest::newRow("Others-2")  << "ChangeTracking/other/list-id-sample/list-sample-tracked.odt" << "DeltaXML";
    QTest::newRow("Others-3")  << "ChangeTracking/other/list-table-list-1/list-table-list-tracked.odt" << "DeltaXML";

    //Multiple and Overlapping changes
    QTest::newRow("Multiple Paragraph Changes")  << "ChangeTracking/multiple-changes/para-add-then-delete/para-add-delete-tracked.odt" << "DeltaXML";
    QTest::newRow("Multiple Span Changes")  << "ChangeTracking/multiple-changes/insert-delete-span/insert-delete-span-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Overlapping")  << "ChangeTracking/overlapping/text-delete-within-added-p/text-delete-within-added-p-tracked.odt" << "DeltaXML";
    QTest::newRow("List Overlapping")  << "ChangeTracking/overlapping/insert-list-item-delete-list/insert-list-item-delete-list-tracked.odt" << "DeltaXML";
    QTest::newRow("Paragraph Add Then Merge")  << "ChangeTracking/multiple-changes/para-add-then-merge/para-add-merge-tracked.odt" << "DeltaXML";
    
    //ODF 1.2 Change Tracking tests     
    QTest::newRow("ODF12 Simple Text Insertion")  << "ChangeTracking/odf12/simple-text-addition/simple-text-addition-tracked.odt" << "ODF12";
    QTest::newRow("ODF12 Simple Text Deletion")  << "ChangeTracking/odf12/simple-text-deletion/simple-text-deletion-tracked.odt" << "ODF12";
    QTest::newRow("ODF12 Text Format Changes")  << "ChangeTracking/odf12/text-format-changes/text-format-changes-tracked.odt" << "ODF12";
    QTest::newRow("ODF12 Paragraph Merge")  << "ChangeTracking/odf12/paragraph-merge/paragraph-merge-tracked.odt" << "ODF12";
}

bool TestChangeTracking::verifyContentXml(QString &originalFileName, QString &roundTripFileName)
{
    KoStore *originalReadStore = KoStore::createStore(originalFileName, KoStore::Read, "", KoStore::Zip);
    QString originalDocumentString;

    QDomDocument originalDocument("originalDocument");
    originalReadStore->open("content.xml");
    originalDocument.setContent(originalReadStore->device());
    QTextStream originalDocumentStream(&originalDocumentString);
    originalDocumentStream << originalDocument.documentElement().namedItem("office:body");
    originalReadStore->close();

    KoStore *roundTripReadStore = KoStore::createStore(roundTripFileName, KoStore::Read, "", KoStore::Zip);
    QString roundTripDocumentString;

    QDomDocument roundTripDocument("roundTripDocument");
    roundTripReadStore->open("content.xml");
    roundTripDocument.setContent(roundTripReadStore->device());
    QTextStream roundTripDocumentStream(&roundTripDocumentString);
    roundTripDocumentStream << roundTripDocument.documentElement().namedItem("office:body");
    roundTripReadStore->close();

    bool returnValue = (originalDocumentString == roundTripDocumentString);
    if (!returnValue) {
        qDebug() << "***********************ORIGINAL DOCUMENT************************";
        qDebug() << originalDocumentString;
        qDebug() << "****************************************************************";
        qDebug() << "***********************AFTER ROUND-TRIP************************";
        qDebug() << roundTripDocumentString;
        qDebug() << "****************************************************************";
        
    }
    return returnValue;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    return QTest::qExec(new TestChangeTracking, argc, argv);
}

#include <TestChangeTracking.moc>
