/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "helper_functions.h"

using std::string;
using std::vector;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
    /**
   * TODO: Set the number of particles. Initialize all particles to
   *   first position (based on estimates of x, y, theta and their uncertainties
   *   from GPS) and all weights to 1.
   * TODO: Add random Gaussian noise to each particle.
   * NOTE: Consult particle_filter.h for more information about this method
   *   (and others in this file).
   */
    num_particles = 100;  // Set the number of particles
    particles.resize(num_particles);

    std::default_random_engine gen;
    std::normal_distribution<double> dist_x(x, std[0]);
    std::normal_distribution<double> dist_y(y, std[1]);
    std::normal_distribution<double> dist_theta(theta, std[2]);

    for (size_t i = 0; i < num_particles; ++i) {
        particles.at(i).x = dist_x(gen);
        particles.at(i).y = dist_y(gen);
        particles.at(i).theta = dist_theta(gen);
        particles.at(i).weight = 1.0;
    }

    is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], 
                                double velocity, double yaw_rate) {
    /**
   * TODO: Add measurements to each particle and add random Gaussian noise.
   * NOTE: When adding noise you may find std::normal_distribution
   *   and std::default_random_engine useful.
   *  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
   *  http://www.cplusplus.com/reference/random/default_random_engine/
   */
    std::default_random_engine gen;
    std::normal_distribution<double> noisy_x(0, std_pos[0]);
    std::normal_distribution<double> noisy_y(0, std_pos[1]);
    std::normal_distribution<double> noisy_theta(0, std_pos[2]);

    for (size_t i = 0; i < num_particles; ++i) {
        double x0 = particles.at(i).x;
        double y0 = particles.at(i).y;
        double theta0 = particles.at(i).theta;

        double x_pre = 0.0;
        double y_pre = 0.0;
        double theta_pre = 0.0;

        if (fabs(yaw_rate) < 0.00001) {
            x_pre = x0 + velocity * delta_t * cos(theta0);
            y_pre = y0 + velocity * delta_t * sin(theta0);
            theta_pre = theta0;
        } else {
            x_pre = x0 + velocity / yaw_rate * (sin(theta0 + yaw_rate * delta_t) - sin(theta0));
            y_pre = y0 + velocity / yaw_rate * (cos(theta0) - cos(theta0 + yaw_rate * delta_t));
            theta_pre = theta0 + yaw_rate * delta_t;
        }
        while (theta_pre > 2 * M_PI) theta_pre -= 2. * M_PI;
        while (theta_pre < 0.0) theta_pre += 2. * M_PI;

        particles.at(i).x = x_pre + noisy_x(gen);
        particles.at(i).y = y_pre + noisy_y(gen);
        particles.at(i).theta = theta_pre + noisy_theta(gen);
    }
}

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted, 
                                     vector<LandmarkObs>& observations) {
    /**
   * TODO: Find the predicted measurement that is closest to each
   *   observed measurement and assign the observed measurement to this
   *   particular landmark.
   * NOTE: this method will NOT be called by the grading code. But you will
   *   probably find it useful to implement this method and use it as a helper
   *   during the updateWeights phase.
   */
    for (LandmarkObs& obs : observations) {
        double distance_min = std::numeric_limits<double>::max();
        for (int i = 0; i < predicted.size(); ++i) {
            LandmarkObs pre = predicted[i];
            double distance2 = (pre.x - obs.x) * (pre.x - obs.x) + (pre.y - obs.y) * (pre.y - obs.y);
            if (distance2 < distance_min) {
                distance_min = distance2;
                obs.id = pre.id;
            }
        }
    }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
                                   const vector<LandmarkObs> &observations,
                                   const Map &map_landmarks) {
    /**
   * TODO: Update the weights of each particle using a mult-variate Gaussian
   *   distribution. You can read more about this distribution here:
   *   https://en.wikipedia.org/wiki/Multivariate_normal_distribution
   * NOTE: The observations are given in the VEHICLE'S coordinate system.
   *   Your particles are located according to the MAP'S coordinate system.
   *   You will need to transform between the two systems. Keep in mind that
   *   this transformation requires both rotation AND translation (but no scaling).
   *   The following is a good resource for the theory:
   *   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
   *   and the following is a good resource for the actual equation to implement
   *   (look at equation 3.33) http://planning.cs.uiuc.edu/node99.html
   */
    for (size_t i = 0; i < num_particles; ++i) {
        double xp = particles.at(i).x;
        double yp = particles.at(i).y;
        double theta = particles.at(i).theta;

        // build all landmark in sensor range, with land mark id
        vector<LandmarkObs> map_in_range;
        double distance_threshold = sensor_range * sensor_range;
        for (auto land_mark : map_landmarks.landmark_list) {
            double dis2 = (xp - land_mark.x_f) * (xp - land_mark.x_f) + (yp - land_mark.y_f) * (yp - land_mark.y_f);
            if (dis2 <= distance_threshold) {
                LandmarkObs map_obs{land_mark.id_i, land_mark.x_f, land_mark.y_f};
                map_in_range.push_back(map_obs);
            }
        }

        // convert all observation in vehicle coordinate to map coordinate, without land mark id (id = 0)
        vector<LandmarkObs> obs_in_map(observations.size());
        for (size_t j = 0; j < observations.size(); ++j) {
            double xc = observations[j].x;
            double yc = observations[j].y;
            obs_in_map[j].x = xp + (cos(theta) * xc) - (sin(theta) * yc);
            obs_in_map[j].y = yp + (sin(theta) * xc) + (cos(theta) * yc);
            obs_in_map[j].id = observations[j].id;
        }

        dataAssociation(map_in_range, obs_in_map);

        double std_x = std_landmark[0];
        double std_y = std_landmark[1];
        particles.at(i).weight = 1.0;

        // calculate the weight of particle
        for (size_t j = 0; j < obs_in_map.size(); ++j) {
            Map::single_landmark_s landmark = map_landmarks.landmark_list[obs_in_map[j].id - 1];
            double x = obs_in_map.at(j).x;
            double y = obs_in_map.at(j).y;
            double ux = landmark.x_f;
            double uy = landmark.y_f;
            double exponent = pow((x - ux), 2) / (2 * pow(std_x, 2)) + pow((y - uy), 2) / (2 * pow(std_y, 2));
            double p_xy = 1. / (2 * M_PI * std_x * std_y) * exp(-exponent);
            particles.at(i).weight *= p_xy;
        }

        weights.push_back(particles.at(i).weight);
    }
}

void ParticleFilter::resample() {
    /**
   * TODO: Resample particles with replacement with probability proportional
   *   to their weight.
   * NOTE: You may find std::discrete_distribution helpful here.
   *   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
   */
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d{std::begin(weights), std::end(weights)};

    // build resampled particles
    vector<Particle> resampled_particles(num_particles);
    for (size_t i = 0; i < num_particles; ++i) {
        resampled_particles[i] = particles[d(gen)];
    }

    particles = resampled_particles;

    weights.clear();
}

void ParticleFilter::SetAssociations(Particle& particle, 
                                     const vector<int>& associations,
                                     const vector<double>& sense_x,
                                     const vector<double>& sense_y) {
    // particle: the particle to which assign each listed association,
    //   and association's (x,y) world coordinates mapping
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates
    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
    vector<int> v = best.associations;
    std::stringstream ss;
    copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
    vector<double> v;

    if (coord == "X") {
        v = best.sense_x;
    } else {
        v = best.sense_y;
    }

    std::stringstream ss;
    copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
