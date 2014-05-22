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

      for (float y=0.0;y<=4;y+=0.5) // Schleife über alle y-Werte
      {
      for (float x=0.0;x<=5.6;x+=0.5)// Schleife über alle x-Werte
      {
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

    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetPosition(4,3,1);
    light->SetFocalPoint(4,3,0);
    light->SetConeAngle(180);
    light->SetIntensity(1);
    renderer->AddLight(light);

    renderWindow->Render();

    windowToImageFilter->SetInput(renderWindow);
    windowToImageFilter->Update();

    std::stringstream ss;
    ss << "pos_"<< x << "_" << y << ".png";
    writer->SetFileName(ss.str().c_str());//aus stringstream wird string und anschl. constant string gemacht
    writer->SetInputConnection(windowToImageFilter->GetOutputPort());
    writer->Write();
}
}
    return 0;
}
}
