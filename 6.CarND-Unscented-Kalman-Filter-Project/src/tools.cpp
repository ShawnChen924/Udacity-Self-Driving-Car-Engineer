#include "tools.h"

using Eigen::VectorXd;
using std::vector;

Tools::Tools() {}

Tools::~Tools() {}

VectorXd Tools::CalculateRMSE(const vector<VectorXd> &estimations,
                              const vector<VectorXd> &ground_truth) {
    /**
    TODO:
      * Calculate the RMSE here.
    */
    VectorXd rmse(4);
    rmse << 0,0,0,0;

    if(estimations.size() == 0){
      //cout << "ERROR - CalculateRMSE () - The estimations vector is empty" << endl;
      return rmse;
    }

    if(ground_truth.size() == 0){
      //cout << "ERROR - CalculateRMSE () - The ground-truth vector is empty" << endl;
      return rmse;
    }

    unsigned int n = estimations.size();
    if(n != ground_truth.size()){
      //cout << "ERROR - CalculateRMSE () - The ground-truth and estimations vectors must have the same size." << endl;
      return rmse;
    }

    for(unsigned int i=0; i < estimations.size(); ++i){
      VectorXd diff = estimations[i] - ground_truth[i];
      diff = diff.array()*diff.array();
      rmse += diff;
    }

    rmse = rmse / n;
    rmse = rmse.array().sqrt();
    return rmse;
}
