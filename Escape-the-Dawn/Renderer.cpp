#include "Renderer.h"

Renderer::Renderer()
{
	m_VSync = false;
	m_DrawNormals = false;
	m_DrawWireframe = false;
}

void Renderer::Initialize()
{
	// Initialize GLFW
	if (!glfwInit()) {
		LOG_ERROR("GLFW: Initialization failed");
		exit(EXIT_FAILURE);
	}

	// Create a window
	WIDTH = 1280;
	HEIGHT = 720;
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", nullptr, nullptr);
	if (!m_Window) {
		LOG_ERROR("GLFW: Failed to create window");
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(m_Window);

	// GL version info
	glGetIntegerv(GL_MAJOR_VERSION, &m_glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &m_glVersion[1]);
	m_glVendor = (GLchar*)glGetString(GL_VENDOR);
	std::stringstream ss;
	ss << m_glVendor << " OpenGL " << m_glVersion[0] << "." << m_glVersion[1];
#ifdef DEBUG
	ss << " DEBUG";
#endif
	LOG_INFO(ss.str().c_str());
	glfwSetWindowTitle(m_Window, ss.str().c_str());

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		LOG_ERROR("GLEW: Initialization failed");
		exit(EXIT_FAILURE);
	}
	
	// Create Camera
	m_Camera = std::make_shared<Camera>(45.f, (float)WIDTH / HEIGHT, 0.01f, 1000.f);
	m_Camera->Position(glm::vec3(0.0f, 0.0f, 2.f));

	glfwSwapInterval(m_VSync);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	LoadContent();
}

void Renderer::Draw(double dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
#ifdef DEBUG
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_BACK, GL_LINE);
#endif

	// Draw models
	m_ShaderProgram.Bind();
	glPolygonMode(GL_FRONT_AND_BACK, m_DrawWireframe ? GL_LINE : GL_FILL);
	DrawModels();
	
#ifdef DEBUG
	// Debug draw normals
	if (m_DrawNormals) {
		m_ShaderProgramNormals.Bind();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		DrawModels();
	}
#endif

	ModelsToRender.clear();

	glfwSwapBuffers(m_Window);
}

void Renderer::DrawModels()
{
	glm::mat4 cameraMatrix = m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix();

	glm::mat4 MVP;
	for (int i = 0; i < ModelsToRender.size(); i++)
	{
		ModelData* modelData = ModelsToRender.at(i);
		auto model = modelData->model;

		MVP = cameraMatrix * modelData->ModelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "model"), 1, GL_FALSE, glm::value_ptr(modelData->ModelMatrix));
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "view"), 1, GL_FALSE, glm::value_ptr(m_Camera->ViewMatrix()));
		glUniform3fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "position"), 3, Light_position.data());
		glUniform3fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "specular"), 3, Light_specular.data());
		glUniform3fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "diffuse"), 3, Light_diffuse.data());
		glUniform1fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "constantAttenuation"), 3, Light_constantAttenuation.data());
		glUniform1fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "linearAttenuation"), 3, Light_linearAttenuation.data());
		glUniform1fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "quadraticAttenuation"), 3, Light_quadraticAttenuation.data());
		glUniform1fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "spotExponent"), 3, Light_spotExponent.data());


		glBindVertexArray(model->VAO);
		for (auto texGroup : model->TextureGroups) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texGroup.Texture->texture); 
			glDrawArrays(GL_TRIANGLES, texGroup.StartIndex, texGroup.EndIndex - texGroup.StartIndex + 1);
		}
	}
}

void Renderer::DrawText()
{
	//DrawShitInTextForm
}

void Renderer::AddTextToDraw()
{
	//Add to draw shit vector
}

void Renderer::AddModelToDraw(std::shared_ptr<Model> _model, glm::vec3 _position, glm::quat _orientation)
{
	glm::mat4 RotationMatrix = glm::toMat4(_orientation);
	glm::mat4 ModelMatrix = glm::translate(glm::mat4(), _position) * RotationMatrix ;
	// You can now use ModelMatrix to build the MVP matrix
	ModelsToRender.push_back(new ModelData(_model, ModelMatrix));
}

void Renderer::AddPointLightToDraw(
	glm::vec3 _position,
	glm::vec3 _specular, 
	glm::vec3 _diffuse, 
	float _constantAttenuation, 
	float _linearAttenuation, 
	float _quadraticAttenuation, 
	float _spotExponent
	)
{
	Light_position.push_back(_position.x);
	Light_position.push_back(_position.y);
	Light_position.push_back(_position.z);
	Light_specular.push_back(_specular.x);
	Light_specular.push_back(_specular.y);
	Light_specular.push_back(_specular.z);
	Light_diffuse.push_back(_diffuse.x);
	Light_diffuse.push_back(_diffuse.y);
	Light_diffuse.push_back(_diffuse.z);
	Light_constantAttenuation.push_back(_constantAttenuation);
	Light_linearAttenuation.push_back(_linearAttenuation);
	Light_quadraticAttenuation.push_back(_quadraticAttenuation);
	Light_spotExponent.push_back(_spotExponent);

}


void Renderer::LoadContent()
{
	auto standardVS = std::shared_ptr<Shader>(new VertexShader("Shaders/Vertex.glsl"));
	auto standardFS = std::shared_ptr<Shader>(new FragmentShader("Shaders/Fragment.glsl"));

	auto normalsGS = std::shared_ptr<Shader>(new GeometryShader("Shaders/Normals.geo.glsl"));
	auto normalsFS = std::shared_ptr<Shader>(new FragmentShader("Shaders/Normals.frag.glsl"));

	m_ShaderProgram.AddShader(standardVS);
	m_ShaderProgram.AddShader(standardFS);
	m_ShaderProgram.Compile();
	m_ShaderProgram.Link();

	m_ShaderProgramNormals.AddShader(normalsGS);
	m_ShaderProgramNormals.AddShader(standardVS);
	m_ShaderProgramNormals.AddShader(normalsFS);
	m_ShaderProgramNormals.Compile();
	m_ShaderProgramNormals.Link();
}
