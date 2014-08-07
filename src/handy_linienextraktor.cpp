#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    Mat Handybild_original, Handybild, dst, cdst;

    // Handybild einlesen
    Handybild_original = imread("Handybild_flaechen.JPG", CV_LOAD_IMAGE_COLOR);

    // Bildgröße um Faktor 0.5 verringern
    resize(Handybild_original, Handybild, Size(), 0.5, 0.5);

    // Auf ungültigen Input checken
    if(! Handybild.data )
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    // Parameter für Canny definieren
    int threshold1 = 60;
    int threshold2 = 40;

    // Trackbars für die beiden Parameter erzeugen
    cv::createTrackbar("Canny1","Binär",&threshold1,100,NULL);
    cv::createTrackbar("Canny2","Binär",&threshold2,100,NULL);

    // Vektor definieren, in dem die Endpunkte der Linien aus HoughLinesP gespeichert werden
    std::vector<Vec4i> lines;

    // c wird durch eine Tastatureingabe definiert
    char c;

    // So lange die Tastatureingabe nicht "s" = save oder "q" = quit ist läuft die Schleife und ermöglicht das Regeln der Trackbars
    while(c != 'q' && c != 's'){

        c = (char)waitKey(2);

        // Canny Kantendetektor
        cv::Canny(Handybild,dst,threshold1,threshold2,3);
        cv::cvtColor(dst,cdst,CV_GRAY2BGR);

        // HoughLinesP bestimmt die Linienparameter und speichert sie in "lines" ab
        HoughLinesP(dst, lines, 1, CV_PI/360, 40, 15, 50 );

        // Schleife über alle Linien
        for( size_t i = 0; i < lines.size(); i++ )
        {
            // Linien in das Bild cdst einzeichnen
            Vec4i l1 = lines[i];
            line( cdst, Point(l1[0], l1[1]), Point(l1[2], l1[3]), Scalar(0,0,255), 1, CV_AA);

            /*
            //            Point p1;
            //            p1.x=
            //            circle(cdst,)
            //            //Eigenschaften berechnen:
            //            Mittelpunkt.x=cvRound((l[0]+l[2])/2);//Berechnung x-Koordinate des Mittelpunkts
            //            Mittelpunkt.y=cvRound((l[1]+l[3])/2);//  ""       y-koordinate
            //            cv::circle(cdst,Mittelpunkt,5,Scalar(0,255,0),2,CV_AA);
            //            Gegenkathete=((l[1]-l[3]));
            //            Ankathete=((l[2]-l[0]));
            //            std::cout<<l[0]<<","<<l[1]<<","<<l[2]<<","<<l[3]<<std::endl;
            //            Theta=atan(Gegenkathete/Ankathete)*360/(2*CV_PI);
            //            //Theta=(CV_PI/2-Theta)*360/(2*CV_PI);
            //            laenge=cvRound(sqrt(Ankathete*Ankathete+Gegenkathete*Gegenkathete));

            //            myfile<<"Mittelpunkt "<<i<<": "<<Mittelpunkt.x<<", "<<Mittelpunkt.y<<"; Theta: " << Theta << "; Länge: " << laenge << ";" <<std::endl;
            */
        }

        // Anzeigen des Originalbildes, des Binärbildes und des Binärbildes mit überlagerten Linien
        namedWindow( "source",WINDOW_NORMAL);
        namedWindow( "detected lines",WINDOW_NORMAL);
        namedWindow("Binär",WINDOW_NORMAL);

        imshow("source", Handybild);
        imshow("detected lines", cdst);
        imshow("Binär",dst);
    }

    // Wenn "s" = save gedrückt dann werden die Linienparameter umgerechnet und abgespeichert
    if(c=='s'){

        // Stream erzeugen, um eine Textdatei schreiben zu können
        std::ofstream myfile;
        myfile.open("Handylinien.txt");

        // Neue Parameter, in welche die vorigen Parameter (Start- und Endpunkt) umgerechnet werden sollen
        cv::Point Mittelpunkt;
        float Gegenkathete, Ankathete, Theta, laenge;

        // Vektoren, in den die umgerechneten Linienparameter (Theta in Thetavektor, übrige Parameter in umgerechneteParameter) gespeichert werden
        std::vector<int> Thetavektor;
        std::vector<int> umgerechneteParameter;

        // Schleife über alle detektierten Linien
        for( size_t i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];

            // Linienparameter von x1,y1 und x2,y2 in Theta, x-Mittelpunkt, y-Mittelpunkt und Länge umrechnen
            Mittelpunkt.x=cvRound((l[0]+l[2])/2);
            Mittelpunkt.y=cvRound((l[1]+l[3])/2);
            Gegenkathete=((l[1]-l[3]));
            Ankathete=((l[2]-l[0]));
            std::cout << l[0] <<","<< l[1 ]<< "," << l[2] << "," << l[3] << std::endl;
            Theta=cvRound(atan(Gegenkathete/Ankathete)*360/(2*CV_PI));
            laenge=cvRound(sqrt(Ankathete*Ankathete+Gegenkathete*Gegenkathete));

            // Mittelpunkte in cdst "malen"
            cv::circle(cdst,Mittelpunkt,7,Scalar(0,255,0),1,CV_AA);

            // Endpunkte in cdst "malen"
            cv::Point Endpunkt1, Endpunkt2;
            Endpunkt1.x=lines[i][0], Endpunkt1.y = lines[i][1];
            Endpunkt2.x = lines[i][2], Endpunkt2.y = lines[i][3];
            cv::circle(cdst,Endpunkt1,5,Scalar(200,55,0),1,CV_AA);
            cv::circle(cdst,Endpunkt2,5,Scalar(200,55,0),1,CV_AA);

            // Thetawert für aktuelle Linie wird in "Thetavektor" geschrieben, andere Parameter in "umgerechneteParameter"
            // Das Ganze dient dazu, die Linien aufsteigend nach Theta zu sortieren
            Thetavektor.push_back(Theta);
            umgerechneteParameter.push_back(Mittelpunkt.x);
            umgerechneteParameter.push_back(Mittelpunkt.y);
            umgerechneteParameter.push_back(laenge);

        }

        std::vector<Vec4i> NachThetaSortiert;
        // Vektor Lines nach der gleichen Vorschrift sortiert wie "NachThetasortiert", damit die Container zusammenpassen!
        std::vector<Vec4i> LinesSortiert;
        // Endgültiger Vektor (Nach Theta sortiert und Linien fusioniert)
        std::vector<Vec4i> Linienfusioniert;

        // Schleife über alle Einträge von "lines"
        for(int Liniennummer = 0; Liniennummer < lines.size(); Liniennummer++)
        {
            // In "Thetavektor" nach kleinstem Theta suchen und zugehörigen Index in "min_index" speichern
            int min_index = std::min_element(Thetavektor.begin(), Thetavektor.end()) - Thetavektor.begin();
            cout << "Index für kleinstes Element: " << min_index << endl;

            // Kleinster Thetawert wird in Vektor "NachThetaSortiert" geschrieben zusammen mit den zugehörigen Parametern
            NachThetaSortiert[Liniennummer][0] = Thetavektor[min_index];
            NachThetaSortiert[Liniennummer][1] = umgerechneteParameter[min_index*3];
            NachThetaSortiert[Liniennummer][2] = umgerechneteParameter[(min_index*3)+1];
            NachThetaSortiert[Liniennummer][3] = umgerechneteParameter[(min_index*3)+2];

            // Gleichzeitig: Sortieren des Vektors "lines" (gleiche Sortiervorschrift wie "NachThetaSortiert" (Sortiert abspeichern in Vektor "LinesSortiert"))
            LinesSortiert[Liniennummer][0] = lines[min_index][0];
            LinesSortiert[Liniennummer][1] = lines[min_index][1];
            LinesSortiert[Liniennummer][2] = lines[min_index][2];
            LinesSortiert[Liniennummer][3] = lines[min_index][3];

            //            // Kleinster Thetawert wird in Textdatei geschrieben zusammen mit den zugehörigen Parametern
            //            myfile << Thetavektor[min_index] << " " << umgerechneteParameter[min_index*3] << " " << umgerechneteParameter[(min_index*3) + 1] << " " << umgerechneteParameter[(min_index*3) + 2] <<std::endl;

            // Kleinsten Eintrag aus Thetavektor und zugehörige Parameter aus umgerechneteParameter löschen damit neuer kleinster Eintrag berechnen kann
            Thetavektor.erase(Thetavektor.begin() + min_index);
            umgerechneteParameter.erase(umgerechneteParameter.begin() + min_index*3, umgerechneteParameter.begin() + min_index*3 + 3);
        }

        // Jetzt sind alle Linien im Vektor "NachThetaSortiert" aufsteigend nach Theta sortiert mit den jeweils zugehörigen Parametern
        // x-Mittelpunkt, y-Mittelpunkt und Länge
        // Jetzt Vektor durchlaufen und nach Linien suchen, die sich aus mehreren Linien zusammensetzen
        // Aktuelle Linie mit der vorigen auf Gemeinsamkeiten überprüfen (daher erst bei der zweiten Linie anfangen)
        for(int Liniennummer = 1; Liniennummer < NachThetaSortiert.size(); Liniennummer++)
        {
            // Wenn Theta der aktuellen Linie +-2° mit der vorigen Linie übereinstimmt: Prüfen, ob die beiden Linien zu einer "fusioniert" werden können
            if(NachThetaSortiert[Liniennummer][0] <= (NachThetaSortiert[Liniennummer-1][0]+2) && NachThetaSortiert[Liniennummer][0] >= (NachThetaSortiert[Liniennummer-1][0]-2))
            {
                // Praktisch: HoughLinesP sortiert die Parameter so: x1,y1,x2,y2 wobei x1 das Ende mit kleinerem x-Wert (weiter links) ist
                // Wenn beide Endpunkte in einem Radius von 10 Pixeln liegen:
                //
                // o----------------------o
                //  o----------------------o
                //
                // Abstand <=10 = Δx²+Δy² (für jeweils beide Endpunkte der Linien)
                int deltax1 = LinesSortiert[Liniennummer][0] - LinesSortiert[(Liniennummer-1)][0];
                int deltay1 = LinesSortiert[Liniennummer][1] - LinesSortiert[(Liniennummer-1)][1];
                if(sqrt(pow(deltax1 , 2) + pow(deltay1 , 2) ) <= 10
                        && sqrt( pow( (LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer-1][2]) , 2 ) + pow( (LinesSortiert[Liniennummer][3] - LinesSortiert[(Liniennummer-1)][3]) , 2 ) ) <= 10)
                {
                    // Die beiden Linien fusionieren und neue Endpunkte in LinesSortiert[Liniennummer]schreiben (und LinesSortiert[Liniennummer-1] löschen)
                    LinesSortiert[Liniennummer][0] = cvRound((LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer-1][0]) / 2); // x1_Neu (Mittelwert)
                    LinesSortiert[Liniennummer][1] = cvRound((LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer-1][1]) / 2); // y1_Neu (Mittelwert)
                    LinesSortiert[Liniennummer][2] = cvRound((LinesSortiert[Liniennummer][2] + LinesSortiert[Liniennummer-1][2]) / 2); // x2_Neu (Mittelwert)
                    LinesSortiert[Liniennummer][3] = cvRound((LinesSortiert[Liniennummer][3] + LinesSortiert[Liniennummer-1][3]) / 2); // y2_Neu (Mittelwert)

                    // Wie löschen??? LinesSortiert[Liniennummer][0...3].delete ?????

                    // Die fusionierte Linie in Theta, Mittelpunkt und Länge-Parameter umrechnen und die entsprechende Zeile in "NachThetaSortiert" durch die neuen Parameter ersetzen
                    // Zusätzlich Einträge in Vektor Linienfusioniert speichern (endgültiger Vektor, der anschließend in Textdatei geschrieben wird
                    int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                    int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                    int Gegenkath_neu = cvRound( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] );
                    int Ankath_neu = cvRound( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] );
                    int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                    int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                    // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                    NachThetaSortiert[Liniennummer][0] = Theta_neu;
                    NachThetaSortiert[Liniennummer][1] = Mittelp_x_neu;
                    NachThetaSortiert[Liniennummer][2] = Mittelp_y_neu;
                    NachThetaSortiert[Liniennummer][3] = laenge_neu;

                    // Neue Linie in "Linienfusioniert" schreiben
                    Linienfusioniert[Liniennummer][0] = Theta_neu;
                    Linienfusioniert[Liniennummer][1] = Mittelp_x_neu;
                    Linienfusioniert[Liniennummer][2] = Mittelp_y_neu;
                    Linienfusioniert[Liniennummer][3] = laenge_neu;
                }

                // Wenn nur jeweils linke Endpunkte übereinstimmen (Überlappung)
                //
                // o----------------------------o Liniennummer-1
                // o---------------o Liniennummer
                else if(sqrt(pow(LinesSortiert[Liniennummer][0] - LinesSortiert[Liniennummer-1][0] , 2) + pow(LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer-1][1] , 2) ) <= 10)
                {
                    // Linker Endpunkt der neuen Linie kann schon berechnet werden (gemittelt)
                    LinesSortiert[Liniennummer][0] = cvRound((LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer-1][0]) / 2); // x1_Neu (Mittelwert)
                    LinesSortiert[Liniennummer][1] = cvRound((LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer-1][1]) / 2); // y1_Neu (Mittelwert)

                    // checken welcher der beiden rechten Endpunkte den größeren Abstand zum neuen linken Endpunkt hat
                    // Wenn der Endpunkt der Linie [Liniennummer-1] einen größeren Abstand hat dann stellt ihr Endpunkt den Endpunkt der neuen Linie dar:
                    if( sqrt( pow(LinesSortiert[Liniennummer][0] - LinesSortiert[Liniennummer-1][2] , 2) + pow(LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer-1][3] , 2))
                            >= sqrt( pow(LinesSortiert[Liniennummer][0] - LinesSortiert[Liniennummer][2] , 2) + pow(LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] , 2)))
                    {
                        LinesSortiert[Liniennummer][2] = LinesSortiert[Liniennummer-1][2];
                        LinesSortiert[Liniennummer][3] = LinesSortiert[Liniennummer-1][3];
                    }

                    // Wenn der Endpunkt der aktuelle Linie [Liniennummer] einen größeren Abstand aufweist bleiben die Einträge für x2 und y2 unverändert!

                    // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                    int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                    int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                    int Gegenkath_neu = cvRound( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] );
                    int Ankath_neu = cvRound( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] );
                    int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                    int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                    // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                    NachThetaSortiert[Liniennummer][0] = Theta_neu;
                    NachThetaSortiert[Liniennummer][1] = Mittelp_x_neu;
                    NachThetaSortiert[Liniennummer][2] = Mittelp_y_neu;
                    NachThetaSortiert[Liniennummer][3] = laenge_neu;

                    // Neue Linie in "Linienfusioniert" schreiben
                    Linienfusioniert[Liniennummer][0] = Theta_neu;
                    Linienfusioniert[Liniennummer][1] = Mittelp_x_neu;
                    Linienfusioniert[Liniennummer][2] = Mittelp_y_neu;
                    Linienfusioniert[Liniennummer][3] = laenge_neu;
                }

                // Wenn nur jeweils rechte Endpunkte übereinstimmen (Überlappung)
                //
                // o----------------------------o Liniennummer-1
                //              o---------------o Liniennummer
                else if(sqrt ( pow( (LinesSortiert[Liniennummer][2] - LinesSortiert[(Liniennummer-1)][2]) , 2) + pow( (LinesSortiert[Liniennummer][3] - LinesSortiert[(Liniennummer-1)][3]) , 2) ) <= 10 )
                {
                    // Rechter Endpunkt der neuen Linie kann schon berechnet werden (gemittelt)
                    LinesSortiert[Liniennummer][2] = cvRound((LinesSortiert[Liniennummer][2] + LinesSortiert[(Liniennummer-1)][2]) / 2); // x2_Neu (Mittelwert)
                    LinesSortiert[Liniennummer][3] = cvRound((LinesSortiert[Liniennummer][3] + LinesSortiert[Liniennummer-1][3]) / 2); // y2_Neu (Mittelwert)

                    // checken welcher der beiden linken Endpunkte den größeren Abstand zum neuen rechten Endpunkt hat
                    // Wenn der Endpunkt der Linie [Liniennummer-1] einen größeren Abstand hat dann stellt ihr Endpunkt den Endpunkt der neuen Linie dar:
                    if( sqrt( pow(LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer-1][0] , 2) + pow(LinesSortiert[Liniennummer][3] - LinesSortiert[Liniennummer-1][1] , 2))
                            >= sqrt( pow(LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] , 2) + pow(LinesSortiert[Liniennummer][3] - LinesSortiert[Liniennummer][1] , 2)))
                    {
                        LinesSortiert[Liniennummer][0] = LinesSortiert[Liniennummer-1][0];
                        LinesSortiert[Liniennummer][1] = LinesSortiert[Liniennummer-1][1];
                    }


                    // Wenn der Endpunkt der aktuelle Linie [Liniennummer] einen größeren Abstand aufweist bleiben die Einträge für x1 und y1 unverändert!

                    // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                    int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                    int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                    int Gegenkath_neu = cvRound( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] );
                    int Ankath_neu = cvRound( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] );
                    int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                    int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );
                }

                // Wenn rechter Endpunkt der aktuellen Linie [Liniennummer-1] mit dem linken Endpunkt der vorigen Linie [Liniennummer] übereinstimmt (Zusammenstückelung einer Kante aus mehreren Linien)
                // Liniennummer             Liniennummer-1
                // o--------------------o o-------------o
                else if(sqrt( pow( LinesSortiert[Liniennummer][0] - LinesSortiert[Liniennummer-1][2] , 2 ) + pow( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer-1][3] , 2 ) ) <= 10 )
                {
                    LinesSortiert[Liniennummer][0] = LinesSortiert[Liniennummer-1][0]; // x1_neu = x1 der vorigen Linie [Liniennummer-1]
                    LinesSortiert[Liniennummer][1] = LinesSortiert[Liniennummer-1][1]; // y1_neu = y1 der vorigen Linie [Liniennummer-1]
                    LinesSortiert[Liniennummer][2] = LinesSortiert[Liniennummer][2]; // x2_neu = x2 der aktuellen Linie [Liniennummer]
                    LinesSortiert[Liniennummer][3] = LinesSortiert[Liniennummer][3]; // y2_neu = y2 der aktuellen Linie [Liniennummer]

                    // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                    int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                    int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                    int Gegenkath_neu = cvRound( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] );
                    int Ankath_neu = cvRound( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] );
                    int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                    int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                    // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                    NachThetaSortiert[Liniennummer][0] = Theta_neu;
                    NachThetaSortiert[Liniennummer][1] = Mittelp_x_neu;
                    NachThetaSortiert[Liniennummer][2] = Mittelp_y_neu;
                    NachThetaSortiert[Liniennummer][3] = laenge_neu;

                    // Neue Linie in "Linienfusioniert" schreiben
                    Linienfusioniert[Liniennummer][0] = Theta_neu;
                    Linienfusioniert[Liniennummer][1] = Mittelp_x_neu;
                    Linienfusioniert[Liniennummer][2] = Mittelp_y_neu;
                    Linienfusioniert[Liniennummer][3] = laenge_neu;
                }

                // Wenn rechter Eckpunkt der aktuellen Linie [Liniennummer] mit den linken Endpunkt der vorigen Linie [Liniennummer-1] übereinstimmt (Zusammenstückelung einer Kante aus mehreren Linien)
                //
                // Liniennummer-1           Liniennummer
                // o--------------------o o-------------o
                else if(sqrt( pow( LinesSortiert[Liniennummer-1][0] - LinesSortiert[Liniennummer][2] , 2 ) + pow( LinesSortiert[Liniennummer-1][1] - LinesSortiert[Liniennummer][3] , 2 ) ) <= 10 )
                {
                    LinesSortiert[Liniennummer][0] = LinesSortiert[Liniennummer][0]; // x1_neu = x1 der aktuellen Linie [Liniennummer]
                    LinesSortiert[Liniennummer][1] = LinesSortiert[Liniennummer][1]; // y1_neu = y1 der aktuellen Linie [Liniennummer]
                    LinesSortiert[Liniennummer][2] = LinesSortiert[Liniennummer-1][2]; // x2_neu = x2 der vorigen Linie [Liniennummer-1]
                    LinesSortiert[Liniennummer][3] = LinesSortiert[Liniennummer-1][3]; // y2_neu = y2 der vorigen Linie [Liniennummer-1]

                    // Jetzt neues Theta, Mittelpunkte und Länge berechnen!
                    int Mittelp_x_neu = cvRound( (LinesSortiert[Liniennummer][0] + LinesSortiert[Liniennummer][2]) / 2 );
                    int Mittelp_y_neu = cvRound( (LinesSortiert[Liniennummer][1] + LinesSortiert[Liniennummer][3]) / 2 );
                    int Gegenkath_neu = cvRound( LinesSortiert[Liniennummer][1] - LinesSortiert[Liniennummer][3] );
                    int Ankath_neu = cvRound( LinesSortiert[Liniennummer][2] - LinesSortiert[Liniennummer][0] );
                    int Theta_neu = cvRound( atan(Gegenkath_neu/Ankath_neu)*360/(2*CV_PI) );
                    int laenge_neu = cvRound( sqrt(Ankath_neu*Ankath_neu + Gegenkath_neu*Gegenkath_neu) );

                    // Vektor "NachThetasortiert" mit neuer Linie aktualisieren für nächsten Schleifendurchlauf
                    NachThetaSortiert[Liniennummer][0] = Theta_neu;
                    NachThetaSortiert[Liniennummer][1] = Mittelp_x_neu;
                    NachThetaSortiert[Liniennummer][2] = Mittelp_y_neu;
                    NachThetaSortiert[Liniennummer][3] = laenge_neu;

                    // Neue Linie in "Linienfusioniert" schreiben
                    Linienfusioniert[Liniennummer][0] = Theta_neu;
                    Linienfusioniert[Liniennummer][1] = Mittelp_x_neu;
                    Linienfusioniert[Liniennummer][2] = Mittelp_y_neu;
                    Linienfusioniert[Liniennummer][3] = laenge_neu;
                }

                // Jetzt nur noch: Linien überschneiden sich, aber kein Endpunkt liegt nahe am anderen
                // o-------------------------o   aktuelle Linie
                //              o------------------------------o   vorige Linie
                // Dazu: Ermittlung, ob rechter Punkt der aktuellen Linie geringen Abstand zur vorigen Linie hat und gleichzeitig linker Punkt der vorigen Linie geringen Abstand zur aktuellen Linie hat






        }




        // Wenn Linie nicht fusioniert werden kann werden Einträge einfach von "NachThetaSortiert" nach "Linienfusioniert" kopiert
        else
        {
            Linienfusioniert[Liniennummer][0] = NachThetaSortiert[Liniennummer][0];
            Linienfusioniert[Liniennummer][1] = NachThetaSortiert[Liniennummer][1];
            Linienfusioniert[Liniennummer][2] = NachThetaSortiert[Liniennummer][2];
            Linienfusioniert[Liniennummer][3] = NachThetaSortiert[Liniennummer][3];
        }
    }

    imwrite("Handybild_detektierte_Linien.jpg",cdst);

    cout<<"Bildgröße="<< Handybild.rows << "x" << Handybild.cols <<endl;
    myfile.close();
}

waitKey();
}
