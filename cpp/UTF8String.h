#ifndef UTF8STRING_H_ //Include guard
#define UTF8STRING_H_

//* DO NOT REMOVE - ONLY WAY TO AVOID CIRCULAR DEPENDENCIES BETWEEN Reactor.h and UTF8String. *//
//Check to see if REACTOR_API has been defined yet, allows us to defined the interface entirely
//within Reactor.h without causing a circular dependency between UTF8String.h
#ifndef REACTOR_API
#ifdef REACTOR_EXPORTS
#define REACTOR_API __declspec(dllexport)
#else
#define REACTOR_API __declspec(dllimport)
#endif
#endif
//* END OF DO NOT REMOVE BLOCK *//

#include <string>
#include <iostream>
namespace rct
{

//!  UTF8 String Class
/*!
 * Common string type class used throughout the SDK
 * to minimize code page issues within Windows and in between
 * different platforms.  Code page used internally is UTF-8, however
 * this is limited to converting the ASCII code page set to the
 * UTF-8 code page set.  LibICU or other measures would need to be
 * integrated into this class for full internationalization support.
 * This class properly handles the cross platform issue of the wchar_t size being
 * either 2 bytes or 4 bytes in width.
 * If the string is valid, it has been populated
 */
class REACTOR_API UTF8String
{
    public:
        typedef wchar_t UTF8Char;
        typedef std::basic_string<UTF8Char>::iterator UTF8Iterator;
        typedef std::basic_string<UTF8Char>::const_iterator UTF8ConstIterator;
    private:
        std::basic_string<UTF8Char> internalStorage_;
        unsigned int crc_;
        unsigned int characterLength_;
        unsigned int sizeInBytes_;
        bool empty_;
        bool valid_;
        bool dirty_;
        bool crcOn_;
        bool dirtySz_;
        bool dirtyLen_;
    private:
        //CRC computation method
        void computeLengthAndSize();
        void computeCRC();
    public:
        //Constructors
        UTF8String();
        UTF8String(const char* inStr);
        UTF8String(const wchar_t* inStr);
        UTF8String(const std::string& inStr);
        UTF8String(const std::wstring& inStr);
        explicit UTF8String(size_t len, UTF8Char ch);

        //Destructor
        ~UTF8String();

        //Copy constructor and assignment operators
        UTF8String(const UTF8String& rhs);
        UTF8String& operator=(const UTF8String& rhs);
        UTF8String& operator=(const char* rhs);
        UTF8String& operator=(const wchar_t* rhs);
        UTF8String& operator=(const std::string& rhs);
        UTF8String& operator=(const std::wstring& rhs);

        //Equality Operators
        inline bool operator==(const UTF8String& rhs);
        inline bool operator==(const char* rhs);
        inline bool operator==(const wchar_t* rhs);
        inline bool operator==(const std::string& rhs);
        inline bool operator==(const std::wstring& rhs);

        //Inequality Operators
        inline bool operator!=(const UTF8String& rhs);
        inline bool operator!=(const char* rhs);
        inline bool operator!=(const wchar_t* rhs);
        inline bool operator!=(const std::string& rhs);
        inline bool operator!=(const std::wstring& rhs);

        //Addition Operators
        UTF8String operator+(const UTF8String& rhs);
        UTF8String operator+(const char* rhs);
        UTF8String operator+(const wchar_t* rhs);
        UTF8String operator+(const std::string& rhs);
        UTF8String operator+(const std::wstring& rhs);

        //Addition Operators (LHS)
        //friend REACTOR_API const UTF8String operator+(const char* lhs, const UTF8String& rhs);
        //friend REACTOR_API const UTF8String operator+(const wchar_t *lhs, const UTF8String& rhs);
        //friend REACTOR_API const UTF8String operator+(const std::string& lhs, const UTF8String& rhs);
        //friend REACTOR_API const UTF8String operator+(const std::wstring& lhs, const UTF8String& rhs);

        //Addition and Assignment
        UTF8String& operator+=(const UTF8String& rhs);
        UTF8String& operator+=(const char* rhs);
        UTF8String& operator+=(const wchar_t* rhs);
        UTF8String& operator+=(const std::string& rhs);
        UTF8String& operator+=(const std::wstring& rhs);

        //Array operator
        UTF8Char& operator[](unsigned int idx);
        const UTF8Char& operator[](unsigned int idx) const;

        //Search methods
        bool contains(const UTF8String& rhs) const;
        bool contains(const char* rhs) const;
        bool contains(const wchar_t* rhs) const;
        bool contains(const std::string& rhs) const;
        bool contains(const std::wstring& rhs) const;

        //Index search methods
        size_t indexOf(const UTF8String& rhs) const;
        size_t indexOf(const char* rhs) const;
        size_t indexOf(const wchar_t* rhs) const;
        size_t indexOf(const std::string& rhs) const;
        size_t indexOf(const std::wstring& rhs) const;

        //String search methods
        inline const rct::UTF8String::UTF8Char* strStr(const UTF8String& rhs) const;
        inline const rct::UTF8String::UTF8Char* strStr(const char* rhs) const;
        inline const rct::UTF8String::UTF8Char* strStr(const wchar_t* rhs) const;
        inline const rct::UTF8String::UTF8Char* strStr(const std::string& rhs) const;
        inline const rct::UTF8String::UTF8Char* strStr(const std::wstring& rhs) const;
        
        //Iterators
        UTF8String::UTF8Iterator begin();
        UTF8String::UTF8Iterator end();

        //Const Iterators
        UTF8String::UTF8ConstIterator cbegin() const;
        UTF8String::UTF8ConstIterator cend() const;

        //Stream Output Operators
        friend REACTOR_API std::ostream& operator<<(std::ostream& oStream, const UTF8String& utf8); 
        friend REACTOR_API std::ostream& operator<<(std::ostream& oStream, UTF8String& utf8);


        //Set functions
        inline bool Set(const char* inStr);
        inline bool Set(const wchar_t* inStr);
        inline bool Set(const std::string& inStr);
        inline bool Set(const std::wstring& inStr);

        //Utility functions
        inline void clear();
        inline bool isEmpty() const;
        inline bool isValid() const;
        inline bool isDirty(bool reCalc=false);
        inline size_t length() const;
        inline size_t size() const;
        inline unsigned int crc();
        inline void SetCRCOn(bool crcFlag);

        //Accessors
        inline const std::basic_string<UTF8Char>& str() const;
        inline const UTF8Char* c_str() const;

        //Conversion by copy accessors (C++0x should make these move accessors)
        std::string nstr() const;
        std::wstring nwstr() const;

        //Conversion by reference accessors
        inline bool cnstr(std::string& output) const;
        inline bool Narrow(std::string& output) const;
    };

    static const UTF8String EmptyString;
}

#endif //UTF8STRING_H_
