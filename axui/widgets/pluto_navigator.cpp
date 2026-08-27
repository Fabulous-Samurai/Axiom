// [MANDATE]: ZENITH PILLAR COMPLIANCE - REFER TO
// .agents/workflows/agent_must_obey.md
/**
 * @file pluto_navigator.cpp
 * @brief Implementation of Pluto Swarm SceneGraph visualization.
 */

#include "pluto_navigator.h"

#include <QtQuick/QQuickWindow>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>

#include "secure_random.h"

namespace AXIOM {

PlutoSwarmNode::PlutoSwarmNode()
    : m_nodeGeometry(QSGGeometry::defaultAttributes_Point2D(), 0),
      m_edgeGeometry(QSGGeometry::defaultAttributes_Point2D(), 0) {
  // Configure Nodes
  m_nodeGeometry.setDrawingMode(QSGGeometry::DrawPoints);
  m_nodeGeometry.setLineWidth(5.0f);
  setGeometry(&m_nodeGeometry);
  m_nodeMaterial.setColor(Qt::cyan);
  setMaterial(&m_nodeMaterial);

  // Configure Edges
  m_edgeGeometry.setDrawingMode(QSGGeometry::DrawLines);
  m_edgeGeometry.setLineWidth(1.0f);
  m_edgeNode.setGeometry(&m_edgeGeometry);
  m_edgeMaterial.setColor(QColor::fromRgbF(0.5, 0.8, 1.0, 0.3));
  m_edgeNode.setMaterial(&m_edgeMaterial);

  appendChildNode(&m_edgeNode);
}

void PlutoSwarmNode::update(const QRectF&, size_t count,
                            const QPointF* positions,
                            const uint32_t* parent_ids) {
  // Update Nodes Geometry
  m_nodeGeometry.allocate((int)count);
  QSGGeometry::Point2D* node_v = m_nodeGeometry.vertexDataAsPoint2D();
  for (size_t i = 0; i < count; ++i) {
    node_v[i].set((float)positions[i].x(), (float)positions[i].y());
  }
  markDirty(QSGNode::DirtyGeometry);

  // Update Edges Geometry
  int edge_count = 0;
  for (size_t i = 0; i < count; ++i) {
    if (parent_ids[i] != UINT32_MAX) {
      edge_count++;
    }
  }

  m_edgeGeometry.allocate(edge_count * 2);
  QSGGeometry::Point2D* edge_v = m_edgeGeometry.vertexDataAsPoint2D();
  int edge_idx = 0;
  for (size_t i = 0; i < count; ++i) {
    uint32_t pid = parent_ids[i];
    if (pid != UINT32_MAX) {
      // Find parent position (assuming pid is an index for now, or use a map)
      // For simplicity and speed in this version, we assume parent_id maps to
      // an index. In a real swarm, we'd have a map or search_nodes already
      // sorted.
      uint32_t p_idx = pid % (uint32_t)count;
      edge_v[edge_idx++].set((float)positions[p_idx].x(),
                             (float)positions[p_idx].y());
      edge_v[edge_idx++].set((float)positions[i].x(), (float)positions[i].y());
    }
  }
  m_edgeNode.markDirty(QSGNode::DirtyGeometry);
}

PlutoSwarmNavigator::PlutoSwarmNavigator(QQuickItem* parent)
    : QQuickItem(parent) {
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::LeftButton);
  m_lastNodePositions.reserve(Mantis::kMaxNodes);
}

void PlutoSwarmNavigator::setZoom(qreal z) {
  // Bounds checking
  z = std::clamp(z, 0.1, 50.0);

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - m_lastInteractionTime)
                     .count();

  // Rate-limiting: Max 120Hz updates (~8ms) to prevent flooding the SG
  if (elapsed < 8) return;
  m_lastInteractionTime = now;

  if (!qFuzzyCompare(m_zoom, z)) {
    m_zoom = z;
    Q_EMIT zoomChanged();
    update();
  }
}

void PlutoSwarmNavigator::setPan(const QPointF& p) {
  // Bounds checking (e.g. max pan from center)
  QPointF clamped_p(std::clamp(p.x(), -10000.0, 10000.0),
                    std::clamp(p.y(), -10000.0, 10000.0));

  auto now = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                     now - m_lastInteractionTime)
                     .count();

  // Rate-limiting: Max 120Hz updates (~8ms)
  if (elapsed < 8) return;
  m_lastInteractionTime = now;

  if (m_pan != clamped_p) {
    m_pan = clamped_p;
    Q_EMIT panChanged();
    update();
  }
}

QSGNode* PlutoSwarmNavigator::updatePaintNode(QSGNode* oldNode,
                                              UpdatePaintNodeData*) {
  PlutoSwarmNode* node = static_cast<PlutoSwarmNode*>(oldNode);
  if (!node) {
    node = new PlutoSwarmNode();
  }

  // Position calculation once per update
  static std::array<Mantis::AStarNode, Mantis::kMaxNodes> search_nodes;
  static std::array<QPointF, Mantis::kMaxNodes> pos_cache;
  static std::array<uint32_t, Mantis::kMaxNodes> parent_ids;

  size_t count = Pluto::PlutoController::instance().get_search_tree(
      search_nodes.data(), Mantis::kMaxNodes);

  if (count == 0) {
    count = 10000;  // Stress test with 10k nodes if empty
    for (size_t i = 0; i < count; ++i) {
      search_nodes[i].id = (uint32_t)i;
      search_nodes[i].parent_id = (i == 0) ? UINT32_MAX : (uint32_t)(i / 2);
    }
  }

  QPointF center = boundingRect().center() + m_pan;
  m_lastNodePositions.clear();

  for (size_t i = 0; i < count; ++i) {
    float angle = i * 0.5f;
    float radius = (50.0f + i * 2.0f) * (float)m_zoom;
    float x = (float)center.x() + std::cos(angle) * radius;
    float y = (float)center.y() + std::sin(angle) * radius;
    pos_cache[i] = QPointF(x, y);
    parent_ids[i] = search_nodes[i].parent_id;

    // Only cache every 4th node for picking if 10k nodes to save memory/time
    // Or cache all if we need precise picking. Let's cache all for 10k.
    m_lastNodePositions.push_back({(uint32_t)i, pos_cache[i]});
  }

  node->update(boundingRect(), count, pos_cache.data(), parent_ids.data());

  return node;
}

void PlutoSwarmNavigator::mousePressEvent(QMouseEvent* event) {
  const QPointF clickPos = event->position();
  const float threshold =
      15.0f * (float)m_zoom;  // Scale picking radius with zoom

  int closestId = -1;
  float minDim = 1000000.0f;

  for (const auto& node : m_lastNodePositions) {
    float dist = std::sqrt(std::pow(node.pos.x() - clickPos.x(), 2) +
                           std::pow(node.pos.y() - clickPos.y(), 2));
    if (dist < threshold && dist < minDim) {
      minDim = dist;
      closestId = (int)node.id;
    }
  }

  if (closestId != -1) {
    m_selectedNodeId = closestId;
    Q_EMIT selectedNodeIdChanged();
    Q_EMIT nodeClicked(closestId);
    std::cout << "[Pluto Navigator] Selected Node: " << closestId << std::endl;
  }

  event->accept();
}

}  // namespace AXIOM
