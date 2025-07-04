#pragma once
#include <memory>
#include <array>

#include "Tests.h"

#include "../buffers/VertexBuffer.h"
#include "../buffers/VertexArray.h"
#include "../buffers/IndexBuffer.h"
#include "../buffers/VertexBufferLayout.h"
#include "../Texture.h"
#include "../maths/maths.h"

struct Vertex {
	float Pos[3];
	float Color[4];
	float TexCoords[2];
	float TexID;
};

namespace test {

	class TestBatchRendering : public Test
	{
	public:
		TestBatchRendering();
		~TestBatchRendering();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;

	private:
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture1;
		std::unique_ptr<Texture> m_Texture2;

		Matx4f m_Proj, m_View;
		Vec3 m_Translation;

		Vec2 m_QuadPosition;

		static std::array<Vertex, 4> CreateQuad(float x, float y, float textureID);
	};

};
