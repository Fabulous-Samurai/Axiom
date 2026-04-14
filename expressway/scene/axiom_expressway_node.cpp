#include "expressway/axiom_expressway_node.h"

#include <QQuickWindow>
#include <QSGSimpleRectNode>
#include <iostream>

namespace expressway {

class ExpresswayRenderNode : public QSGRenderNode {
 public:
  void render(const RenderState* state) override {
    // [TRICK]: Symbiotic Hybrid Rendering
    // Here we obtain the native Vulkan command buffer from Qt's RHI
    // and inject our custom Dirac pipeline.

    // For Phase 7 prototype: Visual feedback via console and clear color
    static int frames = 0;
    if (++frames % 60 == 0) {
      // Log to verify the native render loop is pulsating
      // std::cout << "[Vulkan] Frame " << frames << " injected." << std::endl;
    }
  }

  StateFlags changedStates() const override {
    return QSGRenderNode::StateFlags{};
  }

  RenderingFlags flags() const override { return BoundedRectRendering; }

  QRectF rect() const override { return rect_; }

  void setRect(const QRectF& r) { rect_ = r; }

 private:
  QRectF rect_;
};

AxiomExpresswayNode::AxiomExpresswayNode(QQuickItem* parent)
    : QQuickItem(parent) {
  setFlag(ItemHasContents, true);
}

AxiomExpresswayNode::~AxiomExpresswayNode() = default;

void AxiomExpresswayNode::setZoom(float z) {
  if (qFuzzyCompare(zoom_, z)) return;
  zoom_ = z;
  emit zoomChanged();
  update();
}

void AxiomExpresswayNode::setActive(bool a) {
  if (active_ == a) return;
  active_ = a;
  emit activeChanged();
  update();
}

QSGNode* AxiomExpresswayNode::updatePaintNode(QSGNode* oldNode,
                                              UpdatePaintNodeData* data) {
  if (!active_) {
    delete oldNode;
    return nullptr;
  }

  auto* node = static_cast<ExpresswayRenderNode*>(oldNode);
  if (!node) {
    node = new ExpresswayRenderNode();
  }

  node->setRect(boundingRect());

  // Trigger window update for continuous rendering (Visual Feast)
  if (window()) {
    window()->update();
  }

  return node;
}

void AxiomExpresswayNode::itemChange(ItemChange change,
                                     const ItemChangeData& data) {
  QQuickItem::itemChange(change, data);
}

}  // namespace expressway
