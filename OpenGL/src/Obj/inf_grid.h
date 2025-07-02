#pragma once

#include "Object.h"
#include <memory>

class InfGrid : public Object {
public:
    InfGrid();
    virtual ~InfGrid();

    // The core rendering logic for the grid.
    void OnRender(const Camera& camera) const override;

private:
    // The grid owns its own specific shader.
    std::unique_ptr<Shader> m_Shader;

    // The grid needs a Vertex Array Object (VAO), even if it's "empty".
    unsigned int m_VaoID;
};