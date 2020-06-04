#include "ros.h"

Ros::Ros(){

}

Ros::~Ros(){

}

void Ros::move(float linear, float angular){

    //qDebug() << "line velocity: " << velocity << "angular velocity" << angular;
    this->speed.linear.x = linear; // m/s
    this->speed.angular.z = angular; // rad/s
    this->pub_cmd_vel.publish(this->speed); //
}

void Ros::move(){

    this->pub_cmd_vel.publish(this->speed); //
}

void Ros::run(){

    this->move();

}

void Ros::up(double linear, double angular){

    this->speed.linear.x = linear;
    this->speed.angular.z = 0;
    this->pub_cmd_vel.publish(this->speed);
}

void Ros::down(double linear, double angular){

    this->speed.linear.x = -linear;
    this->speed.angular.z = 0;
    this->pub_cmd_vel.publish(this->speed);

}

void Ros::right(double linear, double angular){

    this->speed.linear.x = 0;
    this->speed.angular.z = -angular;
    this->pub_cmd_vel.publish(this->speed);
}

void Ros::left(double linear, double angular){

    this->speed.linear.x = 0;
    this->speed.angular.z = angular;
    this->pub_cmd_vel.publish(this->speed);
}

void Ros::turn(double linear, double angular){

    this->speed.linear.x = 0;
    this->speed.angular.z = angular;
    this->pub_cmd_vel.publish(this->speed);
}

void Ros::stop(){

    this->speed.linear.x = 0; //
    this->speed.angular.z = 0; //
    this->pub_cmd_vel.publish(this->speed); //
}
