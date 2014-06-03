// These 2 lines are needed due to:
// http://www.vtk.org/Wiki/VTK/VTK_6_Migration/Factories_now_require_defines
#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkGraphicsFactory.h>

#include <vtkCamera.h>
#include <vtkSTLReader.h>
#include <math.h>
#include <sstream>
#include <vtkLight.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <fstream>

using namespace cv;

int main (int argc, char *argv[])
{
    {
      if ( argc != 2 )
        {
        cout << "Required parameters: Filename" << endl;
        return EXIT_FAILURE;
        }

      std::string inputFilename = argv[1];

      vtkSmartPointer<vtkSTLReader> reader =
        vtkSmartPointer<vtkSTLReader>::New();
      reader->SetFileName(inputFilename.c_str());
      reader->Update();
/*
      for (float y=0.0;y<=4;y+=0.5) // Schleife über alle y-Werte
      {
      for (float x=0.0;x<=5.6;x+=0.5)// Schleife über alle x-Werte
      {
      */
      int x=0,y=0,z=0;
    //Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());


    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // a renderer and render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetOffScreenRendering(1);

    renderWindow->SetSize(400,533);
    renderWindow->AddRenderer(renderer);

    // add the actors to the scene
    renderer->AddActor(actor);
    renderer->SetBackground(.3, .6, .3);

    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();

    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();

    //AB HIER IN SCHLEIFE!


    // Camera
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera->SetPosition(x,y,0.5);
    camera->SetFocalPoint(x+1,y,0.5);
    camera->SetRoll(90);
    camera->Pitch(0);
    camera->Yaw(0);
    camera->SetViewAngle(63.1);

    renderer->SetActiveCamera(camera);
/*
    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetPosition(4,3,1);
    light->SetFocalPoint(4,3,0);
    light->SetConeAngle(180);
    light->SetIntensity(1);
    renderer->AddLight(light);
*/
    renderWindow->Render();

    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->Update();


    std::stringstream ss;
    ss << "pos_"<< x << "_" << y << ".png";
    writer->SetFileName(ss.str().c_str());//aus stringstream wird string und anschl. constant string gemacht
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();

    /////Convert VTKImageData to iplimage (OpenCV)
    // http://vtk.1045678.n5.nabble.com/From-vtkImageData-to-Iplimage-OpenCV-td5716020.html
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image = windowToImageFilter->GetOutput();
    //check number of components
    const int numComponents = image->GetNumberOfScalarComponents();
    //Construct the OpenCV Mat
    int dims[3];
    image->GetDimensions(dims);
    cv::Mat openCVImage(dims[1],dims[0],CV_8UC3, image->GetScalarPointer());
    //cv::cvtColor(openCVImage,openCVImage,CV_BGRA2GRAY);// Wenn auskommentiert verändert sich auch VTK-BILD?!?!?!
    //Flip because of different origins between vtk and OpenCV
    cv::flip(openCVImage,openCVImage,0);
    //cv::imshow("Testbild",openCVImage);
    cv::imwrite("TEST_OPENCV.jpg",openCVImage);

    //Kantendetektion!!!
    cv::Mat dst, cdst;
    cv::Canny(openCVImage,dst,50,52,3);
    //TEST: Kantenbild
    cv::imwrite("Kantenbild.jpg",dst);
    cv::cvtColor(dst,cdst,CV_GRAY2BGR);

    cv::Point Mittelpunkt;
    int Gegenkathete, Ankathete, Theta;
    std::ofstream myfile;//Eigenschaften in Textdatei schreiben
    myfile.open("Eigenschaften.txt");
    myfile<<"X-Pos: "<<x<<" Y-Pos: "<<y<<" Z-Pos: "<<z<<std::endl;
    std::vector<Vec4i> lines;
      HoughLinesP(dst, lines, 1, CV_PI/180, 30, 50, 10 );
      for( size_t i = 0; i < lines.size(); i++ )
      {
        Vec4i l = lines[i];
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
        //Eigenschaften berechnen:
        Mittelpunkt.x=cvRound((l[0]+l[2])/2);
        Mittelpunkt.y=cvRound((l[1]+l[3])/2);
        cv:circle(cdst,Mittelpunkt,5,Scalar(0,255,0),2,CV_AA);
        Gegenkathete=cvRound(abs(l[1]-l[3]));
        Ankathete=cvRound(abs(l[0]-l[2]));
        Theta=atan(Gegenkathete/Ankathete);
        Theta=CV_PI/2-Theta;
        //HIER WEITERMACHEN UND alpha, theta und Länge berechnen, anschließend in Textdatei schreiben!!
        //
        //
        //
        myfile<<"Mittelpunkt "<<i<<": "<<Mittelpunkt.x<<", "<<Mittelpunkt.y<<";"<<std::endl;

      }


      cv::imwrite("Lines.jpg",cdst);

      myfile.close();
    /*
}
}
*/
    return 0;
}
}
