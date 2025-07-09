#pragma once
#include "../Graphics/Shader.h"
#include "../Graphics/Texture.h"
#include "../Core/Camera.h"

class Object {
	public:
	virtual ~Object() = default;
	virtual void OnUpdate(float deltaTime) const;
	virtual void OnRender() const;

	void SetVisibility(bool isVisible) { m_IsVisible = isVisible; }
	bool IsVisible() const { return m_IsVisible; }

protected:
	// All objects can be visible or hidden.
	bool m_IsVisible = true;
};