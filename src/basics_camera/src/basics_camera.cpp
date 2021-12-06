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
string input_image_topic, output_image_topic;
bool display_input_image, display_output_image;
int screenmaxx;

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

    //Mostrar la imagen original en una ventana
	  if (display_input_image == true){
	    imshow("Input Image", cv_ptr->image);
	  }
    //Mostrar la imagen de salida en una ventana
    if (display_output_image == true){
    	imshow( "Output Image", cv_ptr->image);
     }

    //Publicamos la imagen de salida
    image_pub_.publish(cv_ptr->toImageMsg());

    waitKey(30);
}

void ReadAndShow(){
  
  ros::NodeHandle nh_;
  image_transport::ImageTransport it_(nh_);

  //topico de imagenes de entrada
  input_image_topic = "/raspicam_node/image";
  //topico de imagenes de salida
  output_image_topic = "/face_detector/raw_image";
  //booleano para abrir la ventana con la imagen original
  display_input_image = true;
  //booleano para abrir la ventana con la imagen de salida  
  display_output_image = true;

  screenmaxx = 640;

  try{
    nh_.getParam("image_input_topic", input_image_topic);
    nh_.getParam("face_detected_image_topic", output_image_topic);
    nh_.getParam("display_original_image", display_input_image);
    nh_.getParam("display_tracking_image", display_output_image);
    nh_.getParam("screenmaxx", screenmaxx);

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
  ReadAndShow();
  ros::spin();
  return 0;
}