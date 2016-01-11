// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2015 by George Wolberg
//
// main.cpp - main() function.
//
// Written by: George Wolberg, 2015
// ======================================================================

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char **argv) {
	// init variables and application font
	QString	      program = argv[0];
	QApplication  app(argc, argv);

	// invoke  MainWindow constructor
	MainWindow window(program);

	// display MainWindow
	window.show();

	return app.exec();
}
