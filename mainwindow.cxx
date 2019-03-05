#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDataObjectToTable.h>
#include <vtkElevationFilter.h>
#include "vtkGenericOpenGLRenderWindow.h"
#include <vtkPolyDataMapper.h>
#include <vtkQtTableView.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkSphereSource.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>

#include <vtkImageData.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageMarchingCubes.h>
#include <vtkSTLWriter.h>
#include <vtkActor.h>

#include <vtkImageViewer2.h>

#include <QFileDialog>
#include <QDir>
#include <QString.h>

#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <sstream>


#include"vtkCommand.h"
#include"vtkSliderWidget.h"
#include"vtkSliderRepresentation2D.h"
#include"vtkProperty.h"

//test
#include <vtkFillHolesFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkConnectivityFilter.h>

#include <vtkDataSetMapper.h>
#include <vtkProperty.h>

#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

#include <vtkCellIterator.h>
#include <vtkGenericCell.h>

#include <vtkNamedColors.h>

#include <vtkPolyDataConnectivityFilter.h>

#include <vtkQuadricDecimation.h>
#include <vtkDecimatePro.h>

//切片状态信息
class StatusMessage {
public:
   static std::string Format(int slice, int maxSlice) {
      std::stringstream tmp;
      tmp << " Slice Number " << slice + 1 << "/" << maxSlice + 1;
      return tmp.str();
   }
};

//切片交互方式
class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
public:
   static myVtkInteractorStyleImage* New();
   vtkTypeMacro(myVtkInteractorStyleImage, vtkInteractorStyleImage);

protected:
   vtkImageViewer2* _ImageViewer;
   vtkTextMapper* _StatusMapper;
   int _Slice;
   int _MinSlice;
   int _MaxSlice;

public:
   void SetImageViewer(vtkImageViewer2* imageViewer) {
      _ImageViewer = imageViewer;
      _MinSlice = imageViewer->GetSliceMin();
      _MaxSlice = imageViewer->GetSliceMax();
      _Slice = _MaxSlice/2;
      cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;
   }

   void SetStatusMapper(vtkTextMapper* statusMapper) {
      _StatusMapper = statusMapper;
   }


protected:
   void MoveSliceForward() {
      if(_Slice < _MaxSlice) {
         _Slice += 1;
         cout << "MoveSliceForward::Slice = " << _Slice << std::endl;
         _ImageViewer->SetSlice(_Slice);
         std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
         _StatusMapper->SetInput(msg.c_str());
         _ImageViewer->Render();
      }
   }

   void MoveSliceBackward() {
      if(_Slice > _MinSlice) {
         _Slice -= 1;
         cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
         _ImageViewer->SetSlice(_Slice);
         std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
         _StatusMapper->SetInput(msg.c_str());
         _ImageViewer->Render();
      }
   }


   virtual void OnKeyDown() {
      std::string key = this->GetInteractor()->GetKeySym();
      if(key.compare("Up") == 0) {
         //cout << "Up arrow key was pressed." << endl;
         MoveSliceForward();
      }
      else if(key.compare("Down") == 0) {
         //cout << "Down arrow key was pressed." << endl;
         MoveSliceBackward();
      }
      // forward event
      vtkInteractorStyleImage::OnKeyDown();
   }


   virtual void OnMouseWheelForward() {
      //std::cout << "Scrolled mouse wheel forward." << std::endl;
      MoveSliceForward();
      // don't forward events, otherwise the image will be zoomed 
      // in case another interactorstyle is used (e.g. trackballstyle, ...)
      // vtkInteractorStyleImage::OnMouseWheelForward();
   }


   virtual void OnMouseWheelBackward() {
      //std::cout << "Scrolled mouse wheel backward." << std::endl;
      if(_Slice > _MinSlice) {
         MoveSliceBackward();
      }
      // don't forward events, otherwise the image will be zoomed 
      // in case another interactorstyle is used (e.g. trackballstyle, ...)
      // vtkInteractorStyleImage::OnMouseWheelBackward();
   }
};

vtkStandardNewMacro(myVtkInteractorStyleImage);

