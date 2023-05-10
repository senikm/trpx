//
//  XML_element.hpp
//  XML_element
//
//  Created by Jan Pieter Abrahams on 17/05/2019.
//

#ifndef XML_element_h
#define XML_element_h

#include <istream>
#include <sstream>
#include <algorithm>

// The XML_element class is derived from std::string. It scans an istream for an XML element with  a
// specified tag. The element can contain attibutes and data. The tag itself is stripped and only its
// attributes and data can be accessed as a string or parsed further by constructing another XML_element.
//
// After reading the specified Element from istream, the istream is left in the condition that exists
// after having read the Element, but not further. This allows mixing XML straightforwardly with
// binary data. For instance, after having read a XML metadata header, the file is positioned where the
// binary data starts.
//
// Constructor:
//  template <typename Stream>
//  XML_element(Stream&& is, std::string const& tag)
//      Stream can be any type of input stream.
//  XML_element(std::string xml_string, std::string const& tag)
//      Delegates to the constructor requiring a Stream, after creating a std::istringstream(xml_string).
//
// Member functions:
//  std::string attribute(std::string name)
//      Returns the named attribute as a std::string.
//  std::string const tag()
//      Returns the tag name that was specified in the construction.
//
//
// An example:
//
//    int main(int argc, const char * argv[]) {
//        std::string xml("<Element1 att0=\"plop\">\n  <Element2 att1=\"1\", att2=\"22\"/>\n</Element1>\n");
//        std::cout << "xml stream:\n" << xml << std::endl <<"Parsing produces:\n";
//        std::cout << "Element 1: " << XML_element(xml, "Element1") << std::endl;
//        std::cout << "Attribute att0 of element 1: " << XML_element(xml, "Element1").attribute("att0") << std::endl;
//        std::cout << "Attribute att1 of element 1 does not exist, so empty string is returned: " << '\"' << XML_element(xml, "Element1").attribute("att1") << '\"' << std::endl;
//        std::cout << "Attribute att1 of element 2: " << XML_element(xml, "Element2").attribute("att1") << std::endl;
//        std::cout << "Attribute att2 of element 2: " << XML_element(xml, "Element2").attribute("att2") << std::endl;
//        std::istringstream xml_stream(xml);
//        XML_element(xml_stream, "Element2");
//        std::string rest;
//        xml_stream >> rest;
//        std::cout << "After reading Element2, the xml_stream is now located ath the end of Element2. \nSubsequent output from theis stream produces:\n"<< rest << std::endl;
//
//        return 0;
//    }
//
// Produces the following output:
//
//    xml stream:
//    <Element1 att0="plop">
//      <Element2 att1="1", att2="22"/>
//    </Element1>
//
//    Parsing produces:
//    Element 1:
//      <Element2 att1="1", att2="22"/>
//    Attribute att0 of element 1: plop
//    Attribute att1 of element 1 does not exist, so an empty string is returned: ""
//    Attribute att1 of element 2: 1
//    Attribute att2 of element 2: 22
//    After reading Element2, the xml_stream is now located at the end of Element2.
//    Subsequent output from this stream produces:
//    </Element1>

class XML_element : public std::string {
public:
    
    template <typename Stream>
    XML_element(Stream&& istream, std::string const& tag) :
    std::string(),
    d_tag(tag) {
        std::string get_tag(d_tag.size(), '\0');
        for (; istream.good() && (get_tag != d_tag); istream.read(&get_tag[0], get_tag.size()))
            for (char c = '\0'; istream.good() && (c != '<'); istream >> c);
        do {
            get_tag.push_back('\0');
            istream.read(&get_tag.back(), 1);
        } while (istream.good() && (get_tag.size() != std::string::npos) && (get_tag.back() != '>'));
        if (istream.good() && (get_tag.size() < std::string::npos) && (get_tag.size() > 2) ) {
            if (get_tag[get_tag.size() - 2] == '/')
                d_attributes = get_tag.substr(d_tag.size(), get_tag.size() - 2);
            else {
                d_attributes = get_tag.substr(d_tag.size(), get_tag.size() - 1 - d_tag.size());
                do {
                    this->push_back('\0');
                    istream.read(&(this->back()), 1);
                } while (istream.good() && (this->size() != std::string::npos) && (this->back() != '>') &&
                         (!std::equal(this->begin() + this->size() - d_tag.size() - 3, this->begin() + this->size() - 3, std::string("</" + d_tag).begin())));
                if (!istream.good() || (this->size() == std::string::npos))
                    this->erase();
            }
        }
    }
    
    XML_element(std::string xml_string, std::string const& tag) :
    XML_element(std::istringstream(xml_string), tag)  {}
    
    std::string attribute(std::string const name) const {
        if (d_attributes.size() == 0)
            return std::string("");
        auto att = std::search(d_attributes.begin(), d_attributes.end(), name.begin(), name.end() );
        if (att == d_attributes.end())
            return std::string("");
        att = 1 + std::find(att, d_attributes.end(), '=');
        att = 1 + std::find(att, d_attributes.end(), '\"');
        auto att_end = std::find(att, d_attributes.end(), '\"');
        return d_attributes.substr(att - d_attributes.begin(), att_end - att);
    }
    
    std::string tag() {return d_tag;}
    
private:
    std::string const d_tag;
    std::string d_attributes;
};

#endif /* XML_element_h */
