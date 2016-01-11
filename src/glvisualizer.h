#ifndef GLVISUALIZER_H
#define GLVISUALIZER_H

#include <QGLWidget>

class glVisualizer : public QGLWidget
{
    Q_OBJECT

public:
    glVisualizer();

    typedef enum {
        VisualizerColorGreen,
        VisualizerColorRed,
        VisualizerColorBlue,
        VisualizerColorPurple,
        VisualizerColorYellow,
        VisualizerColorOrange,
        VisualizerColorCyan
    } VisualizerColor;

    /* Called by outer classes. This function is used to control the m_barJumpTimer */
    void setAnimationActive(bool);
    /* Returns whether animation should be active or not. */
    bool animationIsActive();
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w,int h);
private:
    /* Number of bars in visualizer. */
    static const short NUM_BARS = 100;
    /* Array of bar heights. */
    float m_barHeights[NUM_BARS];

    /* Timer for jump animation. */
    QTimer* m_barJumpTimer;
    /* Timer for drop animation. */
    QTimer* m_barDropTimer;

    /* 2d-array of color values */
    float m_colorArr[3][3];
    /* Holds current color value */
    VisualizerColor m_color;

    /* Sets corresponding color values for m_colorArr */
    void setColor(glVisualizer::VisualizerColor);
public slots:
    /* toggle visualizer color */
    void s_toggleVisualizerColor();
private slots:
    /* Called repeatedly to perform drop effect on bars. */
    void s_redrawDroppingBars();
    /* Randomly reset bar heights. Managed by parent. */
    void s_resetBarHeights();
};

#endif // GLVISUALIZER_H