/* //使用滑块选择切片..........
class sliceSliderCallback:: public vtkCommand
{
public
	static sliceSliderCallback *New()
	{
		return new sliceSliderCallback;
	}
	void Execute(vtkObject *caller, unsigned long, void *)
	{
		vtkSliderWidget *sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
		this->sliderer->SetsliSlice(value);
	}
	sliceSliderCallback():sliderer(nullptr) {}	
} */

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	
	//cout << "begin" << endl;
	
	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowLeft;
	this->ui->qvtkWidgetLeft->SetRenderWindow(renderWindowLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowRight;
	this->ui->qvtkWidgetRight->SetRenderWindow(renderWindowRight);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowDLeft;
	this->ui->qvtkWidgetDLeft->SetRenderWindow(renderWindowDLeft);

	vtkNew<vtkGenericOpenGLRenderWindow> renderWindowDRight;
	this->ui->qvtkWidgetDRight->SetRenderWindow(renderWindowDRight);
	
	connect(this->ui->action_close,SIGNAL(triggered()), this, SLOT(closeThis()));
	
	connect(this->ui->action_input, SIGNAL(triggered()), this, SLOT(input()));
	
	connect(this->ui->action_exportInf, SIGNAL(triggered()),this, SLOT(exportInf()));
	
	connect(this->ui->action_anatomyView, SIGNAL(triggered()),this, SLOT(anatomyView()));
	
	connect(this->ui->action_exportSTL, SIGNAL(triggered()),this, SLOT(exportSTL()));
	
	connect(this->ui->MarchingCubes, SIGNAL(triggered()), this, SLOT(modelMaker_marchingCubes()));
	
	connect(this->ui->DividingCubes, SIGNAL(triggered()),this, SLOT(model_autoRepair()));
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::closeThis()
{
	this->close();
}


void MainWindow::input()
{
	// read DICOM
	QDir dir;
	QString file_path = QFileDialog::getExistingDirectory( this, "请选择DICOM文件所在文件夹");
	if ( file_path.isEmpty() == true ) return;
	
	// 支持带中文路径的读取
	QByteArray ba = file_path.toLocal8Bit();
	const char *file_path_str = ba.data();
	
	QString file_path_str_q;
	file_path_str_q = file_path_str;
	ui->text_inf->append("Choose File:\n" + file_path_str_q);
	
	vtkSmartPointer<vtkDICOMImageReader> DICOMreader = vtkSmartPointer<vtkDICOMImageReader>::New();
	DICOMreader->SetDirectoryName(file_path_str);//DICOM所在文件夹
	DICOMreader->Update();
	
	this->reader = DICOMreader;
}


void MainWindow::exportInf()
{
	vtkImageData* readerImg = nullptr;
	readerImg = this->reader->GetOutput(); 
	
	//确认读取到文件并输出文件信息
	readerImg->GetDimensions(this->dim);
	if ( dim[0] > 2 || dim[1] > 2 || dim[2] > 2 )
		{
			
			QString dim0 = QString::number(dim[0]);
			QString dim1 = QString::number(dim[1]);
			QString dim2 = QString::number(dim[2]);
			QString space = " ";
			ui->text_inf->append("Dimensions:" + space + dim0 + space + dim1 + space + dim2);
			
			QString fileExtensions = this->reader-> GetFileExtensions();
			ui->text_inf->append("fileExtensions: " + fileExtensions);
			
			QString descriptiveName = this->reader->GetDescriptiveName();
			ui->text_inf->append("descriptiveName: " + descriptiveName);
			
			double* pixelSpacing = this->reader->GetPixelSpacing();
			QString pixelS = QString::number(*pixelSpacing,10,5);
			ui->text_inf->append("pixelSpacing: " + pixelS);
			
			int width = this->reader->GetWidth();
			QString wid = QString::number(width);
			ui->text_inf->append("width: " + wid);
			
			int height = this->reader->GetHeight();
			QString heig = QString::number(height);
			ui->text_inf->append("height: " + heig);
			
			float* imagePositionPatient = this->reader->GetImagePositionPatient();
			QString imPP = QString::number(*imagePositionPatient,10,5);
			ui->text_inf->append("imagePositionPatient: " + imPP);
			
			float* imageOrientationPatient = this->reader->GetImageOrientationPatient();
			QString imOP = QString::number(*imageOrientationPatient,10,5);
			ui->text_inf->append("imageOrientationPatient: " + imOP);
			
			int bitsAllocated = this->reader->GetBitsAllocated();
			QString bitsA = QString::number(bitsAllocated);
			ui->text_inf->append("bitsAllocated: " + bitsA);
			
			int pixelRepresentation = this->reader->GetPixelRepresentation();
			QString pixelR = QString::number(pixelRepresentation);
			ui->text_inf->append("pixelRepresentation: " + pixelR);
			
			int numberOfComponents = this->reader->GetNumberOfComponents();
			QString numberO = QString::number(numberOfComponents);
			ui->text_inf->append("numberOfComponents: " + numberO);

			QString transferSyntaxUID = this->reader->GetTransferSyntaxUID();
			ui->text_inf->append("transferSyntaxUID: " + transferSyntaxUID);

			float rescaleSlope = this->reader->GetRescaleSlope();
			QString rescaleS = QString::number(rescaleSlope, 10, 5);
			ui->text_inf->append("rescaleSlope: " + rescaleS);
		 
			float rescaleOffset = this->reader->GetRescaleOffset();
			QString rescaleO = QString::number(rescaleOffset, 10, 5);
			ui->text_inf->append("rescaleOffset: " + rescaleO);

			QString patientName = this->reader->GetPatientName();
			ui->text_inf->append("patientName: " + patientName);

			QString studyUID = this->reader->GetStudyUID();
			ui->text_inf->append("studyUID: " + studyUID);

			QString studyID = this->reader->GetStudyID();
			ui->text_inf->append("studyID: " + studyID);
			
			float gantryAngle = this->reader->GetGantryAngle();
			QString gantryA = QString::number(gantryAngle, 10, 5);
			ui->text_inf->append("gantryAngle: " + gantryA);
		}	
}

void MainWindow::anatomyView()
{
	//横断面、矢状面、冠状面视图
	//用vtkImageViewer2获取数据各个方向的切片
	//横断面面视图
	vtkSmartPointer<vtkImageViewer2> imageViewerLeft =
	vtkSmartPointer<vtkImageViewer2>::New();
	imageViewerLeft->SetInputConnection(this->reader->GetOutputPort());
	imageViewerLeft->SetSliceOrientationToXY();	
	imageViewerLeft->SetSlice(dim[2]/2);
	imageViewerLeft->SetRenderWindow(ui->qvtkWidgetLeft->GetRenderWindow());
	//矢状面视图
	vtkSmartPointer<vtkImageViewer2> imageViewerRight =
	vtkSmartPointer<vtkImageViewer2>::New();
	imageViewerRight->SetInputConnection(this->reader->GetOutputPort());
	imageViewerRight->SetSliceOrientationToXZ();
	imageViewerRight->SetSlice(dim[0]/2);
	imageViewerRight->SetRenderWindow(ui->qvtkWidgetRight->GetRenderWindow());	
	//冠状面视图
	vtkSmartPointer<vtkImageViewer2> imageViewerDLeft =
	vtkSmartPointer<vtkImageViewer2>::New();
	imageViewerDLeft->SetInputConnection(this->reader->GetOutputPort());
	imageViewerDLeft->SetSlice(dim[1]/2);
	imageViewerDLeft->SetSliceOrientationToYZ();
	imageViewerDLeft->SetRenderWindow(ui->qvtkWidgetDLeft->GetRenderWindow());
	
	//横断面当前切片数
	vtkSmartPointer<vtkTextProperty> sliceTextProp1 = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp1->SetFontFamilyToCourier();
	sliceTextProp1->SetFontSize(15);
	sliceTextProp1->SetVerticalJustificationToBottom();
	sliceTextProp1->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> sliceTextMapper1 = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg1 = StatusMessage::Format(dim[2]/2, imageViewerLeft->GetSliceMax());
	sliceTextMapper1->SetInput(msg1.c_str());
	sliceTextMapper1->SetTextProperty(sliceTextProp1);

	vtkSmartPointer<vtkActor2D> sliceTextActor1 = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor1->SetMapper(sliceTextMapper1);
	sliceTextActor1->SetPosition(90, 10);
	
	//冠状面当前切片数
	vtkSmartPointer<vtkTextProperty> sliceTextProp2 = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp2->SetFontFamilyToCourier();
	sliceTextProp2->SetFontSize(15);
	sliceTextProp2->SetVerticalJustificationToBottom();
	sliceTextProp2->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> sliceTextMapper2 = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg2 = StatusMessage::Format(dim[0]/2, imageViewerRight->GetSliceMax());
	sliceTextMapper2->SetInput(msg2.c_str());
	sliceTextMapper2->SetTextProperty(sliceTextProp2);

	vtkSmartPointer<vtkActor2D> sliceTextActor2 = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor2->SetMapper(sliceTextMapper2);
	sliceTextActor2->SetPosition(90, 10);

	//矢状面当前切片数
	vtkSmartPointer<vtkTextProperty> sliceTextProp3 = vtkSmartPointer<vtkTextProperty>::New();
	sliceTextProp3->SetFontFamilyToCourier();
	sliceTextProp3->SetFontSize(15);
	sliceTextProp3->SetVerticalJustificationToBottom();
	sliceTextProp3->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> sliceTextMapper3 = vtkSmartPointer<vtkTextMapper>::New();
	std::string msg3 = StatusMessage::Format(dim[1]/2, imageViewerDLeft->GetSliceMax());
	sliceTextMapper3->SetInput(msg3.c_str());
	sliceTextMapper3->SetTextProperty(sliceTextProp3);

	vtkSmartPointer<vtkActor2D> sliceTextActor3 = vtkSmartPointer<vtkActor2D>::New();
	sliceTextActor3->SetMapper(sliceTextMapper3);
	sliceTextActor3->SetPosition(90, 10);
	
	//提示文本
	vtkSmartPointer<vtkTextProperty> usageTextProp = vtkSmartPointer<vtkTextProperty>::New();
	usageTextProp->SetFontFamilyToCourier();
	usageTextProp->SetFontSize(14);
	usageTextProp->SetVerticalJustificationToTop();
	usageTextProp->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> usageTextMapper = vtkSmartPointer<vtkTextMapper>::New();
	usageTextMapper->SetInput("- Slice with mouse wheel or Up/Down-Key\n- Zoom with pressed right mouse button\n  while dragging\n- adjust the window width with pressed\n  left mouse button while dragging");
	usageTextMapper->SetTextProperty(usageTextProp);

	vtkSmartPointer<vtkActor2D> usageTextActor = vtkSmartPointer<vtkActor2D>::New();
	usageTextActor->SetMapper(usageTextMapper);
	usageTextActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
	usageTextActor->GetPositionCoordinate()->SetValue( 0.05, 0.95);

	//切片方向文本
	vtkSmartPointer<vtkTextProperty> transverseTextProp = vtkSmartPointer<vtkTextProperty>::New();
	transverseTextProp->SetFontFamilyToCourier();
	transverseTextProp->SetFontSize(12);
	transverseTextProp->SetVerticalJustificationToBottom();
	transverseTextProp->SetJustificationToLeft();

	vtkSmartPointer<vtkTextMapper> transverseTextMapper = vtkSmartPointer<vtkTextMapper>::New();
	transverseTextMapper->SetInput("transverse");
	transverseTextMapper->SetTextProperty(transverseTextProp);
	transverseTextMapper->GetTextProperty()->SetColor(0.1, 0.1, 0.1);

	vtkSmartPointer<vtkActor2D> transverseTextActor = vtkSmartPointer<vtkActor2D>::New();
	transverseTextActor->SetMapper(transverseTextMapper);
	transverseTextActor->SetPosition(5, 10);
	
	//交互
	//窗口交互器
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor1 =
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor2 =
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor3 =
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//交互风格
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle1 =
	vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle1->SetImageViewer(imageViewerLeft);
	myInteractorStyle1->SetStatusMapper(sliceTextMapper1);

	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle2 =
	vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle2->SetImageViewer(imageViewerRight);
	myInteractorStyle2->SetStatusMapper(sliceTextMapper2);
	
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle3 =
	vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle3->SetImageViewer(imageViewerDLeft);
	myInteractorStyle3->SetStatusMapper(sliceTextMapper3);
	//将交互风格添加到窗口交互器
	imageViewerLeft->SetupInteractor(renderWindowInteractor1);
	renderWindowInteractor1->SetInteractorStyle(myInteractorStyle1);
	imageViewerRight->SetupInteractor(renderWindowInteractor2);
	renderWindowInteractor2->SetInteractorStyle(myInteractorStyle2);
	imageViewerDLeft->SetupInteractor(renderWindowInteractor3);
	renderWindowInteractor3->SetInteractorStyle(myInteractorStyle3);

	//把文本添加到绘制窗口
	imageViewerLeft->GetRenderer()->AddActor2D(sliceTextActor1);
	imageViewerRight->GetRenderer()->AddActor2D(sliceTextActor2);
	imageViewerDLeft->GetRenderer()->AddActor2D(sliceTextActor3);
	imageViewerLeft->GetRenderer()->AddActor2D(usageTextActor);
	imageViewerLeft->GetRenderer()->AddActor2D(transverseTextActor);
	
	//渲染
	imageViewerLeft->Render();
	imageViewerRight->Render();
	imageViewerDLeft->Render();
	renderWindowInteractor1->Start(); 
	renderWindowInteractor2->Start();
	renderWindowInteractor3->Start();
}

void MainWindow::modelMaker_marchingCubes()
{
	//三维重建
/* 	//高斯平滑
	vtkSmartPointer<vtkImageGaussianSmooth> smooth = 
	vtkSmartPointer<vtkImageGaussianSmooth>::New();
	smooth->SetDimensionality(3);
	smooth->SetInputConnection(reader->GetOutputPort());
	smooth->SetStandardDeviations(1.75, 1.75, 0.0);
	smooth->SetRadiusFactor(1);*/
	
	//移动立方体法
	vtkSmartPointer<vtkImageMarchingCubes> mcubes = 
	vtkSmartPointer<vtkImageMarchingCubes>::New();
	mcubes->SetInputConnection(this->reader->GetOutputPort());
	mcubes->SetValue(0, 99);
	
/* 	//连通域着色
	vtkSmartPointer<vtkPolyDataConnectivityFilter>  connectivityFilter  = 
	vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
	connectivityFilter->SetInputConnection(mcubes->GetOutputPort());
	connectivityFilter->SetExtractionModeToAllRegions();
	connectivityFilter->ColorRegionsOn();
	connectivityFilter->Update(); */
	
	//生成actor
	vtkSmartPointer<vtkPolyDataMapper> mcubesMapper =
	vtkSmartPointer<vtkPolyDataMapper>::New();
	mcubesMapper->SetInputConnection(mcubes->GetOutputPort());
	mcubesMapper->SetScalarRange(mcubes->GetOutput()->GetPointData()->GetArray("RegionId")->GetRange());
	mcubesMapper->Update();

	vtkSmartPointer<vtkActor> mcubesActor =
	vtkSmartPointer<vtkActor>::New();
	mcubesActor->SetMapper(mcubesMapper);	
	
	// VTK Renderer
	vtkSmartPointer<vtkRenderer> drightRenderer =
	vtkSmartPointer<vtkRenderer>::New();
	drightRenderer->AddActor(mcubesActor);	
	
	// VTK/Qt wedded
	ui->qvtkWidgetDRight->GetRenderWindow()->AddRenderer(drightRenderer);
	ui->qvtkWidgetDRight->GetRenderWindow()->Render();
	
	this->poly_cube = mcubes;
}

void MainWindow::model_autoRepair()
{
	vtkSmartPointer<vtkNamedColors> colors =
    vtkSmartPointer<vtkNamedColors>::New();
	
	// Fill the holes
	vtkSmartPointer<vtkFillHolesFilter> fillHoles =
	vtkSmartPointer<vtkFillHolesFilter>::New();
	fillHoles->SetInputConnection(this->poly_cube->GetOutputPort());
	fillHoles->SetHoleSize(100.0);

	// Make the triangle winding order consistent
	vtkSmartPointer<vtkPolyDataNormals> normals =
	vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->SetInputConnection(fillHoles->GetOutputPort());
	normals->ConsistencyOn();
	normals->SplittingOff();
	normals->Update();
	normals->GetOutput()->GetPointData()->
	SetNormals(this->poly_cube->GetOutput()->GetPointData()->GetNormals());

	// How many added cells
	vtkIdType numOriginalCells = this->poly_cube->GetOutput()->GetNumberOfCells();
	vtkIdType numNewCells = normals->GetOutput()->GetNumberOfCells();

	// Iterate over the original cells
	vtkSmartPointer<vtkCellIterator> it = normals->GetOutput()->NewCellIterator();
	vtkIdType numCells = 0;
	for (it->InitTraversal();
	   !it->IsDoneWithTraversal() && numCells < numOriginalCells;
	   it->GoToNextCell(), ++numCells)
	{
	}
	std::cout << "Num original: " << numOriginalCells
			<< ", Num new: " << numNewCells
			<< ", Num added: " << numNewCells - numOriginalCells << std::endl;
	vtkSmartPointer<vtkPolyData> holePolyData =
	vtkSmartPointer<vtkPolyData>::New();
	holePolyData->Allocate(normals->GetOutput(), numNewCells - numOriginalCells);
	holePolyData->SetPoints(normals->GetOutput()->GetPoints());

	vtkSmartPointer<vtkGenericCell> cell =
	vtkSmartPointer<vtkGenericCell>::New();

	// The remaining cells are the new ones from the hole filler
	for (;
	   !it->IsDoneWithTraversal();
	   it->GoToNextCell(), numCells)
	{
	it->GetCell(cell);
	holePolyData->InsertNextCell(it->GetCellType(), cell->GetPointIds());
	}

	// We have to use ConnectivtyFilter and not
	// PolyDataConnectivityFilter since the later does not create
	// RegionIds cell data.
	vtkSmartPointer<vtkConnectivityFilter> connectivity =
	vtkSmartPointer<vtkConnectivityFilter>::New();
	connectivity->SetInputData(holePolyData);
	connectivity->SetExtractionModeToAllRegions();
	connectivity->ColorRegionsOn();
	connectivity->Update();
	std::cout << "Found " 
			<< connectivity->GetNumberOfExtractedRegions()
			<< " holes" << std::endl;

	// Visualize

	// Create a mapper and actor for the fill polydata
	vtkSmartPointer<vtkDataSetMapper> filledMapper =
	vtkSmartPointer<vtkDataSetMapper>::New();
	filledMapper->SetInputConnection(connectivity->GetOutputPort());
	filledMapper->SetScalarModeToUseCellData();
	filledMapper->SetScalarRange(
	connectivity->GetOutput()->GetCellData()->GetArray("RegionId")->GetRange());
	vtkSmartPointer<vtkActor> filledActor =
	vtkSmartPointer<vtkActor>::New();
	filledActor->SetMapper(filledMapper);
	filledActor->GetProperty()->SetDiffuseColor(
	colors->GetColor3d("Peacock").GetData());

	// Create a mapper and actor for the original polydata
	//减少网格中的三角面片数量
	vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
	decimate->SetInputConnection(this->poly_cube->GetOutputPort());
	decimate->SetTargetReduction(0.9);
	decimate->SetAbsoluteError(0.0005);
	decimate->SetFeatureAngle(30);
	decimate->SetErrorIsAbsolute(1);
	decimate->AccumulateErrorOn();
	
	// decimate->SplittingOff();
	vtkSmartPointer<vtkPolyDataMapper> originalMapper =
	vtkSmartPointer<vtkPolyDataMapper>::New();
	originalMapper->SetInputConnection(decimate->GetOutputPort());

	vtkSmartPointer<vtkProperty> backfaceProp =
	vtkSmartPointer<vtkProperty>::New();
	backfaceProp->SetDiffuseColor(colors->GetColor3d("Banana").GetData());

	vtkSmartPointer<vtkActor> originalActor =
	vtkSmartPointer<vtkActor>::New();
	originalActor->SetMapper(originalMapper);
	originalActor->SetBackfaceProperty(backfaceProp);
	originalActor->GetProperty()->SetDiffuseColor(
	colors->GetColor3d("Flesh").GetData());
	originalActor->GetProperty()->SetRepresentationToWireframe();

	// VTK Renderer
	vtkSmartPointer<vtkRenderer> drightRenderer =
	vtkSmartPointer<vtkRenderer>::New();
	drightRenderer->AddActor(originalActor);	
	drightRenderer->AddActor(filledActor);
	
	// VTK/Qt wedded
	ui->qvtkWidgetDRight->GetRenderWindow()->AddRenderer(drightRenderer);
	ui->qvtkWidgetDRight->GetRenderWindow()->Render();
}

void MainWindow::exportSTL()
{
	//导出stl文件用于3D打印
	std::string filename = "out.stl";
	vtkSTLWriter *stlWriter = vtkSTLWriter::New();
	stlWriter->SetFileName(filename.c_str());
	stlWriter->SetInputConnection(this->poly_repair->GetOutputPort());
	stlWriter->Write();
}



