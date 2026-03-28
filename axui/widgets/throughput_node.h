// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO .agents/workflows/agent_must_obey.md
/**
 * @file throughput_node.h
 * @brief High-performance SceneGraph node for zero-copy metric visualization.
 */

#pragma once

#include <QtQuick/QQuickItem>
#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGGeometry>
#include <QtQuick/QSGFlatColorMaterial>
#include <vector>
#include <atomic>
#include "telemetry.h"

namespace AXIOM {

/**
 * @brief AxiomThroughputNode: GPU-bound high-frequency line renderer.
 * 
 * DESIGN RATIONALE:
 * 1. ZERO-COPY: Reads directly from TelemetryScribe's lock-free ring buffer.
 * 2. L1/L2 CACHE OPTIMIZED: Vertex updates are performed in a contiguous memory block
 *    to maximize spatial locality and minimize TLB misses.
 * 3. GIL BYPASS: The SceneGraph thread (Data Plane) handles the vertex upload independently
 *    of the Python UI thread (Control Plane).
 */
class AxiomThroughputNode : public QSGGeometryNode {
public:
    AxiomThroughputNode();
    
    /**
     * @brief Polls the telemetry system and updates GPU vertex data.
     * @param rect The bounding area of the widget.
     * @param scribe The telemetry source.
     */
    void update(const QRectF& rect, TelemetryScribe& scribe);

private:
    QSGGeometry m_geometry;
    QSGFlatColorMaterial m_material;
    
    // Internal cache for historical throughput calculation
    // Aligned to 64 bytes to prevent bank conflicts during high-frequency updates.
    struct alignas(64) ThroughputPoint {
        double timestamp;
        double value;
    };
    std::vector<ThroughputPoint> m_history;
    size_t m_max_points = 2048;
};

/**
 * @brief AxiomThroughputItem: QML wrapper for the SceneGraph node.
 */
class AxiomThroughputItem : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int maxPoints READ maxPoints WRITE setMaxPoints NOTIFY maxPointsChanged)

public:
    explicit AxiomThroughputItem(QQuickItem* parent = nullptr);
    
    QColor color() const { return m_color; }
    void setColor(const QColor& c) { if (m_color != c) { m_color = c; Q_EMIT colorChanged(); update(); } }
    
    int maxPoints() const { return m_max_points; }
    void setMaxPoints(int p) { if (m_max_points != p) { m_max_points = p; Q_EMIT maxPointsChanged(); update(); } }

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;

signals:
    void colorChanged();
    void maxPointsChanged();

private:
    QColor m_color = Qt::cyan;
    int m_max_points = 2048;
};

} // namespace AXIOM
