#pragma once
#include "../Shader.h"
#include "../Texture.h"
#include "../Camera.h"

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