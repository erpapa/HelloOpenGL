//
//  main.cpp
//  HelloOpenGL
//
//  Created by apple on 2018/5/3.
//  Copyright © 2018年 apple. All rights reserved.
//

#include <iostream>
#include <GLFW/glfw3.h>

//GLWF 按键回调
void key_callback( GLFWwindow* window , int key,int scancode, int action, int mode )
{
    
}

int main(int argc, char** argv)
{
    //初始化GLFW
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
    glfwWindowHint(GLFW_OPENGL_PROFILE , GLFW_OPENGL_CORE_PROFILE ); // 开启OpenGL core profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 向前兼容
    glfwWindowHint(GLFW_RESIZABLE , GL_FALSE );       // 窗口尺寸不可变
    
    //创建窗口，glfwCreateWindow前三个参数分别为 窗口宽和高，以及标题
    GLFWwindow* window = glfwCreateWindow(800, 600, "HelloOpenGL", nullptr, nullptr);
    if( window == nullptr ){
        std::cout << "Failed to create GLFW window" << std::endl;
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
    
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();                      // 检查有没有触发什么事件
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  // 设置清空屏幕所用的颜色
        glClear(GL_COLOR_BUFFER_BIT);          // 清空屏幕
        
        glfwSwapBuffers(window);             // 交换颜色缓冲
    }
    
    glfwTerminate(); //释放/删除之前的分配的所有资源    
    return 0;
}
