// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2015 by George Wolberg
//
// MainWindow.cpp - Main Window widget
//
// Written by: George Wolberg, 2015
// ======================================================================

#include <QtWidgets>
#include "MainWindow.h"
#include "glWidget.h"
#include "glvisualizer.h"

#include <tag.h>
#include <fileref.h>
#include<tbytevector.h>
#include<mpegfile.h>
#include<id3v2tag.h>
#include<id3v2frame.h>
#include <attachedPictureFrame.h>

using namespace std;

enum {TITLE, TRACK, TIME, ARTIST, ALBUM, GENRE, PATH};
const int COLS = PATH;

bool caseInsensitive(const QString &s1, const QString &s2)
{
    return s1.toLower() < s2.toLower();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::MainWindow:
//
// Constructor. Initialize user-interface elements.
//
MainWindow::MainWindow	(QString program)
       : m_directory(".") {
    // set the focus for keyPressEvents to GUI
    setFocusPolicy(Qt::StrongFocus);

    // setup GUI with actions, menus, widgets, and layouts
    createActions();	// create actions for each menu item
    createMenus  ();	// create menus and associate actions
    createWidgets();	// create window widgets
    createLayouts();	// create widget layouts

    // populate the list widgets with music library data
    initLists();		// init list widgets

    // set main window titlebar
    QString copyright = "Copyright (C) 2015 by George Wolberg";
    QString version	  = "Version 1.0";
    QString title	  =  QString("%1   (%2)         %3")
                .arg(program).arg(version).arg(copyright);

    setWindowTitle(tr("qTunes"));

    // set central widget and default size
    setCentralWidget(m_mainSplit);
    setMinimumSize(400, 300);
    resize(800, 600);


    loadDirs();
    s_mediaStateChanged(m_device->state());
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::~MainWindow:
//
// Destructor. Save settings.
//
MainWindow::~MainWindow() {}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createActions:
//
// Create actions to associate with menu and toolbar selection.
//
void MainWindow::createActions() {
    // initializes a media player
    m_device = new QMediaPlayer;
    // playlist
    m_playlist = new QMediaPlaylist();
    m_playlist->setCurrentIndex(0);
    m_playlist->setPlaybackMode(QMediaPlaylist::Loop);
    m_device->setPlaylist(m_playlist);

    m_loadAction = new QAction("&Load Music Folder", this);
    m_loadAction->setShortcut(tr("Ctrl+L"));
    connect(m_loadAction, SIGNAL(triggered()), this, SLOT(s_load()));

    m_quitAction = new QAction("&Quit", this);
    m_quitAction->setShortcut(tr("Ctrl+Q"));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    m_aboutAction = new QAction("&About", this);
    m_aboutAction->setShortcut(tr("Ctrl+A"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(s_about()));

    connect(m_playlist, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateSong()));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createMenus:
//
// Create menus and install in menubar.
//
void MainWindow::createMenus() {
    m_fileMenu = menuBar()->addMenu("&File");
    m_fileMenu->addAction(m_loadAction);
    m_fileMenu->addAction(m_quitAction);

    m_helpMenu = menuBar()->addMenu("&Help");
    m_helpMenu->addAction(m_aboutAction);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createWidgets:
//
// Create widgets to put in the layout.
//
void MainWindow::createWidgets() {
    // initialize splitters
    m_mainSplit  = new QSplitter(this);
    m_leftSplit  = new QSplitter(Qt::Vertical, m_mainSplit);
    m_rightSplit = new QSplitter(Qt::Vertical, m_mainSplit);

    // init labels on left side of main splitter
    for(int i=0; i<2; i++) {
        m_labelSide[i] = new QLabel(QString("Label%1").arg(i));
        m_labelSide[i]->setAlignment(Qt::AlignCenter);
    }

    // initialize label on right side of main splitter
    for(int i=0; i<3; i++) {
        // make label widget with centered text and sunken panels
        m_label[i] = new QLabel;
        m_label[i]->setAlignment(Qt::AlignCenter);
        m_label[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }

    // initialize label text
    m_label[0]->setText("<b>Genre<\b>" );
    m_label[1]->setText("<b>Artist<\b>");
    m_label[2]->setText("<b>Album<\b>" );

    // initialize list widgets: genre, artist, album
    for(int i=0; i<3; i++)
        m_panel[i] = new QListWidget;

    // initialize table widget: complete song data
    m_table = new QTableWidget(0, COLS);
    QHeaderView *header = new QHeaderView(Qt::Horizontal,m_table);
    m_table->setHorizontalHeader(header);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setHorizontalHeaderLabels(QStringList() <<
                                       "Name" << "Track" << "Time" << "Artist" << "Album" << "Genre");
    m_table->horizontalHeader()->setSectionsClickable(true);
    m_table->setAlternatingRowColors(1);
    m_table->setShowGrid(1);
    m_table->setEditTriggers (QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

    // initialize variable for sorting in table
    m_ascendSorted = false;

    // initialize labels and sliders for current song playing
    m_albumLabel = new QLabel(QString("Album Cover"));
    m_albumLabel->setAlignment(Qt::AlignCenter);
    m_positionSlider = new QSlider(Qt::Horizontal, this);
    m_positionSlider->setEnabled(false);
    m_positionSlider->setToolTip(tr("Seek"));
    m_positionLabel = new QLabel(QString("0:00"));
    m_infoLabel = new QLabel(QString("Song Info"));
    m_infoLabel->setMaximumHeight(15);
    m_infoLabel->setAlignment(Qt::AlignHCenter);

    // initialize tool buttons and slider dealing with playing songs
    m_stop = new QToolButton(this);
    m_play = new QToolButton(this);
    m_next2 = new QToolButton(this);
    m_previous2 = new QToolButton(this);
    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0,100);
    m_volumeSlider->setSliderPosition(m_device->volume());
    m_muteButton = new QToolButton(this);

    m_shuffle = new QToolButton(this);
    m_shuffle->setCheckable(true);
    m_repeat = new QToolButton(this);
    m_repeat->setCheckable(true);

    //setting the icons for tool buttons
    m_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_next2->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_previous2->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    m_shuffle->setText("Shuffle");
    m_repeat->setText("Repeat");

    // initialize widgets for searching through table items
    m_typeSearch = new QLineEdit();
    m_typeSearch->setMinimumWidth(70);
    m_search = new QComboBox(this);
    m_search->addItem("Search");
    m_search->addItem("Song Title");
    m_search->addItem("Artist");
    m_search->addItem("Album");

    // create gl widgets (visualizer and cover flow)
    m_glWidget= new glWidget();
    m_visualizer = new glVisualizer();
    m_glWidget->update();

    // initialize buttons for gl widgets
    m_next = new QToolButton(this);
    m_next->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_previous = new QToolButton(this);
    m_previous->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_toggleColor = new QPushButton(this);
    m_toggleColor->setText("Toggle Color");

    // create shortcut keys for animation in glWidget
    m_leftMoveAction = new QAction(this);
    m_leftMoveAction->setShortcut(Qt::Key_Left);
    connect(m_leftMoveAction, SIGNAL(triggered()), this, SLOT(s_animateLeft()));
    this->addAction(m_leftMoveAction);

    m_rightMoveAction = new QAction(this);
    m_rightMoveAction->setShortcut(Qt::Key_Right);
    connect(m_rightMoveAction, SIGNAL(triggered()), this, SLOT(s_animateRight()));
    this->addAction(m_rightMoveAction);

    // initialize signal/slot connections dealing with table widget
    connect(m_panel[0],	SIGNAL(itemClicked(QListWidgetItem*)),
            this,		  SLOT(s_panel1   (QListWidgetItem*)));
    connect(m_panel[1],	SIGNAL(itemClicked(QListWidgetItem*)),
            this,		  SLOT(s_panel2   (QListWidgetItem*)));
    connect(m_panel[2],	SIGNAL(itemClicked(QListWidgetItem*)),
            this,		  SLOT(s_panel3   (QListWidgetItem*)));
    connect(m_table,	SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
            this,		  SLOT(s_play()));
    connect(header, SIGNAL(sectionDoubleClicked(int)),
            this, SLOT(s_sortTable(int)));
    connect(m_search, SIGNAL(activated(int)),
            this, SLOT(s_search(int)));

    // initialize signal/slot connections for audio buttons
    connect(m_stop,     SIGNAL(clicked()),
            this,       SLOT(s_stop()));
    connect(m_play,     SIGNAL(clicked()),
            this,       SLOT(s_play2()));
    connect(m_next2,     SIGNAL(clicked()),
            this,       SLOT(s_next()));
    connect(m_previous2, SIGNAL(clicked()),
            this,       SLOT(s_prev()));

    // initialize signal/slot connections for current song playing
    connect(m_device, SIGNAL(stateChanged(QMediaPlayer::State)),
            this, SLOT(s_mediaStateChanged(QMediaPlayer::State)));
    connect(m_device, SIGNAL(positionChanged(qint64)),
            this, SLOT(s_updatePosition(qint64)));
    connect(m_device, SIGNAL(durationChanged(qint64)),
            this, SLOT(s_updateDuration(qint64)));
    connect(m_positionSlider, SIGNAL(valueChanged(int)),
            this, SLOT(s_setPosition(int)));

    connect(m_volumeSlider, SIGNAL(valueChanged(int)),
            m_device, SLOT(setVolume(int)));
    connect(m_muteButton, SIGNAL(clicked()),
            this, SLOT(s_toggleMute()));

    // initialize signal/slot connections dealing with gl widget
    connect(m_toggleColor, SIGNAL(clicked()),
            m_visualizer, SLOT(s_toggleVisualizerColor()));
    connect(m_previous, SIGNAL(clicked()),
            this,       SLOT(s_animateLeft()));
    connect(m_next, SIGNAL(clicked()),
            this,       SLOT(s_animateRight()));

    connect(m_shuffle, SIGNAL(clicked()),
            this, SLOT(s_shuffle()));
    connect(m_repeat, SIGNAL(clicked()),
            this, SLOT(s_repeat()));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createLayouts:
//
// Create layouts for widgets.
//
void MainWindow::createLayouts() {
    // creates a horizontal layout for coverflow tab
    QWidget *cwidget = new QWidget(this);
    QHBoxLayout *chbox = new QHBoxLayout(cwidget);
    chbox->addWidget(m_previous);
    chbox->addWidget(m_glWidget);
    chbox->addWidget(m_next);

    // initialize tab widget
    // adds tabs to widget of coverflow tab layout and visualizer
    m_tabWidget = new QTabWidget();
    m_tabWidget->addTab(cwidget, "Cover Flow");
    m_tabWidget->addTab(m_visualizer, "Visualizer");
    m_tabWidget->setMinimumSize(QApplication::desktop()->width()/3,QApplication::desktop()->height()/3.5f);

    // creates a horizontal layout for search label and widget
    QWidget *swidget = new QWidget(this);
    QHBoxLayout *shbox = new QHBoxLayout(swidget);
    shbox->addWidget(m_typeSearch);
    shbox->addWidget(m_search);
    swidget->setFixedHeight(40);

    // create a one column grid layout,
    // first row consists the album picture of the currently played song
    // next rows consist of labels and list widgets, respectively.
    QWidget	    *gwidget = new QWidget(this);
    QGridLayout *grid   = new QGridLayout(gwidget);
    grid->addWidget(m_albumLabel, 0, 0);
    grid->addWidget(m_label[0], 1, 0);
    grid->addWidget(m_label[1], 3, 0);
    grid->addWidget(m_label[2], 5, 0);
    grid->addWidget(m_panel[0], 2, 0);
    grid->addWidget(m_panel[1], 4, 0);
    grid->addWidget(m_panel[2], 6, 0);

    // create two horizontal layout for labels and widgets associated with playing songs
    QWidget *pwidget = new QWidget(this);
    QHBoxLayout *phbox = new QHBoxLayout(pwidget);
    phbox->addWidget(m_positionSlider);
    phbox->addWidget(m_positionLabel);
    phbox->addWidget(m_shuffle);
    phbox->addWidget(m_repeat);

    QWidget *pwidget2 = new QWidget(this);
    QHBoxLayout *phbox2 = new QHBoxLayout(pwidget2);
    phbox2->addWidget(m_previous2);
    phbox2->addWidget(m_stop);
    phbox2->addWidget(m_play);
    phbox2->addWidget(m_next2);
    phbox2->addStretch();
    phbox2->addWidget(m_muteButton);
    phbox2->addWidget(m_volumeSlider);
    phbox2->addStretch();
    phbox2->addWidget(m_toggleColor);

    // creates a vertical layout
    // contains song title of current song played,
    // pwidget, and tab widget
    QWidget *allwidget = new QWidget(this);
    QVBoxLayout *allvbox = new QVBoxLayout(allwidget);
    allvbox->addWidget(m_infoLabel);
    allvbox->addWidget(pwidget);
    allvbox->addWidget(pwidget2);
    allvbox->addWidget(m_tabWidget);

    // add widgets to splitters
    m_leftSplit ->addWidget(swidget);
    m_leftSplit ->addWidget(gwidget);
    m_rightSplit->addWidget(allwidget);
    m_rightSplit->addWidget(m_table);

    // set main splitter sizes
    setSizes(m_mainSplit, (int)(width ()*.2), (int)(width ()*.8));
    setSizes(m_leftSplit, (int)(height()*.5), (int)(height()*.5));
    setSizes(m_rightSplit,(int)(height()*.4), (int)(height()*.6));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::initLists:
//
// Populate lists with data (first time).
//
void MainWindow::initLists() {
    // error checking
    if(m_listSongs.isEmpty()) return;

    // create separate lists for genres, artists, and albums
    m_listGenre << QString("ALL");
    for(int i=0; i<m_listSongs.size(); i++) {
        m_listGenre << m_listSongs[i][GENRE];
        m_listArtist << m_listSongs[i][ARTIST];
        m_listAlbum << m_listSongs[i][ALBUM];
    }

    // sort each list
    qStableSort(m_listGenre .begin()+1, m_listGenre .end(), caseInsensitive);
    qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
    qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

    // add each list to list widgets, filtering out repeated strings
    for(int i=0; i<m_listGenre.size(); i+=m_listGenre.count(m_listGenre[i]))
        m_panel[0]->addItem(m_listGenre [i]);
    // do same for m_listArtist and m_listAlbum to populate m_panel[1] and m_panel[2]
    for(int i=0; i<m_listArtist.size(); i+=m_listArtist.count(m_listArtist[i]))
        m_panel[1]->addItem(m_listArtist[i]);
    for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
        m_panel[2]->addItem(m_listAlbum[i]);

    // copy data to table widget
    QTableWidgetItem *item[COLS];
    for(int i=0; i<m_listSongs.size(); i++) {
        m_table->insertRow(i);
        for(int j=0; j<COLS; j++) {
            item[j] = new QTableWidgetItem;
            item[j]->setText(m_listSongs[i][j]);
            item[j]->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(i, j, item[j]);
        }
        m_playlist->addMedia(QUrl::fromLocalFile(m_listSongs[i][PATH]));
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::redrawLists:
//
// Re-populate lists with data matching item's text in field x.
//
void MainWindow::redrawLists(QListWidgetItem *listItem, int x) {
    m_table->setRowCount(0);

    // copy data to table widget
    for(int i=0,row=0; i<m_listSongs.size(); i++) {
        // skip rows whose field doesn't match text
        if(m_listSongs[i][x] != listItem->text()) continue;

        m_table->insertRow(row);
        QTableWidgetItem *item[COLS];
        for(int j=0; j<COLS; j++) {
            item[j] = new QTableWidgetItem;
            item[j]->setText(m_listSongs[i][j]);
            item[j]->setTextAlignment(Qt::AlignCenter);
            // put item[j] into m_table in proper row and column j
            m_table->setItem(row, j, item[j]);
        }

        // increment table row index (row <= i)
        row++;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::traverseDirs:
//
// Traverse all subdirectories and collect filenames into m_listSongs.
//
void MainWindow::traverseDirs(QString path) {

    QString			key, val;
    QStringList		list;

    // init listDirs with subdirectories of path
    QDir dir(path);
    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    QFileInfoList listDirs = dir.entryInfoList();

    // init listFiles with all *.mp3 files in path
    QDir files(path);
    files.setFilter(QDir::Files);
    files.setNameFilters(QStringList("*.mp3"));
    QFileInfoList listFiles = files.entryInfoList();

    for(int i=0; i<listFiles.size(); i++) {
        // init list with default values: ""
        for(int j=0; j<=COLS; j++)
            list.insert(j, "");

        // store file pathname into 0th position in list
        QFileInfo fileInfo = listFiles.at(i);
        list.replace(PATH, fileInfo.filePath());

        // convert it from QString to Ascii and store in source using TabLib
        TagLib::FileRef source(QFile::encodeName(fileInfo.filePath()).constData());

        // process all song tags
        // store tag value in proper position in list
        if(!source.isNull()&& source.tag()) {

            // gets tag key
            TagLib::Tag *tag=source.tag();
            if(tag->genre()!="")
                list.replace(GENRE,TStringToQString(tag->genre()));
            if(tag->artist()!="")
                list.replace(ARTIST,TStringToQString(tag->artist()));
            if(tag->album()!="")
                list.replace(ALBUM,TStringToQString(tag->album()));
            if(tag->title()!="")
                list.replace(TITLE,TStringToQString(tag->title()));

            list.replace(TRACK ,QString("%1").arg(tag->track()));

            // gets the length of the source's audio properties and stores length with time format
            if(source.audioProperties()) {
                int seconds=source.audioProperties()->length()%60;
                int minutes=source.audioProperties()->length()/60;

                if(seconds<10)
                    list.replace(TIME,QString("%1:0%2").arg(minutes).arg(seconds));
                else
                    list.replace(TIME,QString("%1:%2").arg(minutes).arg(seconds));
            }
        }

        // append list (song data) into songlist m_listSongs;
        // uninitialized fields are empty strings
        m_listSongs << list;
    }

    // base case: no more subdirectories
    if(listDirs.size() == 0) return;

    // recursively descend through all subdirectories
    for(int i=0; i<listDirs.size(); i++) {
        QFileInfo fileInfo = listDirs.at(i);
        traverseDirs( fileInfo.filePath() );
    }
    return;
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::setSizes:
//
// Set splitter sizes.
//
void MainWindow::setSizes(QSplitter *splitter, int size1, int size2) {
    QList<int> sizes;
    sizes.append(size1);
    sizes.append(size2);
    splitter->setSizes(sizes);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_load:
//
// Slot function for File|Load
//
void MainWindow::s_load() {
    // open a file dialog box
    QFileDialog *fd = new QFileDialog;

    fd->setFileMode(QFileDialog::Directory);
    QString s = fd->getExistingDirectory(0, "Select Folder", m_directory,
             QFileDialog::ShowDirsOnly |
             QFileDialog::DontResolveSymlinks);

    // check if cancel was selected
    if(s == NULL) return;

    // copy full pathname of selected directory into m_directory
    m_directory = s;

    traverseDirs(m_directory);
    saveDir(m_directory);
    initLists();
    initAlbums();
    m_glWidget->loadImages(m_albumsList);

    //initializes the album cover with default image musicnote.png
    QImage temp;
    temp.load("C:/Users/Amy/Documents/Spring 2015/CSC221/v2/musicnote.png");
    m_cover = temp.scaled(100, 100, Qt::KeepAspectRatio);
    m_albumLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_albumLabel->setPixmap(QPixmap::fromImage(m_cover));

    s_mediaStateChanged(m_device->state());
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel1:
//
// Slot function to adjust data if an item in panel1 (genre) is selected.
//
void MainWindow::s_panel1(QListWidgetItem *item) {
    // replaces the table with original items loaded
    if(item->text() == "ALL") {
        m_panel[1] ->clear();
        m_panel[2] ->clear();
        m_panel[0] ->clear();
        m_table->setRowCount(0);
        initLists();
        return;
    }

    // clear lists
    m_panel[1] ->clear();
    m_panel[2] ->clear();
    m_listArtist.clear();
    m_listAlbum .clear();

    // collect list of artists and albums
    for(int i=0; i<m_listSongs.size();i++) {
        if(m_listSongs[i][GENRE]==item->text()) {
            m_listAlbum<<m_listSongs[i][ALBUM];
            m_listArtist<<m_listSongs[i][ARTIST];
        }
    }

    // sort remaining two panels for artists and albums
    qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
    qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

    // add items to panels; skip over non-unique entries
    for(int i=0; i<m_listArtist.size(); i+=m_listArtist.count(m_listArtist[i]))
        m_panel[1]->addItem(m_listArtist[i]);
    for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
        m_panel[2]->addItem(m_listAlbum[i]);
    redrawLists(item, GENRE);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel2:
//
// Slot function to adjust data if an item in panel2 (artist) is selected.
//
void MainWindow::s_panel2(QListWidgetItem *item) {
    // clear lists
    m_panel[2]->clear();
    m_listAlbum.clear();

    // collect list of albums
    for(int i=0; i<m_listSongs.size(); i++) {
        if(m_listSongs[i][ARTIST] == item->text())
            m_listAlbum << m_listSongs[i][ALBUM];
    }

    // sort remaining panel for albums
    qStableSort(m_listAlbum.begin(), m_listAlbum.end(), caseInsensitive);

    // add items to panel; skip over non-unique entries
    for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
        m_panel[2]->addItem(m_listAlbum[i]);

    redrawLists(item, ARTIST);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel3:
//
// Slot function to adjust data if an item in panel3 (album) is selected.
//
void MainWindow::s_panel3(QListWidgetItem *item) {
    redrawLists(item, ALBUM);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_about:
//
// Slot function for Help|About
//
void MainWindow::s_about() {
    QMessageBox::about(this, "About qTunes",
            "<center> qTunes 1.0 </center> \
             <center> by George Wolberg, 2015 </center>");
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::initAlbums():
//
// Places all the album covers into a list of qimages.
//
void MainWindow::initAlbums() {
    // created a list of all the albums
    QStringList albumSongOrder;
    for(int i=0; i<m_listSongs.size(); i++)
        albumSongOrder << m_listSongs[i][ALBUM];

    // add each qimage to the list of qimages, filtering out repeated strings
    for (int k=0; k<m_listAlbum.size(); k+=m_listAlbum.count(m_listAlbum[k])) {
        // gets file data
        int index = albumSongOrder.indexOf(m_listAlbum[k]);
        QString item_title = QString("%1").arg(m_listSongs[index][PATH]);
        QByteArray ba_temp = item_title.toLocal8Bit();
        const char* filepath = ba_temp.data();

        // gets the tag that contains the album image
        TagLib::MPEG::File audioFile(filepath);
        TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true);

        // adds qimage created from the tag's data to the list
        QImage coverArt = initImage(tag);
        m_albumsList << coverArt;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::initImage:
//
// Returns a qimage loaded from the tag's data
// or a default image if the tag does not contain the data.
//
QImage MainWindow::initImage(TagLib::ID3v2::Tag *tag) {
    // looks for picture frames only
    TagLib::ID3v2::FrameList tag_list = tag->frameList("APIC");
    QImage tag_image;

    // if picture frames do not exists, a default image is used for the album cover image
    // else the first frame's data is coverted to a qimage
    if(tag_list.isEmpty()) {
        QString path = QDir::currentPath();
        path.append("musicnote.png");
        tag_image.load(path);
    }
    else {
        TagLib::ID3v2::AttachedPictureFrame *tag_frame = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(tag_list.front());
        tag_image.loadFromData((const uchar *) tag_frame->picture().data(), tag_frame->picture().size());
    }
    return tag_image;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::updateSong:
//
// Displays the album cover and song title of the currently played song.
//
void MainWindow::updateSong() {
    // sets label to the current song's title
    m_infoLabel->setText(m_listSongs[m_playlist->currentIndex()][TITLE] );

    // gets file data
    QString item_title = QString("%1").arg(m_listSongs[m_playlist->currentIndex()][PATH]);
    QByteArray ba_temp = item_title.toLocal8Bit();
    const char* filepath = ba_temp.data();

    // gets the tag that contains the album image
    TagLib::MPEG::File audioFile(filepath);
    TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true);
    QImage coverArt = initImage(tag);

    // scales and positions the image on the label
    m_cover = coverArt.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_albumLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_albumLabel->setPixmap(QPixmap::fromImage(m_cover));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_play:
//
// Slot function for playing a mp3 file by double clicking
// on its corresponding item in table widget.
//
void MainWindow::s_play() {
    if(m_device->error()) return;

    QTableWidgetItem* item = m_table->selectedItems().count() > 0 ? m_table->selectedItems()[0] : NULL;
    if(item) {
        for(int i=0; i<m_listSongs.size(); i++) {
            // skip over songs whose title does not match
            if(m_listSongs[i][TITLE] == item->text()) {
                m_playlist->setCurrentIndex(i);
                m_device->play();
                updateSong();
                i = m_listSongs.size();
            }
        }
    }
    else {
        if(m_listSongs.size()) {
            m_playlist->setCurrentIndex(0);
            m_device->play();
            updateSong();
        }
    }
    updateSong();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_play2():
//
// Slot function for playing a mp3 file using play button.
//
void MainWindow::s_play2() {
    if(m_device->error()) return;

    if(m_device->state()==QMediaPlayer::PlayingState)
        m_device->pause();
    else if(m_device->state()==QMediaPlayer::PausedState)
        m_device->play();
    else {
        QTableWidgetItem* item = m_table->selectedItems().count() > 0 ? m_table->selectedItems()[0] : NULL;
        if(item) {
            for(int i=0; i<m_listSongs.size(); i++) {
                // skip over songs whose title does not match
                if(m_listSongs[i][TITLE] == item->text()) {
                    m_playlist->setCurrentIndex(i);
                    m_device->play();
                    updateSong();
                    i = m_listSongs.size();
                }
            }
        }
        else {
            if(m_listSongs.size()) {
                m_playlist->setCurrentIndex(0);
                m_device->play();
                updateSong();
            }
        }
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_stop():
//
// Slot function that stops the song being played.
//
void MainWindow::s_stop() {
    if(m_device->error()) return;

    // if playing or paused
    if(m_device->state()==QMediaPlayer::PlayingState || m_device->state()==QMediaPlayer::PausedState)
        m_device->stop();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_next():
//
// Slot function for playing the next mp3 file on the playlist.
//
void MainWindow::s_next() {
    if(m_next->isEnabled() && m_device->state()!=QMediaPlayer::StoppedState) {
        m_playlist->next();
        updateSong();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_prev():
//
// Slot function for playing the previous mp3 file on the playlist.
//
void MainWindow::s_prev() {
    if(m_previous->isEnabled() && m_device->state() != QMediaPlayer::StoppedState) {
        m_playlist->previous();
        updateSong();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_mediaStateChanged(QMediaPlayer::State):
//
// Check to see if button should be play or pause button.
// Enable/disable stop button depending on current m_device state.
//
void MainWindow::s_mediaStateChanged(QMediaPlayer::State state) {
    if(state==QMediaPlayer::StoppedState) {
        // set buttons
        m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_play->setEnabled(true);
        m_stop->setEnabled(false);
        m_next2->setEnabled(false);
        m_previous2->setEnabled(false);

        // disable visualizer animation
        m_visualizer->setAnimationActive(false);
        // disable position controls
        m_positionSlider->setEnabled(false);

    }else if(state==QMediaPlayer::PlayingState) {
        m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        m_stop->setEnabled(true);
        m_next2->setEnabled(true);
        m_previous2->setEnabled(true);

        m_visualizer->setAnimationActive(true);
        m_positionSlider->setEnabled(true);
    }else {
        m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        m_play->setEnabled(true);
        m_visualizer->setAnimationActive(false);
        m_positionSlider->setEnabled(true);
    }

    // if no songs in list, disable the play button as well
    // all other buttons are already disabled since media is stoped
    if(!m_listSongs.size())
        m_play->setEnabled(false);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_updatePosition:
//
// Slot function correponds the position of the song to the label and slider.
//
void MainWindow::s_updatePosition(qint64 position) {
    m_positionSlider->setValue(position);

    QTime duration(0, position / 60000, qRound((position % 60000) / 1000.0));
    m_positionLabel->setText(duration.toString(tr("mm:ss")));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_updateDuration:
//
// Slot function correponds the position of the song to the slider.
// Allows user to position the point at which the song is being played.
//
void MainWindow::s_updateDuration(qint64 duration) {
    m_positionSlider->setRange(0, duration);
    m_positionSlider->setEnabled(duration > 0);
    m_positionSlider->setPageStep(duration / 10);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow:s_setPosition:
//
// Slot function for setting the point at which the song is being played.
//
void MainWindow::s_setPosition(int position) {
    if (qAbs(m_device->position() - position) > 99)
        m_device->setPosition(position);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_search:
//
// Slot function for searching through table widget.
//
void MainWindow::s_search(int in) {
    // matches in (integer selected from m_search) with corresponding field value
    int index = 0;
    bool search = true;
    if(in == 1)
        index = 0;
    else if (in == 2)
        index = 3;
    else if (in == 3)
        index = 4;
    else
        search = false;

    m_searchText = m_typeSearch->text().toLower();
    m_table->setRowCount(0);

    // copy data to table widget
    for (int i=0, row=0; i<m_listSongs.size(); i++) {
        // skip rows whose field doesn't match text
        if (search&&!(((m_listSongs[i][index]).toLower()).contains(m_searchText))) continue;

        m_table->insertRow(row);
        QTableWidgetItem *item[COLS];
        for (int j=0; j<COLS; j++) {
            item[j] = new QTableWidgetItem;
            item[j]->setText(m_listSongs[i][j]);
            item[j]->setTextAlignment(Qt::AlignCenter);
            // put item[j] into m_table in proper row and column j
            m_table->setItem(row, j, item[j]);
        }
        // increment table row index (row <= i)
        row++;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_sortTable:
//
// Slot function for sorting items in table widget.
//
void MainWindow::s_sortTable(int colNum) {
    // sorts items in descending order
    if (m_ascendSorted) {
        m_table->sortByColumn(colNum, Qt::DescendingOrder);
        m_ascendSorted = false;
    }

    // sorts items in ascending order
    else {
        m_table->sortByColumn(colNum, Qt::AscendingOrder);
        m_ascendSorted = true;
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_animateLeft:
//
// Slot function to animate coverflow to the left.
//
void MainWindow::s_animateLeft() {
    m_glWidget->startAnimate(true);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_animateRight:
//
// Slot function to animate coverflow to the right.
//
void MainWindow::s_animateRight() {
    m_glWidget->startAnimate(false);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::keyPressEvent:
//
// Create shortcut keys for animation in glWidget
//
void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Left) {
        s_animateLeft();
    }
    else if (event->key() == Qt::Key_Right) {
        s_animateRight();
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::saveDir:
//
// Save loaded directory.
//
void MainWindow::saveDir(QString path) {
    QSettings setting(QSettings::NativeFormat, QSettings::UserScope, "CS221", "qTune");
    setting.setValue("dirs",path);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::loadDirs:
//
// Pulls previous directory and opens the prompt window.
//
void MainWindow::loadDirs() {
    QSettings setting(QSettings::NativeFormat, QSettings::UserScope, "CS221", "qTune");
    QString p = setting.value("dirs","").toString();
    QString dir = QDir::toNativeSeparators(p);
    if(dir.isEmpty()) return;
    m_directory = dir;
    openPrompt *prompt = new openPrompt;
    connect(prompt, SIGNAL(load()), this, SLOT(s_loadPrev()));
    prompt->exec();
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_loadPrev:
//
// Slot function to load previous directories.
//
void MainWindow::s_loadPrev() {
    traverseDirs(m_directory);
    initLists();
    initAlbums();
    m_glWidget->loadImages(m_albumsList);
}




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_toggleMute:
//
// Slot function for muting the song being played.
//
void MainWindow::s_toggleMute() {
    m_device->setMuted(!m_device->isMuted());

    if(m_device->isMuted()) {
        m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
        m_volumeSlider->setEnabled(false);
    }else {
        m_muteButton->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
        m_volumeSlider->setEnabled(true);
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_shuffle:
//
// Slot function for shuffling the order of the mp3 files being played.
//
void MainWindow::s_shuffle() {
    m_playlist->setPlaybackMode(m_shuffle->isChecked() ? QMediaPlaylist::Random : QMediaPlaylist::Loop);
    // both repeat and shuffle can't be checked at the same time so uncheck the other
    m_repeat->setChecked(false);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_repeat:
//
// Slot function for repeating the song being played.
//
void MainWindow::s_repeat() {
    m_playlist->setPlaybackMode(m_repeat->isChecked() ? QMediaPlaylist::CurrentItemInLoop : QMediaPlaylist::Loop);
    // both repeat and shuffle can't be checked at the same time so uncheck the other
    m_shuffle->setChecked(false);
}
