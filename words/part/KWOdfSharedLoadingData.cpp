/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KWOdfSharedLoadingData.h"
#include "KWOdfLoader.h"
#include "KWDocument.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWCopyShape.h"

#include <KoTextShapeData.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>

#include <QTextCursor>
#include <kdebug.h>

KWOdfSharedLoadingData::KWOdfSharedLoadingData(KWOdfLoader *loader)
        : KoTextSharedLoadingData()
        , m_loader(loader)
{
    KoShapeLoadingContext::addAdditionalAttributeData(
        KoShapeLoadingContext::AdditionalAttributeData(
            KoXmlNS::text, "anchor-type", "text:anchor-type"));
    KoShapeLoadingContext::addAdditionalAttributeData(
        KoShapeLoadingContext::AdditionalAttributeData(
            KoXmlNS::text, "anchor-page-number", "text:anchor-page-number"));
}

void KWOdfSharedLoadingData::shapeInserted(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context, KoTextAnchor *anchor)
{
    shape->removeAdditionalAttribute("text:anchor-type");
    const KoXmlElement *style = 0;
    if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::draw, "style-name"), "graphic",
                    context.odfLoadingContext().useStylesAutoStyles());
    }

    KoTextShapeData *text = qobject_cast<KoTextShapeData*>(shape->userData());
    if (text) {
        KWTextFrameSet *fs = 0;
        KWFrame *previous = m_nextFrames.value(shape->name());
        if (previous)
            fs = dynamic_cast<KWTextFrameSet*>(previous->frameSet());
        if (fs == 0) {
            fs = new KWTextFrameSet(m_loader->document());
            fs->setName(m_loader->document()->uniqueFrameSetName(shape->name()));
            m_loader->document()->addFrameSet(fs);
        }

        KWFrame *frame = new KWFrame(shape, fs, anchor);
        if (style) {
            if (! fillFrameProperties(frame, *style))
                return; // done
        }

        KoXmlElement textBox(KoXml::namedItemNS(element, KoXmlNS::draw, "text-box"));
        if (frame && !textBox.isNull()) {
            QString nextFrame = textBox.attributeNS(KoXmlNS::draw, "chain-next-name");
            if (! nextFrame.isEmpty()) {
#ifndef NDEBUG
                if (m_nextFrames.contains(nextFrame))
                    kWarning(32001) << "Document has two frames with the same 'chain-next-name' value, strange things may happen";
#endif
                m_nextFrames.insert(nextFrame, frame);
            }

            if (textBox.hasAttributeNS(KoXmlNS::fo, "min-height")) {
                frame->setMinimumFrameHeight(KoUnit::parseValue(textBox.attributeNS(KoXmlNS::fo, "min-height")));
                KoShape *shape = frame->shape();
                QSizeF newSize = shape->size();
                if (newSize.height() < frame->minimumFrameHeight()) {
                    newSize.setHeight(frame->minimumFrameHeight());
                    shape->setSize(newSize);
                }
            }
        }
    } else {
        KWFrameSet *fs = new KWFrameSet();
        fs->setName(m_loader->document()->uniqueFrameSetName(shape->name()));
        KWFrame *frame = new KWFrame(shape, fs, anchor);
        if (style)
            fillFrameProperties(frame, *style);
        m_loader->document()->addFrameSet(fs);
    }
}

bool KWOdfSharedLoadingData::fillFrameProperties(KWFrame *frame, const KoXmlElement &style)
{
    frame->setFrameBehavior(Words::IgnoreContentFrameBehavior);
    KoXmlElement properties(KoXml::namedItemNS(style, KoXmlNS::style, "graphic-properties"));
    if (properties.isNull())
        return frame;

    QString copy = properties.attributeNS(KoXmlNS::draw, "copy-of");
    if (! copy.isEmpty()) {
        //TODO "return false" and untested code...
#if 0
        // untested... No app saves this currently..
        foreach (KWFrame *f, frame->frameSet()->frames()) {
            if (f->shape()->name() == copy) {
                KWCopyShape *shape = new KWCopyShape(f->shape());
                new KWFrame(shape, frame->frameSet(), frame->anchoredPageNumber());
                delete frame;
                return false;
            }
        }
#endif
    }

    QString overflow = properties.attributeNS(KoXmlNS::style, "overflow-behavior", QString());
    if (overflow == "clip")
        frame->setFrameBehavior(Words::IgnoreContentFrameBehavior);
    else if (overflow == "auto-create-new-frame")
        frame->setFrameBehavior(Words::AutoCreateNewFrameBehavior);
    else
        frame->setFrameBehavior(Words::AutoExtendFrameBehavior);
    QString newFrameBehavior = properties.attributeNS(KoXmlNS::calligra, "frame-behavior-on-new-page", QString());
    if (newFrameBehavior == "followup")
        frame->setNewFrameBehavior(Words::ReconnectNewFrame);
    else if (newFrameBehavior == "copy")
        frame->setNewFrameBehavior(Words::CopyNewFrame);
    else
        frame->setNewFrameBehavior(Words::NoFollowupFrame);

    return true;
}
