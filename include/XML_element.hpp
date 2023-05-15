//
//  XML_element.hpp
//  XML_element
//
//  Created by Jan Pieter Abrahams on 17/05/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef XML_element_h
#define XML_element_h

#include <istream>
#include <string>
#include <sstream>
#include <string_view>
#include <limits>
#include <algorithm>
#include <variant>

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
//  XML_element(std::istream& is, std::string const& tag)
//  XML_element(std::istream&& is, std::string const& tag)
//      istr is skipped until '<' + tag + '>' is encountered. Then all the XMA data are parsed into the
//      XML_element, until '</' + tag + '>' is encountered, and scanning the file stops after having read
//      the end tag. Any attributes of tag are parsed into XML_element. Tags can be empty (when the end with
//      “/>”), in which case only the attributes of tag are parsed into the XML_element.
//  XML_element(std::string const& xml_string)
//  XML_element(std::string const& xml_string, std::string const& tag)
//      Delegates to the constructor requiring a std::itream, after creating a std::istringstream(xml_string).
//
// Member functions:
//  std::string const attribute(std::string const& att_name) const
//      Returns the attribute of the tag of XML_element that has the name 'att_name'. An XML_element line does
//      not have to have attributes. If none of the attributes is named 'att_name', an empty string is returned.
//  std::string const attribute(std::size_t n) const
//      Returns the n-th attribute of an XML_element. If n is larger than the number of attributes, an empty
//      string is returned.
//  std::string const tag()
//      Returns the tag name of the XML_element that was used for extracting it from the XML data. If no tag name
//      was specified when the XML_element was created, an empty string is returned.
//  std::string const tag(std::string const& tag)
//      Returns the XML data encoded by the first occurrence of requested tag as an XML_element.
//  std::vector<XML_element> const multitag(std::string const& tag)
//      Returns all requested tags as a vector, in the order in which they occur in original XML data.
//  std::vector<T> const value<T>(std::string const& tag)
//      Returns a vector of the requested type, containing the numerical values of the requested tag of the
//      XML_element.
//  std::vector<std::vector<T>> const multivalue<T>(std::string const& tag)
//      Returns a vector of all the requested value<T>s.
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
//      <!-- comment " > -->
//      <values> 1 2 3 4 </values>
//      <Element2 att1="1", att2="22"/>
//    </Element1>
//
//    Parsing produces:
//    Element 1:
//      <!-- comment " > -->
//      <values> 1 2 3 4 </values>
//      <Element2 att1="1", att2="22"/>
//    Attribute att0 of element 1: plop
//    Attribute att1 of element 1 does not exist, so an empty string is returned: ""
//    Attribute att1 of element 2: 1
//    Attribute att2 of element 2: 22
//    After reading Element2, the xml_stream is now located at the end of Element2.
//    Subsequent output from this stream produces:
//    </Element1>

//template <typename Stream = std::monostate>
class XML_element : public std::string {
    static auto const all = std::numeric_limits<std::streamsize>::max();
    std::monostate none;
    
    class s_param {
        friend XML_element;// <Stream>;
        s_param(std::string const& s, std::size_t from, std::size_t size) : s(s), from(from), size(size) {}
        friend void swap(s_param& left, s_param &right) {std::swap(left.from, right.from); std::swap(left.size, right.size);}
        std::string const& s;
        std::size_t from;
        std::size_t size;
        
        std::size_t const to() const noexcept {return from + size;}
        
    public:
        s_param(s_param const& other) : s(other.s), from(other.from), size(other.size) {}
        constexpr auto& operator=(s_param const& other) noexcept {from = other.from; size = other.size; return *this;}

        template <typename T>
        operator const T() const {
            T r;
            std::string const s1(s, from, size);
            std::istringstream(s1) >> r;
            return r;
        }

        operator const std::string() const { return std::string(s, from, size); }
        
        std::string const tag() const {
            if (s[from] != '<') return "";
            std::size_t tagsize = s.find_first_of(" >", from) - from - 1;
            if (s[from + tagsize] == '/') --tagsize;
            return std::string(s, from + 1, tagsize);
        }
        
        operator  XML_element() const { return XML_element(std::string(*this), tag()); }
    };

public:
    XML_element(std::istream& istr, std::string const& tag) :
    XML_element(std::move(istr), tag) {}
     
