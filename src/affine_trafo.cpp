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

//TEST
#include "vtkRotationalExtrusionFilter.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWarpTo.h"
#include "vtkAppendFilter.h"
#include "vtkCellArray.h"
#include "vtkConeSource.h"
#include "vtkContourFilter.h"
#include "vtkCubeSource.h"
#include "vtkDataSetMapper.h"
#include "vtkImplicitModeller.h"
#include "vtkLODActor.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRotationalExtrusionFilter.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkWarpTo.h"
#include "vtkTextActor.h"
#include "vtkTextProperty.h"

#include "vtkAxesActor.h"
#include <vtkPropPicker.h>
#include <vtkObjectFactory.h>
#include <vtkInteractorStyleTrackballCamera.h>

#include <vtkAffineRepresentation2D.h>
#include <vtkAffineWidget.h>
#include <vtkAppendPolyData.h>
#include <vtkCommand.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkPlaneSource.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkSTLWriter.h>

#include <vtkPoints.h>
#include <string>

#include "vtkProperty.h"
#include "vtkCamera.h"

#include <vtkTransformPolyDataFilter.h>

class vtkAffineCallback : public vtkCommand
{
public:
    static vtkAffineCallback *New()
    { return new vtkAffineCallback; }
    virtual void Execute(vtkObject *caller, unsigned long, void*);
    vtkAffineCallback():Actor(0),AffineRep(0)
    {
        this->Transform = vtkTransform::New();
    }
    ~vtkAffineCallback()
    {
        this->Transform->Delete();
    }
    vtkActor *Actor;
    vtkAffineRepresentation2D *AffineRep;
    vtkTransform *Transform;
    vtkSmartPointer<vtkTransformPolyDataFilter> PolyDataFilter;
};

class vtkSaveModelCallback : public vtkCommand
{
public:
    static vtkSaveModelCallback *New()
    { return new vtkSaveModelCallback; }
    virtual void Execute(vtkObject *caller, unsigned long, void*);

    vtkSaveModelCallback()
    {
        filename = new std::string("modelTransformed.stl");// /opt/modelTransformed.stl
        model = vtkSmartPointer<vtkPolyData>::New();
    }

    ~vtkSaveModelCallback()
    {
        delete filename;
    }


    std::string* filename;
    vtkSmartPointer<vtkPolyData>  model;

    vtkTransformPolyDataFilter* PolyDataFilter;
};

void vtkSaveModelCallback::Execute(vtkObject *caller, unsigned long, void *)
{
    vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);

    if(*(iren->GetKeySym()) != 's')
        return;
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    writer->SetFileName(filename->c_str());

    writer->SetInputConnection(PolyDataFilter->GetOutputPort());
    writer->Update();
}

