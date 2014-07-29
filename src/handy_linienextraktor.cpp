#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
    Mat Handybild_original, Handybild, dst, cdst;
    char* Bild = "Handybild_flaechen.JPG";
    Handybild_original = imread(Bild, CV_LOAD_IMAGE_COLOR);  // Read the file
    // Bildgröße verringern
    resize(Handybild_original, Handybild, Size(), 0.5, 0.5);


    if(! Handybild.data )                              // Check for invalid input
    {
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    int threshold1 = 60;
    int threshold2 = 40;
    int threshold3 = 3;
    namedWindow( "source",WINDOW_NORMAL);
    namedWindow( "detected lines",WINDOW_NORMAL);
    namedWindow("Binär",WINDOW_NORMAL);
    cv::createTrackbar("Canny1","Binär",&threshold1,100,NULL);
    cv::createTrackbar("Canny2","Binär",&threshold2,100,NULL);
        std::vector<Vec4i> lines;
    char c;

    while(c != 'q' && c != 's'){
        c = (char)waitKey(2);
        cv::Canny(Handybild,dst,threshold1,threshold2,3);
        cv::cvtColor(dst,cdst,CV_GRAY2BGR);

        cout<<c<<endl;


        HoughLinesP(dst, lines, 1, CV_PI/360, 40, 15, 50 );

        for( size_t i = 0; i < lines.size(); i++ )
        {
//            Vec4i l = lines[i];
            Vec4i l1 = lines[i];
            line( cdst, Point(l1[0], l1[1]), Point(l1[2], l1[3]), Scalar(0,0,255), 1, CV_AA);

            Point p1;
            //p1.x=
            //circle(cdst,)
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
        }

       // std::cout<<"Dimension von lines: "<<lines.size()<<std::endl;


        imshow("source", Handybild);
        imshow("detected lines", cdst);
        imshow("Binär",dst);
    }

    if(c=='s'){
        std::ofstream myfile;//Eigenschaften in Textdatei schreiben
        myfile.open("Handylinien.txt");

        cv::Point Mittelpunkt;
        float Gegenkathete, Ankathete, Theta, laenge;

        for( size_t i = 0; i < lines.size(); i++ )
        {
            Vec4i l = lines[i];
//            line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, CV_AA);
            //Eigenschaften berechnen:
            Mittelpunkt.x=cvRound((l[0]+l[2])/2);//Berechnung x-Koordinate des Mittelpunkts
            Mittelpunkt.y=cvRound((l[1]+l[3])/2);//  ""       y-koordinate
            cv::circle(cdst,Mittelpunkt,5,Scalar(0,255,0),2,CV_AA);
            Gegenkathete=((l[1]-l[3]));
            Ankathete=((l[2]-l[0]));
            std::cout << l[0] <<","<< l[1 ]<< "," << l[2] << "," << l[3] << std::endl;
            Theta=cvRound(atan(Gegenkathete/Ankathete)*360/(2*CV_PI));
            //Theta=(CV_PI/2-Theta)*360/(2*CV_PI);
            laenge=cvRound(sqrt(Ankathete*Ankathete+Gegenkathete*Gegenkathete));
            // Mittelpunkt x, Mittelpunkt y, Theta, Länge
            myfile << Mittelpunkt.x << " " << Mittelpunkt.y << " " << Theta << " " << laenge << std::endl;
        }
        cout<<"Bildgröße="<< Handybild.rows << "x" << Handybild.cols <<endl;
        myfile.close();
    }

    waitKey();
}
