#include "ukf.h"
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::VectorXd;

/**
 * Initializes Unscented Kalman filter
 */
UKF::UKF() {
    // if this is false, laser measurements will be ignored (except during init)
    use_laser_ = true;

    // if this is false, radar measurements will be ignored (except during init)
    use_radar_ = true;

    // init state
    is_initialized_ = false;

    // state dimensions
    n_x_ = 5;

    // aug state dimensions
    n_aug_ = 7;

    // lambda value
    lambda_ = 3 - n_x_;

    // initial state vector
    x_ = VectorXd(n_x_);

    // initial covariance matrix
    P_ = MatrixXd(n_x_, n_x_);
    P_.setIdentity();

    Xsig_pred_ = MatrixXd(n_x_, 2 * n_aug_ + 1);

    // Process noise standard deviation longitudinal acceleration in m/s^2
    std_a_ = 1.5;

    // Process noise standard deviation yaw acceleration in rad/s^2
    std_yawdd_ = 0.5;

    /**
   * DO NOT MODIFY measurement noise values below.
   * These are provided by the sensor manufacturer.
   */

    // Laser measurement noise standard deviation position1 in m
    std_laspx_ = 0.15;

    // Laser measurement noise standard deviation position2 in m
    std_laspy_ = 0.15;

    // Radar measurement noise standard deviation radius in m
    std_radr_ = 0.3;

    // Radar measurement noise standard deviation angle in rad
    std_radphi_ = 0.03;

    // Radar measurement noise standard deviation radius change in m/s
    std_radrd_ = 0.3;

    /**
   * End DO NOT MODIFY section for measurement noise values
   */

    /**
   * TODO: Complete the initialization. See ukf.h for other member properties.
   * Hint: one or more values initialized above might be wildly off...
   */

    // weights
    weights_ = VectorXd(2 * n_aug_ + 1);
    weights_.fill(0.5 / (lambda_ + n_aug_));
    weights_(0) = lambda_ / (lambda_ + n_aug_);

    R_radar_ = MatrixXd(3, 3);
    R_radar_ << std_radr_ * std_radr_, .0, .0,
            .0, std_radphi_ * std_radphi_ , .0,
            .0, .0, std_radrd_ * std_radrd_;

    R_lidar_ = MatrixXd(2, 2);
    R_lidar_ << std_laspx_ * std_laspx_, .0,
            .0, std_laspy_ * std_laspy_;
}

UKF::~UKF() {}

void UKF::ProcessMeasurement(MeasurementPackage meas_package) {
    /**
   * TODO: Complete this function! Make sure you switch between lidar and radar
   * measurements.
   */
    if (!is_initialized_) {
        if (meas_package.sensor_type_ == MeasurementPackage::SensorType::RADAR) {
            double rho = meas_package.raw_measurements_(0);
            double phi = meas_package.raw_measurements_(1);
            double rho_dot = meas_package.raw_measurements_(2);
            double px = rho * sin(phi);
            double py = rho * cos(phi);
            double vx = rho_dot * sin(phi);
            double vy = rho_dot * cos(phi);
            double v = sqrt(vx*vx + vy*vy);
            x_ << px, py, v, 0.0, 0.0;
        } else {
            x_ << meas_package.raw_measurements_(0), meas_package.raw_measurements_(1), 0.0, 0.0, 0.0;
        }
        time_us_ = meas_package.timestamp_;
        is_initialized_ = true;
        return;
    }

    double dt_s = (meas_package.timestamp_ - time_us_) / 1e6;
    time_us_ = meas_package.timestamp_;

    Prediction(dt_s);

    if (meas_package.sensor_type_ == MeasurementPackage::RADAR && use_radar_) {
        UpdateRadar(meas_package);
    }
    if (meas_package.sensor_type_ == MeasurementPackage::LASER && use_laser_) {
        UpdateLidar(meas_package);
    }
}

