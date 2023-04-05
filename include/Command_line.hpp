//
//  CL_parser.hpp
//  CL_parser
//
//  Created by Jan Pieter Abrahams on 25/07/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef CL_parser_h
#define CL_parser_h

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>

// Command_line.hpp is a light-weight parser specifically created for parsing (argc, *argv[]) command line parameters.
//
// It defines two classes: Command_line and Command_line_tag.
//
// Command_line_tag:
//      Command_line_tag(std::string const& tag, std::string const& help, std::vector<std::string> const& default_params = {".*"})
//      Constructor that takes a tag, a help string describing the tag and a vector<string> with default arguments as parameters.
//      The default_params define the default values of the parameters associated with the specified tag, and also the number of
//      of allowed parameters. The default string ".*" indicates that the number of parameters is not fixed and could be zero.
//      It is derived from std::string, which contains the input tag.
//  bool const found() const
//      Returns true if the Command_line_tag was found in a Command_line.
//  operator std::vector<T>()
//      Type-cast the parameters to T and return them as a std::vector<T>. Parameters that were not found, get the default values.
//
// Example:
//  Command_line_tag tag0 {"tag0", "two integers", {"0", "10"}};
//      Defines a Command_line_tag with "tag0" as tag, a help string and two parameters, with defaults "0" and "10"
//
//
// Command_line (derived from std::vector<std::string>>)
//  Command_line(int argc, const char *argv[], Command_line_tag ... tags)
//      Constructor using argc and argv that could be command_line arguments of main(argc, argv). The
//      variadic arguments 'tag' define the command line tags and their default values.
//  std::string const& app_name() const
//      Returns the name and location of the program.
//  bool found(std::string const& tag) const
//      Returns true if tag was present in the Command_line.
//  std::string const attribute(std::size_t n) const
//      Returns the n-th attribute, which is the string / one of the strings located between the program
//      name and the first tag in the command line. A command line does not have to have attributes.
//  Command_line_tag const tag(std::string const& tag_name)
//      Returns the first Command_line_tag specified by tag_name that contains the parameters from the
//      Command_line. If there is no tag_name found, the returned Command_line_tag contains the default values.
//  std::vector<Command_line_tag> const multitag(Command_line_tag const& tag_name, std::size_t max_tags = -1)
//      Returns all Command_line_tags and their parameters in the command line that are specified by tag_name.
//      If max_tags == -1, all the tags with tag_name are returned. Otherwise max_tags determines the size of
//      the returned vector. If there is no tag_name found, the returned Command_line_tag contains the default
//      values.
//  std::vector<T> const value<T>(std::string const& tag)
//      Returns a vector of the requested type, containing the numerical values of the requested
//      Command_line_tag of the command line, with default values for unspecified parameters.
//  std::vector<std::vector<T>> const multivalue<T>(std::string const& tag)
//      Returns a vector of all the requested value<T>s.
//  std::string const help()
//      Returns a help string, using the information specified by the Command_line_tags.
//
// Example:
//    int main(int argc, const char * argv[]) {
//        Command_line_tag inp_size("-size", {"512", "512"},
//                                  "Area detector size in pixels");
//        Command_line_tag inp_pix("-pixel", {"0.05", "0.05"},
//                                  "Pixel size in mm");
//        Command_line_tag inp_dead("-dead_pixel", {".*"},
//                                  "Dead pixel coordinates.");
//        Command_line input(argc, argv, inp_size, inp_pix, inp_dead);
//
//        std::vector<int> siz = input.value<int>("-size");
//        std::vector<float> pix = input.value<float>("-pixel");
//        std::vector<std::vector<int>> dead =
//                               input.multivalue<int>("-dead_pixel");
//
//        std::cout << "detector type: " << input.attribute(0) << "\n";
//        std::cout << "size: " << siz[0] << " " << siz[1] << "\n";
//        std::cout << "pitch: " << pix[0] << " " << pix[1] << "\n";
//        std::cout << dead.size() << " dead pixels" << "\n";
//        std::cout << "Running: " << input.app_name() << std::endl;
//        std::cout << "\nUsage:\n" << input.help() << std::endl;
//    }
//
//      With the command line:
// ./Detector Medipix3 -pixel 0.049 0.051 -size 256 -dead_pixel 420 102 -dead_pixel 421 102
//      This yields:
//
//    detector type: Medipix3
//    size: 256 512
//    pitch: 0.049 0.051
//    2 dead pixels
//    Running: /Users/janpieterabrahams/Library/Developer/Xcode/DerivedData/Command_line-ewzfuzffzqplcwcdttaawxitpika/Build/Products/Debug/Command_line
//
//    Usage:
//    -size: Area detector size in pixels
//        default: 512 512
//    -pixel: Pixel size in mm
//        default: 0.05 0.05
//    -dead_pixel: Dead pixel coordinates.
//        default: .*


