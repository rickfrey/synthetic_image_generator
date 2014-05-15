#include <vtkPolyData.h>
#include <vtkSTLReader.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
 
int main ( int argc, char *argv[] )
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
 
  // Visualize
  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();// vtkPolyDataMapper is a class that maps polygonal data (i.e., vtkPolyData)
  mapper->SetInputConnection(reader->GetOutputPort());
 
  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();   // vtkActor is used to represent an entity (Einheit) in a rendering scene
                                        // Verbindung zu mapper (Geometrische Definition), actor hat infos über Abstand etc. (z.B. auch Textur)
  actor->SetMapper(mapper);

  //TEST!!!///////////////////////////
  vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New(); // vtkCamera is a virtual camera for 3D rendering
  camera->SetPosition(0,0,10);
  camera->SetFocalPoint(0,0,0);
  ///////////////////////////////////

  vtkSmartPointer<vtkRenderer> renderer =
    vtkSmartPointer<vtkRenderer>::New();    // A renderer is an object that controls the rendering process for objects. Rendering
                                            // is the process of converting geometry, a specification for lights, and
                                            // a camera view into an image

  //TEST!!!//////////////////////////
  renderer->SetActiveCamera(camera);
  ///////////////////////////////////

  vtkSmartPointer<vtkRenderWindow> renderWindow =
    vtkSmartPointer<vtkRenderWindow>::New(); // create a window for renderers to draw into
  renderWindow->AddRenderer(renderer);
  //TEST///
  //renderWindow->SetSize(600,400);
  /////////

  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New(); // Ermöglichst Interaktion mit Maus/ Tastatur
  renderWindowInteractor->SetRenderWindow(renderWindow);
 
  renderer->AddActor(actor);
  renderer->SetBackground(.3, .6, .3); // Background color green
 
  renderWindow->Render();

  //TEST!!!///////////////////
  vtkSmartPointer<vtkWindowToImageFilter> ImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  ImageFilter->SetInput(renderWindow);  // vtkWindowToImageFilter provides methods needed to read the data in
                                        // a vtkWindow and use it as input to the imaging pipeline. This is
                                        // useful for saving an image to a file for example
  vtkSmartPointer<vtkPNGWriter> pngWriter = vtkSmartPointer<vtkPNGWriter>::New(); // Writes PNG files
  pngWriter->SetFileName("output.png");
  pngWriter->SetInput(ImageFilter->GetOutput());
  pngWriter->Write();
  ////////////////////////////

  renderWindowInteractor->Start();

 
  return EXIT_SUCCESS;
}
