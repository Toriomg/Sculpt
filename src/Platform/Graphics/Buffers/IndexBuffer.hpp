#pragma once
#include "Platform/CoreUtils/Log.hpp"
#include "Platform/CoreUtils/glewDbg.hpp"

class IndexBuffer {
public:
    IndexBuffer(
        unsigned int const* data,
        unsigned int count);  // Constructor to create a Index buffer with given data and size
    ~IndexBuffer();           // Destructor to clean up the Index buffer

    void Bind() const;
    static void Unbind() ;

    inline unsigned int GetCount() const {
        return m_Count;
    }  // Returns the count of indices in the buffer
private:
    unsigned int m_RendererID{};
    unsigned int m_Count;
};
