/**
 * @file brain_node.cpp
 * @date July 2012
 * @author marks
 */

// C/C++
#include <sstream>
#include <stdlib.h>
#include <time.h>

// Workspace
#include <utils/LibRosUtil/RosMath.h>

// Project
#include "brain_node.h"

using namespace lib_ros_util;
using namespace std;
using namespace geometry_msgs;

namespace multiple_persons_brain {

BrainNode::BrainNode()
    : n_( "~" ),
      map_frame_( "/map" )
{
    // Target points
    std::string target_list;
    n_.param( "targets", target_list, std::string( "" ));
    std::stringstream target_ss( target_list );
    double target_x, target_y;
    while ( target_ss >> target_x && target_ss >> target_y ) {
        geometry_msgs::Point p;
        p.x = target_x;
        p.y = target_y;
        p.z = 0;
        targets_.push_back( p );
        ROS_INFO( "Added target coordinates: %f %f", target_x, target_y );
    }
    ROS_ASSERT( targets_.size() > 1 );

    // Number of person robots
    int num_persons( 0 );
    n_.param( "num_persons", num_persons, 0 );
    ROS_ASSERT( num_persons > 0 );

    // Index of the first person robot
    int first_idx( 0 );
    n_.param( "first_idx", first_idx, 0 );

    // For each person robot
    persons_.resize( num_persons );
    for ( int i = 0; i < num_persons; ++i ) {
        persons_[i].idx = first_idx + i;
        std::stringstream ss;
        ss << "/move_base_simple_robot_" << persons_[i].idx << "/goal";
        persons_[i].target_pub = n_.advertise<geometry_msgs::PoseStamped>( ss.str(), 1, true );
    }
    ROS_INFO( "Added %d person robots. Index of first person robot is: %d", num_persons, first_idx );

    // Create update timer
    update_timer_ = n_.createTimer( ros::Duration( 0.1 ), &BrainNode::updateCallback, this );

    // Random seed
    srand( time( NULL ));
}

void BrainNode::updateCallback( const ros::TimerEvent &e )
{
    Pose p;

    // For each simulated person
    for ( size_t i = 0; i < persons_.size(); ++i ) {
        // Get current position
        if ( !getRobotPose( persons_[i].idx, p ))
            continue;

        // Check distance to target
        if ( !persons_[i].has_target
             || RosMath::distance2d( p.position, persons_[i].target ) < 1.0 ) {
            // Select new target randomly
            selectAndPublishTarget( persons_[i], p );
        }
    }
}

void BrainNode::selectAndPublishTarget( Person &person, const geometry_msgs::Pose &p )
{
    PoseStamped new_target;
    int target_idx = rand() % targets_.size();
    new_target.pose.orientation = p.orientation;
    new_target.pose.position = targets_[ target_idx ];
    new_target.header.frame_id = map_frame_;
    new_target.header.stamp = ros::Time::now();
    person.target_pub.publish( new_target );
    person.target = new_target.pose.position;
    person.has_target = true;

    ROS_INFO( "Published new target. Idx: %d", person.idx );
}

bool BrainNode::getRobotPose( const int robot_idx, Pose &p )
{
    string robot_frame = getRobotName( robot_idx, "base_link" );
    tf::StampedTransform trafo;
    try {
        tf_.waitForTransform( robot_frame, map_frame_, ros::Time::now(), ros::Duration( 0.1 ));
        tf_.lookupTransform( map_frame_, robot_frame, ros::Time::now(), trafo );
    } catch ( tf::TransformException &ex ) {
        ROS_ERROR( "Cannot lookup transform. Reason: %s", ex.what());
        return false;
    }
    TransformStamped msg;
    tf::transformStampedTFToMsg( trafo, msg );
    p.position.x = msg.transform.translation.x;
    p.position.y = msg.transform.translation.y;
    p.position.z = 0;
    p.orientation = msg.transform.rotation;

    return true;
}

std::string BrainNode::getRobotName( const int robot_idx, const std::string &name )
{
    stringstream ss;
    ss << "/robot_" << robot_idx << "/" << name;
    return ss.str();
}

void BrainNode::spin()
{
    ros::spin();
}

} // namespace

int main( int argc, char* argv[] )
{
    ros::init( argc, argv, "brain_node" );
    multiple_persons_brain::BrainNode node;
    node.spin();
    return 0;
}