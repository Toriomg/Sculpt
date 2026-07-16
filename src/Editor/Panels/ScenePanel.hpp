// Scene-level panel: global model transform applied to all entities, and editor camera settings.
#pragma once
#include "Editor/Panels/Panel.hpp"
#include "Platform/CoreUtils/Math/maths.hpp"

class Camera;

class ScenePanel : public Panel {
public:
    explicit ScenePanel(Camera* camera);
    void OnImGuiRender() override;
    Matx4f GetGlobalTransform() const;
    // Analytically inverts T * Ry * S_per_axis * S_uniform — used to recover base transforms.
    Matx4f GetInverseGlobalTransform() const;

private:
    Camera* m_Camera;
    Vec3 m_Translation{0.0f, 0.0f, 0.0f};
    float m_Rotation = 0.0f;
    Vec3 m_Scaling{1.0f, 1.0f, 1.0f};
    float m_Scalar = 1.0f;
};