    XML_element(std::istream&& istr, std::string const& tag) :
    std::string(),
    d_tag(tag),
    d_attributes(_get_attributes(istr)) {
        if (istr.good() && !_prev_is(istr, "/>")) { // only do if this is not an empty element
            static_cast<std::string&>(*this) = _read_element(istr, tag);
            d_param = _find_all_params(std::string(*this));
        }
    }
    
    XML_element(std::istream& istr) :
    std::string(std::istreambuf_iterator<char>(istr), std::istreambuf_iterator<char>()),
    d_tag(""),
    d_attributes(""),
    d_param(_find_all_params(std::string(*this)))
    {}
    
    XML_element(std::string const& xml_string) :
    std::string(xml_string),
    d_tag(""),
    d_attributes(""),
    d_param(_find_all_params(std::string(*this))){}
    
    XML_element(std::string const& xml_string, std::string const& tag) :
    XML_element(std::istream(std::istringstream(xml_string).rdbuf()), tag)  {}

    std::string const attribute(std::string const& name) const noexcept {
        for (int i = 0; i <= (d_attributes.size() - name.size() - 3); ++i) {
            if ((d_attributes[i + name.size()] == '=') && (name == d_attributes.substr(i, name.size()))) {
                auto strt = 1 + (i += name.size() + 1);
                for (char q = d_attributes[i]; d_attributes[++i] != q;);
                return d_attributes.substr(strt, i - strt);
            }
            else if (d_attributes[i] == '=')
                for (char q = d_attributes[++i]; d_attributes[++i] != q;);
        }
        return "";
    }

    std::string const attribute(std::size_t const& n) const noexcept {
        std::string r;
        std::size_t i=0;
        std::size_t found = 0;
        for ( ; (i <= d_attributes.size()) && (found != (n+1)); ++i, ++found)
            i = d_attributes.find('=');
        if (found == (n+1)) {
            i = d_attributes.find_first_of("'\"", i);
            for (std::size_t j = i+1 ; d_attributes[j] != d_attributes[i]; ++j)
                r.push_back(d_attributes[j]);
        }
        return r;
    }

    std::string tag() {return d_tag;}
    
    std::vector<XML_element> const multitag(std::string const& tag, std::size_t max_tags = -1) const noexcept {
        std::vector<XML_element> r;
        for (int i=0; (max_tags != 0) && (i != d_param.size()); ++i)
            if (d_param[i].tag() == tag) {
                r.push_back(d_param[i]);
                --max_tags;
            }
        return r;
    }

    template <typename T>
    std::vector<T> const value(std::string const& tag) const noexcept { return multitag(tag, 1)[0]; }

    template <typename T>
    std::vector<std::vector<T>> const multivalue(std::string const& tag, std::size_t max_tags = -1) const noexcept {
        std::vector<std::vector<T>> r;
        auto tmp = multitag(tag, max_tags);
        for (auto const& t: tmp)
            r.push_back(t);
        return r;
    }


    XML_element const tag(std::string const& tag) const noexcept { return multitag(tag, 1)[0]; }

    template <typename T>
    operator std::vector<T>() const {
        std::vector<T> r;
        std::istringstream str(std::string(*this));
        for (T tmp; (str >> tmp).good(); ) r.push_back(tmp);
        return r;
    }
        
private:
    std::vector<s_param> d_param;
    std::string const d_tag;
    std::string d_attributes;

    std::string _get_attributes(std::istream& istr) {
        if (d_tag == "")
            return "";
        _find_tag(istr, d_tag);
        std::string r = std::string(_read_upto(istr, '>'), d_tag.size() + 1);
        r.pop_back();
        if ((r.size() > 1) && (r.back() == '/'))
            r.pop_back();
        return r;
    }

    void _find_tag(std::istream& istr, std::string const& tag) {
        for (istr.ignore(all, '<'); !_next_is(istr, tag); istr.ignore(all, '<')) {
            if (_next_is(istr, "![CDATA["))
                for (_read_upto(istr, ']'); !_next_is(istr, "]>"); _read_upto(istr, ']'));
            else if (_next_is(istr, "!--"))
                for (_read_upto(istr, '-'); !_next_is(istr, "->"); _read_upto(istr, '-'));
        }
        istr.unget();
    }

