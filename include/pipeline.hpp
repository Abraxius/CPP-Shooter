#pragma once

struct Pipeline {
    Pipeline(std::string vertex_shader_path, std::string fragment_shader_path) {
         // compile shader at runtime
        const GLchar* vertexShaderString = load_shader(vertex_shader_path);
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderString, nullptr);
        glCompileShader(vertexShader);

        // check results
        GLint success;
        std::vector<GLchar> infoLog(512);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, infoLog.size(), nullptr, infoLog.data());
            std::cout << infoLog.data() << "\n";
        }

        // now we do the same for pixel/fragment shader
        const GLchar* fragmentShaderString = load_shader(fragment_shader_path);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderString, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, infoLog.size(), nullptr, infoLog.data());
            std::cout << infoLog.data() << "\n";
        }

        // to combine all shader stages, we create a shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, infoLog.size(), nullptr, infoLog.data());
            std::cout << infoLog.data() << "\n";
        }
        glDeleteShader(vertexShader); // can safely delete after linking
        glDeleteShader(fragmentShader); // can safely delete after linking
    }
    ~Pipeline() {
        glDeleteProgram(shaderProgram);
    }

    void bind() {
        glUseProgram(shaderProgram);
    }

    GLuint framebuffer;
    GLuint framebufferTexture;
private:
    GLuint shaderProgram;
};