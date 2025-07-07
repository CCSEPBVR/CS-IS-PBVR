/*****************************************************************************/
/**
 *  @file   main.cpp
 *  @brief  Example program to display a simple triangle using OpenXR.
 *  @author Keisuke Tajiri
 */
 /*****************************************************************************/
 // SupportQt
#include <kvs/qt/Application>
// SupportOpenXR
#include <kvs/openxr/Screen>
// KVS
#include <kvs/OpenGL>
#include <kvs/PaintEventListener>


/*===========================================================================*/
/**
 *  @brief  Main function.
 *  @param  argc [i] argument count
 *  @param  argv [i] argument values
 */
 /*===========================================================================*/
int main(int argc, char** argv)
{
    // Application.
    kvs::qt::Application app(argc, argv);

    // Screen.
    kvs::openxr::Screen screen(&app);
    screen.setTitle("SimpleTriangle");
    screen.create();

    // User specified event.
    kvs::PaintEventListener paint_event([]()
        {
            kvs::OpenGL::PushAttrib(GL_ALL_ATTRIB_BITS);
            kvs::OpenGL::Enable(GL_DEPTH_TEST);
            glBegin(GL_TRIANGLES);
            glColor3ub(255, 0, 0); glVertex3d(0.0, 3.0, 0.0);
            glColor3ub(0, 255, 0); glVertex3d(3.0, -3.0, 0.0);
            glColor3ub(0, 0, 255); glVertex3d(-3.0, -3.0, 0.0);
            glEnd();
            kvs::OpenGL::PopAttrib();
        });
    screen.addEvent(&paint_event);
    return app.run();
}
