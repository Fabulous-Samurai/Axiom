// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file pluto_navigator.h
 * @brief SceneGraph visualization for Pluto Swarm search tree.
 */

#pragma once

#include <QtQuick/QQuickItem>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGGeometry>
#include <QtQuick/QSGFlatColorMaterial>
#include <vector>
#include "pluto_controller.h"

namespace AXIOM {

/**
 * @brief PlutoSwarmNode: Renders nodes and edges of the search tree.
 */
class PlutoSwarmNode : public QSGGeometryNode {
public:
    PlutoSwarmNode();
    void update(const QRectF& rect, size_t count, const QPointF* positions, const uint32_t* parent_ids);

private:
    QSGGeometry m_nodeGeometry;
    QSGGeometry m_edgeGeometry;
    QSGFlatColorMaterial m_nodeMaterial;
    QSGFlatColorMaterial m_edgeMaterial;

    // Sub-nodes for nodes and edges
    QSGGeometryNode m_edgeNode;
};

/**
 * @brief PlutoSwarmNavigator: QML wrapper for the Pluto search tree visualization.
 */
class PlutoSwarmNavigator : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QColor nodeColor READ nodeColor WRITE setNodeColor NOTIFY nodeColorChanged)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged)
    Q_PROPERTY(int selectedNodeId READ selectedNodeId NOTIFY selectedNodeIdChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QPointF pan READ pan WRITE setPan NOTIFY panChanged)

public:
    explicit PlutoSwarmNavigator(QQuickItem* parent = nullptr);

    QColor nodeColor() const { return m_nodeColor; }
    void setNodeColor(const QColor& c) { if (m_nodeColor != c) { m_nodeColor = c; Q_EMIT nodeColorChanged(); update(); } }

    QColor edgeColor() const { return m_edgeColor; }
    void setEdgeColor(const QColor& c) { if (m_edgeColor != c) { m_edgeColor = c; Q_EMIT edgeColorChanged(); update(); } }

    int selectedNodeId() const { return m_selectedNodeId; }

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal z);

    QPointF pan() const { return m_pan; }
    void setPan(const QPointF& p);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void nodeColorChanged();
    void edgeColorChanged();
    void selectedNodeIdChanged();
    void zoomChanged();
    void panChanged();
    void nodeClicked(int nodeId);

private:
    QColor m_nodeColor = Qt::cyan;
    QColor m_edgeColor = QColor::fromRgbF(125/255.0, 211/255.0, 252/255.0, 0.3);
    int m_selectedNodeId = -1;
    qreal m_zoom = 1.0;
    QPointF m_pan = QPointF(0, 0);

    // Hit testing data
    struct NodePos {
        uint32_t id;
        QPointF pos;
    };
    std::vector<NodePos> m_lastNodePositions;
    std::chrono::steady_clock::time_point m_lastInteractionTime;
};

} // namespace AXIOM
