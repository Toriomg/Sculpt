#pragma once

#include <memory>
#include "../Core/Camera.h"
#include "../Graphics/Buffers/VertexArray.h"
#include "../Graphics/Shading/Shader.h"

class InfGrid {
public:
    InfGrid();
    virtual ~InfGrid();

    // The core rendering logic for the grid.
    void OnRender(const Camera& camera, bool CameraPersEnabled) const;

private:
    // The grid owns its own specific shader.
    std::unique_ptr<Shader> m_Shader;
};