void vtkAffineCallback::Execute(vtkObject*, unsigned long vtkNotUsed(event), void*)
{
    this->AffineRep->GetTransform(this->Transform);
    this->Actor->SetUserTransform(this->Transform);
    PolyDataFilter->SetTransform(this->Transform);
    PolyDataFilter->Update();
}

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

    // set camera
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New(); // vtkCamera is a virtual camera for 3D rendering
    camera->SetPosition(-1.9,0,1.5); // EINHEIT: m
    camera->SetFocalPoint(0,0,0);
    camera->SetRoll(90);
    camera->Pitch(10);
    camera->Yaw(0);
    camera->SetViewAngle(63.1); // Der Angle of View der realen Kamera kann über Focal length und Sensorgröße ausgerechnet werden (Wikipedia)!!!


    vtkSmartPointer<vtkRenderer> renderer =
            vtkSmartPointer<vtkRenderer>::New();    // A renderer is an object that controls the rendering process for objects. Rendering
    // is the process of converting geometry, a specification for lights, and
    // a camera view into an image

    renderer->SetActiveCamera(camera);

    vtkSmartPointer<vtkRenderWindow> renderWindow =
            vtkSmartPointer<vtkRenderWindow>::New(); // create a window for renderers to draw into
    renderWindow->AddRenderer(renderer);
    renderWindow->SetSize(400,533); // Wenn auskommentiert wird die Karte perfekt auf den Bildschirm gerendert aber output.png ist schwarz?!?!?!?!?!!

    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
            vtkSmartPointer<vtkRenderWindowInteractor>::New(); // Ermöglicht Interaktion mit Maus/ Tastatur
    renderWindowInteractor->SetRenderWindow(renderWindow);
    vtkInteractorStyleSwitch::SafeDownCast(renderWindowInteractor->GetInteractorStyle())->SetCurrentStyleToTrackballCamera();

    renderer->AddActor(actor);
    renderer->GradientBackgroundOn();
    renderer->SetBackground(1,1,1);
    renderer->SetBackground2(0,0,0.5);

    renderWindow->Render();

    //Add Axes
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetOrigin(0.,0.,0.);
    axes->SetConeRadius(0.2);
    axes->SetNormalizedShaftLength(1,1,1);
    axes->SetNormalizedTipLength(0.2,0.2,0.2);
    axes->SetTotalLength(2,2,2);
    axes->AxisLabelsOff();
    renderer->AddViewProp( axes );

    vtkSmartPointer<vtkTransformPolyDataFilter> PolyDataFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    PolyDataFilter->SetInputConnection(reader->GetOutputPort());

    vtkSmartPointer<vtkActor> planeActor = vtkSmartPointer<vtkActor>::New();
    planeActor->GetProperty()->SetRepresentationToWireframe();
    planeActor->GetProperty()->SetColor(1,0,0);

    // Add Affine Widget
    vtkSmartPointer<vtkAffineWidget> affineWidget = vtkSmartPointer<vtkAffineWidget>::New();
    affineWidget->SetInteractor(renderWindowInteractor);
    affineWidget->CreateDefaultRepresentation();
    vtkAffineRepresentation2D::SafeDownCast(affineWidget->GetRepresentation())->PlaceWidget(actor->GetBounds());

    vtkSmartPointer<vtkAffineCallback> affineCallback = vtkSmartPointer<vtkAffineCallback>::New();
    affineCallback->Actor = actor;
    affineCallback->AffineRep = vtkAffineRepresentation2D::SafeDownCast(affineWidget->GetRepresentation());
    affineCallback->PolyDataFilter = PolyDataFilter;

    affineWidget->AddObserver(vtkCommand::InteractionEvent,affineCallback);
    affineWidget->AddObserver(vtkCommand::EndInteractionEvent,affineCallback);
    affineWidget->On();

    vtkSmartPointer<vtkSaveModelCallback> saveModelCallback = vtkSmartPointer<vtkSaveModelCallback>::New();
    saveModelCallback->PolyDataFilter = PolyDataFilter;

    renderWindowInteractor->AddObserver ( vtkCommand::KeyPressEvent, saveModelCallback );


    // Bild erzeugen
//    vtkSmartPointer<vtkWindowToImageFilter> ImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
//    ImageFilter->SetInput(renderWindow);
//    ImageFilter->SetMagnification(1);
//    ImageFilter->Update();

//    vtkSmartPointer<vtkPNGWriter> pngWriter = vtkSmartPointer<vtkPNGWriter>::New(); // Writes PNG files
//    pngWriter->SetFileName("output.png");
//    pngWriter->SetInput(ImageFilter->GetOutput());
//    pngWriter->Update();
//    pngWriter->Write();

    renderWindowInteractor->Start();

    ////OFFSCREEN RENDERING: VTK_OPENGL_HAS_OSMESA im advanced-build-modus einschalten!
    ////Fehlermeldung: "[VTK] GL/osmesa.h: no such file or directory"
    //// http://tips.enderunix.org/view.php?id=2338&lang=en (in order to install vtk without getting this error, libosmesa-dev must be installed. for instance, issue "sudo apt-get install libosmesa6-dev" on ubuntu)
    ///// TROTZDEM FEHLER!!!
    return EXIT_SUCCESS;
}
