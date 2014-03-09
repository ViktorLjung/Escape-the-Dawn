#include "Renderer.h"

Renderer::Renderer()
{
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
	glEnable(GL_DEPTH_TEST);

	// Create Camera
	m_Camera = std::make_shared<Camera>(90.f, WIDTH / HEIGHT, 0.01f, 1000.f);

	LoadContent();
}

void Renderer::Draw(double _dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);

	m_ShaderProgram.Bind();

	glm::mat4 cameraMatrix = m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix();

	glm::mat4 MVP;
	for(int i = 0; i < ModelsToRender.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		//FIXA MED FLERA TEXTURER
		glBindTexture(GL_TEXTURE_2D, ModelsToRender[i]->model->texture[0]->texture); 
		MVP = cameraMatrix; // * ModelMatrix;
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram.GetHandle(), "MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
		glBindVertexArray(ModelsToRender[i]->model->VAO);
		glDrawArrays(GL_TRIANGLES, 0, ModelsToRender[i]->model->Vertices.size());
	}

	ModelsToRender.clear();
// 	Vertices;
// 	std::vector<glm::vec3> Normals;
// 	std::vector<glm::vec2> TextureCoords;

	glfwSwapBuffers(m_Window);
}

void Renderer::DrawText()
{
	//DrawShitInTextForm
}

void Renderer::AddTextToDraw()
{
	//Add to draw shit vector
}

void Renderer::AddModelToDraw(Model* _model, glm::mat4 _modelMatrix)
{
	ModelsToRender.push_back(new ModelData(_model, _modelMatrix));
}

//Fixa med shaders, l�gga in alla verts osv.

void Renderer::LoadContent()
{
	m_ShaderProgram.AddShader(std::unique_ptr<Shader>(new VertexShader("Shaders/Vertex.glsl")));
	m_ShaderProgram.AddShader(std::unique_ptr<Shader>(new FragmentShader("Shaders/Fragment.glsl")));
	m_ShaderProgram.Compile();
	m_ShaderProgram.Link();

	projectionMatrix = glm::perspective(45.0f, (float)WIDTH / HEIGHT, 0.01f, 100.0f);
}
