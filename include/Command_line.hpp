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
// It defines two classes: Command_line and Command_line_option.
//
// Command_line_option:
//      Command_line_option(std::string const& option, std::string const& help, std::vector<std::string> const& default_params = std::vector<std::string>())
//      Constructor that takes a option, a help string describing the option and a vector<string> with default arguments as parameters.
//      The default_params define the default values of the parameters associated with the specified option, and also the number of
//      of allowed parameters. The default string ".*" indicates that the number of parameters is not fixed and could be zero.
//      It is derived from std::string, which contains the input option.
//  template <typename T> std::vector<T> param(std::size_t indx = 0) const
//      Type-cast the Command_line_option parameter with the supplied index to type T and return it. If a parameter is
//      not included in the command line, it will get the default value, unless the parameter is mandatory (so with
//      an empty default string), in which case an exception is thrown.
//  template <typename T> std::vector<T> all_params(std::size_t indx = 0) const
//      Type-cast the Command_line_option parameter with the supplied index to type T and return it. If a parameter is
//      not included in the command line, it will get the default value, unless the parameter is mandatory (so with
//      an empty default string), in which case an exception is thrown.
//  bool const found() const
//      Returns true if the Command_line_option was found in a Command_line.
//  std::vector<std::ptrdiff_t> const positions() const
//      Returns the positions as indices of the command line arguments where the Command_line_option was found in a Command_line.
//
// Example:
//  Command_line_option option0 {"-opt0", "two integers", {"0", "10"}};
//      Defines a Command_line_option with "-opt0" as option, a help string and two parameters, with default "0" and "10"
//   Command_line_option option1{"-opt1", "two mandatory integers", {"", ""}};
//      Defines a Command_line_option with "-opt1" as the option, a help string, and two parameters with default values "0" and "10".
//
// Command_line (derived from std::vector<std::string>>)
//  Command_line(int argc, const char *argv[], std::vector<Command_line_option> const& options)
//      Constructor using argc and argv that could be command_line arguments of main(argc, argv). The
//      vector 'options' defines the command line options and their default values.
//  std::vector<std::string> const& arguments() const noexcept
//      Returns the arguments of the command line.
//  std::vector<std::string> params()
//      Returns the parameters of the command line: all arguments that are not options or option parameters.
//  Command_line_option const option(std::string const& option_name)
//      Retrieves the Command_line_option specified by option_name with the values from the command line.
//      If the option is not found, or (some) values ar missing, the returned Command_line_option replacing missing values with their defaults.
//  std::string const help()
//      Returns a help string, using the information specified by the Command_line_options.
//
// Example:
//    int main(int argc, const char * argv[]) {
//        Command_line_option inp_size("-size", "Area detector size in pixels", {"512", "512"});
//        Command_line_option inp_pix("-pixel", "Pixel size in mm", {"0.05", "0.05"});
//        Command_line_option inp_dead("-dead_pixel", "Dead pixel coordinates.");
//        Command_line_option inp_verbose("-verbose", "Give lots of output when this keyword is present.", {});
//        Command_line input(argc, argv, inp_size, inp_pix, inp_dead, inp_verbose);
//
//        std::vector<int> siz = input.option("-size");
//        std::vector<float> pix = input.option("-pixel");
//        std::vector<std::vector<int>> dead = input.multivalue<int>("-dead_pixel");
//        std::vector<bool> verbose = input.option("-verbose"); //== "false";
//        std::vector<std::string> inp_data = input.data();
//        if (input.found("-verbose") &&  input.option("-verbose").found()){
//            std::cout << "Running: " << input.app_name() << std::endl;
//            std::cout << "\nUsage:\n" << input.help() << std::endl;
//        }
//        std::cout << "detector type: " << input.attribute(0) << "\n";
//        std::cout << "size: " << siz[0] << " " << siz[1] << "\n";
//        std::cout << "pitch: " << pix[0] << " " << pix[1] << "\n";
//        std::cout << dead.size() << " dead pixels" << "\n";
//        std::cout << "remaining data: " << inp_data[0] << " "<< inp_data[1] << " "<< inp_data[2] << " "<< inp_data[3] << "\n";
//    }
//
//      With the command line: ./Detector Medipix3 -pixel 0.049 0.051 -dead_pixel 420 102  -size 256 -dead_pixel 421 102 -verbose 1 2 3 4
//      This yields:
//    Running: /Users/abrahams/Library/Developer/Xcode/DerivedData/Command_line-faryocxqelmvksfijlumrwevgcxe/Build/Products/Debug/Command_line
//
//    Usage:
//    -size: Area detector size in pixels
//        default: 512 512
//    -pixel: Pixel size in mm
//        default: 0.05 0.05
//    -dead_pixel: Dead pixel coordinates.
//        default: .*   (number of parameters unspecified; no parameters by default)
//    -verbose: Give lots of output when this keyword is present.
//
//    detector type: Medipix
//    size: 512 512
//    pitch: 0.049 0.051
//    2 dead pixels
//    remaining data: 1 2 3 4

