#pragma once

class Panel {
public:
    virtual ~Panel() = default;
    virtual void OnImGuiRender() = 0;
    bool IsVisible = true;
};
