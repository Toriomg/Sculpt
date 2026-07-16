#include "LayerStack.hpp"
#include <algorithm>

void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
    layer->OnAttach();
    // Regular layers are inserted at [0..insertIndex); overlay layers go after insertIndex.
    // This keeps overlays on top of regular layers regardless of push order.
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
    m_LayerInsertIndex++;
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find_if(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex,
                           [layer](std::unique_ptr<Layer> const& p) { return p.get() == layer; });
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        (*it)->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}
