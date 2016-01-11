#include<QGLWidget>
#include <QtOpenGL>
#include "glWidget.h"
#include <iostream>

using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::glWidget:
//
// Constructor. Initialize used variables
//
glWidget::glWidget() {
    //size of squares
    //alter if square size is changed
    m_size = 2;
    
    m_albNum = 10; //number of albums shown (must be even and one will be clipped)
    m_change = 0; //step in animation
    m_timer = new QTimer;
    m_dir = 1; //direction (1=left)
    m_current = 0;  //current album
    m_listLength = 10; //number of albums displayed
    m_loaded = false; //images loaded?
    setAutoBufferSwap(true);
    
    //connect timer to animation
    connect(m_timer, SIGNAL(timeout()), this, SLOT(s_animate()));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::~glWidget:
//
// Destructor. Save settings.
//
glWidget::~glWidget() {}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::initializeGL:
//
// Sets up environment.
//
void glWidget::initializeGL() {
    glClearColor(0.0, 0.0, 0.0,0.0);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::resizeGL:
//
// Reshape handler routine. Called after reshaping window.
//
void glWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float) w/h, 1.0, 1000);
    gluLookAt(0,0,-4,0,0,0,0,1,0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::paintGL:
//
//draws frames
//
void glWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    int alb;
    
    //which album to display first depending on direction
    if(m_dir==-1)
        alb = m_current+m_albNum;
    else
        alb = m_current;
    
    //translate to position where the first image will be displayed
    glTranslatef(m_dir*(-(m_albNum+m_change))*m_size/2,0,0);
    
    //display the album covers
    for(int i=0; i<m_albNum; i++) {
       
        //translates to spot and stores location
        glTranslatef(m_dir*m_size,0,0);
        glPushMatrix();
        
        //draws album that will rotate to middle
        if(i==(m_albNum/2)) {
            glTranslatef(m_dir*(m_change/2)*m_size/2,0,-1);
            glRotatef(m_dir*90*((1-m_change/2)),0,1,0);
            glTranslatef(-m_dir*m_size/2,0,0);
            square(alb+(m_dir*i),true);
        }
        
        //draws album that is in the middle
        else if(i==(m_albNum/2)-1) {
            glTranslatef(-m_dir*(1-m_change/2)*m_size/2,0,-1);
            glRotatef(-m_dir*90*(m_change/2),0,1,0);
            glTranslatef(m_dir*m_size/2,0,0);
            square(alb+(m_dir*i),true);
        }
        
        //draws albums before middle
        else if(i<(m_albNum-1)/2) {
            glRotatef(m_dir*90,0,1,0);
            square(alb+(m_dir*i),false);
        }
        
        //draw albums after middle
        else if(i>(m_albNum-1)/2) {
            glTranslatef(m_dir*2*(m_albNum-i),0,0);
            
            //draw albums in backwards order so they 
            //dont draw on top of each other
            for(int j=i; j<m_albNum; j++) {
                glTranslatef(-m_dir*m_size,0,0);
                glPushMatrix();
                glRotatef(m_dir*(-90),0,1,0);
                square((m_dir*(m_albNum-1-j+i))+alb,false);
                glPopMatrix();
            }
            glPopMatrix();
            
            //break out because finished drawing everything
            break;
        }
        glPopMatrix();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::square:
//
// Draws one album cover.
//
void glWidget::square(int index, bool flip) {
    
    //handles when index is larger than length of Qimage list
    //and converts to index in the list
    index = index%m_listLength;
    if(index<0) {
        index*=-1;
        //if(index!=0)
            index = m_listLength-index;
    }
    
    // creates a texture
    // else a blank square is created instead
    if(m_loaded&&index<m_listLength) {

            // enables texture mapping
            glEnable(GL_TEXTURE_2D);

            // selects texture to bind
            glBindTexture(GL_TEXTURE_2D, m_texture[index]);


        // draws filled album cover polygon flipped
        if(flip) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor3f(.8, .8, .7);
            glBegin(GL_QUADS);
                glTexCoord2f(0, 1);	glVertex3f(1.0,1.0,0.0);
                glTexCoord2f(1, 1);	glVertex3f(-1.0,1.0,0.0);
                glTexCoord2f(1, 0);	glVertex3f(-1.0,-1.0,0.0);
                glTexCoord2f(0, 0);	glVertex3f(1.0,-1.0,0.0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        
        //draws filled album cover not flipped
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor3f(.8, .8, .7);
                glBegin(GL_QUADS);
                glTexCoord2f(1, 1);	glVertex3f(1.0,1.0,0.0);
                glTexCoord2f(0, 1);	glVertex3f(-1.0,1.0,0.0);
                glTexCoord2f(0, 0);	glVertex3f(-1.0,-1.0,0.0);
                glTexCoord2f(1, 0);	glVertex3f(1.0,-1.0,0.0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
        
        // draws a red outline on the polygon
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_LINE_SMOOTH);
        glColor3f(.8, 0, 0);
        glBegin(GL_QUADS);
             glVertex3f(1.0,1.0,0.0);
             glVertex3f(-1.0,1.0,0.0);
             glVertex3f(-1.0,-1.0,0.0);
             glVertex3f(1.0,-1.0,0.0);
        glEnd();
        glDisable(GL_LINE_SMOOTH);
        glFlush();
    }
    
    //creates empty white squares for initial start up
    else {
        glBegin(GL_POLYGON);
            glVertex3f(1.0,1.0,0.0);
            glVertex3f(-1.0,1.0,0.0);
            glVertex3f(-1.0,-1.0,0.0);
            glVertex3f(1.0,-1.0,0.0);
        glEnd();
        glFlush();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::loadImages:
//
// Adds images to the list of qimages.
//
void glWidget::loadImages(QList<QImage> imgs) {
    m_loaded =true;
    glEnable(GL_TEXTURE_2D);

    for(int i=0; i<imgs.size(); i++) {
//        m_imagelist << imgs[i];
        // storage for one texture


        // loads and binds the texture from a qimage
        m_texture << bindTexture(imgs[i]);
        glBindTexture  (GL_TEXTURE_2D,   m_texture[i]);
        // sets texture parameters
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    }
    glDisable(GL_TEXTURE_2D);
    m_listLength = imgs.size();
    m_current=0;
    updateGL();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::s_animate:
//
// Slot function keep animation happening
//
void glWidget::s_animate() {
    //controlls speed of the animation
    m_change += .05;
    updateGL();
    
    //determines when animation stops
    if(m_change>=2) {
        
        //stop and reset animation
        m_timer->stop();
        m_change=0;
        
        //update current location
        if(m_dir==1)
            m_current++;
        else
            m_current--;
        
        //handles if m_current is not an index
        if(m_current >= m_listLength)
            m_current = 0;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glWidget::startAnimate:
//
// start animation and sets conditions
//
void glWidget::startAnimate(bool left) {
    //sets conditions based on direction
    if(left) {
        
        //need to change m_current if the direction is different from before
        if(m_dir==-1)
            m_current += 2;
        m_dir = 1;
    }
    else {
        
        //need to change m_current if the direction is different from before
        if(m_dir==1)
           m_current -= 2;
        m_dir = -1;
    }
    
    //start animation
    m_timer->start(10);
}
