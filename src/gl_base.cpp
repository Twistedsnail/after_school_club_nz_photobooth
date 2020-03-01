#include "gl_base.h"

static void check_compile_result(GLuint target, GLuint shader, GLint result) {
    if(result != -1) return;

    printf("%s compile failed:\n", target == GL_VERTEX_SHADER ? "Vertex shader" : "Fragment shader");

    GLint log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

    char *log = (char *)malloc(log_length);
    glGetShaderInfoLog(shader, log_length, NULL, log);
    printf("Log: %s\n", log);
    free(log);
}

static std::string load_shader(std::string path) {
	std::string shader_txt, line;
	std::ifstream in_file(path);

	while(std::getline(in_file, line)) {
		shader_txt += line + "\n";
	}
    in_file.close();

    return shader_txt;
}

static GLuint compile_shader(GLuint target, std::string code) {
    GLuint shader = glCreateShader(target);

    const char *shader_ptr = code.c_str();
	const int length = code.length();
	glShaderSource(shader, 1, &shader_ptr, &length);
    glCompileShader(shader);

	GLint result;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

    check_compile_result(target, shader, result);

    return shader;
}

GLuint setup_gl_program(std::string vshader_path, std::string fshader_path) {
    std::string vshader_code = load_shader(vshader_path);
    std::string fshader_code = load_shader(fshader_path);

    GLuint v_result = compile_shader(GL_VERTEX_SHADER, vshader_code);
    GLuint f_result = compile_shader(GL_FRAGMENT_SHADER, fshader_code);

    GLuint program = glCreateProgram();
    glAttachShader(program, v_result);
    glAttachShader(program, f_result);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(!status) printf("Program failed\n");

    return program;
}

GLuint create_array_buffer(GLfloat *buffer, GLuint attrib, unsigned buffer_size, unsigned attrib_size) {
    GLuint BO;
	glGenBuffers(1, &BO);
	glBindBuffer(GL_ARRAY_BUFFER, BO);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer, GL_STATIC_DRAW);
	glVertexAttribPointer(attrib, attrib_size, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*attrib_size, NULL);
    glEnableVertexAttribArray(attrib);
    return BO;
}

void create_texture(GLuint *tex_ptr) {
    glGenTextures(1, tex_ptr);
    glBindTexture(GL_TEXTURE_2D, *tex_ptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
