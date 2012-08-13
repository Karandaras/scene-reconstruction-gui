#pragma once
#define _USE_MATH_DEFINES
#include <math.h> 

#include <gtkmm.h>
#include <gdk/gdk.h>

#include <iostream>
#include <google/protobuf/message.h>

#include "transport/Transport.hh"
#include "transport/TransportTypes.hh"
#include "gazebo_config.h"
#include "math/Pose.hh"

namespace SceneReconstruction {
  /** @class Converter "converter.h"
   * Class to encapsulate methods for conversion of data types
   * @author Bastian Klingen
   */
  class Converter {
    public:
      /** converts google::protobuf::unint32 to Glib::ustring 
       * @param in input as google::protobuf::uint32
       * @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(google::protobuf::uint32 in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts bool to Glib::ustring 
       * @param in input as bool
       * @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(bool in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts double to Glib::ustring 
       * @param in input as double
       * @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(double in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts gazebo::msgs::Vector3d to Glib::ustring 
       * @param in input as gazebo::msgs::Vector3d
       * @param round precision for rounding, -1 to disable
       * @return Glib::ustring representation of in
       */
      static Glib::ustring convert(gazebo::msgs::Vector3d in, int round = -1) {
        gazebo::math::Vector3 v = gazebo::msgs::Convert(in); 
        if(round != -1)
          v.Round(round);
        
        std::stringstream convert;
        convert << "X: ";
        convert << v.x;
        convert << " Y: ";
        convert << v.y;
        convert << " Z: ";
        convert << v.z;

        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts gazebo::msgs::Quaternion to Glib::ustring 
       * @param in input as gazebo::msgs::Quaternion
       * @param round precision for rounding, -1 to disable
       * @param as_euler choose euler or quaternion representation
       * @return Glib::ustring representation of in
       */
      static Glib::ustring convert(gazebo::msgs::Quaternion in, int round = -1, bool as_euler = false) {
        gazebo::math::Quaternion q = gazebo::msgs::Convert(in); 
        if(round != -1)
          q.Round(round);

        std::stringstream convert;
        if(as_euler) {
          gazebo::math::Vector3 v = q.GetAsEuler();
          v *= 180/M_PI;
          if(round != -1)
            v.Round(round);
        
          convert << "R: ";
          convert << v.x;
          convert << " P: ";
          convert << v.y;
          convert << " Y: ";
          convert << v.z;
        }
        else {
          convert << "W: ";
          convert << q.w;
          convert << " X: ";
          convert << q.x;
          convert << " Y: ";
          convert << q.y;
          convert << " Z: ";
          convert << q.z;
        }
        
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts gazebo::msgs::Pose to Glib::ustring 
       * @param in input as gazebo::msgs::Pose
       * @param part part of the pose to display, 1 for position, 2 for orientation
       * @param round precision for rounding, -1 to disable
       * @param as_euler choose euler or quaternion representation
       * @return Glib::ustring representation of in
       */
      static Glib::ustring convert(gazebo::msgs::Pose in, int part, int round = -1, bool as_euler = false) {
        if(part == 0) { // position
          return convert(in.position(), round);
        }
        else if (part == 1) { // orientation
          return convert(in.orientation(), round, as_euler);
        }
        else
          return "";
      }
  };
}
