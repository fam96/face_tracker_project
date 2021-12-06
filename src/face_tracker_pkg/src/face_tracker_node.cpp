/*
 * Copyright (C) 2017, Lentin Joseph and Qbotics Labs Inc.
 * Email id : qboticslabs@gmail.com
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the names of Stanford University or Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
* This code will track the faces using ROS 
*/



//ROS headers
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

//Open-CV headers
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect.hpp"

//Centroid message headers
#include <face_tracker_pkg/centroid.h>

//OpenCV window name
static const std::string OPENCV_WINDOW = "raw_image_window";
static const std::string OPENCV_WINDOW_1 = "face_detector";


using namespace std;
using namespace cv;

class Face_Detector
{

  //privado------------------------------------------
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher image_pub_;
 //publica la posicion de la cara con coordenadas x,y
  ros::Publisher face_centroid_pub;
//mensaje con coordenadas x-y
  face_tracker_pkg::centroid face_centroid;

  string input_image_topic, output_image_topic, haar_file_face;
  int face_tracking, display_original_image, display_tracking_image, center_offset, screenmaxx;

  //publico-------------------------------------------
public:
//constructor que hace que it_ sea igual a nh_
  Face_Detector()
    : it_(nh_)
  {


  //Loading Default values

  //topico donde esta las imagenes de entrada
  input_image_topic = "/usb_cam/image_raw";
  //topico donde esta las imagenes de salida
  output_image_topic = "/face_detector/raw_image";
  haar_file_face = "/home/robot/face.xml";
  face_tracking = 1;
  display_original_image = 1;
  display_tracking_image = 1;
  screenmaxx = 640;
  //offset entre el centro y la posicion de la cara
  center_offset = 100;

  //Accessing parameters from track.yaml

  try{
  nh_.getParam("image_input_topic", input_image_topic);
  nh_.getParam("face_detected_image_topic", output_image_topic);
  nh_.getParam("haar_file_face", haar_file_face);
  nh_.getParam("face_tracking", face_tracking);
  nh_.getParam("display_original_image", display_original_image);
  nh_.getParam("display_tracking_image", display_tracking_image);
  nh_.getParam("center_offset", center_offset);
  nh_.getParam("screenmaxx", screenmaxx);

  ROS_INFO("Successfully Loaded tracking parameters");
  }

  catch(int e)
  {
   
      ROS_WARN("Parameters are not properly loaded from file, loading defaults");
	
  }

    // Subscribe to input video feed and publish output video feed
    image_sub_ = it_.subscribe(input_image_topic, 1, 
      &Face_Detector::imageCb, this);
    image_pub_ = it_.advertise(output_image_topic, 1);
   
    face_centroid_pub = nh_.advertise<face_tracker_pkg::centroid>("/face_centroid",10);


  }
  //destructor que destruye las ventanas de imagenes de entrada y salida
  ~Face_Detector()
  {
    if( display_original_image == 1 or display_tracking_image == 1)
    	cv::destroyWindow(OPENCV_WINDOW);
  }
  //funcion callback que intenta convertir la imagen dentro del topico de la camara, en un formato
  //aceptado por opencv
  void imageCb(const sensor_msgs::ImageConstPtr& msg)
  {

    cv_bridge::CvImagePtr cv_ptr;
    namespace enc = sensor_msgs::image_encodings;
    //trata de convetir la imagen en un formato para opencv
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    //Si no lo logra, te muestra el error
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }


	    string cascadeName = haar_file_face;
            CascadeClassifier cascade;
	    if( !cascade.load( cascadeName ) )
	    {
		cerr << "ERROR: Could not load classifier cascade" << endl;
		
	    }

      //Mostrar la imagen original en una ventana
	    if (display_original_image == 1){
		imshow("Original Image", cv_ptr->image);
	    }

            detectAndDraw( cv_ptr->image, cascade );
            //publicamos la imagen de salida con las marcas y detecciones
            image_pub_.publish(cv_ptr->toImageMsg());

            waitKey(30);
  
}
 //funcion de deteccion de caras
void detectAndDraw( Mat& img, CascadeClassifier& cascade)
{
    double t = 0;
    double scale = 1;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;
    //convierte la imagen de colores a escala de grises
    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale ;
    //redimensionamos la imagen con una interpolacion bilineal
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    //genera una ecualizacion de un histograma, para mejorar el contraste y brillo
    equalizeHist( smallImg, smallImg );

    t = (double)cvGetTickCount();
    //generamos una deteccion de caras sobre la imagen de entrada
    cascade.detectMultiScale( smallImg, faces,
        1.1, 15, 0
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
   
    t = (double)cvGetTickCount() - t;
    //generamos las marcas en la imagen de salida
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        //se toma una cara
        Rect r = faces[i];
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        //dibujamos un circulo sobre la cara
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );

   	    face_centroid.x = center.x;
   	    face_centroid.y = center.y;

  
           
        //publicamos un mensaje con la posicion x-y de la cara
  	    face_centroid_pub.publish(face_centroid);

        }
        else
        //y si no se da el aspect_ratio, entonces lo encerramos con un rectangulo
            rectangle( img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)),
                       cvPoint(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);

    }

    //Añadimos lineas verticales para definir el centro, y los limites izquierdo y derecho

    Point pt1, pt2,pt3,pt4,pt5,pt6;

    //Linea central
    pt1.x = screenmaxx / 2;
    pt1.y = 0;
 
    pt2.x = screenmaxx / 2;
    pt2.y = 480;


    //Limite izquierdo
    pt3.x = (screenmaxx / 2) - center_offset;
    pt3.y = 0;

    pt4.x = (screenmaxx / 2) - center_offset;
    pt4.y = 480;

    //LImite derecho
    pt5.x = (screenmaxx / 2) + center_offset;
    pt5.y = 0;

    pt6.x = (screenmaxx / 2) + center_offset;
    pt6.y = 480;


    line(img,  pt1,  pt2, Scalar(0, 0, 255),0.2);
    line(img,  pt3,  pt4, Scalar(0, 255, 0),0.2);
    line(img,  pt5,  pt6, Scalar(0, 255, 0),0.2);

    //escribimos el la palabra "Izquierda", "Derecha" o "Centro", segun corresponda
    putText(img, "Left", cvPoint(50,240), FONT_HERSHEY_SIMPLEX, 1, cvScalar(255,0,0), 2, CV_AA);
    putText(img, "Center", cvPoint(280,240), FONT_HERSHEY_SIMPLEX, 1, cvScalar(0,0,255), 2, CV_AA);
    putText(img, "Right", cvPoint(480,240), FONT_HERSHEY_SIMPLEX, 1, cvScalar(255,0,0), 2, CV_AA);

    if (display_tracking_image == 1){
      //mostramos la imagen de salida
    	imshow( "Face tracker", img );
     }

}


 
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "Face tracker");
  Face_Detector ic;
  ros::spin();
  return 0;
}
