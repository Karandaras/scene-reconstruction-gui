#pragma once
#include <string>
#include <gdk/gdk.h>

namespace SceneReconstruction {
  /** @class AnsiConverter "ansiconverter.h"
   *  Class to encapsulate methods for conversion of ansi escape codes
   *  @author Bastian Klingen
   */
  class AnsiConverter {
    public:
      /** removes ansi-tags "\033[*m" from the string
       *  @param text the text to clean
       *  @return Glib::ustring converted and formatted representation of text
       */
      static Glib::ustring ansi_to_textview(std::string text) {
        std::ostringstream out;
        bool tag = false;
        for(unsigned int i=0; i<text.length();i++) {
          if((int)text.at(i) == 27)
            tag = true;
          else if((int)text.at(i) == 109 && tag)
            tag = false;
          else if(!tag)
            out << text.at(i);
        }

        Glib::ustring result;
        result = out.str();

        return result;
      }

      /** checks if a string contains ansi-tags
       *  @param text the text to check
       *  @return bool true if text contains ansi tags
       */
      static bool check_ansi_tags(std::string text) {
        if(text.find("\033[") != std::string::npos)
          return true;
        else
          return false;
      }

      /** seperates ansi-tags divided by ;
       *  @param text the text to check
       *  @return std::vector a vector containing the ansi-tags
       */
      static std::vector< std::string > get_ansi_tags(std::string text) {
        std::vector< std::string > taglist;
        size_t pos_start = text.find("\033[")+1;
        size_t pos_end   = text.find("m", pos_start);
        size_t pos = 0;
        text = text.substr(pos_start, pos_end-pos_start);
        while(pos != std::string::npos) {
          size_t n = text.find(";", pos+1);
          if(n == std::string::npos)
            n = text.length() - pos;
          else
            n = n - pos - 1;

          taglist.push_back(text.substr(pos+1, n));
          pos = text.find(";", pos+1);
        }
        return taglist;
      }
 };
}
