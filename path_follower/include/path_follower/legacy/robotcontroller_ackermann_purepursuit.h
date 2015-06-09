/*
 * RobotcontrollerAckermannGeometrical.h
 *
 *  Created on: Apr 25, 2015
 *      Author: holly
 */

#ifndef NAVIGATION_PATH_FOLLOWER_INCLUDE_PATH_FOLLOWER_CONTROLLER_ROBOTCONTROLLER_ACKERMANN_GEOMETRICAL_H_
#define NAVIGATION_PATH_FOLLOWER_INCLUDE_PATH_FOLLOWER_CONTROLLER_ROBOTCONTROLLER_ACKERMANN_GEOMETRICAL_H_

#include <path_follower/controller/robotcontroller_interpolation.h>
#include <path_follower/utils/parameters.h>

#include <visualization_msgs/Marker.h>


class Robotcontroller_Ackermann_PurePursuit: public RobotController_Interpolation
{
public:
	Robotcontroller_Ackermann_PurePursuit(PathFollower* _path_follower);
	virtual ~Robotcontroller_Ackermann_PurePursuit();

	virtual void stopMotion();
	virtual void start();
	virtual bool isOmnidirectional() const {
		return true;
	}

protected:
	virtual MoveCommandStatus computeMoveCommand(MoveCommand* cmd);
	virtual void publishMoveCommand(const MoveCommand &cmd) const;

private:

    struct ControllerParameters : public RobotController_Interpolation::InterpolationParameters {
		P<double> factor_lookahead_distance;
        P<double> vehicle_length;

		ControllerParameters() :
			factor_lookahead_distance(this, "~factor_lookahead_distance", 0.5, "lookahead distance factor"),
            vehicle_length(this, "~vehicle_length", 0.3, "axis-centre distance")
		{}

    } params;

    const RobotController_Interpolation::InterpolationParameters& getParameters() const
    {
        return params;
    }

	double computeAlpha(double& lookahead_distance, const Eigen::Vector3d& pose) const;

	ros::NodeHandle node_handle;
	ros::Publisher path_interpol_pub;

	MoveCommand move_cmd;
};

#endif /* NAVIGATION_PATH_FOLLOWER_INCLUDE_PATH_FOLLOWER_CONTROLLER_ROBOTCONTROLLER_ACKERMANN_GEOMETRICAL_H_ */