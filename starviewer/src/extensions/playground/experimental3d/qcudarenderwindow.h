#ifndef QCUDARENDERWINDOW_H
#define QCUDARENDERWINDOW_H


#ifdef CUDA_AVAILABLE
#include <GL/glew.h>
#endif
#include <QGLWidget>


namespace udg {


class QCudaRenderWindow : public QGLWidget {

//    Q_OBJECT  // no funciona (?????!?!?!?!??!!!!!!!!!??!???????!???!)

public:

    QCudaRenderWindow( QColor backgroundColor, int renderSize );
    virtual ~QCudaRenderWindow();

    GLuint pixelBufferObject() const;

protected:

    virtual void initializeGL();
    virtual void resizeGL( int width, int height );
    virtual void paintGL();

private:

    QColor m_backgroundColor;
    int m_renderSize;
    GLuint m_pixelBufferObject;
    GLuint m_texture;

};


}


#endif // QCUDARENDERWINDOW_H
