#pragma once

#include "Object.h"
#include "../buffers/VertexArray.h"
#include <memory>

class InfGrid : public Object {
public:
    InfGrid();
    virtual ~InfGrid();

    // The core rendering logic for the grid.
    void OnRender(const Camera& camera, bool CameraPersEnabled) const;

private:
    // The grid owns its own specific shader.
    std::unique_ptr<Shader> m_Shader;
};