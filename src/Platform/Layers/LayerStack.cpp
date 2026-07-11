#include "LayerStack.h"
#include <algorithm>

void LayerStack::PushLayer(std::unique_ptr<Layer> layer) {
    layer->OnAttach();
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, std::move(layer));
    m_LayerInsertIndex++;
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find_if(
        m_Layers.begin(),
        m_Layers.begin() + m_LayerInsertIndex,
        [layer](const std::unique_ptr<Layer>& p) { return p.get() == layer; }
    );
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        (*it)->OnDetach();
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}
