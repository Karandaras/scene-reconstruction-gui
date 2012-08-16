#pragma once
#include <streambuf>
#include <pthread.h>
#include "ansiconverter.h"

namespace SceneReconstruction {
  template <class charT = char, class traits = std::char_traits<charT> >
  /** @class TextBufferStreamBuffer "loggingtools.h"
   * StreamBuffer to log from std::cout to a Gtk::TextBuffer
   * @author Bastian Klingen
   */

  class TextBufferStreamBuffer : public std::basic_streambuf<charT, traits>
  {
    public:
      /** Constructor
       * @param buffer The TextBuffer object of the TextView
       */
      TextBufferStreamBuffer(Glib::RefPtr<Gtk::TextBuffer> buffer)
      {
        this->buffer = buffer;
        tags["0"] = Gtk::TextBuffer::Tag::create("normal");
        this->buffer->get_tag_table()->add(tags["0"]);
        tags["1"] = Gtk::TextBuffer::Tag::create("bold");
        tags["1"]->property_weight() = 900;
        this->buffer->get_tag_table()->add(tags["1"]);
        tags["4"] = Gtk::TextBuffer::Tag::create("underlined");
        tags["4"]->property_underline() = Pango::UNDERLINE_SINGLE;
        this->buffer->get_tag_table()->add(tags["4"]);
        tags["5"] = Gtk::TextBuffer::Tag::create("blink");
        this->buffer->get_tag_table()->add(tags["5"]);
        tags["7"] = Gtk::TextBuffer::Tag::create("reverse");
        this->buffer->get_tag_table()->add(tags["7"]);
        tags["30"] = Gtk::TextBuffer::Tag::create("fg_black");
        tags["30"]->property_foreground() = "black";
        this->buffer->get_tag_table()->add(tags["30"]);
        tags["40"] = Gtk::TextBuffer::Tag::create("bg_black");
        tags["40"]->property_background() = "black";
        this->buffer->get_tag_table()->add(tags["40"]);
        tags["31"] = Gtk::TextBuffer::Tag::create("fg_red");
        tags["31"]->property_foreground() = "red";
        this->buffer->get_tag_table()->add(tags["31"]);
        tags["41"] = Gtk::TextBuffer::Tag::create("bg_red");
        tags["41"]->property_background() = "red";
        this->buffer->get_tag_table()->add(tags["41"]);
        tags["32"] = Gtk::TextBuffer::Tag::create("fg_green");
        tags["32"]->property_foreground() = "green";
        this->buffer->get_tag_table()->add(tags["32"]);
        tags["42"] = Gtk::TextBuffer::Tag::create("bg_green");
        tags["42"]->property_background() = "green";
        this->buffer->get_tag_table()->add(tags["42"]);
        tags["33"] = Gtk::TextBuffer::Tag::create("fg_yellow");
        tags["33"]->property_foreground() = "yellow";
        this->buffer->get_tag_table()->add(tags["33"]);
        tags["43"] = Gtk::TextBuffer::Tag::create("bg_yellow");
        tags["43"]->property_background() = "yellow";
        this->buffer->get_tag_table()->add(tags["43"]);
        tags["34"] = Gtk::TextBuffer::Tag::create("fg_blue");
        tags["34"]->property_foreground() = "blue";
        this->buffer->get_tag_table()->add(tags["34"]);
        tags["44"] = Gtk::TextBuffer::Tag::create("bg_blue");
        tags["44"]->property_background() = "blue";
        this->buffer->get_tag_table()->add(tags["44"]);
        tags["35"] = Gtk::TextBuffer::Tag::create("fg_magenta");
        tags["35"]->property_foreground() = "magenta";
        this->buffer->get_tag_table()->add(tags["35"]);
        tags["45"] = Gtk::TextBuffer::Tag::create("bg_magenta");
        tags["45"]->property_background() = "magenta";
        this->buffer->get_tag_table()->add(tags["45"]);
        tags["36"] = Gtk::TextBuffer::Tag::create("fg_cyan");
        tags["36"]->property_foreground() = "cyan";
        this->buffer->get_tag_table()->add(tags["36"]);
        tags["46"] = Gtk::TextBuffer::Tag::create("bg_cyan");
        tags["46"]->property_background() = "cyan";
        this->buffer->get_tag_table()->add(tags["46"]);
        tags["37"] = Gtk::TextBuffer::Tag::create("fg_white");
        tags["37"]->property_foreground() = "white";
        this->buffer->get_tag_table()->add(tags["37"]);
        tags["47"] = Gtk::TextBuffer::Tag::create("bg_white");
        tags["47"]->property_background() = "white";
        this->buffer->get_tag_table()->add(tags["47"]);
        tag_open = false;
        tag_close = false;
      }
      /** Destructor */
      ~TextBufferStreamBuffer()
      {
        tags.clear();
      }
    protected:
      Glib::RefPtr<Gtk::TextBuffer> buffer;
      std::ostringstream line;
      bool tag_open, tag_close;
      std::vector< Glib::RefPtr<Gtk::TextTag> > cur_tags;

      // Tags for the TextView
      std::map<std::string, Glib::RefPtr<Gtk::TextTag> > tags;

      std::streamsize xsputn(const charT * text, std::streamsize length)
      {
        std::ostringstream tmp;
        for(int i=0; i<length; i++) {
          if(*text == '\10') {
            line << tmp.str();
            tmp.str("");
            process_line();
            this->buffer->insert(this->buffer->end(), "\n");
          }
          else
            tmp << *text;

          text++;
        }

        line << tmp.str();
        
        if(tmp.str().find("\033[0m") != std::string::npos && tag_open) {
          process_line();
          tag_close = true;
        }
        else if(tmp.str().find("\033[") != std::string::npos) {
          if(tag_close) {
            this->buffer->insert(this->buffer->end(), "\n");
            tag_close = false;
          }
          tag_open = true;
        }
        else if(tag_open && tag_close) {
          process_line();
        }

        return length;
      }

      void process_line() {
        if(AnsiConverter::check_ansi_tags(line.str())) {
          set_tags(AnsiConverter::get_ansi_tags(line.str()));
          this->buffer->insert_with_tags(this->buffer->end(), AnsiConverter::ansi_to_textview(line.str()), cur_tags);
        }
        else
          this->buffer->insert(this->buffer->end(), AnsiConverter::ansi_to_textview(line.str()));

        line.str("");
      }
      
      void set_tags(std::vector< std::string > string_tags) {
        cur_tags.clear();
        for(std::vector< std::string >::iterator it = string_tags.begin(); it<string_tags.end(); it++) {
          cur_tags.push_back(tags[*it]);
        }
      }
   };
}
