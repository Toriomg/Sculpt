#include "LayerStack.h"
#include <algorithm>

LayerStack::LayerStack() : m_LayerInsertIndex(0) {}

LayerStack::~LayerStack() {
    for (Layer* layer : m_Layers) {
        // Call the layer's shutdown logic first
        layer->OnDetach();
        // Then delete the layer from memory
        delete layer;
    }
}

void LayerStack::PushLayer(Layer* layer) {
    // Use emplace to insert the layer at the position of the insert index.
    // This keeps all normal layers grouped together.
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
    layer->OnAttach();// Call the layer's initialization logic.
}

void LayerStack::PopLayer(Layer* layer) {
    // Use std::find to locate the layer in our vector
    auto it = std::find(m_Layers.begin(), m_Layers.begin() + m_LayerInsertIndex, layer);
    if (it != m_Layers.begin() + m_LayerInsertIndex) {
        // Call the layer's shutdown logic BEFORE removing it
        layer->OnDetach();
        // Erase the layer from the vector
        m_Layers.erase(it);
        // Decrement the insert index since a normal layer was removed
        m_LayerInsertIndex--;
    }
}