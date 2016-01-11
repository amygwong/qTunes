#include "glvisualizer.h"
#include <QTimer>
#include <ctime>

#ifdef _WIN32
    #include "Windows.h"
    #include "gl/glu.h"
#else
    #include "glu.h"
#endif

// To draw the bars

/* Position of bar's base. */
const float BASE_POSITION = -0.7f;
/* Minimum height of bar. */
const float MIN_HEIGHT = 0.01f;
// the distance of the first (and last) bar(s) from the side of the canvas
float DISTANCE_FROM_SIDES = 0.05f;

/* Drop rate for bars. */
const float DROP_RATE = 0.03f;

/* Number of colors. */
const short NUM_COLORS = 7;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::glVisualizer():
//
// Constructor.
//
glVisualizer::glVisualizer()
{
    m_barDropTimer = new QTimer();
    m_barJumpTimer = new QTimer();

    m_color = VisualizerColorGreen;

    // initially set all bars to min height
    for(int i = 0; i < NUM_BARS; ++i) {
        m_barHeights[i] = MIN_HEIGHT;
    }

    // connect timer to redraw bars
    connect(m_barDropTimer, SIGNAL(timeout()),
            this, SLOT(s_redrawDroppingBars()));
    // jump bar timer jumps bars up randomly
    connect(m_barJumpTimer, SIGNAL(timeout()),
            this, SLOT(s_resetBarHeights()));

    // redraw bars every 25ms (they will start to drop)
    m_barDropTimer->start(25);

    setColor((VisualizerColor)(m_color % 5));

    srand(time(NULL));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::resizeGL(int,int):
//
// Resizes.
//
void glVisualizer::resizeGL(int w, int h){
    glViewport(0, 0, w, h);
};



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::glVisualizer():
//
// Constructor.
//
void glVisualizer::initializeGL() {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::painGL():
//
// Repaints the bars.  Decreases bar heights every time this is called.
//
void glVisualizer::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);

    // the left point of the first bar (x-coordinate)
    float leftPoint = DISTANCE_FROM_SIDES - 1.0f;

    // width of each bar is the canvas's length divided by the number of bars
    const float canvasWidth = 2.0f;
    float barWidth = (canvasWidth - (2*DISTANCE_FROM_SIDES))/NUM_BARS;

    // in paintGL()
    // draw [NUM_BARS] bars
    for(int i = 0; i < NUM_BARS; ++i) {
        // every time paintGL() is called, bar heights decrease by DROP_RATE
        m_barHeights[i] -= DROP_RATE;
        // shouldn't go below min height
        m_barHeights[i] = m_barHeights[i] < MIN_HEIGHT ? MIN_HEIGHT : m_barHeights[i];

        glBegin(GL_QUADS);
            glColor3d(m_colorArr[0][0], m_colorArr[0][1], m_colorArr[0][2]);
            glVertex2f(leftPoint, BASE_POSITION);
            glColor3d(m_colorArr[1][0], m_colorArr[1][1], m_colorArr[1][2]);
            glVertex2f(leftPoint, BASE_POSITION + m_barHeights[i]);
            glColor3d(m_colorArr[2][0], m_colorArr[2][1], m_colorArr[2][2]);
            glVertex2f(leftPoint + barWidth, BASE_POSITION + m_barHeights[i]);
            glColor3d(m_colorArr[1][0], m_colorArr[1][1], m_colorArr[1][2]);
            glVertex2f(leftPoint + barWidth, BASE_POSITION);
        glEnd();

        // set new left point for the next bar
        leftPoint += barWidth;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::s_redrawDroppingBars():
//
// Slot called by timer to redraw bars and perform dropping effect.
//
void glVisualizer::s_redrawDroppingBars() {
    repaint();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::s_resetBarHeights():
//
// Creates a new random height for each bar.
// Only applies new height if it is greater than current height.
//
void glVisualizer::s_resetBarHeights() {
    // (maximum height for each bar) * 100 to avoid unnecessary casting
    int barMaxHeight = 150;
    int barMinHeight = 50;

    for(int i = 0; i < NUM_BARS; ++i) {
            // new height is higher than the current height and less than the maximum height
            float newHeight = (rand() % (barMaxHeight - barMinHeight) + barMinHeight) / 100.0f;
            // if random height is larger than current bar height, set new height
            m_barHeights[i] = newHeight >= m_barHeights[i] ? newHeight : m_barHeights[i];
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::setAnimationActive(bool):
//
// Sets the jump animation to active or inactive.
//
void glVisualizer::setAnimationActive(bool b) {
    if(b) {
        if(!m_barJumpTimer->isActive())
            m_barJumpTimer->start(500);
    }else {
        if(m_barJumpTimer->isActive())
            m_barJumpTimer->stop();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::animationIsActive():
//
// Accessor for animation activity.
//
bool glVisualizer::animationIsActive() {
    return m_barJumpTimer->isActive();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::setColor(glVisualizer::VisualizerColor):
//
// Sets color of bars.
//
void glVisualizer::setColor(glVisualizer::VisualizerColor vc) {
    switch(vc) {
    case VisualizerColorGreen: {
        m_colorArr[0][0] = 0.0f; m_colorArr[0][1] = 0.6f; m_colorArr[0][2] = 0.0f;
        m_colorArr[1][0] = 0.0f; m_colorArr[1][1] = 0.8f; m_colorArr[1][2] = 0.0f;
        m_colorArr[2][0] = 0.0f; m_colorArr[2][1] = 1.0f; m_colorArr[2][2] = 0.0f;
    }
        break;
    case VisualizerColorRed: {
        m_colorArr[0][0] = 0.6f; m_colorArr[0][1] = 0.0f; m_colorArr[0][2] = 0.0f;
        m_colorArr[1][0] = 0.8f; m_colorArr[1][1] = 0.0f; m_colorArr[1][2] = 0.0f;
        m_colorArr[2][0] = 1.0f; m_colorArr[2][1] = 0.0f; m_colorArr[2][2] = 0.0f;
    }
        break;
    case VisualizerColorBlue: {
        m_colorArr[0][0] = 0.0f; m_colorArr[0][1] = 0.0f; m_colorArr[0][2] = 0.6f;
        m_colorArr[1][0] = 0.0f; m_colorArr[1][1] = 0.0f; m_colorArr[1][2] = 0.8f;
        m_colorArr[2][0] = 0.0f; m_colorArr[2][1] = 0.0f; m_colorArr[2][2] = 1.0f;
    }
        break;
    case VisualizerColorPurple: {
        m_colorArr[0][0] = 0.6f; m_colorArr[0][1] = 0.0f; m_colorArr[0][2] = 0.6f;
        m_colorArr[1][0] = 0.8f; m_colorArr[1][1] = 0.0f; m_colorArr[1][2] = 0.8f;
        m_colorArr[2][0] = 1.0f; m_colorArr[2][1] = 0.0f; m_colorArr[2][2] = 1.0f;
    }
        break;
    case VisualizerColorYellow: {
        m_colorArr[0][0] = 0.6f; m_colorArr[0][1] = 0.6f; m_colorArr[0][2] = 0.0f;
        m_colorArr[1][0] = 0.8f; m_colorArr[1][1] = 0.8f; m_colorArr[1][2] = 0.0f;
        m_colorArr[2][0] = 1.0f; m_colorArr[2][1] = 1.0f; m_colorArr[2][2] = 0.0f;
    }
        break;
    case VisualizerColorOrange: {
        m_colorArr[0][0] = 0.6f; m_colorArr[0][1] = 0.39f; m_colorArr[0][2] = 0.0f;
        m_colorArr[1][0] = 0.8f; m_colorArr[1][1] = 0.52f; m_colorArr[1][2] = 0.0f;
        m_colorArr[2][0] = 1.0f; m_colorArr[2][1] = 0.65f; m_colorArr[2][2] = 0.0f;
    }
        break;
    default: {
        m_colorArr[0][0] = 0.0f; m_colorArr[0][1] = 0.6f; m_colorArr[0][2] = 0.6f;
        m_colorArr[1][0] = 0.0f; m_colorArr[1][1] = 0.8f; m_colorArr[1][2] = 0.8f;
        m_colorArr[2][0] = 0.0f; m_colorArr[2][1] = 1.0f; m_colorArr[2][2] = 1.0f;
    }
        break;
    }
    paintGL();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// glVisualizer::s_toggleVisualizerColor():
//
// Slot to be connected to the click signal of a button in an external class.
//
void glVisualizer::s_toggleVisualizerColor() {
    /* increment color and loop around if color limit is exceeded */
    short newColor = (m_color + 1) % NUM_COLORS;
    m_color = (VisualizerColor)newColor;
    // will set m_colorArr values to corresponding glVisualizer::VisualizerColor
    setColor(m_color);
}
