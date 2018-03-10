#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <sensor_msgs/image_encodings.h>  
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "std_msgs/String.h"
#include <sstream>
#include "face_detection.h"
using namespace std;
using namespace cv;
long imgnum=0;
string peoplename;
Mat idpic;//反馈图片
image_transport::Publisher image_pub;  
seeta::FaceDetection detector("/home/tiger/catkin_ws/src/face_detector/src/seeta_fd_frontal_v1.0.bin");
int max(int a ,int b)
{
if(a>b)return a;
else{return b;}
}
float min(float a ,float b)
{
if(a>b)return b;
else{return a;}
}
vector<seeta::FaceInfo> facedet(Mat &img_gray)
{
  seeta::ImageData img_data;
  img_data.data = img_gray.data;
  img_data.width = img_gray.cols;
  img_data.height = img_gray.rows;
  img_data.num_channels = 1;

  long t0 = cv::getTickCount();
  std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
  long t1 = cv::getTickCount();
  double secs = (t1 - t0)/cv::getTickFrequency();
  cout << "Detections takes " << secs << " seconds " << endl;
  cout << "Image size (wxh): " << img_data.width << "x" 
      << img_data.height << endl;
 return faces;
}
void chatterCallback(const std_msgs::String::ConstPtr& msg2)
 {
   peoplename=msg2->data.c_str();
  
 }
void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
   Mat face;//人脸图片 
   sensor_msgs::ImagePtr msg1;
 
 //  sensor_msgs::ImagePtr msg1;//人脸ros消息
   ROS_INFO("%s", "HEAR");
    cv_bridge::CvImagePtr cv_ptr;  
   ROS_INFO("%s", "HEAR1");
    try  
      {  
         cv_ptr = cv_bridge::toCvCopy(msg, "bgr8");  
      }  
    catch (cv_bridge::Exception& e)  
      {  
         ROS_ERROR("cv_bridge exception: %s", e.what());  
         return;  
      }  
    Mat img,img_gray;  
    img = cv_ptr->image;  
    cvtColor(img,img_gray,CV_RGB2GRAY);  
    vector<seeta::FaceInfo> faces=facedet(img_gray);
    cv::Rect face_rect;
    int32_t num_face = static_cast<int32_t>(faces.size());
if(num_face==0)peoplename="陌生人";
    for (int32_t i = 0; i < num_face; i++) 
  {
       face_rect.x = faces[i].bbox.x;
       face_rect.y = faces[i].bbox.y;
       face_rect.width = faces[i].bbox.width;
       face_rect.height = faces[i].bbox.height;
       cv::rectangle(img, face_rect, CV_RGB(0, 0, 255), 4, 8, 0);
   //    putText(img,peoplename, Point(face_rect.x, face_rect.y-10),FONT_HERSHEY_SIMPLEX,1,CV_RGB(255, 0, 0));//红色字体注释
  }
if(num_face>0){
cout<<face_rect.y+1<<endl;
cout<<face_rect.x+1<<endl;
face=img(Range(max(face_rect.y,0),min(face_rect.y+ face_rect.height,img.rows)),Range(max(face_rect.x,0),min(face_rect.x+face_rect.width-1 ,img.cols)));
 //imgnum++;
//if (imgnum>2)imgnum=0;
//msg1.data="/home/zhoukan/下载/face/1.png";
//imwrite(msg1.data,face);

//face_name_pub.publish(msg1);
msg1= cv_bridge::CvImage(std_msgs::Header(), "bgr8", face).toImageMsg();
image_pub.publish(msg1);
}
 // cv::imshow("Test", img);
 // cv::waitKey(3);
}


int main(int argc, char** argv)
 {
  detector.SetMinFaceSize(80);
  detector.SetScoreThresh(2.f);
  detector.SetImagePyramidScaleFactor(0.8f);
  detector.SetWindowStep(4, 4);

  ros::init(argc, argv, "facedetector");
  ros::NodeHandle nh;
//  cv::namedWindow("Test", CV_WINDOW_AUTOSIZE);
  image_transport::ImageTransport it(nh);
  image_pub= it.advertise("face_img", 1); 
//  face_name_pub=nh.advertise<std_msgs::String>("face_name", 1);
  image_transport::Subscriber sub = it.subscribe("imgmsg", 1, imageCallback);
  ros::Subscriber sub1 = nh.subscribe("id_name", 1, chatterCallback);
 
 ros::spin();
 return 0;

}