namespace jpa {

class Command_line;

/**
 * @class Command_line_option
 * @brief Represents a command-line option with optional parameters and default values.
 *
 * The `Command_line_option` class is used to define a command-line option along with a help string describing the option and
 * a vector of default parameters. The default parameters define the default values of the parameters associated with
 * the specified option and also the number of allowed parameters. The default string ".*" indicates that the number of
 * parameters is not fixed and could be zero. It is derived from `std::string`, which contains the input option. If a default string is empty,
 * supplying an option parameter is mandatory, and an exception will be thrown if such a mandatory parameter is omitted.
 *
 * Example usage:
 * @code{.cpp}
 * Command_line_option option0{"-opt0", "two integers", {"0", "10"}};
 * // Defines a Command_line_option with "-opt0" as the option, a help string, and two parameters with default values "0" and "10".
 * Command_line_option option1{"-opt1", "two mandatory integers", {"", ""}};
 * // Defines a Command_line_option with "-opt1" as the option, a help string, and two parameters with default values "0" and "10".
 * @endcode
 */
class Command_line_option : public std::string {
    struct Key_value;
    friend Command_line;
public:
    
    /**
     * @brief Constructor that takes a option, a help string, and default parameters as parameters.
     *
     * @param option The option for the command-line argument.
     * @param help A help string describing the option.
     * @param default_params A vector of of strings, indicating the number of option parameters and their default values. If default_params is empty or omitted, the option is a flag without option parameters. If default_params contains empty strings, this indicates there is no default, and providing an option paameter is mandatory.
     */
    Command_line_option(std::string const& option, std::string const& help, std::vector<std::string> const& default_params = std::vector<std::string>()) noexcept :
    std::string(option),
    d_default_params([&] {
        std::vector<Key_value> r;
        for (auto& p : default_params) r.push_back(p);
        return r;
    }()),
    d_help(help){ };
    
    
    /**
     * @brief Type-cast each Command_line_option parameter to T and return  as a std::vector<std::vector<T>>.
     *
     * Used if a command line has multiple instances of the same option, for instance "-try 1 2 -try 2 5 - try 12 0".
     * In that case all_params<int>() returns a vector of vector<int>: {{1, 2}, {2, 5}, {12, 0}}.
     * If the number of option parameters is smaller than required, remaining parameters get the default values. Throws
     * an exception in the case of missing mandatory parameters.
     *
     * @tparam T The type to which the parameters should be cast.
     * @return A std::vector<T> containing the parameters.
     *
     * @throws std::string If a parameter is mandatory and not found in the command line.
     */
    template <typename T> std::vector<std::vector<T>> all_params() const {
        std::vector<std::vector<T>> r;
        for (std::size_t i = 0; i != std::max(d_values.size(), 1ul); ++i) //{
            r.push_back(param<T>(i));
        return r;
    }
    
    /**
     * @brief Type-cast one of the Command_line_option parameters to T and return it
     *
     * Type-cast the Command_line_option parameter with the supplied index to type T and return it. If a parameter is
     * not included in the command line, it will get the default value, unless the parameter is mandatory (so with
     * an empty default string), in which case an exception is thrown.
     *
     * @tparam T The type to which the parameters should be cast.
     * @param indx The index of the Command_line_option parameter to retrieve.
     * @return A std::vector<T> containing the parameters.
     *
     * @throws std::string If the specified parameter is mandatory and not found in the command line.
     */
    template <typename T> std::vector<T> param(std::size_t indx = 0) const {
        std::vector<T> r;
        if (found())
            try {
                assert (indx < d_values.size());
                for (auto &val : d_values[indx])
                    r.push_back(val);
            }
        catch(std::string const& msg) {
            std::cerr << msg << " in:\n   " + *this + " ";
            for(auto &val : d_values[indx])
                std::cerr << val + " ";
            std::cerr << std::endl;
        }
        else {
            assert (indx == 0);
            for(auto &val : d_default_params)
                r.push_back(val);
        }
        return r;
    }
    
