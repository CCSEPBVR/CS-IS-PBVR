#include "TransferFunctionEditor.h"
#include "ui_TransferFunctionEditor.h"
#include <kvs/ColorMapBar>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
class TriangleRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    TriangleRenderer(QWidget *parent = nullptr) : QOpenGLWidget(parent)
    {
        this->setMinimumHeight(80);
        this->setMaximumHeight(80);
    }

protected:
    void initializeGL() override
    {
        initializeOpenGLFunctions();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set clear color to white
    }

    void paintGL() override
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a simple triangle
        glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); // Red color
        glVertex3f(0.0f, 1.0f, 0.0f); // Top vertex
        glColor3f(0.0f, 1.0f, 0.0f); // Green color
        glVertex3f(-1.0f, -1.0f, 0.0f); // Bottom-left vertex
        glColor3f(0.0f, 0.0f, 1.0f); // Blue color
        glVertex3f(1.0f, -1.0f, 0.0f); // Bottom-right vertex
        glEnd();
    }

    void resizeGL(int w, int h) override
    {
        glViewport(0, 0, w, h);
    }
};

TransferFunctionEditor::TransferFunctionEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferFunctionEditor)
{
    ui->setupUi(this);
    TriangleRenderer* tr1 = new TriangleRenderer();
    TriangleRenderer* tr2 = new TriangleRenderer();
    TriangleRenderer* tr3 = new TriangleRenderer();
    TriangleRenderer* tr4 = new TriangleRenderer();

    ui->colorMapBarGLayout->addWidget( tr1, 0,0,1,1 );
    ui->colorMapHistogramGLayout->addWidget( tr2, 0,0,1,1 );
    ui->opacityMapBarGLayout->addWidget( tr3, 0,0,1,1 );
    ui->opacityMapHistogramGLayout->addWidget( tr4, 0,0,1,1 );
}

TransferFunctionEditor::~TransferFunctionEditor()
{
    delete ui;
}


