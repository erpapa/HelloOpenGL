//
//  main.cpp
//  HelloOpenGL
//
//  Created by apple on 2018/5/3.
//  Copyright © 2018年 apple. All rights reserved.
//

#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "Point.h"
#include "Vectoriser.h"
#include "poly2tri/poly2tri.h"

struct Vector3df
{
    float x, y, z;
};

struct Tri
{
    Vector3df a, b, c;
};

//GLWF 按键回调
void key_callback( GLFWwindow* window , int key,int scancode, int action, int mode )
{

}

//GLWF 窗口大小回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

float add_character(std::vector<Tri> &tris, FT_Face face, char ch, int bezier_steps, float extrude, float offset)
{
    static FT_UInt prevCharIndex = 0, curCharIndex = 0;
    static FT_Pos  prev_rsb_delta = 0;
    
    curCharIndex = FT_Get_Char_Index( face, ch );
    if(FT_Load_Glyph( face, curCharIndex, FT_LOAD_DEFAULT ))
        printf("FT_Load_Glyph failed\n");
    
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph ))
        printf("FT_Get_Glyph failed\n");
    
    if(glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        printf("Invalid Glyph Format\n");
        exit(0);
    }
    
    short nCountour = 0;
    nCountour = face->glyph->outline.n_contours;
    
    // int startIndex = 0, endIndex = 0;
    // FT_Outline* o = &face->glyph->outline;
    
    if(FT_HAS_KERNING( face ) && prevCharIndex) {
        FT_Vector  kerning;
        FT_Get_Kerning( face, prevCharIndex, curCharIndex, FT_KERNING_DEFAULT, &kerning );
        offset += kerning.x >> 6;
    }
    
    if ( prev_rsb_delta - face->glyph->lsb_delta >= 32 )
        offset -= 1.0f;
    else if ( prev_rsb_delta - face->glyph->lsb_delta < -32 )
        offset += 1.0f;
    
    prev_rsb_delta = face->glyph->rsb_delta;
    
    Vectoriser *vectoriser = new Vectoriser(face->glyph, bezier_steps);
    for(size_t c = 0; c < vectoriser->ContourCount(); ++c) {
        const Contour* contour = vectoriser->GetContour(c);
        
        for(size_t p = 0; p < contour->PointCount() - 1; ++p) {
            const double* d1 = contour->GetPoint(p);
            const double* d2 = contour->GetPoint(p + 1);
            Tri t1;
            t1.a.x = (d1[0]/64.0f) + offset;
            t1.a.y = d1[1]/64.0f;
            t1.a.z = 0.0f;
            t1.b.x = (d2[0]/64.0f) + offset;
            t1.b.y = d2[1]/64.0f;
            t1.b.z = 0.0f;
            t1.c.x = (d1[0]/64.0f) + offset;
            t1.c.y = d1[1]/64.0f;
            t1.c.z = extrude;
            tris.push_back(t1);
            
            Tri t2;
            t2.a.x = (d1[0]/64.0f) + offset;
            t2.a.y = d1[1]/64.0f;
            t2.a.z = extrude;
            t2.b.x = (d2[0]/64.0f) + offset;
            t2.b.y = d2[1]/64.0f;
            t2.b.z = extrude;
            t2.c.x = (d2[0]/64.0f) + offset;
            t2.c.y = d2[1]/64.0f;
            t2.c.z = 0.0f;
            tris.push_back(t2);
        }
        
        if (contour->GetDirection()) {
            std::vector<p2t::Point*> polyline;
            for(size_t p = 0; p < contour->PointCount(); ++p) {
                const double* d = contour->GetPoint(p);
                polyline.push_back(new p2t::Point((d[0]/64.0f) + offset, d[1]/64.0f));
            }
            
            p2t::CDT* cdt = new p2t::CDT(polyline);
            for(size_t cm = 0; cm < vectoriser->ContourCount(); ++cm) {
                const Contour* sm = vectoriser->GetContour(cm);
                if(c != cm && !sm->GetDirection() && sm->IsInside(contour)) {
                    std::vector<p2t::Point*> pl;
                    for(size_t p = 0; p < contour->PointCount(); ++p) {
                        const double* d = contour->GetPoint(p);
                        pl.push_back(new p2t::Point((d[0]/64.0f) + offset, d[1]/64.0f));
                    }
                    cdt->AddHole(pl);
                }
            }
            
            cdt->Triangulate();
            std::vector<p2t::Triangle*> ts = cdt->GetTriangles();
            for(int i = 0; i < ts.size(); i++) {
                p2t::Triangle* ot = ts[i];
                
                Tri t1;
                t1.a.x = ot->GetPoint(0)->x;
                t1.a.y = ot->GetPoint(0)->y;
                t1.a.z = 0.0f;
                t1.b.x = ot->GetPoint(1)->x;
                t1.b.y = ot->GetPoint(1)->y;
                t1.b.z = 0.0f;
                t1.c.x = ot->GetPoint(2)->x;
                t1.c.y = ot->GetPoint(2)->y;
                t1.c.z = 0.0f;
                tris.push_back(t1);
                
                Tri t2;
                t2.a.x = ot->GetPoint(0)->x;
                t2.a.y = ot->GetPoint(0)->y;
                t2.a.z = extrude;
                t2.b.x = ot->GetPoint(1)->x;
                t2.b.y = ot->GetPoint(1)->y;
                t2.b.z = extrude;
                t2.c.x = ot->GetPoint(2)->x;
                t2.c.y = ot->GetPoint(2)->y;
                t2.c.z = extrude;
                tris.push_back(t2);
            }
            delete cdt;
        }
    }
    
    delete vectoriser;
    vectoriser = NULL;
    
    prevCharIndex = curCharIndex;
    float chSize = face->glyph->advance.x >> 6;
    return offset + chSize;
}

