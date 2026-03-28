#pragma once

#include <QQuickItem>
#include <QSGSimpleTextureNode>
#include <QSGRenderNode>
#include <vulkan/vulkan.h>
#include <mutex>

namespace expressway {

/**
 * @brief AxiomExpresswayNode — Custom SceneGraph node for High-Performance Vulkan Rendering.
 * This class bridges the Expressway Vulkan core with Qt Quick.
 */
class AxiomExpresswayNode : public QQuickItem {
    Q_OBJECT
    Q_PROPERTY(float zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)

public:
    explicit AxiomExpresswayNode(QQuickItem* parent = nullptr);
    ~AxiomExpresswayNode() override;

    float zoom() const { return zoom_; }
    void setZoom(float z);

    bool isActive() const { return active_; }
    void setActive(bool a);

signals:
    void zoomChanged();
    void activeChanged();
    void frameRendered();

protected:
    // SceneGraph integration
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    
    // Geometry/Size changes
    void itemChange(ItemChange change, const ItemChangeData& data) override;

private:
    float zoom_ = 1.0f;
    bool active_ = true;
    std::mutex render_mutex_;

    // Vulkan synchronization for zero-copy
    // This will eventually hold handles to shared memory or external textures
};

} // namespace expressway
