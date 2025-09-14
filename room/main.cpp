#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QKeyEvent>

// A custom OpenGL widget to render our 3D room.
class RoomWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    RoomWidget(QWidget* parent = nullptr) : QOpenGLWidget(parent)
    {
        // Set up the scene state
        m_rotation = 0.0f;
        setFocusPolicy(Qt::StrongFocus); // Enable keyboard input
    }

protected:
    void initializeGL() override
    {
        // Initialize OpenGL functions
        initializeOpenGLFunctions();

        // Set the background color (light gray)
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

        // Enable depth test for correct rendering of overlapping faces
        glEnable(GL_DEPTH_TEST);

        // Render polygons as lines (wireframe mode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void resizeGL(int w, int h) override
    {
        // Set the viewport
        glViewport(0, 0, w, h);

        // Reset the projection matrix to handle window resizing
        m_projection.setToIdentity();
        m_projection.perspective(45.0f, GLfloat(w) / GLfloat(h), 0.1f, 100.0f);
    }

    void paintGL() override
    {
        // Clear the screen and the depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up the view matrix for the camera
        QMatrix4x4 viewMatrix;
        viewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.5f), // Camera position moved closer
                         QVector3D(0.0f, 0.0f, 0.0f), // Point to look at
                         QVector3D(0.0f, 1.0f, 0.0f)); // Up direction

        // Rotate the view based on the current rotation value
        viewMatrix.rotate(m_rotation, 0.0f, 1.0f, 0.0f);

        // Set the projection and modelview matrices
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_projection.data());
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(viewMatrix.data());

        // Draw the simple wireframe box
        drawBox();
    }
    
    void keyPressEvent(QKeyEvent* event) override
    {
        switch (event->key()) {
            case Qt::Key_Left:
                m_rotation -= 5.0f;
                update();
                break;
            case Qt::Key_Right:
                m_rotation += 5.0f;
                update();
                break;
            default:
                QOpenGLWidget::keyPressEvent(event);
                break;
        }
    }

private:
    void drawBox()
    {
        // Set the color for the lines
        glColor3f(0.0f, 0.0f, 0.0f); // Black lines

        // A cube's vertices
        GLfloat vertices[] = {
            // Front face
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            // Back face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,

            // Top face
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            // Bottom face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            // Right face
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,

            // Left face
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f
        };

        // Indices to draw the faces
        GLubyte indices[] = {
            0, 1, 2, 3, // Front
            4, 5, 6, 7, // Back
            8, 9, 10, 11, // Top
            12, 13, 14, 15, // Bottom
            16, 17, 18, 19, // Right
            20, 21, 22, 23 // Left
        };

        // Draw the box using quads (wireframe mode will render lines)
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, vertices);
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

private:
    QMatrix4x4 m_projection;
    GLfloat m_rotation;
};

int main(int argc, char *argv[])
{
    // Initialize the Qt application
    QApplication app(argc, argv);

    // Create and show the main window
    RoomWidget window;
    window.setWindowTitle("Simple 3D Wireframe Box");
    window.resize(800, 600);
    window.show();

    return app.exec();
}
