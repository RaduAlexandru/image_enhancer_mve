//Iterate through a scene folder, read all undistorted images and enhance them with calche
//Also can backup the original (unmodified image) and restore them back

#include <iostream>
#include <iomanip>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <algorithm>
#include <iterator>
#include <boost/filesystem.hpp>
#include <boost/iterator/filter_iterator.hpp>
namespace fs = boost::filesystem;



int main (int argc, char** argv ){

  if (argc!=3){
    std::cout << "Invalid number of arguments" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "\t enhancer <option> <scene-dir>"  << std::endl << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t -b : Create a backup of the original images " << std::endl;
    std::cout << "\t -r : Restore original images" << std::endl;
    std::cout << "\t -r : Enhance the contrast of the images " << std::endl;
    return 2;

  }

  std::string option= argv[1];
  std::string scene_path= argv[2];

  bool backup=false;
  bool restore=false;
  bool enhance=false;

  if (option=="-b"){
    std::cout << "backing up" << std::endl;
    backup=true;
  }else if (option=="-r"){
    std::cout << "restoring" << std::endl;
    restore=true;
  }else{
    std::cout << "enhance" << std::endl;
    enhance=true;
  }


  // std::string path="/media/alex/Data/Master/SHK/sfm/scripts/colmap2mve/scene/views/";

  //get path
  std::string path= scene_path + "/views/";
  fs::path path_boost(path);
  fs::path absPath = fs::absolute(path_boost);
  fs::path full_p = fs::canonical(absPath);
  path=full_p.string();


  std::cout << "path is " << path << std::endl;


  //get number of views in the scene
  fs::path p(path);
  fs::directory_iterator dir_first(p), dir_last;
  auto pred = [](const fs::directory_entry& p)
  {
      return fs::is_directory(p);
  };
  int num_views = std::distance(boost::make_filter_iterator(pred, dir_first, dir_last),
                  boost::make_filter_iterator(pred, dir_last, dir_last));




  for (size_t i = 0; i < num_views; i++) {

    std::ostringstream num_str;
    num_str <<  std::setfill('0') << std::setw(4) << i << std::endl;

    std::string num_stripped;
    num_stripped=num_str.str();
    if (!num_stripped.empty() && num_stripped[num_stripped.length()-1] == '\n') {
      num_stripped.erase(num_stripped.length()-1);
    }


    if(backup){
      std::cout << "backing up " << i << std::endl;
      std::string img_name= "undistorted.png";
      std::string img_path=path +  "/view_" + num_stripped+ ".mve/" + img_name;
      cv::Mat img = cv::imread(img_path, CV_LOAD_IMAGE_COLOR);   // Read the file
      img_path=path +  "/view_" + num_stripped+ ".mve/" + "undistorted_original.png";
      cv::imwrite( img_path, img );
    }
    if(restore){
      std::cout << "restoring " << i << std::endl;
      std::string img_name= "undistorted_original.png";
      std::string img_path=path +  "/view_" + num_stripped+ ".mve/" + img_name;
      cv::Mat img = cv::imread(img_path, CV_LOAD_IMAGE_COLOR);   // Read the file
      img_path=path +  "/view_" + num_stripped+ ".mve/" + "undistorted.png";
      cv::imwrite( img_path, img );

      //Create the downsampled images
      for (size_t ds_idx = 0; ds_idx < 4; ds_idx++) {
        cv::Size size (img.cols/2, img.rows/2);
        cv::resize(img,img,size);
        img_path=path +  "/view_" + num_stripped+ ".mve/" + "undist-L"  + std::to_string(ds_idx+1) +  ".png";
        cv::imwrite( img_path, img );
      }

    }

    if (enhance){
      std::cout << "enchance " << i << std::endl;
      std::string img_name= "undistorted.png";
      std::string img_path=path +  "/view_" + num_stripped+ ".mve/" + img_name;

      cv::Mat img = cv::imread(img_path, CV_LOAD_IMAGE_COLOR);

      //Attempt 1 clache
      cv::Mat lab_image;
      cv::cvtColor(img, lab_image, CV_BGR2Lab);

      std::vector<cv::Mat> lab_planes(3);
      cv::split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

      cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
      clahe->setClipLimit(10);
      clahe->apply(lab_planes[0], lab_planes[0]);
      cv::merge(lab_planes, lab_image);

      cv::cvtColor(lab_image, img, CV_Lab2BGR);


      cv::imwrite( img_path, img );
    }

  }


  return 0;
}
