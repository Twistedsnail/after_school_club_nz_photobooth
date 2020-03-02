#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>

extern GLuint program, edge_buffer, face_buffer, pos_atr, uv_atr, tex_lay;
extern GLuint is_textured_unif, fill_colour_unif, offset_unif, scale_unif;

GLuint setup_gl_program(std::string vshader_path, std::string fshader_path);
GLuint create_array_buffer(GLfloat *buffer, GLuint attrib, unsigned buffer_size, unsigned attrib_size);
void create_texture(GLuint *tex_ptr);
void init_gl();