    /**
     * @brief Returns true if the Command_line_option was included in a Command_line.
     *
     * @return True if the Command_line_option was found; otherwise, false.
     */
    bool const found() const noexcept {return positions().size() != 0;}
    
    /**
     * @brief Returns the positions as indices of the command line arguments where the Command_line_option was found in a Command_line.
     *
     * @return Vector of of Command_line_option positions
     */
    std::vector<std::ptrdiff_t> const positions() const {return d_found_at;}
    
private:
    std::vector<Key_value> const d_default_params;
    std::string const d_help;
    std::vector<std::vector<Key_value>> d_values;
    std::vector<std::ptrdiff_t> d_found_at;
    
    struct Key_value : std::string {
        Key_value(std::string const& s) : std::string(s) {};
        
        template <typename T>
        operator const T() const {
            T r;
            if ((std::istringstream(std::string(*this)) >> r).fail())
                throw "Incorrect command line option parameter \"" + *this + "\"";
            return r;
        }
    };
};

/**
 * @class Command_line
 * @brief Represents a command-line utility for parsing command-line arguments with options and default values.
 *
 * The `Command_line` class is used for parsing command line with parameters, flags and options with mandatory and/or default values. It allows
 * defining command-line options and specify their default values, making it easy to retrieve and use command-line arguments.
 *
 * Example usage:
 * @code{.cpp}
 * int main(int argc, const char * argv[]) {
 *     using namespace jpa;
 *     try {
 *         Command_line input(argc, argv, {
 *             {"-size", "Area detector size in pixels", {"", "512"}},
 *             {"-pixel", "Pixel size in mm", {"0.05", "0.05"}},
 *             {"-dead_pixel", "Dead pixel coordinates.", {"", ""}},
 *             {"-verbose", "Give lots of output when this keyword is present."}
 *             });
 *         auto siz = input.option("-size").param<int>();
 *          std::vector<float> pix = input.option("-pixel").param<float>();
 *         std::vector<std::vector<int>> dead = input.option("-dead_pixel").all_params<int>();
 *         std::vector<std::string> inp_data = input.params();
 *         if (input.option("-verbose").found()){
 *             std::cout << "\nUsage:\n" << input.help() << std::endl;
 *         }
 *         std::cout << "detector type: " << input.arguments()[1] << "\n";
 *         std::cout << "size: " << siz[0] << " " << siz[1] << "\n";
 *         std::cout << "pitch: " << pix[0] << " " << pix[1] << "\n";
 *         std::cout << dead.size() << " dead pixels" << "\n";
 *         std::cout << "remaining data: ";
 *         for (auto& t : inp_data)
 *             std::cout << t << " ";
 *         std::cout << "\n";
 *     }
 *     catch(std::string const& msg) {
 *         std::cerr << msg << std::endl;
 *         exit(0);
 *     }
 * }
 * @endcode *
 *      With the command line:
 *      <pre>
 *      Medipix3 -pixel 0.049 0.051 -dead_pixel 420 102 -dead_pixel 421 102 -verbose 1 2 3 4
 *      </pre>
 *      this yields:
 * <pre>
 *    Running: CL_test
 *
 *    Usage:
 *    -size: Area detector size in pixels
 *        default: 512 512
 *    -pixel: Pixel size in mm
 *        default: 0.05 0.05
 *    -dead_pixel: Dead pixel coordinates.
 *        default: .*   (number of parameters unspecified; no parameters by default)
 *    -verbose: Give lots of output when this keyword is present.
 *
 *    detector type: Medipix
 *    size: 512 512
 *    pitch: 0.049 0.051
 *    2 dead pixels
 *    remaining data: 1 2 3 4
 *  </pre>
 */
class Command_line : private std::vector<std::string> {
public:
    /**
     * @brief Constructor for initializing the Command_line with command-line arguments and options with default values.
     *
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line argument strings.
     * @param options Vector of command line options,  and their explanations and default values.
     * @see Command_line_option
     */
    Command_line(int argc, const char *argv[], std::vector<Command_line_option> const& options) :
    vector([&]{
        vector<std::string> r;
        for (int i=0; i<argc; ++i)
            r.push_back(argv[i]);
        return r;
    }()),
    d_options(options)
    {
        for (auto& option : d_options) {
            for (auto cursor = std::find(begin(), end(), option); cursor != end(); cursor = std::find(cursor, end(), option)) {
                option.d_found_at.push_back(cursor - begin());
                option.d_values.push_back(std::vector<Command_line_option::Key_value>());
                auto next_option_p = std::find_first_of(++cursor, end(), d_options.begin(), d_options.end());
                if (option.d_default_params.size() != 0 && option.d_default_params[0] == ".*")
                    while (cursor != next_option_p)
                        option.d_values.back().push_back(*(cursor++));
                else {
                    for (auto& default_val : option.d_default_params)
                        option.d_values.back().push_back(default_val);
                    std::copy(cursor, std::min(cursor + option.d_default_params.size(), next_option_p), option.d_values.back().begin());
                }
                auto missing_values = option.d_default_params.size();
                for (auto const& v : option.d_values.back())
                    missing_values -= v.size() == 0 ? 0 : 1;
                if (missing_values == 1)
                    throw "There is 1 mandatory value missing for command line option " + option;
                else if (missing_values != 0)
                    throw "There are " + std::to_string(missing_values) + " mandatory values missing for command line option " + option;
            }
        }
        for (auto param = this->begin() + 1; param != this->end(); ) {
            Command_line_option opt = this->option(*param);
            if (opt == "")
                d_parameters.push_back(*(param++));
            else {
                auto next_opt = std::find_first_of(++param, end(), d_options.begin(), d_options.end());
                if (opt.d_default_params.size() != 0 && opt.d_default_params[0] == ".*")
                    param = next_opt;
                else
                    param = std::min(param + opt.d_default_params.size(), next_opt);
            }
        }
    }
    
