#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageMarchingCubes.h>
#include <vtkPolyData.h>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
	
private slots:
	void closeThis();
	void input();
	void exportInf();
	void anatomyView();
	void exportSTL();
	void modelMaker_marchingCubes();
	void model_autoRepair();

private:
    Ui::MainWindow *ui;
	vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	vtkSmartPointer<vtkImageMarchingCubes> poly_cube = vtkSmartPointer<vtkImageMarchingCubes>::New();
	vtkSmartPointer<vtkImageMarchingCubes> poly_repair = vtkSmartPointer<vtkImageMarchingCubes>::New();
	int dim[3];
};

#endif // MAINWINDOW_H