bool compile_shader(GLuint *shader, GLenum type, const char *shader_str)
{
    GLint status;
    const GLchar *source = shader_str;
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE)
    {
        GLint logLength;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(*shader, logLength, &logLength, log);
            std::cout << log << std::endl;
            free(log);
        }
    }
    return status == GL_TRUE;
}

GLuint create_program(const char *vertString, const char *fragString)
{
    GLuint vertShader;
    GLuint fragShader;
    GLuint program = glCreateProgram();
    if (!compile_shader(&vertShader, GL_VERTEX_SHADER, vertString)) {
        std::cout << "Failed to compile vertex shader" << std::endl;
    }
    if (!compile_shader(&fragShader, GL_FRAGMENT_SHADER, fragString)) {
        std::cout << "Failed to compile fragment shader" << std::endl;
    }
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    
    GLint status;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        return -1;
    }
    if (vertShader) {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader) {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    return program;
}

int main(int argc, char** argv)
{
    const char *char_font = "hwxk.ttf";
    const char *char_str = "Hi";
    int char_height = 48;
    int bezier_steps = 4;
    float extrude = 16;
    
    FT_Library library;
    if (FT_Init_FreeType( &library ))
        printf("FT_Init_FreeType failed\n");
    
    FT_Face face;
    if (FT_New_Face( library, char_font, 0, &face ))
        printf("FT_New_Face failed (there is probably a problem with your font file\n");
    
    int char_size = char_height << 6;
    FT_Set_Char_Size( face, char_size, char_size, 96, 96);
    
    std::vector<Tri> tris;
    float offset = 0;
    for (int i = 0; i < strlen(char_str); i++) {
        offset = add_character(tris, face, char_str[i], bezier_steps, extrude, offset);
    }
    
    int vert_ount = (int)tris.size() * 3;
    int vert_size = vert_ount * 3;
    float *vertices = (float *)malloc(sizeof(float) * vert_size);
    for (int i = 0, j = 0; i < tris.size(); i+=1, j+=9) {
        Tri t = tris[i];
        vertices[j] = t.a.x / char_size * 64.0;
        vertices[j + 1] = t.a.y / char_size * 64.0;
        vertices[j + 2] = t.a.z / char_size * 64.0;
        
        vertices[j + 3] = t.b.x / char_size * 64.0;
        vertices[j + 4] = t.b.y / char_size * 64.0;
        vertices[j + 5] = t.b.z / char_size * 64.0;
        
        vertices[j + 6] = t.c.x / char_size * 64.0;
        vertices[j + 7] = t.c.y / char_size * 64.0;
        vertices[j + 8] = t.c.z / char_size * 64.0;
    }
    
    // 初始化GLFW
    glfwInit();
#ifdef __APPLE__
    // Select OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // 主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // 次版本号
#else
    // Select OpenGL 4.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // 开启OpenGL core profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 向前兼容
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE ); // 窗口尺寸不可变

    //创建窗口，glfwCreateWindow前三个参数分别为 窗口宽和高，以及标题
    GLFWwindow *window = glfwCreateWindow(800, 600, "HelloOpenGL", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        free(vertices);
        glfwTerminate();  // 销毁窗口和资源
        return -1;
    }
    glfwMakeContextCurrent(window);  //将我们窗口的上下文设置为当前线程的主上下文

    int width , height ;
    glfwGetFramebufferSize(window, &width, &height);  //得到窗口大小

    /*  glViewport函数前两个参数控制窗口左下角的位置。第三个和第四个参数控制渲染窗口的宽度和高度（像素)
     这里也可以把glViewport设置的比 glfw 的窗口小。
     */
    glViewport(0, 0, width, height);
    // 通过GLFW注册我们的函数至合适的回调
    glfwSetKeyCallback(window ,key_callback);
    // 注册窗口大小变化回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    const char *vertString = "#version 330 core\n\
    layout (location = 0) in vec3 position;\n\
    uniform mat4 transformMatrix;\n\
    void main()\n\
    {\n\
    gl_Position = transformMatrix * vec4(position.x, position.y, position.z, 1.0);\n\
    }\0";
    const char *fragString = "#version 330 core\n\
    out vec4 FragColor;\n\
    void main()\n\
    {\n\
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n\
    }\0";
    GLuint program = create_program(vertString, fragString);
    if (program == -1) {
        std::cout << "Failed to create program" << std::endl;
        free(vertices);
        glfwTerminate();  // 销毁窗口和资源
        return -1;
    }
    GLuint transform_uniform = glGetUniformLocation(program, "transformMatrix");

    // 创建顶点缓存
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vert_size * sizeof(float), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    
    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    
    float degrees = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // 设置清空屏幕所用的颜色
        glClear(GL_COLOR_BUFFER_BIT);          // 清空屏幕
        // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // 默认填充模式
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 线框模式

        glUseProgram(program);
        // x轴方向旋转
        float radians = degrees * (M_PI / 180);
        float cos = cosf(radians);
        float sin = sinf(radians);
        GLfloat matrix[] = { 1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, cos, sin, 0.0f,
            0.0f, -sin, cos, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f };
        glUniformMatrix4fv(transform_uniform, 1, GL_FALSE, matrix);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vert_ount);
        
        glfwSwapBuffers(window);             // 交换颜色缓冲
        glfwPollEvents();                   // 检查有没有触发什么事件
        degrees += 0.01;
    }
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glValidateProgram(program);
    glDeleteProgram(program);
    free(vertices);
    glfwTerminate(); //释放/删除之前的分配的所有资源
    return 0;
}
