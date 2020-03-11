#include "gl_base.h"

GLuint program, edge_buffer, face_buffer, pos_atr, uv_atr, tex_lay;
GLuint is_textured_unif, fill_colour_unif, pos_offset_unif, pos_scale_unif, width_unif, height_unif, uv_scale_unif, uv_offset_unif;

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

static void get_locations() {
    pos_atr = glGetAttribLocation(program, "position");
    uv_atr = glGetAttribLocation(program, "tex_coord");

    is_textured_unif = glGetUniformLocation(program, "is_textured");
    fill_colour_unif = glGetUniformLocation(program, "fill_colour");
    pos_offset_unif = glGetUniformLocation(program, "pos_offset");
    pos_scale_unif = glGetUniformLocation(program, "pos_scale");
    tex_lay = glGetUniformLocation(program, "tex_layer");
    uv_offset_unif = glGetUniformLocation(program, "uv_offset");
    uv_scale_unif = glGetUniformLocation(program, "uv_scale");

    width_unif = glGetUniformLocation(program, "screen_width");
    height_unif = glGetUniformLocation(program, "screen_height");
}

static void load_panel_buffers() {
    GLfloat triangle[12] = {0.f, 1.f, 0.f, 1.f, 1.f, 0.f, 1.f,0.f, 0.f, 0.f,0.f, 0.f};
    GLfloat uvs[8] = {0.f, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f};
    GLushort faces[6] = {0, 1, 2, 0, 2, 3};
    GLushort edges[5] = {0, 1, 2, 3, 0};

	(void)create_array_buffer(triangle, pos_atr, sizeof(triangle), 3);
	(void)create_array_buffer(uvs, uv_atr, sizeof(uvs), 2);

	glGenBuffers(1, &edge_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edges), edges, GL_STATIC_DRAW);

	glGenBuffers(1, &face_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, face_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), faces, GL_STATIC_DRAW);
}

void load_pixels(GLsizei width, GLsizei height, const void *pixels) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_SHORT, pixels);
}

void init_gl() {
    program = setup_gl_program("../data/vshader", "../data/fshader");
    glUseProgram(program);

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    get_locations();

    load_panel_buffers();
    glUniform1i(tex_lay, 0);
}