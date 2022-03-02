/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   mlx_init.c                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: W2Wizard <w2.wizzard@gmail.com>              +#+                     */
/*                                                   +#+                      */
/*   Created: 2021/12/28 00:24:30 by W2Wizard      #+#    #+#                 */
/*   Updated: 2022/03/02 03:51:46 by lde-la-h      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "MLX42/MLX42_Int.h"

//= Private =//

static void	framebuffer_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}

static bool	mlx_create_buffers(mlx_t *mlx)
{
	mlx_ctx_t* mlxctx = mlx->context;

	mlxctx->zdepth = 0;
	glActiveTexture(GL_TEXTURE0);
	glGenVertexArrays(1, &(mlxctx->vao));
	glGenVertexArrays(1, &(mlxctx->vao));
	glGenBuffers(1, &(mlxctx->vbo));
	glBindVertexArray(mlxctx->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mlxctx->vbo);

	// Vertex XYZ coordinates
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), NULL);
	glEnableVertexAttribArray(0);

	// UV Coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void *)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform1i(glGetUniformLocation(mlxctx->shaderprogram, "OutTexture"), 0);

	// NOTE: Call manually once to calculate View Projection Matrix
	glfwSetWindowSizeCallback(mlx->window, &mlx_on_resize);
	mlx_on_resize(mlx->window, mlx->width, mlx->height);
	return (true);
}


/**
 * Compiles the given shader source code, of a given shader type.
 * Returns shader object via param.
 * 
 * @param code The shader source code.
 * @param Type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER or ...
 * @return Non-zero on success, else 0.
 */
static uint32_t mlx_compile_shader(const char* code, int32_t type)
{
	GLuint shader;
	int32_t success;
	char infolog[512] = {0};

	if (!code || (shader = glCreateShader(type)) == 0) 
		return (0);

	GLint len = strlen(code);
	glShaderSource(shader, 1, &code, &len);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(infolog), NULL, infolog);
		fprintf(stderr, "%s", infolog);
		return (0);
	}
	return (shader);
}

static bool	mlx_init_render(mlx_t *mlx)
{
	uint32_t vshader = 0;
	uint32_t fshader = 0;
	char infolog[512] = {0};
	mlx_ctx_t* mlxctx = mlx->context;

	glfwMakeContextCurrent(mlx->window);
	glfwSetFramebufferSizeCallback(mlx->window, framebuffer_callback);
	glfwSetWindowUserPointer(mlx->window, mlx);
	glfwSwapInterval(MLX_SWAP_INTERVAL);

	// Load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		return (mlx_error(MLX_GLADFAIL));
	
	// Compile & link shaders TODO: Separate error codes for shaders
	if (!(vshader = mlx_compile_shader(vert_shader, GL_VERTEX_SHADER)))
		return (mlx_error(MLX_SHDRFAIL));
	if (!(fshader = mlx_compile_shader(frag_shader, GL_FRAGMENT_SHADER)))
		return (mlx_error(MLX_SHDRFAIL));
	if (!(mlxctx->shaderprogram = glCreateProgram()))
		return (mlx_error(MLX_SHDRFAIL));
	glAttachShader(mlxctx->shaderprogram, vshader);
	glAttachShader(mlxctx->shaderprogram, fshader);
	glLinkProgram(mlxctx->shaderprogram);

	int32_t success; // Check if linking succeeded 
	glGetProgramiv(mlxctx->shaderprogram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(mlxctx->shaderprogram, sizeof(infolog), NULL, infolog);
		fprintf(stderr, "%s", infolog);
		return (mlx_error(MLX_SHDRFAIL));
	}
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glUseProgram(mlxctx->shaderprogram);
	return (mlx_create_buffers(mlx));
}

//= Public =//

mlx_t* mlx_init(int32_t width, int32_t height, const char* title, bool resize)
{
	mlx_errno = 0;
	if (width <= 0 || height <= 0 || !title)
		return ((void*)mlx_error(MLX_NULLARG));
	
	bool init;
	if (!(init = glfwInit()))
		return ((void*)mlx_error(MLX_GLFWFAIL));

	mlx_t* mlx;
	if (!(mlx = calloc(1, sizeof(mlx_t))))
		return ((void*)mlx_error(MLX_MEMFAIL));
	if (!(mlx->context = calloc(1, sizeof(mlx_ctx_t))))
	{
		free(mlx);
		return ((void*)mlx_error(MLX_MEMFAIL));
	}
	mlx->width = width;
	mlx->height = height;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, resize);
	if (!(mlx->window = glfwCreateWindow(width, height, title, NULL, NULL)))
	{
		mlx_terminate(mlx);
		return ((void*)mlx_error(MLX_WINFAIL));
	}
	if (!mlx_init_render(mlx))
	{
		mlx_terminate(mlx);
		return (NULL);
	}
	return (mlx);
}
