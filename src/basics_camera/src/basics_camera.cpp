//ROS headers
#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

//Open-CV headers
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/objdetect.hpp"

//OpenCV window name
static const std::string OPENCV_WINDOW = "raw_image_window";
static const std::string OPENCV_WINDOW_1 = "face_detector";


using namespace std;
using namespace cv;


image_transport::Subscriber image_sub_;
image_transport::Publisher image_pub_;
string input_image_topic, output_image_topic,haar_file_face;
bool display_input_image, display_output_image,face_tracking;
int screenmaxx,center_offset;

//funcion de deteccion de caras
void Detection( Mat& img, CascadeClassifier& cascade)
{
    double t = 0;
    double scale = 1;
    vector<Rect> faces, faces2;
    Mat gray, smallImg;
    //convierte la imagen de colores a escala de grises
    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale ;
    //redimensionamos la imagen con una interpolacion bilineal
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR );
    //genera una ecualizacion de un histograma, para mejorar el contraste y brillo
    equalizeHist( smallImg, smallImg );

    //generamos una deteccion de caras sobre la imagen de entrada
    cascade.detectMultiScale( smallImg, faces,1.1, 15, 0|CASCADE_SCALE_IMAGE,Size(30, 30) );

    //mostramos las coordenadas de la cara
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        //se toma una cara
        Rect r = faces[i];
        int x,y;

        double aspect_ratio = (double)r.width/r.height;

        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
          x = cvRound((r.x + r.width*0.5)*scale);
          y = cvRound((r.y + r.height*0.5)*scale);

          ROS_INFO("X: %d - Y: %d - Face: %d",x,y,i);
        }
    }
}

//funcion callback del topico imagen
void imageCb(const sensor_msgs::ImageConstPtr& msg){

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

    //Cargamos el clasificador de cascada
    string cascadeName = haar_file_face;
    CascadeClassifier cascade;
	  if( !cascade.load( cascadeName ) )
	  {
		  ROS_ERROR("ERROR: Could not load classifier cascade");
      return;
	  }

    //Mostrar la imagen original en una ventana
	  if (display_input_image == true){
	    imshow("Input Image", cv_ptr->image);
	  }
    Detection(cv_ptr->image,cascade);
    //Mostrar la imagen de salida en una ventana
    if (display_output_image == true){
    	imshow( "Output Image", cv_ptr->image);
     }

    //Publicamos la imagen de salida
    image_pub_.publish(cv_ptr->toImageMsg());

    waitKey(30);
}

//funcion de suscripcion y publicacion de topicos y carga de parametros
void SubAndPub(){
  
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_(nh_);

  //topico de imagenes de entrada
  input_image_topic = "/raspicam_node/image";
  //topico de imagenes de salida
  output_image_topic = "/face_detector/raw_image";
  //direccion de algoritmo de rastreo de caras
  haar_file_face= "/home/ubuntu/catkin_ws/src/face_tracker_pkg/src/face.xml"; 
  //booleano para abrir la ventana con la imagen original
  display_input_image = true;
  //booleano para abrir la ventana con la imagen de salida  
  display_output_image = true;
  //tama  o maximo de la imagen?
  screenmaxx = 640;
  //offset de la cara al centro de la imagen
  center_offset=100;

  //Cargamos los parametros necesarios
  try{
    nh_.getParam("image_input_topic", input_image_topic);
    nh_.getParam("face_detected_image_topic", output_image_topic);
    nh_.getParam("display_original_image", display_input_image);
    nh_.getParam("display_tracking_image", display_output_image);
    nh_.getParam("center_offset", center_offset);
    nh_.getParam("screenmaxx", screenmaxx);

    nh_.getParam("haar_file_face", haar_file_face);
    nh_.getParam("face_tracking", face_tracking);
    

    ROS_INFO("Successfully Loaded tracking parameters");
  }

  catch(int e)
  {
    ROS_WARN("Parameters are not properly loaded from file");
  }

  //Suscribimos el topico de la imagen de entrada
  image_sub_ = it_.subscribe(input_image_topic, 1, imageCb);
  //Publicamos el topico de la imagen de salida
  image_pub_ = it_.advertise(output_image_topic, 1);

}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "basics_camera");
  SubAndPub();
  ros::spin();
  return 0;
}