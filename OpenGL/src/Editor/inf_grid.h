#pragma once

#include <memory>
#include "../Core/Camera.h"
#include "../Graphics/VertexArray.h"
#include "../Graphics/Shader.h"

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