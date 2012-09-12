#pragma once
#define _USE_MATH_DEFINES

#define SHIFTSEQUENCE "  "

#include <math.h> 

#include <gtkmm.h>
#include <gdk/gdk.h>

#include <iostream>
#include <google/protobuf/message.h>

#include <gazebo/transport/Transport.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/gazebo_config.h>
#include <gazebo/math/Pose.hh>

namespace SceneReconstruction {
  /** @class Converter "converter.h"
   *  Class to encapsulate methods for conversion of data types
   *  @author Bastian Klingen
   */
  class Converter {
    public:
      /** converts google::protobuf::unint32 to Glib::ustring 
       *  @param in input as google::protobuf::uint32
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(google::protobuf::uint32 in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts bool to Glib::ustring 
       *  @param in input as bool
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(bool in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts double to Glib::ustring 
       *  @param in input as double
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring(double in) {
        std::stringstream convert;
        convert << in;
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts time in msec as double to a formatted Glib::ustring 
       *  @param in time in msec as double
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring to_ustring_time(double in) {
        int hour, min, sec, msec;
        hour = (int)(in/3600000);
        in  -= hour*3600000;
        min  = (int)(in/60000);
        in  -= min*60000;
        sec  = (int)(in/1000);
        in  -= sec*1000;
        msec = (int)in;
        std::stringstream convert;
        if(hour > 0)
          convert << hour << ":";

        if(min < 10)
          convert << "0";
        convert << min << ":";

        if(sec < 10)
          convert << "0";
        convert << sec << ".";

        if(msec < 100)
          convert << "0";
        if(msec < 10)
          convert << "0";
        convert << msec;

        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts vector given by three doubles to Glib::ustring 
       *  @param x double x value
       *  @param y double y value
       *  @param z double z value
       *  @param round precision for rounding, -1 to disable
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring convert(double x, double y, double z, int round = -1) {
        std::stringstream convert;
        if(round != -1) {
          gazebo::math::Vector3 v(x,y,z);
          v.Round(round);
          convert << "X: ";
          convert << v.x;
          convert << " Y: ";
          convert << v.y;
          convert << " Z: ";
          convert << v.z;
        }
        else {
          convert << "X: ";
          convert << x;
          convert << " Y: ";
          convert << y;
          convert << " Z: ";
          convert << z;
        }

        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts gazebo::msgs::Vector3d to Glib::ustring 
       *  @param in input as gazebo::msgs::Vector3d
       *  @param round precision for rounding, -1 to disable
       *  @return Glib::ustring representation of in
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

      /** converts a quaternionen given by four doubles to Glib::ustring 
       *  @param w double w value
       *  @param x double x value
       *  @param y double y value
       *  @param z double z value
       *  @param round precision for rounding, -1 to disable
       *  @param as_euler choose euler or quaternion representation
       *  @return Glib::ustring representation of in
       */
      static Glib::ustring convert(double w, double x, double y, double z, int round = -1, bool as_euler = false) {
        std::stringstream convert;
        if(round == -1 && !as_euler) {
          convert << "W: ";
          convert << w;
          convert << " X: ";
          convert << x;
          convert << " Y: ";
          convert << y;
          convert << " Z: ";
          convert << z;
        }
        else {
          gazebo::math::Quaternion q(w,x,y,z); 

          if(round != -1)
            q.Round(round);

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
        }
        
        Glib::ustring ret(convert.str());
        return ret;
      }

      /** converts gazebo::msgs::Quaternion to Glib::ustring 
       *  @param in input as gazebo::msgs::Quaternion
       *  @param round precision for rounding, -1 to disable
       *  @param as_euler choose euler or quaternion representation
       *  @return Glib::ustring representation of in
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
       *  @param in input as gazebo::msgs::Pose
       *  @param part part of the pose to display, 1 for position, 2 for orientation
       *  @param round precision for rounding, -1 to disable
       *  @param as_euler choose euler or quaternion representation
       *  @return Glib::ustring representation of in
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

      /** converts JSON-Style input to a more human-readable output
       *  @param json JSON-Style input
       *  @return Glib::ustring human-readable version of json
       */
      static Glib::ustring parse_json(std::string json) {
        std::stringstream out;
        unsigned int tabs = 0;
        for(unsigned int i=0; i<json.length(); i++) {
          if(json[i] == '{' && i!=0) {
            tabs++;
            out << "\n" << shift(tabs);
          }
          else if(json[i] == '}') {
            tabs--;
            out << "\n" << shift(tabs);
          }
          else if(json[i] == '[') {
            tabs++;
            out << "\n" << shift(tabs);
          }
          else if(json[i] == ']') {
            tabs--;
            out << "\n" << shift(tabs);
          }

          out << json[i];

          if(json[i] == '{') {
            tabs++;
            out << "\n" << shift(tabs);
          }
          else if(json[i] == '}') {
            tabs--;
          }
          else if(json[i] == '[') {
            tabs++;
            out << "\n" << shift(tabs);
          }
          else if(json[i] == ']') {
            tabs--;
          }
          else if(json[i] == ',') {
            out << "\n" << shift(tabs);
          }
        }
        return out.str();
      }
    
    private:
      static std::string shift(unsigned int count) {
        std::string result = "";
        for(unsigned int i=0; i<count; i++) {
          result += SHIFTSEQUENCE;
        }
        return result;
      }
  };
}
