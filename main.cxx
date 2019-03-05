#include "mainwindow.h"
#include <QApplication>

#include <QSurfaceFormat>
#include "QVTKOpenGLWidget.h"

int main(int argc, char *argv[])
{
	//需要确保为VTK渲染创建适当的OpenGL上下文。
	QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());
	
	// QT
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
