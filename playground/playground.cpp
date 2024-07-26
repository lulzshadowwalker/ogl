#include <stdio.h>
#include <stdlib.h>

// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Handle platform independant window and input
#include <GLFW/glfw3.h>

// 3D maths
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <common/shader.hpp>

using namespace glm;

// x, y, z, w
// w: 0  -> direction
// w: 1  -> position in space

void testPointTranslation()
{
    printf("Point Translation:\n");
    glm::mat4 matrix = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f)); // This gives you the translation matrix. it does not actually do the translation
    glm::vec4 vertex(10.0f, 10.0f, 10.0f, 1.0f); // `w` is 1 then it's a point. you can translate a point.
    glm::vec4 transformedVertex = matrix * vertex;
    printf("(%f, %f, %f, %f)\n", transformedVertex.x, transformedVertex.y, transformedVertex.z, transformedVertex.w);
}

void testDirectionTranslation()
{
    printf("Direction Translation:\n");
    glm::mat4 matrix = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f)); // This gives you the translation matrix. it does not actually do the translation
    // `w` is 0 then it's a direction. you cannot translate a direction. it has no effect.
    // which makes sense, because `w` being 0 the last item in the vertex, would be multiplied by the translation (last column) in the matrix so you end up simply preserving the original vertex without any translation.
    glm::vec4 vertex(10.0f, 10.0f, 10.0f, 0.0f);
    glm::vec4 transformedVertex = matrix * vertex;
    printf("(%f, %f, %f, %f)\n", transformedVertex.x, transformedVertex.y, transformedVertex.z, transformedVertex.w);
}

// This line actually performs the scaling FIRST, and THEN the rotation, and THEN the translation. This is how matrix multiplication works.
// Writing the operations in another order wouldn’t produce the same result.
//
// TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector;

int main( void )
{
    // testPointTranslation();
    // testDirectionTranslation();
    // return 0;
    
    // Initialise GLFW
    glewExperimental = true; // Needed for core profile
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
    
    // Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
    window = glfwCreateWindow( 1024, 768, "lulzie's world", NULL, NULL);
    if( window == NULL ) 
    {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not OpenGL 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if ( glewInit() != GLEW_OK )
    {
        fprintf( stderr, "Failed to initialize GLEW\n" );
        return -1;
    }
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    // Create a vertex array and set it as the current one
    //
    // once your window is created (= after the OpenGL Context creation) and before any other OpenGL call.
    GLuint VertextArrayID;
    glGenVertexArrays(1, &VertextArrayID);
    glBindVertexArray(VertextArrayID);
    
    // (0, 0) being the center of the screen is something you can’t change, it’s built in your graphics card.
    
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        0.0f,  1.0f, 0.0f,
    };
    
    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    
    GLuint programID = LoadShaders( "/Users/lulz/dev/ogl/SimpleVertexShader.vertexshader", "/Users/lulz/dev/ogl/SimpleFragmentShader.fragmentshader" );
    
    // Projection matrix: 45° Field of View, 4:3 ratio, display range: 0.1 unit <-> 100 units
    float width = 200;
    float height = 200;
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);

    // Or, for an ortho camera:
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

    // Model matrix: an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    // Our ModelViewProjection: multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
    
    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    do{
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glUseProgram( programID );
        
        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        
        { // Draw
            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
            glVertexAttribPointer(
                                  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                                  3,                  // size
                                  GL_FLOAT,           // type
                                  GL_FALSE,           // normalized?
                                  0,                  // stride
                                  (void*)0            // array buffer offset
                                  );
            // Draw the triangle !
            glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
            glDisableVertexAttribArray(0);
        }
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_SEMICOLON ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );
    
    return 0;
}
