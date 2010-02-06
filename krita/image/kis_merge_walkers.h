/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_MERGE_WALKERS_H
#define __KIS_MERGE_WALKERS_H

class KRITAIMAGE_EXPORT KisMergeWalker : KisGraphWalker
{
public:
    struct JobItem {
        KisNodeSP m_node;
        NodePosition m_position;

        /**
         * The rect that should be prepared on this node.
         * E.g. area where the filter applies on filter layer
         * or an area of a paint layer that will be copied to
         * the projection.
         */
        QRect m_applyRect;
    };

    typedef QStack<JobItem> NodeStack;

public:
    KisMergeWalker()
    {
    }

    void collectRects(KisNodeSP node, const QRect& requestedRect) {
        clear();
        m_resultChangeRect = requestedRect;
        m_requestedRect = requestedRect;
        m_startNode = node;
        startTrip(node);
    }

    // return a reference for efficiency reasons
    inline NodeStack& nodeStack() {
        return m_mergeTask;
    }

    inline QRect& accessRect() {
        return m_resultAccessRect;
    }

    inline bool needRectVaries() {
        return m_needRectVaries;
    }

    inline bool changeRectVaries() {
        return m_changeRectVaries;
    }

protected:
    inline void clear() {
        m_resultAccessRect = /*m_resultChangeRect =*/
            m_childNeedRect = m_lastNeedRect = QRect();

        m_needRectVaries = m_changeRectVaries = false;
        m_mergeTask.clear();

        // Not needed really. Think over removing.
        //m_startNode = 0;
        //m_requestedRect = QRect();
    }

    inline void pushJob(KisNodeSP node, NodePosition position, QRect applyRect) {
        JobItem item = {node, position, applyRect};
        m_mergeTask.push(item);
    }

    inline bool dependOnLowerNodes(KisNodeSP node) {
        /**
         * FIXME: Clone Layers should be considered too
         */
        return qobject_cast<KisAdjustmentLayer*>(node.data());
    }

    void registerChangeRect(KisNodeSP node) {
        QRect currentChangeRect = node->changeRect(m_resultChangeRect);

        if(!m_changeRectVaries)
            m_changeRectVaries = currentChangeRect != m_resultChangeRect;

        m_resultChangeRect = currentChangeRect;
    }

    void registerNeedRect(KisNodeSP node, NodePosition position) {
        if(m_mergeTask.isEmpty())
            m_resultAccessRect = m_childNeedRect =
                m_lastNeedRect = m_resultChangeRect;

        QRect currentNeedRect;

        switch(position) {
        case N_TOPMOST:
            m_lastNeedRect = m_childNeedRect;
        case N_NORMAL:
            if(!m_lastNeedRect.isEmpty()) {
                pushJob(node, position, m_lastNeedRect);
            }
            else if(dependOnLowerNodes(node)) {
                /**
                 * FIXME: This case seems to never happen.
                 * Obviously, no layer will report zero needRect
                 * lying above filthy node.
                 */
                qWarning() << "Merge walker thought this was not possible!";
                m_lastNeedRect = getChangeRectForNode(node, m_startNode,
                                                      m_requestedRect);
                pushJob(node, position, m_lastNeedRect);
            }

            m_lastNeedRect = node->needRect(m_lastNeedRect,
                                            KisNode::NORMAL);
            m_childNeedRect = m_lastNeedRect;
            break;
        case N_LOWER:
        case N_BOTTOMMOST:
            if(!m_lastNeedRect.isEmpty()) {
                pushJob(node, position, m_lastNeedRect);
                m_lastNeedRect = node->needRect(m_lastNeedRect,
                                                KisNode::BELOW_FILTHY);
            }
            break;
        default:
            qFatal("Merge visitor: node position(%d) is out of range", position);
        }

        if(!m_needRectVaries)
            m_needRectVaries = m_resultAccessRect != m_lastNeedRect;
        m_resultAccessRect |= m_lastNeedRect;
    }

private:
    /**
     * The result variables.
     * By the end of a recursion they will store a complete
     * data for a successful merge operation.
     */
    QRect m_resultAccessRect;
    bool m_needRectVaries;
    bool m_changeRectVaries;
    NodeStack m_mergeTask;
    // FIXME: Think over moving to temporary
    QRect m_resultChangeRect;

    /**
     * Temporary variables
     */
    KisNodeSP m_startNode;
    QRect m_requestedRect;

    QRect m_childNeedRect;
    QRect m_lastNeedRect;
};

#endif /* __KIS_MERGE_WALKERS_H */

