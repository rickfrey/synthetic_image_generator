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
    if ( argc != 2 )
    {
        cout << "Required parameters: Filename" << endl;
        return EXIT_FAILURE;
    }

    // STL-Modell (mit neuem Koordinatensystem!) laden:
    std::string inputFilename = argv[1];
    vtkSmartPointer<vtkSTLReader> reader =
            vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(inputFilename.c_str());
    reader->Update();

    std::ofstream myfile;
    myfile.open("Eigenschaften.txt");

    //Visualize
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    // a renderer and render window
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetOffScreenRendering(1);

    // Bildgröße anpassen!
    renderWindow->SetSize(1536,2048);// Davor: 1200,1599
    renderWindow->AddRenderer(renderer);

    // add the actors to the scene
    renderer->AddActor(actor);
    renderer->SetBackground(.3, .6, .3);


    //float x=1.8,y=1.8,z=0.6;
    //int pitch=-30, yaw=60;
    float z=0.6;
    bool firstrun = true;

    // Schleife über alle Kameraposen
    for (float x = 0.2; x <= 3.8; x += 0.5) // Schleife über alle y-Werte
    {
        for (float y = 0.2; y <= 3.4; y += 0.5)// Schleife über alle x-Werte
        {
            for(int pitch = -50; pitch < -11; pitch += 20)
            {
                for(int yaw = 0; yaw < 360; yaw += 10)
                {

                    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
                    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();

                    // Kameraparameter und -pose setzen
                    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
                    camera->SetPosition(x,y,z);
                    camera->SetFocalPoint(x+1,y,z);
                    camera->SetRoll(90);
                    camera->Pitch(pitch);
                    camera->Yaw(yaw);
                    camera->SetViewAngle(70); // 63.1 für Galaxy, 49.9 für iPhone 3GS

                    renderer->SetActiveCamera(camera);

                    /*  // Lichtquelle manuell setzen
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

                    // NACHHER AUSKOMMENTIEREN SONST BILD FÜR JEDE POSE!!!
                    // Abspeichern des Bildes in der aktuellen Pose
//                    std::stringstream ss;
//                    ss << "pos_"<< x << "_" << y << "_" << pitch << "_" << yaw << ".png";
//                    writer->SetFileName(ss.str().c_str());//aus stringstream wird string und anschl. constant string gemacht
//                    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
//                    writer->Write();

                    /////Convert VTKImageData to iplimage (OpenCV)/////////////////////////////////////
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
                    //cv::imwrite("TEST_OPENCV.jpg",openCVImage);

                    //Kantendetektion
                    cv::Mat dst, cdst;
                    cv::Canny(openCVImage,dst,50,52,3);

                    // NACHHER AUSKOMMENTIEREN SONST BILD FÜR JEDE POSE!!!
                    // Binäres Bild speichern:
                    //    cv::imwrite("Kantenbild.jpg",dst);
                    // cv::cvtColor(dst,cdst,CV_GRAY2BGR);

                    cv::Point Mittelpunkt;
                    float Gegenkathete, Ankathete, Theta, laenge;

                    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    // Binärbild als input: Pose in Textdatei, alle mit HoughLinesP detektierte Linien in Textdatei:
                    //                        std::ofstream myfile;

                    //                        // Textdatei soll nur beim ersten Durchlauf geöffnet werden
                    //                        if(firstrun==true)
                    //                        {
                    //                            myfile.open("Eigenschaften.txt");
                    //                        }
                    //                        firstrun=false;

                    // Pose in Textdatei schreiben
                    myfile << x << " " << y << " " << z << " " << pitch << " " << yaw << std::endl;
                    std::vector<Vec4i> lines;
                    HoughLinesP(dst, lines, 1, CV_PI/360, 90, 30, 50 );
                    for( size_t i = 0; i < lines.size(); i++ )
                    {
                        Vec4i l = lines[i];
                        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
                        //Eigenschaften berechnen:
                        Mittelpunkt.x=cvRound((l[0]+l[2])/2);//Berechnung x-Koordinate des Mittelpunkts
                        Mittelpunkt.y=cvRound((l[1]+l[3])/2);//  ""       y-koordinate
                        cv::circle(cdst,Mittelpunkt,5,Scalar(0,255,0),2,CV_AA);
                        Gegenkathete=((l[1]-l[3]));
                        Ankathete=((l[2]-l[0]));
                        std::cout<<l[0]<<","<<l[1]<<","<<l[2]<<","<<l[3]<<std::endl;
                        Theta=cvRound(atan(Gegenkathete/Ankathete)*360/(2*CV_PI));
                        //Theta=(CV_PI/2-Theta)*360/(2*CV_PI);
                        laenge=cvRound(sqrt(Ankathete*Ankathete+Gegenkathete*Gegenkathete));

                        // Parameter der detektierten Linie in Textdatei schreiben:
                        myfile << Mittelpunkt.x << " " << Mittelpunkt.y << " " << Theta << " " << laenge <<std::endl;
                    }
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                    // Visualisierung der detektierten Linien
                    /*      std::cout<<"Dimension von lines: "<<lines.size()<<std::endl;
      cv::Point P1,P2,P3;
      P1.x=127;
      P1.y=309;
      P2.x=200;
      P2.y=304;
      P3.x=100;
      P3.y=10;
      cv::circle(cdst,P1,7,Scalar(255,0,0),2,CV_AA);
      cv::circle(cdst,P2,7,Scalar(255,0,0),2,CV_AA);
      cv::circle(cdst,P3,7,Scalar(255,0,0),2,CV_AA);
*/
                    // Binär- mit überlagertem Kantenbild speichern:
                    //      cv::imwrite("Lines.jpg",cdst);


                }
            }
        }
    }
    myfile.close();
    return 0;
}
