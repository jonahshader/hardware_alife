#include <SDL.h>
#include <glad/glad.h>
#include <iostream>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const char *vertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;
    out vec3 ourColor;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        ourColor = aColor;
    }
)";

const char *fragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    in vec3 ourColor;
    void main() {
        FragColor = vec4(ourColor, 1.0);
    }
)";

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_Window *window = SDL_CreateWindow("OpenGL 4.6 with SDL2 and GLAD", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
  if (!window)
  {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  if (!glContext)
  {
    std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

  // Shader compilation and program creation
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Vertex data for a colorful triangle
  float vertices[] = {
      // positions         // colors
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f};

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  bool quit = false;
  SDL_Event e;

  while (!quit)
  {
    while (SDL_PollEvent(&e) != 0)
    {
      if (e.type == SDL_QUIT)
      {
        quit = true;
      }
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window);
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram);

  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}