    std::vector<s_param> _find_comments_and_data(std::string const& s) {
        std::vector<s_param> r;
        for (s_param p = _find_interval(s, "<!--", "-->"); p.from  != std::string::npos; p = _find_interval(s, "<!--", "-->", p.to()))
            r.push_back(p);
        for (s_param p = _find_interval(s, "<![CDATA[", "]]>"); p.from  != std::string::npos; p = _find_interval(s, "<![CDATA[", "]]>", p.to()))
            r.push_back(p);
        std::sort(r.begin(), r.end(), [](s_param const& left, s_param const& right) {return left.from < right.from;});
        for (int i = 0; i!=r.size(); ++i)
            for (int j = i+1; (j!=r.size()) && (r[j].from < (r[i].from + r[i].size)); ++j)
                r.erase(r.begin() + j--);
        return r;
    }
    
    std::vector<s_param> _find_all_params(std::string const& s) {
        std::vector<s_param> skip = _find_comments_and_data(s);
        std::vector<s_param> r;
        for (s_param param(*this, _ignore(s, 0, skip), 1); param.from < s.size(); param.from = _ignore(s, param.to(), skip)) {
            if (s[param.from] != '<') {
                for (param.size = 1; !_is_white(s[param.to()]) && (s[param.to()] != '<'); ++param.size);
                r.push_back(param);
            }
            else {
                param.size = s.find('>', param.from) + 1 - param.from;
                if (s[param.to() - 2] == '/')
                    r.push_back(param);
                else {
                    std::size_t end_tag_size = s.find_first_of(" >", param.from) - param.from - 1;
                    std::string const end_tag = "</" + s.substr(param.from + 1, end_tag_size) + '>';
                    param.size = s.find(end_tag, param.to()) - param.from + end_tag.size();
                    for (auto& ignore: skip)
                        if ((param.to() > ignore.from) && (param.to() < ignore.to()))
                            param.size = s.find(end_tag, param.to()) - param.from;
                    r.push_back(param);
                }
            }
        }
        r.insert(r.begin(), skip.begin(), skip.end());
        std::sort(r.begin(), r.end(), [](s_param const& left, s_param const& right) {return left.from < right.from;});
        return r;
    }
    
    s_param _find_interval(std::string const& s, std::string const& from, std::string const& to, std::size_t pos = 0) {
        std::size_t param_start = s.find(from, pos);
        if (param_start == std::string::npos)
            return s_param(*this, param_start, 0);
        else
            return s_param(*this, param_start, s.find(to, param_start + from.size()) - param_start + to.size());
    }
    
    std::size_t _ignore(std::string const& s, std::size_t pos, std::vector<s_param> skip = std::vector<s_param>()) {
        std::size_t start = pos;
        pos = start - 1;
        std::size_t i = 0;
        do {
            pos = start;
            while (_is_white(s[start]) && (start < s.size()))
                ++start;
            for ( ; (i != skip.size()) && (start >= skip[i].from); ++i)
                if ((start >= skip[i].from) && (start < skip[i].to()))
                    start = skip[i].to();
        } while ((pos != start) && (start < s.size()));
        return start;
    }

    std::string _read_upto(std::istream& istr, char const c) {
        std::string r;
        for (r.push_back(istr.get()); istr.good() && (r.back() != c); r.push_back(istr.get()));
        return r;
    }
    
    std::string _read_element(std::istream& istr, std::string const& tag) {
        std::string element = _read_upto(istr, '<');
        for (; !_next_is(istr, '/' + tag + '>'); element += _read_upto(istr, '<')) {
            if (_next_is(istr, "![CDATA["))
                for (element += _read_upto(istr, '>'); !element.compare(element.size() - 3, 3, "]]>"); element += _read_upto(istr, '>'));
            else if (_next_is(istr, "!--"))
                for (element += _read_upto(istr, '>'); !element.compare(element.size() - 3, 3,"-->"); element += _read_upto(istr, '>'));
        }
        element.pop_back();
        return element;
    }
    
    bool _is_white(char const c) const noexcept {return (c == 0x20) || (c==0x09) || (c==0x0D) || (c==0x0A);}
    
    bool _next_is(std::istream& istr, std::string const& s) {
        std::string tst(s.size(), 0);
        istr.read(&tst[0], s.size());
        istr.seekg(-s.size(), std::ios_base::cur);
        return tst == s;
    }
    
    bool _prev_is(std::istream& istr, std::string const& s) {
        std::string tst(s.size(), 0);
        istr.seekg(-s.size(), std::ios_base::cur);
        istr.read(&tst[0], s.size());
        return tst == s;
    }
};

#endif /* XML_element_h */