class Command_line;

class Command_line_tag : public std::string {
    struct Key_value;
    friend Command_line;
public:
    Command_line_tag(std::string const& tag, std::string const& help, std::vector<std::string> const& default_params = {".*"}) :
    std::string(tag),
    d_default_params(default_params),
    d_help(help){
        for (auto const& p : default_params)
            d_value.push_back(Key_value(p));
    };
    
    template <typename T> operator std::vector<T>() const {
        std::vector<T> r;
        for (std::size_t i = 0; i != d_value.size(); ++i)
            r.push_back(d_value[i]);
        return r;
    }
    
    bool const found() const {return d_found;}
    
private:
    std::vector<std::string> const d_default_params;
    std::string const d_help;
    std::vector<Key_value> d_value;
    bool d_found = false;
    
    struct Key_value : std::string {
        Key_value(std::string const& s) : std::string(s) {};
        
        template <typename T>
        operator const T() const {
            T r;
            std::istringstream(std::string(*this)) >> r;
            return r;
        }
    };
};

class Command_line : private std::vector<std::string const> {
public:
    template <typename ... KEY>
    Command_line(int argc, const char *argv[], KEY&& ... tag) :
    vector([&]{
        vector<std::string const> r;
        for (int i=0; i<argc; ++i)
            r.push_back(argv[i]);
        return r;
    }()),
    d_tags([&]{ std::vector<Command_line_tag const> r; (r.push_back(std::forward<KEY>(tag)), ...); return r; }())
    { };
    
    std::string const& app_name() const {return (*this)[0];}
    
    bool found(std::string const& tag) const {
        return std::find(begin(), end(), tag) != end();
    }
    
    Command_line_tag const tag(std::string const& tag) const noexcept { return multitag(tag)[0]; }
    
    std::vector<Command_line_tag> const multitag(std::string const& tag) const noexcept {
        auto const& tag_params = std::find(d_tags.begin(), d_tags.end(), tag);
        assert(tag_params != d_tags.end());
        Command_line_tag clt = *std::find(d_tags.begin(), d_tags.end(), tag);
        std::vector<Command_line_tag> r(1, clt);
        for (auto cursor = std::find(begin(), end(), tag); cursor != end(); ) {
            r[0].d_found = true;
            auto next_tag_p = std::find_first_of(++cursor, end(), d_tags.begin(), d_tags.end());
            if (r.back().d_default_params.size() != 0 && r.back().d_default_params[0] == ".*")
                for (r.back().d_value.pop_back(); cursor != next_tag_p ; ++cursor)
                    r.back().d_value.push_back(*(cursor));
            else if (r.back().d_default_params.size() != 0)
                for (auto p = r.back().d_value.begin(); (p != r.back().d_value.end()) && ((cursor) != next_tag_p) && (cursor != end()); ++p, ++cursor)
                    *p = *cursor;
            if (end() != (cursor = std::find(cursor, end(), tag)))
                r.push_back(clt);
        }
        return r;
    }
    
    template <typename T>
    std::vector<T> const value(std::string const& tag) const noexcept { return multitag(tag)[0]; }
    
    template <typename T>
    std::vector<std::vector<T>> const multivalue(std::string const& tag) const noexcept {
        std::vector<std::vector<T>> r;
        auto tmp = multitag(tag);
        for (auto const& t: tmp)
            r.push_back(t);
        return r;
    }
    
    std::string const& attribute(std::size_t const n) const noexcept { return (*this)[n+1]; }
    
    std::string const help() const noexcept {
        std::string r;
        for (auto& k: d_tags) {
            r += static_cast<std::string>(k) + ": " + k.d_help + '\n';
            if (k.d_value.size() > 0) {
                r += "    default: ";
                for (auto& p: k.d_value)
                    r += p + ' ';
                if (k.d_value[0] == ".*")
                    r += "  (number of parameters unspecified; no parameters by default)";
                r += '\n';
            }
        }
        return r;
    }
    
private:
    std::vector<Command_line_tag const> const d_tags;
 };

#endif /* CL_parser_h */
