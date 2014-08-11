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

#include <algorithm>

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
    renderWindow->SetSize(3000,4000);// Je mehr Pixel desto besser funktioniert HoughLinesP!! Davor: 1200,1599; iPhone: 1536,2048
    renderWindow->AddRenderer(renderer);

    // add the actors to the scene
    renderer->AddActor(actor);
    renderer->SetBackground(1, 1, 1);


    //float x=1.8,y=1.8,z=0.6;
    //int pitch=-30, yaw=60;
    float z=0.6;
    int roll=90;

    float x=0.2,y=0.2;
    int pitch=0, yaw=50;

    //    // Schleife über alle Kameraposen
    //    for (float x = 0.2; x <= 3.8; x += 0.5) // Schleife über alle y-Werte
    //    {
    //        for (float y = 0.2; y <= 3.4; y += 0.5)// Schleife über alle x-Werte
    //        {
    //            for(int pitch = -50; pitch < -11; pitch += 20)
    //            {
    //                for(int yaw = 0; yaw < 360; yaw += 10)
    //                {

    vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
    vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();

    // Kameraparameter und -pose setzen
    vtkSmartPointer<vtkCamera> camera = vtkSmartPointer<vtkCamera>::New();
    camera->SetPosition(x,y,z);
    camera->SetFocalPoint(x+1,y,z);
    camera->SetRoll(roll);
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
    cv::imwrite("TEST_OPENCV.jpg",openCVImage);// AUSKOMMENTIEREN!

    //Kantendetektion
    cv::Mat dst, blured_pic, cdst;
    cv::blur( openCVImage, blured_pic, Size(3,3) );
    cv::Canny(blured_pic,dst,50,52,3);

    // NACHHER AUSKOMMENTIEREN SONST BILD FÜR JEDE POSE!!!
    // Binäres Bild speichern:
    cv::imwrite("Kantenbild.jpg",dst);// AUSKOMMENTIEREN
    cv::cvtColor(dst,cdst,CV_GRAY2BGR);// AUSKOMMENTIEREN


    cv::Point Mittelpunkt;
    float Gegenkathete, Ankathete;
    int Theta, laenge;

    // Pose in Textdatei schreiben
    myfile << x << " " << y << " " << z << " " << roll << " " << pitch << " " << yaw << std::endl;




    //    PROBLEM!!!!!!!!!!/////////////////
    //    HoughLinesP teilt lange Kanten in mehrere Linien hintereinander (mit fast identischer Ausrichtung) auf
    //    Anpassen der Parameter!?!?!?!?!?!




    // HoughLinesP-Algorithmus speichert Parameter in Vektor "lines"
    std::vector<Vec4i> lines;
    HoughLinesP(dst, lines, 0.9, CV_PI/720, 90, 20, 50 );

    // Vektoren, in den die umgerechneten Linienparameter (Theta in Thetavektor, übrige Parameter in umgerechneteParameter) gespeichert werden
    std::vector<int> Thetavektor;
    std::vector<int> umgerechneteParameter;

    // Linienparameter von x1,y1 und x2,y2 in Theta, x-Mittelpunkt, y-Mittelpunkt und Länge umrechnen
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];

        // Aktualle Linie in "cdst" malen (Nur zur Visualisierung)
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);

        // Linienparameter von x1,y1 und x2,y2 in Theta, x-Mittelpunkt, y-Mittelpunkt und Länge umrechnen
        Mittelpunkt.x=cvRound((l[0]+l[2])/2);
        Mittelpunkt.y=cvRound((l[1]+l[3])/2);
        Gegenkathete=((l[1]-l[3]));
        Ankathete=((l[2]-l[0]));
        Theta=cvRound(atan(Gegenkathete/Ankathete)*360/(2*CV_PI));
        laenge=cvRound(sqrt(Ankathete*Ankathete+Gegenkathete*Gegenkathete));

        // Mittelpunkt der aktuellen Linie in "cdst" malen (nur zur Visualisierung)
        cv::circle(cdst,Mittelpunkt,5,Scalar(0,255,0),2,CV_AA);

        // Ausgabe der Endpunkte der aktuellen Linie
        std::cout<<l[0]<<","<<l[1]<<","<<l[2]<<","<<l[3]<<std::endl;


        // 1. Neuen Vektor std::vector<Vec4i> umgerechneteParameter initialisieren, in den nacheinander alle Linienparameter geschrieben werden (Theta zuerst)
        // 2. In Vektor nach kleinstem Thetawert suchen -> Zugehörige Parameter in Textdatei schreiben und die entsprechende Zeile im Vektor löschen
        // 3. so lange wie noch Werte => Im neuen Vektor müssten jetzt die Parameter aufsteigend nach Theta sortiert sein

        // Thetawert für aktuelle Linie wird in "Thetavektor" geschrieben, andere Parameter in "umgerechneteParameter"
        // Das Ganze dient dazu, die Linien aufsteigend nach Theta zu sortieren
        Thetavektor.push_back(Theta);
        umgerechneteParameter.push_back(Mittelpunkt.x);
        umgerechneteParameter.push_back(Mittelpunkt.y);
        umgerechneteParameter.push_back(laenge);

    }

    // Jetzt sind alle Linien in die Parameter Theta, x-Mittelp., y-Mittelp. und Länge umgerechnet und in der ursprünglichen
    // Reihenfolge gespeichert (gleiche Linienreihenfolge wie in "lines")
    // Parameter gesspeichert in: Theta in Thetavektor, übrige Parameter in "umgerechneteParameter"

    // In Vektor "NachThetaSortiert" werden die umgerechneten Parameter so sortiert, dass die Linien mit kleinstem Thetawert zuerst kommen
    std::vector<Vec4i> NachThetaSortiert;
    // Vektor Lines nach der gleichen Vorschrift sortiert wie "NachThetasortiert", damit die Container zusammenpassen!
    std::vector<Vec4i> LinesSortiert;
    // Vektor mit Einträgen nach Theta sortiert und Linien fusioniert
    std::vector<Vec4i> Linienfusioniert;
    // Gleiche Einträge wie Linienfusioniert, nur dass dieselbe Linie nicht doppelt auftaucht
    std::vector<Vec4i> Linienfusioniert2;

    // Schleife über alle Einträge von "lines"
    int AnzahlFusionen=0;
    int linessize = lines.size();
    for(int Liniennummer = 0; Liniennummer < linessize; Liniennummer++)
    {
        // In "Thetavektor" nach kleinstem Theta suchen und zugehörigen Index in "min_index" speichern
        int min_index = std::min_element(Thetavektor.begin(), Thetavektor.end()) - Thetavektor.begin();
        cout << "Index für kleinstes Element: " << min_index << endl;

        //            Vec4i tmpVec;
        //            tmpVec.

        // Kleinster Thetawert wird in Vektor "NachThetaSortiert" geschrieben zusammen mit den zugehörigen Parametern
        NachThetaSortiert.push_back(Vec4i(Thetavektor[min_index] , umgerechneteParameter[(min_index*3)] , umgerechneteParameter[((min_index*3)+1)] , umgerechneteParameter[((min_index*3)+2)] ));

        // Gleichzeitig: Sortieren des Vektors "lines" (gleiche Sortiervorschrift wie "NachThetaSortiert" (Sortiert abspeichern in Vektor "LinesSortiert"))
        LinesSortiert.push_back(Vec4i( lines[min_index][0] , lines[min_index][1] , lines[min_index][2] , lines[min_index][3] ));


        //            // Kleinster Thetawert wird in Textdatei geschrieben zusammen mit den zugehörigen Parametern
        //            myfile << Thetavektor[min_index] << " " << umgerechneteParameter[min_index*3] << " " << umgerechneteParameter[(min_index*3) + 1] << " " << umgerechneteParameter[(min_index*3) + 2] <<std::endl;

        // Kleinsten Eintrag aus Thetavektor und zugehörige Parameter aus umgerechneteParameter löschen damit neuer kleinster Eintrag berechnen kann
        Thetavektor.erase(Thetavektor.begin() + min_index);
        umgerechneteParameter.erase(umgerechneteParameter.begin() + min_index*3, umgerechneteParameter.begin() + min_index*3 + 3);
        lines.erase(lines.begin()+min_index);
    }

    // Jetzt sind alle Linien im Vektor "NachThetaSortiert" aufsteigend nach Theta sortiert mit den jeweils zugehörigen Parametern
    // x-Mittelpunkt, y-Mittelpunkt und Länge
    // Jetzt Vektor durchlaufen und nach Linien suchen, die sich aus mehreren Linien zusammensetzen
    // Aktuelle Linie mit der nächsten auf Gemeinsamkeiten überprüfen
    for(int Liniennummer = 0; Liniennummer < NachThetaSortiert.size(); Liniennummer++)
    {
        // Wenn Theta der aktuellen Linie +-2° mit der vorigen Linie übereinstimmt: Prüfen, ob die beiden Linien zu einer "fusioniert" werden können
        if(NachThetaSortiert[Liniennummer][0] <= ( (NachThetaSortiert[Liniennummer+1][0]) + 2) && NachThetaSortiert[Liniennummer][0] >= ( (NachThetaSortiert[Liniennummer+1][0]) - 2))
        {


            // Praktisch: HoughLinesP sortiert die Parameter so: x1,y1,x2,y2 wobei x1 das Ende mit kleinerem x-Wert (weiter links) ist
            // Wenn beide Endpunkte in einem Radius von 10 Pixeln liegen:
            //
            // o----------------------o
            //  o----------------------o
            //
            // Abstand <=10 = Δx²+Δy² (für jeweils beide Endpunkte der Linien)
            int deltax1 = LinesSortiert[Liniennummer][0] - LinesSortiert[(Liniennummer+1)][0];
            int deltay1 = LinesSortiert[Liniennummer][1] - LinesSortiert[(Liniennummer+1)][1];
            if(sqrt(pow(deltax1 , 2) + pow(deltay1 , 2) ) <= 10
                    && sqrt( pow( (LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer+1][2]) , 2 ) + pow( (LinesSortiert[Liniennummer][3] - LinesSortiert[(Liniennummer+1)][3]) , 2 ) ) <= 10)
            {
                AnzahlFusionen++;

                // Die beiden Linien fusionieren und neue Endpunkte in LinesSortiert[Liniennummer+1]schreiben    NEIN: (und LinesSortiert[Liniennummer-1] löschen)
                LinesSortiert[Liniennummer+1][0] = cvRound((LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer+1][0]) / 2); // x1_Neu (Mittelwert)
                LinesSortiert[Liniennummer+1][1] = cvRound((LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer+1][1]) / 2); // y1_Neu (Mittelwert)
                LinesSortiert[Liniennummer+1][2] = cvRound((LinesSortiert[Liniennummer][2] + LinesSortiert[Liniennummer+1][2]) / 2); // x2_Neu (Mittelwert)
                LinesSortiert[Liniennummer+1][3] = cvRound((LinesSortiert[Liniennummer][3] + LinesSortiert[Liniennummer+1][3]) / 2); // y2_Neu (Mittelwert)

                // Wie löschen??? LinesSortiert[Liniennummer][0...3].delete ?????

                // Die fusionierte Linie in Theta, Mittelpunkt und Länge-Parameter umrechnen und die entsprechende Zeile in "NachThetaSortiert" durch die neuen Parameter ersetzen
                // Zusätzlich Einträge in Vektor Linienfusioniert speichern (endgültiger Vektor, der anschließend in Textdatei geschrieben wird
                int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer+1][0] + LinesSortiert[Liniennummer+1][2]) / 2 );
                int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer+1][1] + LinesSortiert[Liniennummer+1][3]) / 2 );
                float Gegenkath_neu = LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer+1][3];
                float Ankath_neu = LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer+1][0];
                int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                NachThetaSortiert[Liniennummer+1][0] = Theta_neu;
                NachThetaSortiert[Liniennummer+1][1] = Mittelp_x_neu;
                NachThetaSortiert[Liniennummer+1][2] = Mittelp_y_neu;
                NachThetaSortiert[Liniennummer+1][3] = laenge_neu;

                cout << "Theta_neu= " << Theta_neu << endl;
                cout << "Ankathete_neu= " << Ankath_neu << endl;
                cout << "Gegenkath_neu= " << Gegenkath_neu << endl;
                cout << "Test: Länge der 3. Linie= " << NachThetaSortiert[2][3] << endl;

                // Neue Linie in Vektor "Linienfusioniert" schreiben
                Linienfusioniert.push_back(Vec4i( Theta_neu , Mittelp_x_neu , Mittelp_y_neu , laenge_neu ));
            }


            // Wenn nur jeweils linke Endpunkte übereinstimmen (Überlappung)
            //
            // o----------------------------o Liniennummer-1
            // o---------------o Liniennummer
            else if(sqrt(pow(LinesSortiert[Liniennummer][0] - LinesSortiert[Liniennummer+1][0] , 2) + pow(LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer+1][1] , 2) ) <= 10)
            {
                // Linker Endpunkt der neuen Linie kann schon berechnet werden (gemittelt)
                LinesSortiert[Liniennummer+1][0] = cvRound((LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer+1][0]) / 2); // x1_Neu (Mittelwert)
                LinesSortiert[Liniennummer+1][1] = cvRound((LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer+1][1]) / 2); // y1_Neu (Mittelwert)

                // checken welcher der beiden rechten Endpunkte den größeren Abstand zum neuen linken Endpunkt hat
                // Wenn der Endpunkt der aktuellen Linie [Liniennummer] einen größeren Abstand hat dann stellt ihr Endpunkt den Endpunkt der neuen Linie [Liniennummer+1] dar:
                if( sqrt( pow(LinesSortiert[Liniennummer+1][0] - LinesSortiert[Liniennummer][2] , 2) + pow(LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer][3] , 2))
                        >= sqrt( pow(LinesSortiert[Liniennummer+1][0] - LinesSortiert[Liniennummer+1][2] , 2) + pow(LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer+1][3] , 2)))
                {
                    LinesSortiert[Liniennummer+1][2] = LinesSortiert[Liniennummer][2];
                    LinesSortiert[Liniennummer+1][3] = LinesSortiert[Liniennummer][3];
                }

                // Wenn der Endpunkt der nächsten Linie [Liniennummer+1] einen größeren Abstand aufweist zum neu berechneten linken Endpunkt bleiben die Einträge für x2 und y2 unverändert!

                // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer+1][0] + LinesSortiert[Liniennummer+1][2]) / 2 );
                int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer+1][1] + LinesSortiert[Liniennummer+1][3]) / 2 );
                float Gegenkath_neu = LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer+1][3];
                float Ankath_neu = LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer+1][0];
                int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                NachThetaSortiert[Liniennummer+1][0] = Theta_neu;
                NachThetaSortiert[Liniennummer+1][1] = Mittelp_x_neu;
                NachThetaSortiert[Liniennummer+1][2] = Mittelp_y_neu;
                NachThetaSortiert[Liniennummer+1][3] = laenge_neu;

                // Neue Linie in "Linienfusioniert" schreiben
                Linienfusioniert.push_back(Vec4i( Theta_neu , Mittelp_x_neu , Mittelp_y_neu , laenge_neu ));
            }


            // Wenn nur jeweils rechte Endpunkte übereinstimmen (Überlappung)
            //
            // o----------------------------o Liniennummer-1
            //              o---------------o Liniennummer
            else if(sqrt ( pow( (LinesSortiert[Liniennummer][2] - LinesSortiert[(Liniennummer+1)][2]) , 2) + pow( (LinesSortiert[Liniennummer][3] - LinesSortiert[(Liniennummer+1)][3]) , 2) ) <= 10 )
            {
                // Rechter Endpunkt der neuen Linie kann schon berechnet werden (gemittelt)
                LinesSortiert[Liniennummer+1][2] = cvRound((LinesSortiert[Liniennummer][2] + LinesSortiert[(Liniennummer+1)][2]) / 2); // x2_Neu (Mittelwert)
                LinesSortiert[Liniennummer+1][3] = cvRound((LinesSortiert[Liniennummer][3] + LinesSortiert[Liniennummer+1][3]) / 2); // y2_Neu (Mittelwert)

                // checken welcher der beiden linken Endpunkte den größeren Abstand zum neuen rechten Endpunkt hat
                // Wenn der Endpunkt der aktuellen Linie [Liniennummer] einen größeren Abstand hat dann stellt ihr Endpunkt den Endpunkt der neuen Linie dar:
                if( sqrt( pow(LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer][0] , 2) + pow(LinesSortiert[Liniennummer+1][3] - LinesSortiert[Liniennummer][1] , 2))
                        >= sqrt( pow(LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer+1][0] , 2) + pow(LinesSortiert[Liniennummer+1][3] - LinesSortiert[Liniennummer+1][1] , 2)))
                {
                    LinesSortiert[Liniennummer+1][0] = LinesSortiert[Liniennummer][0];
                    LinesSortiert[Liniennummer+1][1] = LinesSortiert[Liniennummer][1];
                }

                // Wenn der Endpunkt der aktuelle Linie [Liniennummer] einen größeren Abstand aufweist bleiben die Einträge für x1 und y1 unverändert!

                // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer+1][0] + LinesSortiert[Liniennummer+1][2]) / 2 );
                int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer+1][1] + LinesSortiert[Liniennummer+1][3]) / 2 );
                float Gegenkath_neu = LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer+1][3];
                float Ankath_neu = LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer+1][0];
                int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                NachThetaSortiert[Liniennummer+1][0] = Theta_neu;
                NachThetaSortiert[Liniennummer+1][1] = Mittelp_x_neu;
                NachThetaSortiert[Liniennummer+1][2] = Mittelp_y_neu;
                NachThetaSortiert[Liniennummer+1][3] = laenge_neu;

                // Neue Linie in "Linienfusioniert" schreiben
                Linienfusioniert.push_back(Vec4i( Theta_neu , Mittelp_x_neu , Mittelp_y_neu , laenge_neu ));
            }


            // Wenn rechter Endpunkt der aktuellen Linie [Liniennummer] mit dem linken Endpunkt der folgenden Linie [Liniennummer+1] übereinstimmt (Zusammenstückelung einer Kante aus mehreren Linien)
            // Liniennummer             Liniennummer+1
            // o--------------------o o-------------o
            else if(sqrt( pow( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer+1][0] , 2 ) + pow( LinesSortiert[Liniennummer][3] - LinesSortiert[Liniennummer+1][1] , 2 ) ) <= 10 )
            {
                LinesSortiert[Liniennummer+1][0] = LinesSortiert[Liniennummer][0]; // x1_neu = x1 der aktuellen Linie [Liniennummer]
                LinesSortiert[Liniennummer+1][1] = LinesSortiert[Liniennummer][1]; // y1_neu = y1 der aktuellen Linie [Liniennummer]
                LinesSortiert[Liniennummer+1][2] = LinesSortiert[Liniennummer+1][2]; // x2_neu = x2 der folgenden Linie [Liniennummer+1]
                LinesSortiert[Liniennummer+1][3] = LinesSortiert[Liniennummer+1][3]; // y2_neu = y2 der folgenden Linie [Liniennummer+1]

                // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer+1][0] + LinesSortiert[Liniennummer+1][2]) / 2 );
                int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer+1][1] + LinesSortiert[Liniennummer+1][3]) / 2 );
                float Gegenkath_neu = LinesSortiert[Liniennummer+1][1] - LinesSortiert[Liniennummer+1][3];
                float Ankath_neu = LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer+1][0];
                int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                NachThetaSortiert[Liniennummer+1][0] = Theta_neu;
                NachThetaSortiert[Liniennummer+1][1] = Mittelp_x_neu;
                NachThetaSortiert[Liniennummer+1][2] = Mittelp_y_neu;
                NachThetaSortiert[Liniennummer+1][3] = laenge_neu;

                // Neue Linie in "Linienfusioniert" schreiben
                Linienfusioniert.push_back(Vec4i( Theta_neu , Mittelp_x_neu , Mittelp_y_neu , laenge_neu ));
            }

            // Wenn rechter Eckpunkt der aktuellen Linie [Liniennummer] mit den linken Endpunkt der vorigen Linie [Liniennummer-1] übereinstimmt (Zusammenstückelung einer Kante aus mehreren Linien)
            //
            // Liniennummer+1           Liniennummer
            // o--------------------o o-------------o
            else if(sqrt( pow( LinesSortiert[Liniennummer+1][2] - LinesSortiert[Liniennummer][0] , 2 ) + pow( LinesSortiert[Liniennummer+1][3] - LinesSortiert[Liniennummer][1] , 2 ) ) <= 10 )
            {
                LinesSortiert[Liniennummer+1][0] = LinesSortiert[Liniennummer+1][0]; // x1_neu = x1 der folgenden Linie [Liniennummer+1]
                LinesSortiert[Liniennummer+1][1] = LinesSortiert[Liniennummer+1][1]; // y1_neu = y1 der folgenden Linie [Liniennummer+1]
                LinesSortiert[Liniennummer+1][2] = LinesSortiert[Liniennummer][2]; // x2_neu = x2 der aktuellen Linie [Liniennummer]
                LinesSortiert[Liniennummer+1][3] = LinesSortiert[Liniennummer][3]; // y2_neu = y2 der aktuellen Linie [Liniennummer]

                // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                float Gegenkath_neu = LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3];
                float Ankath_neu = LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0];
                int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                NachThetaSortiert[Liniennummer+1][0] = Theta_neu;
                NachThetaSortiert[Liniennummer+1][1] = Mittelp_x_neu;
                NachThetaSortiert[Liniennummer+1][2] = Mittelp_y_neu;
                NachThetaSortiert[Liniennummer+1][3] = laenge_neu;

                // Neue Linie in "Linienfusioniert" schreiben
                Linienfusioniert.push_back(Vec4i( Theta_neu , Mittelp_x_neu , Mittelp_y_neu , laenge_neu ));
            }
            /*
                // Jetzt nur noch: Linien überschneiden sich, aber kein Endpunkt liegt nahe am anderen
                // o-------------------------o   aktuelle Linie
                //              o------------------------------o   vorige Linie
                // Dazu: Ermittlung, ob rechter Punkt der aktuellen Linie geringen Abstand zur vorigen Linie hat und gleichzeitig linker Punkt der vorigen Linie geringen Abstand zur aktuellen Linie hat
*/


            // Wenn Winkelbedingung erfüllt aber Linien trotzdem nicht fusioniert werden können (weil zu weit auseinander)
            // Dann soll die aktuelle Linie in Linienfusioniert übernommen werden
            else{ Linienfusioniert.push_back(Vec4i( NachThetaSortiert[Liniennummer][0] , NachThetaSortiert[Liniennummer][1] , NachThetaSortiert[Liniennummer][2] , NachThetaSortiert[Liniennummer][3] ));
                Linienfusioniert2.push_back(Vec4i( NachThetaSortiert[Liniennummer][0] , NachThetaSortiert[Liniennummer][1] , NachThetaSortiert[Liniennummer][2] , NachThetaSortiert[Liniennummer][3] ));


                //myfile << Linienfusioniert[Liniennummer][0] << " " << Linienfusioniert[Liniennummer][1] << " " << Linienfusioniert[Liniennummer][2] << " " << Linienfusioniert[Liniennummer][3] <<std::endl;
            }

        }


        // Wenn Linie nicht fusioniert werden (weil Winkeldifferenz zu groß) kann werden Einträge einfach von "NachThetaSortiert" nach "Linienfusioniert" kopiert
        else
        {
            Linienfusioniert.push_back(Vec4i( NachThetaSortiert[Liniennummer][0] , NachThetaSortiert[Liniennummer][1] , NachThetaSortiert[Liniennummer][2] , NachThetaSortiert[Liniennummer][3] ));
            Linienfusioniert2.push_back(Vec4i( NachThetaSortiert[Liniennummer][0] , NachThetaSortiert[Liniennummer][1] , NachThetaSortiert[Liniennummer][2] , NachThetaSortiert[Liniennummer][3] ));

            //myfile << Linienfusioniert[Liniennummer][0] << " " << Linienfusioniert[Liniennummer][1] << " " << Linienfusioniert[Liniennummer][2] << " " << Linienfusioniert[Liniennummer][3] <<std::endl;
        }

    }

    // Linienanzahl begrenzen, um Anzahl der zu berechnenden Kombinationen zu beschränken
    int maxAnzahlLinien = 12;
    if (Linienfusioniert2.size() > maxAnzahlLinien)
    {
        std::vector<int> Laengenvektor,Indexvektor;

        // Die Werte für die Linienlängen werden in den Vektor "Laengenvektor" kopiert
        for(int i=0; i < Linienfusioniert2.size(); i++){
            Laengenvektor.push_back(Linienfusioniert2[i][3]);
        }

        // Jedes Element aus "Laengenvektor" bekommt einen Index zugewiesen, der seiner Größe (Linienlänge) entspricht (die längste Linie bekommt index 1)
        for(int i = 0; i < maxAnzahlLinien; i++){
            int max_index = std::max_element(Laengenvektor.begin(), Laengenvektor.end()) - Laengenvektor.begin();
            Indexvektor.push_back(max_index);

            // anschließend wird maximales Element in Längenvektor 0 gesetzt, damit neues maximales berechnet werden kann
            Laengenvektor[max_index] = 0;
        }

        // Linienfusioniert2 zeilenweise durchgehen und in Textdatei schreiben falls die entsprechende Zeilennummer im Indexvektor enthalten ist
        for(int i = 0; i < Linienfusioniert2.size(); i++){

            // Wenn der Indexvektor das Element i enthält (also die aktuelle Linienlänge zu den längsten x Linien gehört wird sie in die Textdatei geschrieben
        if(std::find(Indexvektor.begin(), Indexvektor.end(), i) != Indexvektor.end()) {
            myfile << Linienfusioniert2[i][0] << " " << Linienfusioniert2[i][1] << " " << Linienfusioniert2[i][2] << " " << Linienfusioniert2[i][3] <<std::endl;
        }
        }
    }
    cv::imwrite("Lines.jpg",cdst);// DANACH AUSKOMMENTIEREN


    //                }
    //            }
    //        }
    //    }

    myfile.close();

    return 0;
}