    /**
     * @brief Retrieves the Command_line_option specified by option_name with the values from the command line.
     *
     * If the option is not found, or (some) values ar missing, the returned Command_line_option replacing missing values with their defaults.
     *
     * @param option_name The name of the option to retrieve.
     * @return The Command_line_option associated with the specified option_name.
     */
    Command_line_option const& option(std::string const& option_name) const noexcept {
        auto r = std::find(d_options.begin(), d_options.end(), option_name);
        if (r != d_options.end())
            return *std::find(d_options.begin(), d_options.end(), option_name);
        return d_invalid;
    }
    
    /**
     * @brief Returns the arguments of the command line.
     *
     * @return Arguments as a std::vector<std::string> const&.
     */
    std::vector<std::string> const& arguments() const noexcept { return *this; }
    
    /**
     * @brief Returns a help string using the information specified by the Command_line_options.
     *
     * @return A help string describing the available options and their default values.
     */
    std::string const help() const noexcept {
        std::string r;
        for (auto& k: d_options) {
            r += static_cast<std::string>(k) + ": " + k.d_help + '\n';
            if (k.d_default_params.size() > 0) {
                r += "    default: ";
                for (auto& p: k.d_default_params) {
                    if (p == "")
                        r += "(no default: mandatory parameter) ";
                    else
                        r += p + ' ';
                }
                if (k.d_default_params[0] == ".*")
                    r += "  (number of parameters unspecified; no parameters by default)";
                r += '\n';
            }
        }
        return r;
    }
    
    /**
     * @brief Returns a vector of strings containing all the arguments of a command line are not options or option parameters
     *
     * The parameters of the command line are all the arguments that are not options or option parameters.
     * Example:
     * @code{.cpp}
     *     std::vector<std::string> filenames = input.params();
     * @endcode
     * now 'filenames' will be a std::vector<std::string> if the command line is:
     * <pre>
     *  "process -verbose *"
     *  </pre>
     *
     * @return A vector of strings containing the data that are not options or option parameters.
     */
    std::vector<std::string> params() { return d_parameters; }
    
private:
    std::vector<Command_line_option> d_options;
    std::vector<std::string> d_parameters;
    Command_line_option const d_invalid = Command_line_option("","");
    
};

} //end namespace jpa

#endif /* CL_parser_h */
