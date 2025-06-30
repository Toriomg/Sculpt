#pragma once
#include <memory>
#include <array>

#include "Tests.h"

#include "../maths/maths.h"
#include "../buffers/VertexBuffer.h"
#include "../buffers/VertexArray.h"
#include "../buffers/IndexBuffer.h"
#include "../buffers/VertexBufferLayout.h"
#include "../Texture.h"

struct Vertex3 {
	float Pos[3];
	float Color[4];
	float TexCoords[2];
	float TexID;
};

namespace test {

	class test3 : public Test
	{
	public:
		test3();
		~test3();

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
		float m_Rotation;
		Vec3 m_Scaling;
		float m_scalar = 1;

		Vec3 m_QuadPosition;
		float m_FOV = 90.0f;
		float m_NearClip = 0.1f;
		float m_FarClip = 1000.0f;

		static std::array<Vertex3, 24> CreateCube(float x, float y, float z, float size);
	};

};
