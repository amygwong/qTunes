#ifndef GLWIDGET_H
#define GLWIDGET_H

#include<QGLWidget>
#include <QtOpenGL>

#ifdef _WIN32
    #include "Windows.h"
    #include "gl/glu.h"
#else
    #include "glu.h"
#endif

class glWidget : public QGLWidget
{
    Q_OBJECT

public:
    //constructor
    glWidget();

    //destructor
    ~glWidget();
    void        startAnimate(bool left);
    void        loadImages(QList<QImage> imgs);

public slots:
    void        s_animate();

private:
    bool                m_loaded;
    int                 m_size;
    int                 m_albNum;
    int                 m_dir;
    int                 m_listLength;
    int                 m_current;
    double              m_change;
    QTimer              *m_timer;
    QList<GLuint>       m_texture;

    void        square(int index, bool flip);



protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w,int h);



};

#endif
