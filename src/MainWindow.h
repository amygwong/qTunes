#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QtMultimedia>
#include <QDebug>
#include <id3v2tag.h>
#include "glWidget.h"
#include "openPrompt.h"

class glVisualizer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    //! Constructor.
    MainWindow(QString);

    //! Destructor.
    ~MainWindow();

    public slots:
    // slots
    void s_load();
    void s_panel1(QListWidgetItem*);
    void s_panel2(QListWidgetItem*);
    void s_panel3(QListWidgetItem*);
    void s_about();

    void s_play();
    void s_play2();
    void s_stop();
    void s_next();
    void s_prev();
    void s_search(int);

    void s_updateDuration(qint64);
    void s_updatePosition(qint64);
    void s_setPosition(int);

    void s_animateLeft();
    void s_animateRight();
    void s_sortTable(int);
    void s_loadPrev();

    void s_mediaStateChanged(QMediaPlayer::State);
    void s_toggleMute();

    void s_shuffle();
    void s_repeat();

    // other functions
    void updateSong();
    QImage initImage(TagLib::ID3v2::Tag *);

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void createActions();
    void createMenus();
    void createWidgets();
    void createLayouts();
    void initLists();
    void redrawLists(QListWidgetItem *, int);
    void traverseDirs(QString);
    void setSizes(QSplitter *, int, int);
    void initAlbums();
    void loadDirs();
    void saveDir(QString path);

    // actions
    QAction		*m_loadAction;
    QAction		*m_quitAction;
    QAction		*m_aboutAction;
    QAction     *m_leftMoveAction;
    QAction     *m_rightMoveAction;

    // menus
    QMenu		*m_fileMenu;
    QMenu		*m_helpMenu;
    QMenu       *m_searchMenu;

    // widgets
    QSplitter	*m_mainSplit;
    QSplitter	*m_leftSplit;
    QSplitter	*m_rightSplit;

    QLineEdit   *m_typeSearch;
    QLabel		*m_labelSide[2];
    QLabel		*m_label[3];
    QListWidget 	*m_panel[3];
    QTableWidget	*m_table;
    QTabWidget      *m_tabWidget;

    // string lists
    QString		   m_directory;
    QString        m_searchText;
    QStringList	   m_listGenre;
    QStringList	   m_listArtist;
    QStringList	   m_listAlbum;
    QList<QStringList> m_listSongs;

    // player variables
    QMediaPlayer     *m_device;
    QMediaPlaylist   *m_playlist;

    QToolButton      *m_play;
    QToolButton      *m_stop;
    QToolButton      *m_next2;
    QToolButton      *m_previous2;
    QToolButton      *m_muteButton;

    QToolButton      *m_shuffle;
    QToolButton      *m_repeat;

    QComboBox        *m_search;
    QSlider          *m_volumeSlider;
    QSlider          *m_positionSlider;
    QLabel           *m_positionLabel;
    QLabel           *m_infoLabel;
    QLabel           *m_albumLabel;

    // images
    QImage           m_cover;
    QList<QImage>    m_albumsList;

    // cover flow
    glWidget         *m_glWidget;
    QToolButton      *m_next;
    QToolButton      *m_previous;

    // visualizer
    glVisualizer     *m_visualizer;
    QPushButton      *m_toggleColor;
    QTimer           *m_visualizerTimer;

    // table widget
    bool             m_ascendSorted;

};

#endif // MAINWINDOW_H