void UKF::Prediction(double delta_t) {
    /**
   * TODO: Complete this function! Estimate the object's location.
   * Modify the state vector, x_. Predict sigma points, the state,
   * and the state covariance matrix.
   */

    /// Generate Sigma Points
    // create augmented mean vector
    VectorXd x_aug = VectorXd(n_aug_);
    x_aug.head(5) = x_;
    x_aug(5) = 0;
    x_aug(6) = 0;

    // create augmented state covariance
    MatrixXd P_aug = MatrixXd(n_aug_, n_aug_);
    P_aug.fill(0.0);
    P_aug.topLeftCorner(n_x_, n_x_) = P_;
    P_aug(n_x_, n_x_) = std_a_ * std_a_;
    P_aug(n_x_+1, n_x_+1) = std_yawdd_ * std_yawdd_;
    MatrixXd A = P_aug.llt().matrixL();

    // create augmented sigma points
    MatrixXd Xsig_aug = MatrixXd(n_aug_, 2 * n_aug_ + 1);
    Xsig_aug.block(0, 0, n_aug_, 1) = x_aug;
    MatrixXd tmp(n_aug_, n_aug_);
    for (int i = 0; i < n_aug_; i++) {
        tmp.col(i) = x_aug;
    }
    Xsig_aug.block(0, 1, n_aug_, n_aug_) = tmp + sqrt(lambda_ + n_aug_) * A;
    Xsig_aug.block(0, n_aug_+1, n_aug_, n_aug_) = tmp - sqrt(lambda_ + n_aug_) * A;

    /// Predict Sigma Points
    for (int i = 0; i < Xsig_aug.cols(); i++) {
        VectorXd state = Xsig_aug.col(i);
        double v = state(2);
        double phi = state(3);
        double phi_dot = state(4);
        double v_a = state(5);
        double v_phi = state(6);

        VectorXd noise(5);
        noise << 0.5 * delta_t * delta_t * cos(phi) * v_a,
                 0.5 * delta_t * delta_t * sin(phi) * v_a,
                 delta_t * v_a,
                 0.5 * delta_t * delta_t * v_phi,
                 delta_t * v_phi;

        VectorXd delta_x(5);
        if (fabs(phi_dot) < 0.0000001) {
            delta_x << v * cos(phi) * delta_t,
                       v * sin(phi) * delta_t,
                       0.0,
                       phi_dot * delta_t,
                       0.0;
        } else {
            delta_x << v * (sin(phi + phi_dot * delta_t) - sin(phi)) / phi_dot,
                       v * (-cos(phi + phi_dot * delta_t) + cos(phi)) / phi_dot,
                       0.0,
                       phi_dot * delta_t,
                       0.0;
        }
        Xsig_pred_.col(i) = state.head(5) + delta_x + noise;
    }

    /// Calculate Mean state x_ and Update P_
    x_ = Xsig_pred_ * weights_;

    P_.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        // state difference
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        // angle normalization
        while (x_diff(3)> M_PI) x_diff(3)-=2.*M_PI;
        while (x_diff(3)<-M_PI) x_diff(3)+=2.*M_PI;
        P_ = P_ + weights_(i) * x_diff * x_diff.transpose();
    }
}

