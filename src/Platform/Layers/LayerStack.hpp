#pragma once
#include "Layer.hpp"
#include <vector>
#include <memory>

class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack() = default;

    void PushLayer(std::unique_ptr<Layer> layer);
    void PopLayer(Layer* layer);
    bool isEmpty() const { return m_Layers.empty(); }

    auto begin()  { return m_Layers.begin(); }
    auto end()    { return m_Layers.end(); }
    auto rbegin() { return m_Layers.rbegin(); }
    auto rend()   { return m_Layers.rend(); }

private:
    std::vector<std::unique_ptr<Layer>> m_Layers;
    uint32_t m_LayerInsertIndex = 0;
};
