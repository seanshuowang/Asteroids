#include "Renderer.h"
#include "Texture.h"
#include "GameEngine\EntitySystem\Components\SpriteComponent.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

void GLClearError()
{
	// Clear errors
	while (glGetError() != GL_NO_ERROR);

}

bool GLLogCall(const char *function, const char *file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << ") " << function << " Line " << line << std::endl;
		return false;
	}

	return true;
}

Renderer::Renderer(Game *game)
	:mGame(game)
	,m_r(0.0f)
	,m_increment(0.05f)
{
}

Renderer::~Renderer()
{
	// Unbind shader
	m_spriteShader->Unbind();
	delete m_spriteShader;
}

bool Renderer::Initialize(float screenWidth, float screenHeight)
{
	mScreenWidth = screenWidth;
	mScreenHeight = screenHeight;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	// Use OpenGL's core profile
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	// Specify version 3.3
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	// Request a color buffer with 8-bits per RGBA channel
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	// Enable double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Force OpenGL to use hardware acceleration
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	mMainWindow = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		static_cast<int>(mScreenWidth), static_cast<int>(mScreenHeight), SDL_WINDOW_OPENGL);
	if (!mMainWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	// Create OpenGL context and attach it to our window
	mMainContext = SDL_GL_CreateContext(mMainWindow);

	/* This makes our buffer swap synchronized with the monitor's vertical refresh */
	//SDL_GL_SetSwapInterval(1);
	



	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		SDL_Log("Failed to initialize GLEW.");
		return false;
	}

	// On some platforms, GLEW will emit a benign error code,
	// so clear it
	glGetError();

	// GL blend
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	LoadShaders();
	// Create VAO
	CreateSpriteVerts();

	return true;
}

void Renderer::CreateSpriteVerts()
{
	float positions[] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	// Create VAO and index buffer for the sprite quad
	// all sprites use the same vertices
	m_VAO = new VertexArray();

	// Create and bind vertex buffer

	VertexBuffer vb(positions, 4 * 5 * sizeof(float));
	// Specify vertex attributes
	// Create and bind index buffer
	VertexBufferLayout layout;
	layout.Push<float>(3);
	layout.Push<float>(2);

	// Bind VAO and add vertex buffer to it
	m_VAO->AddBuffer(vb, layout);

	// Bind array element buffer after VAO is bound
	// This information is stored in VAO
	// Note: If no VAO is bound, you cannot bind a index buffer object 
	IndexBuffer ib(indices, 6);

}

void Renderer::LoadShaders()
{
	m_spriteShader = new Shader("res/shaders/Basic.shader");
	m_spriteShader->Bind();
	//m_spriteShader->SetUniform4f("u_color", 0.8f, 0.3f, 0.8f, 1.0f);

	Texture texture;
	texture.Load("Assets/Ship01.png");
	texture.Bind(); // slot 0
	m_spriteShader->SetUniform1i("u_texture", 0);

}

void Renderer::Shutdown()
{

	SDL_GL_DeleteContext(mMainContext);
	SDL_DestroyWindow(mMainWindow);
}

void Renderer::Draw()
{
	// Set the clear color to gray
	glClearColor(0.86f, 0.86f, 0.86f, 1.0f);
	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT);

	m_spriteShader->Bind();
	m_VAO->Bind();

	for (auto sprite : mSprites)
	{
		sprite->Draw(m_spriteShader);
	}

	//for (auto wireframe : mWireframes)
	//{
	//	wireframe->Draw(mRenderer);
	//}

	//GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

	// Swap the buffers
	SDL_GL_SwapWindow(mMainWindow);

	//TTF_Font *font = TTF_OpenFont("Assets/Carlito-Regular.ttf", 24);
	//if (font == nullptr)
	//{
	//	SDL_Log("Failed to load font");
	//}

}

void Renderer::AddSprite(SpriteComponent *sprite)
{
	int myDrawOrder = sprite->mDrawOrder;
	auto iter = mSprites.begin();
	for (;
		iter != mSprites.end();
		++iter)
	{
		if (myDrawOrder < (*iter)->mDrawOrder)
		{
			break;
		}
	}
	mSprites.insert(iter, sprite);
}

void Renderer::RemoveSprite(SpriteComponent *sprite)
{
	auto iter = std::find(mSprites.begin(), mSprites.end(), sprite);
	mSprites.erase(iter);
}