#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <list>
#include <algorithm>
#include <vector>
#include <numeric>
#include <map>
using namespace std;

#include <eigen3/Eigen/Dense>
using namespace Eigen;

// #include <ros/console.h>
// #include <ros/assert.h>

#include "parameters.h"

class FeaturePerFrame
{
public:
  FeaturePerFrame(const Eigen::Matrix<double, 7, 1> &_point, double td)
  {
    point.x() = _point(0);
    point.y() = _point(1);
    point.z() = _point(2);
    uv.x() = _point(3);
    uv.y() = _point(4);
    velocity.x() = _point(5);
    velocity.y() = _point(6);
    cur_td = td;
  }
  double cur_td;   //相机与IMU不同步时的相差时间
  Vector3d point;  //特征点在相机坐标系下的坐标，z轴此时不为1
  Vector2d uv;     //特征点在该帧的图像坐标
  Vector2d velocity;   //该帧观测到该点的速度
  double z; 
  bool is_used;
  double parallax;
  MatrixXd A;
  VectorXd b;
  double dep_gradient;
};

class FeaturePerId    //管理一个特征点
{
public:
  const int feature_id;
  int start_frame;   //第一次出现该特征点的帧号
  vector<FeaturePerFrame> feature_per_frame;    //管理看到该特征点的帧，注意不是Frame类，它仅仅代表把该点映射到一帧

  int used_num;      //出现的次数
  bool is_outlier;   //是否是坏点
  bool is_margin;    //是否边缘化该特征点
  double estimated_depth;      //该特征点对应的逆深度
  int solve_flag; // 该特征点是否被三角化成LandMark

  Vector3d gt_p;

  FeaturePerId(int _feature_id, int _start_frame)
      : feature_id(_feature_id), start_frame(_start_frame),
        used_num(0), estimated_depth(-1.0), solve_flag(0)
  {
  }

  int endFrame();  //得到跟踪到该特征点的最后一帧编号
};

class FeatureManager
{
public:
  FeatureManager(Matrix3d _Rs[]);

  void setRic(Matrix3d _ric[]);

  void clearState();

  int getFeatureCount();

  bool addFeatureCheckParallax(int frame_count, const map<int, vector<pair<int, Eigen::Matrix<double, 7, 1>>>> &image, double td);
  void debugShow();
  vector<pair<Vector3d, Vector3d>> getCorresponding(int frame_count_l, int frame_count_r);

  //void updateDepth(const VectorXd &x);
  void setDepth(const VectorXd &x);
  void removeFailures();
  void clearDepth(const VectorXd &x);
  VectorXd getDepthVector();
  void triangulate(Vector3d Ps[], Vector3d tic[], Matrix3d ric[]);
  void removeBackShiftDepth(Eigen::Matrix3d marg_R, Eigen::Vector3d marg_P, Eigen::Matrix3d new_R, Eigen::Vector3d new_P);
  void removeBack();
  void removeFront(int frame_count);
  void removeOutlier();
  list<FeaturePerId> feature;
  int last_track_num;

private:
  double compensatedParallax2(const FeaturePerId &it_per_id, int frame_count);
  const Matrix3d *Rs;
  Matrix3d ric[NUM_OF_CAM];
};

#endif