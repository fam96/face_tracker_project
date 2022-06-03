#include <ros/ros.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Int32.h>
#include <iostream>
#include "wiringPi.h"
#include <softTone.h>

#define DIR_PIN 23
#define STEP_PIN 24
#define ANG_STEP 0.1125

//Tracker parameters

int center_offset, center_pos,center_right,center_left,right,left;
int freq_slow=500,freq_fast=1000,t_move_slow=500,t_move_fast=250;
void track_face(int x)
{
	if(x>center_right && x<right){
		digitalWrite(DIR_PIN,HIGH);
		softToneWrite(STEP_PIN, freq_slow);
		delay(t_move_slow);
		softToneWrite(STEP_PIN,0);
		ROS_INFO("RIGHT");
	}
	else if(x>right){
        digitalWrite(DIR_PIN,HIGH);
        softToneWrite(STEP_PIN, freq_fast);
        delay(t_move_fast);
        softToneWrite(STEP_PIN,0);
        ROS_INFO("RIGHT-RIGHT");
    }
    else if(x<center_left && x>left){
        digitalWrite(DIR_PIN,LOW);
        softToneWrite(STEP_PIN, freq_slow);
        delay(t_move_slow);
        softToneWrite(STEP_PIN,0);
        ROS_INFO("LEFT");
    }
    else if(x<left){
        digitalWrite(DIR_PIN,LOW);
        softToneWrite(STEP_PIN, freq_fast);
        delay(t_move_fast);
        softToneWrite(STEP_PIN,0);
        ROS_INFO("LEFT-LEFT");
    }
    else{
        ROS_INFO("CENTER");
    }


}
//Callback of the topic /numbers
void x_position_callback(const std_msgs::Int32::ConstPtr& msg)
{
	ROS_INFO("Recieved X = [%d]",msg->data);
	
	//Calling track face function
	track_face(msg->data);
}

int main(int argc, char **argv)
{

	//Loading servo configurations of the dynamixel tracker
	wiringPiSetup();
	pinMode(STEP_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);
	int f=softToneCreate(STEP_PIN);

    ROS_INFO("%d",f);

	//Initializing ROS node with a name of demo_topic_subscriber
	ros::init(argc, argv,"stepper_control");
	//Created a nodehandle object
	ros::NodeHandle node_obj;
	//Offset de la posicion central
	center_offset = 100;
	center_pos = 640; 

	try{
	  node_obj.getParam("center_pos", center_pos);
	  node_obj.getParam("center_offset", center_offset);

	  ROS_INFO("Successfully Loaded tracking parameters");
	}

	catch(int e)
	{
	      ROS_WARN("Parameters are not properly loaded from file, loading defaults");
	}
	center_left = center_pos - center_offset;
	left=center_pos-(center_offset*2);
	center_right = center_pos + center_offset;
	right=center_pos+(center_offset*2);

	//Create a subscriber object
	ros::Subscriber number_subscriber = node_obj.subscribe("/x_position",10,x_position_callback);
	//Spinning the node
	ros::spin();
	return 0;
}