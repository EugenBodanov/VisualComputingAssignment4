#include "debug.h"

#include "shader.h"

const std::string vertex_shader_code_debug = R"END(
    #version 330 core

    layout(location = 0) in vec3 aPosition;
    layout(location = 1) in vec3 aColor;
    out vec3 tColor;

    uniform mat4 uProj;
    uniform mat4 uView;

    void main()
    {
        gl_Position = uProj * uView * vec4(aPosition, 1.0);
        tColor = aColor;
    }
)END";

const std::string fragment_shader_code_debug = R"END(
    #version 330 core

    in vec3 tColor;
    out vec4 fragColor;

    void main(void)
    {
        fragColor = vec4(tColor, 1.0);
    }
)END";

struct VisualDebug
{
    ShaderProgram shader;

    GLuint vao = 0;
    GLuint vbo = 0;

    float pointSize = 64.0f;
    float lineSize = 8.0f;

    std::vector<DebugVertex> points;
    std::vector<DebugVertex> lines;
    std::vector<DebugVertex> triangles;
};

VisualDebug sVisualDebugger;

void debugInit()
{
    sVisualDebugger.shader = shaderCreate(vertex_shader_code_debug, fragment_shader_code_debug);

    glGenVertexArrays(1, &sVisualDebugger.vao);
    glGenBuffers(1, &sVisualDebugger.vbo);

    glBindVertexArray(sVisualDebugger.vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, sVisualDebugger.vbo);
        glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(DebugVertex), nullptr, GL_DYNAMIC_DRAW);
        glCheckError();

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*) offsetof(DebugVertex, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*) offsetof(DebugVertex, color));
        glCheckError();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void debugShutdown()
{
    shaderDelete(sVisualDebugger.shader);
    glDeleteBuffers(1, &sVisualDebugger.vbo);
    glDeleteBuffers(1, &sVisualDebugger.vao);
}

void debugDrawPoints(const std::vector<DebugVertex>& points)
{
    sVisualDebugger.points.insert(sVisualDebugger.points.end(), points.begin(), points.end());
}

void debugDrawLines(const std::vector<DebugVertex>& lines)
{
    sVisualDebugger.lines.insert(sVisualDebugger.lines.end(), lines.begin(), lines.end());
}

void debugDrawTriangles(const std::vector<DebugVertex>& triangles)
{
    sVisualDebugger.triangles.insert(sVisualDebugger.triangles.end(), triangles.begin(), triangles.end());
}

void debugDraw(const Camera& camera)
{
    glBindVertexArray(sVisualDebugger.vao);
    glUseProgram(sVisualDebugger.shader.id);
    shaderUniform(sVisualDebugger.shader, "uProj",  cameraProjection(camera));
    shaderUniform(sVisualDebugger.shader, "uView",  cameraView(camera));

    glDisable(GL_DEPTH_TEST);

    /*  points */
    if(!sVisualDebugger.points.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, sVisualDebugger.vbo);
        glBufferData(GL_ARRAY_BUFFER, sVisualDebugger.points.size() * sizeof(DebugVertex), sVisualDebugger.points.data(), GL_DYNAMIC_DRAW);
        glCheckError();

        glPointSize(sVisualDebugger.pointSize);
        glDrawArrays(GL_POINTS, 0, sVisualDebugger.points.size());

        sVisualDebugger.points.clear();
    }

    /*  lines */
    if(!sVisualDebugger.lines.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, sVisualDebugger.vbo);
        glBufferData(GL_ARRAY_BUFFER, sVisualDebugger.lines.size() * sizeof(DebugVertex), sVisualDebugger.lines.data(), GL_DYNAMIC_DRAW);
        glCheckError();

        glLineWidth(sVisualDebugger.lineSize);
        glDrawArrays(GL_LINES, 0, sVisualDebugger.lines.size());

        sVisualDebugger.lines.clear();
    }

    /*  triangles */
    if(!sVisualDebugger.triangles.empty())
    {
        glBindBuffer(GL_ARRAY_BUFFER, sVisualDebugger.vbo);
        glBufferData(GL_ARRAY_BUFFER, sVisualDebugger.triangles.size() * sizeof(DebugVertex), sVisualDebugger.triangles.data(), GL_DYNAMIC_DRAW);
        glCheckError();

        glDrawArrays(GL_TRIANGLES, 0, sVisualDebugger.triangles.size());

        sVisualDebugger.triangles.clear();
    }

    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
    glUseProgram(0);
}