void UKF::UpdateLidar(MeasurementPackage meas_package) {
    /**
   * TODO: Complete this function! Use lidar data to update the belief
   * about the object's position. Modify the state vector, x_, and
   * covariance, P_.
   * You can also calculate the lidar NIS, if desired.
   */

    /// Calculate S
    int n_z = 2;
    MatrixXd Zsig = Xsig_pred_.block(0, 0, n_z, 2 * n_aug_ + 1);

    VectorXd z_pred = VectorXd(n_z);
    z_pred = Zsig * weights_;

    // measurement covariance matrix S
    MatrixXd S = MatrixXd(n_z, n_z);
    S.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        VectorXd z_diff = Zsig.col(i) - z_pred;
        S = S + weights_(i) * z_diff * z_diff.transpose();
    }

    S = S + R_lidar_;

    /// Update
    VectorXd z = meas_package.raw_measurements_;

    MatrixXd Tc = MatrixXd(n_x_, n_z);
    Tc.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        while(x_diff(3) < -M_PI) x_diff(3) += 2*M_PI;
        while(x_diff(3) > M_PI)  x_diff(3) -= 2*M_PI;
        VectorXd z_diff = Zsig.col(i) - z_pred;
        while(z_diff(1) < -M_PI) z_diff(1) += 2*M_PI;
        while(z_diff(1) > M_PI)  z_diff(1) -= 2*M_PI;

        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
    }

    MatrixXd K = Tc * S.inverse();

    VectorXd z_diff = z - z_pred;
    while(z_diff(1) < -M_PI) z_diff(1) += 2*M_PI;
    while(z_diff(1) > M_PI)  z_diff(1) -= 2*M_PI;

    x_ = x_ + K * z_diff;
    P_ = P_ - K * S * K.transpose();

    NIS_laser_ = z_diff.transpose() * S.inverse() * z_diff;
}

void UKF::UpdateRadar(MeasurementPackage meas_package) {
    /**
   * TODO: Complete this function! Use radar data to update the belief
   * about the object's position. Modify the state vector, x_, and
   * covariance, P_.
   * You can also calculate the radar NIS, if desired.
   */

    /// Calculate S
    int n_z = 3;
    MatrixXd Zsig = MatrixXd(n_z, 2 * n_aug_ + 1);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        double px = Xsig_pred_.col(i)(0);
        double py = Xsig_pred_.col(i)(1);
        double v = Xsig_pred_.col(i)(2);
        double yaw = Xsig_pred_.col(i)(3);

        double rho = sqrt(px*px + py*py);
        double phi = atan2(py, px);
        double phi_dot = (px*cos(yaw)*v + py*sin(yaw)*v) / rho;
        Zsig.col(i)(0) = rho;
        Zsig.col(i)(1) = phi;
        Zsig.col(i)(2) = phi_dot;
    }

    VectorXd z_pred = VectorXd(n_z);
    z_pred = Zsig * weights_;

    MatrixXd S = MatrixXd(n_z, n_z);
    S.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        VectorXd z_diff = Zsig.col(i) - z_pred;
        // angle normalization
        while (z_diff(1)> M_PI) z_diff(1)-=2.*M_PI;
        while (z_diff(1)<-M_PI) z_diff(1)+=2.*M_PI;
        S = S + weights_(i) * z_diff * z_diff.transpose();
    }
    S = S + R_radar_;

    /// Update
    VectorXd z = meas_package.raw_measurements_;

    MatrixXd Tc = MatrixXd(n_x_, n_z);
    Tc.fill(0.0);
    for (int i = 0; i < 2 * n_aug_ + 1; i++) {
        VectorXd x_diff = Xsig_pred_.col(i) - x_;
        while(x_diff(3) < -M_PI) x_diff(3) += 2*M_PI;
        while(x_diff(3) > M_PI)  x_diff(3) -= 2*M_PI;
        VectorXd z_diff = Zsig.col(i) - z_pred;
        while(z_diff(1) < -M_PI) z_diff(1) += 2*M_PI;
        while(z_diff(1) > M_PI)  z_diff(1) -= 2*M_PI;

        Tc = Tc + weights_(i) * x_diff * z_diff.transpose();
    }

    MatrixXd K = Tc * S.inverse();

    VectorXd z_diff = z - z_pred;
    while(z_diff(1) < -M_PI) z_diff(1) += 2*M_PI;
    while(z_diff(1) > M_PI)  z_diff(1) -= 2*M_PI;

    x_ = x_ + K * z_diff;
    P_ = P_ - K * S * K.transpose();

    NIS_radar_ = z_diff.transpose() * S.inverse() * z_diff;
}
