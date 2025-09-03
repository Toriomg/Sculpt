#pragma once
#include "Layer.h"
#include <vector>

class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    void PushLayer(Layer* layer);
    void PopLayer(Layer* layer);
	bool isEmpty() const { return m_Layers.empty(); }

    // Iterators to allow the Application to loop through the layers
    std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
	// Returns an iterator to the end of the vector
    std::vector<Layer*>::iterator end() { return m_Layers.end(); }
    std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
    std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

private:
    std::vector<Layer*> m_Layers;
    unsigned int m_LayerInsertIndex;
};