///
/// @file gui_main.cpp
///
/// @Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
///

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFont font;
    font.setFamily(font.defaultFamily());
    app.setFont(font);

    MainWindow w;
    w.show();

    return app.exec();
}
