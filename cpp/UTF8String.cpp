#include "stdafx.h"
#include "UTF8String.h"
#include <boost/iterator.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <functional>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <stlsoft/shims/access/string.hpp>
#include <boost/crc.hpp>
#include "StringUtilities.h"

namespace rct
{
    static const size_t UTF8Sz = sizeof(UTF8String::UTF8Char);
    //!Internal wide character allocation method - utilizes built in std::basic_string allocators
    /*!
      \param len Number of characters to allocate
      \param fullPreserve Create the mask in a way that the upper bytes of the characters are preserved
      \param mask Mask value to set
      \param blank Blank character utilized to pre-fill the string
      \param internalStorage The std::basic_string object storing the characters
     */
    template<typename S, typename T>
    bool AllocateInternal(            
        unsigned int len, 
        bool fullPreserve, 
        S& mask, T& blank,
        std::basic_string<T>& internalStorage)
    {
        //Ensure len is not zero
        if (len == 0)return(false);
        size_t maskSize = sizeof(S);
        size_t blankSize = sizeof(T);
        //Set mask based on 2 bytes per character
        if (maskSize == 2)
        {
            mask = ((fullPreserve) ? 0xffff : 0x00ff);
        }
        //Set mask based on 4 bytes per character
        else if (maskSize == 4)
        {
            mask = ((fullPreserve) ? 0xffffffff : 0x000000ff);
        }
        //If not two bytes or four bytes per internal character allocation point,
        //we cannot allocate the string
        else
        {
            return(false);
        }

        //Set blank based on the size of the required blank character
        if (blankSize == 2)
        {
            blank = 0x0000;
        }
        else if (blankSize == 4)
        {
            blank = 0x00000000;
        }
        //Invalid blank size
        else
        {
            return(false);
        }

        //If we get here, we have either a two byte or four byte allocation point
        //with a properly sized and blank fill character
        try
        {
            internalStorage = std::basic_string<T>(
                static_cast<std::basic_string<T>::size_type>(len), 
                static_cast<T>(blank));
            return(true);
        }
        catch(std::runtime_error& rEx)
        {
            //Really bad things have happened, return false, no allocation
            rEx; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //Bad things have happened, return false, no allocation
            eX; //TODO: Log it
            return(false);
        }
        //Something is not right if we get here
        return(false);
    }

    //!Internal wide character allocation method - utilizes built in std::basic_string allocators
    /*!
      \param sizeIncrease Number of extra characters to allocate
      \param fullPreserve Create the mask in a way that the upper bytes of the characters are preserved
      \param mask Mask value to set
      \param blank Blank character utilized to pre-fill the string
      \param internalStorage The std::basic_string object storing the characters
     */
    template<typename S, typename T>
    bool ReAllocateInternal(            
        unsigned int sizeIncrease, 
        bool fullPreserve, 
        S& mask, T& blank,
        std::basic_string<T>& internalStorage)
    {
        //Ensure len is not zero
        if (sizeIncrease == 0)return(false);
        size_t maskSize = sizeof(S);
        size_t blankSize = sizeof(T);
        //Set mask based on 2 bytes per character
        if (maskSize == 2)
        {
            mask = ((fullPreserve) ? 0xffff : 0x00ff);
        }
        //Set mask based on 4 bytes per character
        else if (maskSize == 4)
        {
            mask = ((fullPreserve) ? 0xffffffff : 0x000000ff);
        }
        //If not two bytes or four bytes per internal character allocation point,
        //we cannot allocate the string
        else
        {
            return(false);
        }

        //Set blank based on the size of the required blank character
        if (blankSize == 2)
        {
            blank = 0x0000;
        }
        else if (blankSize == 4)
        {
            blank = 0x00000000;
        }
        //Invalid blank size
        else
        {
            return(false);
        }

        //If we get here, we have either a two byte or four byte allocation point
        //with a properly sized and blank fill character
        try
        {
            std::basic_string<T>::size_type curSize = internalStorage.size();
            std::basic_string<T>::size_type incSize = static_cast<std::basic_string<T>::size_type>(sizeIncrease);
            std::basic_string<T>::size_type newSize = curSize + incSize;
            //Ensure the string has at least the proper amount of storage space
            if (internalStorage.capacity() < newSize)
            {
                internalStorage.reserve(newSize);
            }
            //Append the extra number of blank characters to the string
            internalStorage.append(sizeIncrease, blank);
            return(true);
        }
        catch(std::runtime_error& rEx)
        {
            //Really bad things have happened, return false, no allocation
            rEx; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //Bad things have happened, return false, no allocation
            eX; //TODO: Log it
            return(false);
        }
        //Something is not right if we get here
        return(false);
    }

    //! Copies utf-8 code points from a memory location and into the destination std::basic_string object
    /*! Each code point is transformed via the mask prior to placement in the destination string
     */
    template <typename S, typename T>
    bool CopyData(S* src, unsigned int byteLength, T mask, std::basic_string<T>& dest)
    {
        //Create iterator to push utf-8 characters into internal storage
        try
        {
            S* inputPtr = src;
            std::basic_string<T>::iterator intCurIter = dest.begin();
            std::basic_string<T>::iterator intEndIter = dest.end();    
            while(intCurIter != intEndIter)
            {
                char32_t codePoint = *inputIter;
                //Trim the point data according to the mask set
                *intCurIter = static_cast<T>(codePoint & mask);
                //Increment the iterators
                ++intCurIter;
                ++inputPtr;
            }
            return(true);
        }
        catch(std::runtime_error& rErr)
        {
            //We reached the end of the input data by error
            rErr; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //If we hit this, something is wrong with the input data
            eX; //TODO: Log it
            return(false);
        }
        return(false);
    }

    //! Copies raw character data (non-utf8 data) from a memory source to the destination std::basic_string object
    /*! Ensures characters being copied are placed in a unicode standard variable prior to insertion into the std::basic_string
        \param src Pointer to memory location holding the character data
        \param byteLength Number of bytes (not characters) to be copied
        \param mask Mask utilized to transform the characters prior to being placed in the std::basic_string object
        \param dest Destination string object
     */
    template <typename S, typename T>
    bool CopyRawData(S* src, unsigned int byteLength, T mask, std::basic_string<T>& dest)
    {
        //Create iterator to push raw characters into internal storage
        try
        {
            S* srcPtr = src;
            S* endPtr = src+byteLength;
            std::basic_string<T>::iterator intCurIter = dest.begin();
            std::basic_string<T>::iterator intEndIter = dest.end();    
            while(srcPtr != endPtr)
            {
                S curChar = *srcPtr;
                //Trim the point data according to the mask set
                *intCurIter = static_cast<T>(curChar & mask);
                //Increment the pointer and the iterator
                ++srcPtr;
                ++intCurIter;
            }
            return(true);
        }
        catch(std::runtime_error& rEx)
        {
            //We reached the end of the input data by error
            rEx; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //If we hit this, something is wrong with the input data
            eX; //TODO: Log it
            return(false);
        }
        return(false);
    }

    //! Copies utf-8 code points from a memory location and into the index adjusted destination std::basic_string object
    /*! Each code point is transformed via the mask prior to placement in the destination string
       
     */
    template <typename S, typename T>
    bool AppendData(S* src, unsigned int destIdx, unsigned int byteLength, T mask, std::basic_string<T>& dest)
    {
        //Create iterator to push utf-8 characters into internal storage 
        try
        {
            S* inputPtr = src;
            std::basic_string<T>::iterator intCurIter = dest.begin() + destIdx;
            std::basic_string<T>::iterator intEndIter = dest.end();    
            while(intCurIter != intEndIter)
            {
                char32_t codePoint = *inputIter;
                //Trim the point data according to the mask set
                *intCurIter = static_cast<T>(codePoint & mask);
                //Increment the iterators
                ++intCurIter;
                ++inputPtr;
            }
            return(true);
        }
        catch(std::runtime_error& rErr)
        {
            //We reached the end of the input data by error
            rErr; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //If we hit this, something is wrong with the input data
            eX; //TODO: Log it
            return(false);
        }
        return(false);
    }

    //! Copies raw character data (non-utf8 data) from a memory source to the destination std::basic_string object
    /*! Ensures characters being copied are placed in a unicode standard variable prior to insertion into the std::basic_string
        \param src Pointer to memory location holding the character data
        \param byteLength Number of bytes (not characters) to be copied
        \param mask Mask utilized to transform the characters prior to being placed in the std::basic_string object
        \param dest Destination string object
     */
    template <typename S, typename T>
    bool AppendRawData(S* src, unsigned int destIdx, unsigned int byteLength, T mask, std::basic_string<T>& dest)
    {
        //Create iterator to push raw characters into internal storage
        try
        {
            S* srcPtr = src;
            S* endPtr = src+byteLength;
            std::basic_string<T>::iterator intCurIter = dest.begin()+destIdx;
            std::basic_string<T>::iterator intEndIter = dest.end();    
            while(srcPtr != endPtr)
            {
                S curChar = *srcPtr;
                //Trim the point data according to the mask set
                *intCurIter = static_cast<T>(curChar & mask);
                //Increment the pointer and the iterator
                ++srcPtr;
                ++intCurIter;
            }
            return(true);
        }
        catch(std::runtime_error& rEx)
        {
            //We reached the end of the input data by error
            rEx; //TODO: Log it
            return(false);
        }
        catch(std::exception& eX)
        {
            //If we hit this, something is wrong with the input data
            eX; //TODO: Log it
            return(false);
        }
        return(false);
    }

    template<class S, class T>
    bool ConvertIn(S* data, unsigned int byteLength, T* begin, T* end, bool add, std::basic_string<UTF8String::UTF8Char>& internalStorage)
    {
        if (data == nullptr)return(false);
        bool rt = false;
        //unsigned int byteLength = strlen(data);
        //See if the data is already utf8
        /*if (*data != (S)'\0') 
        {
            char32_t maskIn = 0;
            wchar_t blank;
            //If we are not adding, or our internalStorage is empty,
            //perform a normal allocate and copy
            if (!add || internalStorage.empty())
            {
                if (AllocateInternal<char32_t, UTF8String::UTF8Char>(
                    wcslen(data), 
                    true, 
                    maskIn, 
                    blank, 
                    internalStorage))
                {
                    UTF8String::UTF8Char mask = static_cast<UTF8String::UTF8Char>(maskIn);
                    rt = CopyData<S, UTF8String::UTF8Char>(data, byteLength, mask, internalStorage);
                }
            }
            //Otherwise we are adding
            else
            {
                unsigned int destIdx = internalStorage.size();
                if (ReAllocateInternal<char32_t, UTF8String::UTF8Char>(
                    wcslen(data),
                    true,
                    maskIn,
                    blank,
                    internalStorage))
                {
                    UTF8String::UTF8Char mask = static_cast<UTF8String::UTF8Char>(maskIn);
                    rt = AppendData<S, UTF8String::UTF8Char>(data, destIdx, byteLength, mask, internalStorage);
                }
            }
        }
        //Not utf-8 input (must assume it is ASCII/ANSI code page), 
        //up convert characters to wchar_t size and store
        else
        {*/
            unsigned int maskIn = 0;
            UTF8String::UTF8Char blank;
            //We must up convert characters from single byte to wide char
            //If we are not adding or our internalStorage is empty
            //perform a normal allocate and copy
            if (!add || internalStorage.empty())
            {
                if (AllocateInternal<unsigned int, UTF8String::UTF8Char>(
                    byteLength, 
                    false, 
                    maskIn, 
                    blank, 
                    internalStorage))
                {
                    UTF8String::UTF8Char mask = static_cast<UTF8String::UTF8Char>(maskIn);
                    rt = CopyRawData<S, UTF8String::UTF8Char>(data, byteLength, mask, internalStorage);
                }
            }
            else
            {
                unsigned int destIdx = internalStorage.size();
                if (ReAllocateInternal<unsigned int, UTF8String::UTF8Char>(
                    byteLength,
                    false,
                    maskIn,
                    blank,
                    internalStorage))
                {
                    UTF8String::UTF8Char mask = static_cast<UTF8String::UTF8Char>(maskIn);
                    rt = AppendRawData<S, UTF8String::UTF8Char>(data, destIdx, byteLength, mask, internalStorage);
                }
            }
        //}
        return(rt);
    }

    void UTF8String::computeLengthAndSize()
    {
        if (!this->valid_ || this->empty_)return;
        if (this->dirtyLen_)
        {
            this->characterLength_ = wcslen(this->internalStorage_.data());
        }
        if (this->dirtySz_ || this->dirtyLen_)
        {
            this->sizeInBytes_ = UTF8Sz * (this->characterLength_+1);
        }
        this->dirtyLen_ = false;
        this->dirtySz_ = false;
    }

    //! Compute CRC for the string for easy comparison purposes,
    /*! Computation of length and size is also performed
     *! if necessary
     */
    void UTF8String::computeCRC()
    {
        //Before creating a new CRC, ensure we have data to process and if we even
        //need to compute a CRC
        if (this->dirtyLen_ || this->dirtySz_)
        {
            this->computeLengthAndSize();
        }
        if (this->dirty_ && this->crcOn_)
        {
            this->crc_ = 0;
            boost::crc_32_type crc32;
            //Process string character buffer directly
            crc32.process_block(&this->internalStorage_[0], &this->internalStorage_[this->characterLength_-1]);
            this->crc_ = crc32.checksum();
            this->dirty_ = false;
        }
    }

    //! Default constructor
    /*! 
     *  Creates an empty string
     */
    UTF8String::UTF8String() : 
        internalStorage_(), 
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(false),
        crcOn_(true),
        dirtySz_(false),
        dirtyLen_(false)
    {}

    //! Constructor - converts from a normal character buffer
    /*!
     *  Creates a utf-8 string via conversion of a narrow character string
     *  \param inStr Raw character pointer to string data
     */
    UTF8String::UTF8String(const char* inStr) : 
        internalStorage_(), 
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(true),
        crcOn_(true),
        dirtySz_(true),
        dirtyLen_(true)
    {
        Set(inStr);
    }

    //! Constructor - converts from a wide character buffer
    /*!
     *  Creates a utf-8 string via conversion of a wide character string
     *  \param inStr Raw wide character pointer to wide string data
     */
    UTF8String::UTF8String(const wchar_t* inStr) : 
        internalStorage_(), 
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(true),
        crcOn_(true),
        dirtySz_(true),
        dirtyLen_(true)
    {
        Set(inStr);
    }

    //! Constructor - converts from a std::string object
    /*!
     *  Creates a utf-8 string via conversion of a std::string object
     *  \param inStr Standard string object reference
     */
    UTF8String::UTF8String(const std::string& inStr) : 
        internalStorage_(), 
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(true),
        crcOn_(true),
        dirtySz_(true),
        dirtyLen_(true)
    {
        Set(inStr);
    }

    //! Constructor - converts from a std::wstring object
    /*!
     *  Creates a utf-8 string via conversion of a std::wstring object
     *  \param inStr Standard wide string object reference
     */
    UTF8String::UTF8String(const std::wstring& inStr) : 
        internalStorage_(), 
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(true),
        crcOn_(true),
        dirtySz_(true),
        dirtyLen_(true)
    {
        Set(inStr);
    }

    //! Constructor - creates a string of a specific size with a specific character
    /*!
     *  \param len - Length, in characters, of string to create
     *  \param ch  - Character to fill the string with
     */
    UTF8String::UTF8String(size_t len, UTF8Char ch) :
        internalStorage_(len, ch),
        crc_(0),
        characterLength_(0),
        sizeInBytes_(0),
        empty_(true), 
        valid_(true),
        dirty_(true),
        crcOn_(true),
        dirtySz_(true),
        dirtyLen_(true)
    {
        empty_ = internalStorage_.empty();
        valid_ = true;
        computeCRC();
    }
    

    //! Constructor - overriden copy constructor
    /*!
     *  Creates a copy of the UTF8String object via
     *  manual copying of internal class members
     *  \param inStr Object to copy
     */
    UTF8String::UTF8String(const UTF8String& rhs) : 
        internalStorage_(rhs.internalStorage_), 
        crc_(rhs.crc_),
        characterLength_(rhs.characterLength_),
        sizeInBytes_(rhs.sizeInBytes_),
        valid_(rhs.valid_),
        empty_(rhs.empty_),
        dirty_(rhs.dirty_),
        crcOn_(rhs.crcOn_),
        dirtySz_(rhs.dirtySz_),
        dirtyLen_(rhs.dirtyLen_)
    {
        computeCRC();
    }

    //! Overloaded assignment operator - UTF8String
    /*!
     *  Copies the UTF8String object via
     *  manual copying of internal class members
     *  \param rhs Object to assign
     */
    UTF8String& UTF8String::operator=(const UTF8String& rhs)
    {
        if (this != &rhs)
        {
            internalStorage_ = rhs.internalStorage_;
            characterLength_ = rhs.characterLength_;
            sizeInBytes_ = rhs.sizeInBytes_;
            crc_ = rhs.crc_;
            valid_ = rhs.valid_;
            empty_ = rhs.empty_;
            dirty_ = rhs.dirty_;
            crcOn_ = rhs.crcOn_;
            dirtySz_ = rhs.dirtySz_;
            dirtyLen_ = rhs.dirtyLen_;
            computeCRC();
        }
        return(*this);
    }

    //! Overloaded assignment operator - const char*
    /*!
     *  Copies the contents pointed at by the rhs
     *  pointer into the internal storage area
     *  \param rhs Location in memory containing the string
     */
    UTF8String& UTF8String::operator=(const char* rhs)
    {
        if (rhs != nullptr)
        {
            this->Set(rhs);
        }
        return(*this);
    }

    //! Overloaded assignment operator - const wchar_t*
    UTF8String& UTF8String::operator=(const wchar_t* rhs)
    {
        if (rhs != nullptr)
        {
            this->Set(rhs);
        }
        return(*this);
    }

    UTF8String& UTF8String::operator=(const std::string& rhs)
    {
        if (!rhs.empty())
        {
            this->Set(rhs);
        }
        return(*this);
    }

    UTF8String& UTF8String::operator=(const std::wstring& rhs)
    {
        if (!rhs.empty())
        {
            this->Set(rhs);
        }
        return(*this);
    }


    //! Equality operator
    /*!
     */
    bool UTF8String::operator==(const UTF8String& rhs)
    {
        if (!valid_ || !rhs.valid_)return(false);
        //If both objects are marked as empty strings, they are equal
        if (empty_ && rhs.empty_)return(true); 
        //If one is empty and the other is not, they are not equal
        if (empty_ != rhs.empty_)return(false);
        //Ensure CRC values are valid for both sides
        if (!this->dirty_ && !rhs.dirty_)
        {
            //If the CRC values do not match, these strings are not equal
            if (this->crc_ != rhs.crc_)
            {
                return(false);
            }
            //If the CRC values do match, the strings are equal
            else
            {
                return(true);
            }
        }        
        if (this->characterLength_ != rhs.characterLength_)return(false);
        //If the object being compared to occupies the same place
        //in memory, then they are the same
        if (this == &rhs)return(true);
        //if we get here, perform the string comparison operation
        return(internalStorage_.compare(rhs.internalStorage_) == 0);
    }

    bool UTF8String::operator==(const char* rhs)
    {
        if (!rhs)return(false);
        if (!valid_)return(false);
        size_t len = strlen(rhs);
        if (len <= 0)return(false);
        if (empty_ && len == 0)return(true);
        if (length() != len)return(false);
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        bool rt = true;
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<char>(0x000000ff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        return(rt);
    }

    bool UTF8String::operator==(const wchar_t* rhs)
    {
        if (!rhs)return(false);
        if (!valid_)return(false);
        size_t len = wcslen(rhs);
        if (!valid_ || len <= 0)return(false);
        if (empty_ && len == 0)return(true);
        if (this->length() != len)return(false);
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        bool rt = true;
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<wchar_t>(0x0000ffff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        return(rt);
    }

    bool UTF8String::operator==(const std::string& rhs)
    {
        if (!valid_)return(false);
        if (empty_ && rhs.empty())return(true);
        size_t len = rhs.length();
        if (this->length() != len)return(false);
        bool rt = true;
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<char>(0x000000ff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        

        return(rt);
    }

    bool UTF8String::operator==(const std::wstring& rhs)
    {
        if (!valid_)return(false);
        if (empty_ != rhs.empty())return(false);
        if (empty_ && rhs.empty())return(true);
        size_t len = rhs.length();
        if (this->length() != len)return(false);
        bool rt = true;
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<wchar_t>(0x0000ffff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        return(rt);
    }

    //! InEquality operator
    /*!
     */

    //TODO: Start HERE - make length const again!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    bool UTF8String::operator!=(const UTF8String& rhs)
    {
        //If the object being compared to occupies the same place
        //in memory, then they are not inequal
        if (this == &rhs)return(false);
        //If this object or the other object are marked as invalid
        //then a true comparison cannot be made, making them inequal
        if (!valid_ || !rhs.valid_)return(true);
        //If both objects are marked as empty strings, they are not inequal
        if (empty_ && rhs.empty_)return(false);
        //If the empties do not match, they are inequal
        if (empty_ != rhs.empty_)return(true);
        //If the CRC values match, these strings are not inequal
        if (!dirty_ && !rhs.dirty_)
        {
            if (crc_ == rhs.crc_)
            {
                return(false);
            }
            else
            {
                return(true);
            }
        }
        if (this->characterLength_ != rhs.characterLength_)return(true);
        //If they are at the same location in memory, they are not inequal
        if (this == &rhs)return(false);
        //if we get here, perform the string comparison operation
        return(this->internalStorage_.compare(rhs.internalStorage_) != 0);
    }

    bool UTF8String::operator!=(const char* rhs)
    {
        if (rhs == nullptr || !valid_)return(true);
        size_t len = strlen(rhs);
        if (!valid_ || len <= 0)return(true);
        if (empty_ && len == 0)return(false);
        if (this->length() != len)return(true);
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        bool fullMatch = true;
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != (char)(0x000000ff & rChar))
            {
                fullMatch = false;
                break;
            }
            ++rBeg;
        }
        return(!fullMatch);
    }

    bool UTF8String::operator!=(const wchar_t* rhs)
    {
        if (rhs == nullptr)return(true);
        size_t len = wcslen(rhs);
        if (!valid_ || len <= 0)return(true);
        if (empty_ && len == 0)return(false);
        if (this->length() != len)return(true);
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        bool fullMatch = true;
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<wchar_t>(0x0000ffff & rChar))
            {
                fullMatch = false;
                break;
            }
            ++rBeg;
        }
        return(!fullMatch);
    }

    bool UTF8String::operator!=(const std::string& rhs)
    {
        if (!valid_)return(false);
        if (empty_ && rhs.empty())return(true);
        size_t len = rhs.length();
        if (this->length() != len)return(false);
        bool rt = true;
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<char>(0x000000ff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        return(rt);
    }

    bool UTF8String::operator!=(const std::wstring& rhs)
    {
        if (!valid_)return(false);
        if (empty_ != rhs.empty())return(true);
        if (empty_ && rhs.empty())return(false);
        size_t len = rhs.length();
        if (this->length() != len)return(true);
        bool rt = true;
        std::basic_string<UTF8Char>::const_iterator rBeg = cbegin();
        std::basic_string<UTF8Char>::const_iterator rEnd = cend();
        for (size_t i = 0; i < len; ++i)
        {
            UTF8Char rChar = *rBeg;
            if (rhs[i] != static_cast<wchar_t>(0x0000ffff & rChar))
            {
                rt = false;
                break;
            }
            ++rBeg;
        }
        return(rt);
    }

    //! Overloaded addition operator to add a UTF8String
    UTF8String UTF8String::operator+(const UTF8String& rhs)
    {
        if (rhs.valid_ && !rhs.empty_)
        {
            if (this->valid_ && !this->empty_)
            {
                return(UTF8String(this->internalStorage_ + rhs.internalStorage_));
            }
            else
            {
                return(rhs);
            }
        }
        else
        {
            return(*this);
        }
    }

    //! Overloaded addition operator to add a const char* string
    UTF8String UTF8String::operator+(const char* rhs)
    {
        if (rhs)
        {        
            if (this->valid_ && !this->empty_)
            {
                return(*this + UTF8String(rhs));
            }
            else
            {
                return(UTF8String(rhs));
            }
        }
        else
        {
            return(*this);
        }
    }

    //! Overloaded addition operator to add a const wchar_t* string
    UTF8String UTF8String::operator+(const wchar_t* rhs)
    {
        if (rhs)
        {        
            if (this->valid_ && !this->empty_)
            {
                return(*this + UTF8String(rhs));
            }
            else
            {
                return(UTF8String(rhs));
            }
        }
        else
        {
            return(*this);
        }
    }

    //! Overloaded addition operator to add a const std::string
    UTF8String UTF8String::operator+(const std::string& rhs)
    {
        if (!rhs.empty())
        {
            if (this->valid_ && !this->empty_)
            {
                return(*this + UTF8String(rhs));
            }
            else
            {
                return(UTF8String(rhs));
            }
        }
        else
        {
            return(*this);
        }
    }

    //! Overloaded addition operator to add a const std::wstring
    UTF8String UTF8String::operator+(const std::wstring& rhs)
    {
        if (!rhs.empty())
        {
            if (this->valid_ && !this->empty_)
            {
                return(*this + UTF8String(rhs));                
            }
            else
            {
                return(UTF8String(rhs));
            }
        }
        else
        {
            return(*this);
        }
    }

    //! Overloaded add and assign operator
    UTF8String& UTF8String::operator+=(const UTF8String& rhs)
    {
        if (this == &rhs)
        {
            return(*this);
        }

        if (rhs.valid_ && !rhs.empty_)
        {
            if (valid_ && !empty_)
            {
                if (dirtyLen_)this->computeLengthAndSize();
                size_t len=0, sZ=0;
                if (rhs.dirtyLen_)
                {
                    StringUtilities::ComputeSizeAndLength(rhs.str(), sZ, len);
                }
                else
                {
                    len = rhs.length();
                    sZ = rhs.size();
                }
                size_t thisLen = this->length();
                //Increase internal storage size
                this->internalStorage_.resize(len + thisLen, 0);
                int srcIdx = 0;
                //Copy in data
                for (size_t i = thisLen; i < thisLen+len; ++i)
                {
                    this->internalStorage_[i] = rhs[srcIdx];
                    ++srcIdx;
                }
                this->dirty_ = true;
                this->dirtyLen_ = true;
                this->dirtySz_ = true;
                this->computeLengthAndSize();
                this->computeCRC();
            }
            else
            {
                *this = rhs;
                this->dirty_ = true;
                this->dirtyLen_ = true;
                this->dirtySz_ = true;
                this->computeLengthAndSize();
                this->computeCRC();
            }
        }
        return(*this);
    }

    //! Overloaded addition assignment operator for char*
    UTF8String& UTF8String::operator+=(const char* rhs)
    {
        if (rhs != nullptr && *rhs != '\0')
        {
            if (valid_ && !empty_)
            {
                *this += UTF8String(rhs);
            }
            else
            {
                this->Set(rhs);
            }
        }
        return(*this);
    }

    //! Overloaded addition assignment operator for wchar_t*
    UTF8String& UTF8String::operator+=(const wchar_t* rhs)
    {
        if (rhs != nullptr && *rhs != L'\0')
        {
            if (valid_ && !empty_)
            {
                *this += UTF8String(rhs);
            }
            else
            {
                this->Set(rhs);
            }
        }
        return(*this);
    }

    //! Overloaded addition assignment operator for std::string
    UTF8String& UTF8String::operator+=(const std::string& rhs)
    {
        if (!rhs.empty())
        {
            if (valid_ && !empty_)
            {
                *this += UTF8String(rhs);
            }
            else
            {
                this->Set(rhs);
            }
        }
        return(*this);
    }

    //! Overloaded addition assignment operator for std::wstring
    UTF8String& UTF8String::operator+=(const std::wstring& rhs)
    {
        if (!rhs.empty())
        {
            if (valid_ && !empty_)
            {
                *this += UTF8String(rhs);
            }
            else
            {
                this->Set(rhs);
            }
        }
        return(*this);
    }

    //! Index based access to internal string characters
    UTF8String::UTF8Char& UTF8String::operator[](unsigned int idx)
    {
        //Assume that the string is modified as this method is non-const
        dirty_ = true;
        if (!valid_ || empty_ || dirtyLen_ || idx >= characterLength_)
        {
            return(reinterpret_cast<UTF8Char&>(*(internalStorage_.begin())));
        }
        return(internalStorage_[idx]);
    }

    //! Index based access to internal string characters - const version    
    const UTF8String::UTF8Char& UTF8String::operator[](unsigned int idx) const
    {
        if (!valid_ || empty_ || dirtyLen_ || idx >= characterLength_)return(reinterpret_cast<const UTF8Char&>(*(internalStorage_.data())));
        return(internalStorage_[idx]);
    }

    //! Compute the boolean version of a successful indexOf search for a UTF8String object
    bool UTF8String::contains(const UTF8String& rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ ||  
            !rhs.valid_ || rhs.empty_ || rhs.dirtyLen_)
        {
            return(false);
        }
        size_t idx = this->indexOf(rhs);
        if (idx == std::wstring::npos)
        {
            return(false);
        }
        return(true);
    }

    //! Compute the boolean version of a successful indexOf search for a char* string
    bool UTF8String::contains(const char* rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ || rhs == nullptr)return(false);
        return(this->indexOf(rhs) != std::wstring::npos);
    }

    //! Compute the boolean version of a successful indexOf search for a wchar_t* string
    bool UTF8String::contains(const wchar_t* rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ || rhs == nullptr)return(false);
        return(this->indexOf(rhs) != std::wstring::npos);
    }

    //! Compute the boolean version of a successful indexOf search for a std::string
    bool UTF8String::contains(const std::string& rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ || rhs.empty())return(false);
        return(this->indexOf(rhs) != std::wstring::npos);
    }

    //! Compute the boolean version of a successful indexOf search for a std::wstring
    bool UTF8String::contains(const std::wstring& rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ || rhs.empty())return(false);
        return(this->indexOf(rhs) != std::wstring::npos);
    }

    //!Returns an index to the string where the matching UTF8String input has been found
    size_t UTF8String::indexOf(const UTF8String& rhs) const
    {
        if (!valid_ || empty_ || dirtyLen_ || 
            !rhs.valid_ || rhs.empty_ || rhs.dirtyLen_)
        {
            return(std::wstring::npos);
        }
        const std::wstring& wStr = rhs.str();
        size_t len = internalStorage_.length();
        size_t rhsLen = rhs.internalStorage_.length(); 
        size_t matchIdx = 0;
        bool fndMatch = false;
        bool overrun = false;

        for (size_t i = 0; i < len; ++i)
        {
            //Pick the current character of the primary string as the starting point
            //for the substring match
            UTF8Char thisChar = internalStorage_[i];
            matchIdx = 0;
            fndMatch = false;
            bool startMatchSeq = false;
            size_t matchedChars = 0;            
            //First match against a primary string char against 
            //the passed in string to search for...
            for (size_t j = 0; j < rhsLen; ++j)
            {
                //If we have not started matching the substring
                if (!startMatchSeq)
                {
                    //If we find the initial matching character
                    if (thisChar == rhs.internalStorage_[j])
                    {
                        //Set the state of the search to substring matching
                        startMatchSeq = true;
                        matchedChars = 1;
                        //Set the matching index to the point in this string where we start
                        matchIdx = i;
                    }
                    //Otherwise, do not waste anymore time in this loop
                    //This will allow us to search against the next
                    //character in the primary string
                    else
                    {
                        break;
                    }
                }
                //Otherwise we are in substring character matching mode
                else
                {
                    //Compute the proper index into the primary string
                    size_t thisIdx = matchIdx + j;
                    //If we are still within the bounds of the primary string
                    if (thisIdx < len)
                    {
                        //If the primary string character matches against the current character
                        //we are searching for, increase the matched character count                        
                        if (internalStorage_[thisIdx] == rhs.internalStorage_[j])
                        {
                            matchedChars++;
                            //If we have found all characters in the substring matched against
                            //the primary string, mark that the match was found and kill this inner loop
                            if (matchedChars == rhsLen)
                            {
                                fndMatch = true;
                                break;
                            }
                        }
                        //If we miss just one character, the search counters must be reset and the
                        //and we need to get out of this inner loop
                        else
                        {
                            startMatchSeq = false;
                            matchedChars = 0;
                            break;
                        }
                    }
                    //Otherwise, we have exceeded the bounds of the primary string, indicating
                    //that we cannot find the passed in substring within the primary string
                    else
                    {
                        overrun = true;
                        break;
                    }

                }//End of the check for the two different search states

            } //End of inner search for loop

            //If our search reached a final state of no match, or complete match, break
            //out of the primary loop
            if (overrun || fndMatch)
            {
                break;
            }
        }

        //Return match status
        if (fndMatch)
        {
            return(matchIdx);
        }
        return(std::wstring::npos);
    }

    //!Returns an index to the string where the matching const char* input has been found
    size_t UTF8String::indexOf(const char* rhs) const
    {
        if (!valid_ || empty_ || rhs == nullptr)return(std::wstring::npos);
        UTF8String uRhs(rhs);
        if (uRhs.isValid() && !uRhs.isEmpty())
        {
            return(this->indexOf(uRhs));
        }
        return(std::wstring::npos);
    }

    //!Returns an index to the string where the matching wchar_t* input has been found
    size_t UTF8String::indexOf(const wchar_t* rhs) const
    {
        if (!valid_ || empty_ || rhs == nullptr)return(std::wstring::npos);
        UTF8String uRhs(rhs);
        if (uRhs.isValid() && !uRhs.isEmpty())
        {
            return(this->indexOf(uRhs));
        }
        return(std::wstring::npos);
    }

    //!Returns an index to the string where the matching std::string input has been found
    size_t UTF8String::indexOf(const std::string& rhs) const
    {
        if (!valid_ || empty_ || rhs.empty())return(std::wstring::npos);
        UTF8String uRhs(rhs);
        if (uRhs.isValid() && !uRhs.isEmpty())
        {
            return(this->indexOf(uRhs));
        }
        return(std::wstring::npos);
    }

    //!Returns an index to the string where the matching std::wstring input has been found
    size_t UTF8String::indexOf(const std::wstring& rhs) const
    {
        if (!valid_ || empty_ || rhs.empty())return(std::wstring::npos);
        UTF8String uRhs(rhs);
        if (uRhs.isValid() && !uRhs.isEmpty())
        {
            return(this->indexOf(uRhs));
        }
        return(std::wstring::npos);
    }

    //!Returns a pointer to the string where the matching UTF8String input has been found
    const rct::UTF8String::UTF8Char* UTF8String::strStr(const UTF8String& rhs) const
    {
        if (!valid_ || empty_ || !rhs.valid_ || rhs.empty_)return(nullptr);
        size_t idx = this->indexOf(rhs);
        if (idx != std::wstring::npos)
        {
            return(&(this->internalStorage_[idx]));
        }
        return(nullptr);
    }
            
    //!Returns a pointer to the string where the matching const char* input has been found
    const rct::UTF8String::UTF8Char* UTF8String::strStr(const char* rhs) const
    {
        if (!valid_ || empty_ || rhs == nullptr)return(nullptr);
        size_t idx = this->indexOf(rhs);
        if (idx != std::wstring::npos)
        {
            return(&(this->internalStorage_[idx]));
        }
        return(nullptr);
    }

    //!Returns a pointer to the string where the matching wchar_t* input has been found
    const rct::UTF8String::UTF8Char* UTF8String::strStr(const wchar_t* rhs) const
    {
        if (!valid_ || empty_ || rhs == nullptr)return(nullptr);
        size_t idx = this->indexOf(rhs);
        if (idx != std::wstring::npos)
        {
            return(&(this->internalStorage_[idx]));
        }
        return(nullptr);
    }

    //!Returns a pointer to the string where the matching std::string input has been found
    const rct::UTF8String::UTF8Char* UTF8String::strStr(const std::string& rhs) const
    {
        if (!valid_ || empty_ || rhs.empty())return(nullptr);
        size_t idx = this->indexOf(rhs);
        if (idx != std::wstring::npos)
        {
            return(&(this->internalStorage_[idx]));
        }
        return(nullptr);
    }

    //!Returns a pointer to the string where the matching std::wstring input has been found
    const rct::UTF8String::UTF8Char* UTF8String::strStr(const std::wstring& rhs) const
    {
        if (!valid_ || empty_ || rhs.empty())return(nullptr);
        size_t idx = this->indexOf(rhs);
        if (idx != std::wstring::npos)
        {
            return(&(this->internalStorage_[idx]));
        }
        return(nullptr);
    }

    //! Return an iterator to the beginning of the UTF8String
    UTF8String::UTF8Iterator UTF8String::begin()
    {
        //Assume the string is modified as this is not const
        this->dirty_ = true;
        return(internalStorage_.begin());
    }

    //! Return an iterator to the end of the UTF8String
    UTF8String::UTF8Iterator UTF8String::end()
    {
        //Assume the string is modified as this is not const
        this->dirty_ = true;
        return(internalStorage_.end());
    }

    //! Return a const iterator to the beginning of the UTF8String
    UTF8String::UTF8ConstIterator UTF8String::cbegin() const
    {
        return(internalStorage_.cbegin());
    }

    //! Return a const iterator to the end of the UTF8String
    UTF8String::UTF8ConstIterator UTF8String::cend() const
    {
        return(internalStorage_.cend());
    }
    
    //! Destructor
    UTF8String::~UTF8String()
    {
        clear();
    }
    
    //! Set a normal narrow byte string
    bool UTF8String::Set(const char* inStr)
    {
        if (!inStr)return(false);
        size_t len = strlen(inStr);
        if (ConvertIn<const char, const char>(inStr, len, inStr, inStr+len, false, internalStorage_))
        {
            valid_ = true;
            empty_ = internalStorage_.empty();
            dirty_ = true;
            dirtySz_ = true;
            dirtyLen_ = true;
            this->computeCRC();
            return(true);
        }
        else
        {
            clear();
        }
        return(false);
    }

    //! Set a wide character string
    bool UTF8String::Set(const wchar_t* inStr)
    {
        if (!inStr)return(false);
        size_t len = wcslen(inStr);
        if (ConvertIn<const wchar_t, const wchar_t>(inStr, len, inStr, inStr+len, false, internalStorage_))
        {
            valid_ = true;
            empty_ = internalStorage_.empty();
            dirty_ = true;
            dirtySz_ = true;
            dirtyLen_ = true;
            this->computeCRC();
            return(true);
        }
        else
        {
            clear();
        }
        return(false);
    }

    //!Set a standard string
    bool UTF8String::Set(const std::string& inStr)
    {
        if (inStr.empty())return(false);
        size_t len = inStr.size();
        //if (convertIn(inStr, false))
        if (ConvertIn<const char, const char>(inStr.data(), len, inStr.data(), inStr.data()+len, false, internalStorage_))
        {
            valid_ = true;
            empty_ = internalStorage_.empty();
            dirty_ = true;
            dirtySz_ = true;
            dirtyLen_ = true;
            this->computeCRC();
            return(true);
        }
        else
        {
            clear();
        }
        return(false);
    }

    //!Set a wide string
    bool UTF8String::Set(const std::wstring& inStr)
    {
        if (inStr.empty())return(false);
        size_t len = inStr.size();
        if (ConvertIn<const wchar_t, const wchar_t>(inStr.data(), len, inStr.data(), inStr.data()+len, false, internalStorage_))
        {
            valid_ = true;
            empty_ = internalStorage_.empty();
            dirty_ = true;
            dirtySz_ = true;
            dirtyLen_ = true;
            this->computeCRC();
            return(true);
        }
        else
        {
            clear();
        }
        return(false);
    }

    //! Overloaded stream operator which sends output stream data from a utf8 string
    std::ostream& operator<<(std::ostream& oStream, const UTF8String& utf8)
    {
        if (utf8.isValid() && !utf8.isEmpty())
        {
            oStream << utf8.internalStorage_.c_str();
        }
        else
        {
            if (!utf8.isValid())
            {
#ifdef _DEBUG
                oStream << "(Invalid UTF8String)";
#else
                oStream << "";
#endif
            }
            else
            {
#ifdef _DEBUG
                oStream << "(Empty UTF8String)";
#else
                oStream << "";
#endif
            }
        }
        return(oStream);
    }

    std::ostream& operator<<(std::ostream& oStream, UTF8String& utf8)
    {
        if (utf8.isValid() && !utf8.isEmpty())
        {
            oStream << utf8.internalStorage_.c_str();
        }
        else
        {
            if (!utf8.isValid())
            {
#ifdef _DEBUG
                oStream << "(Invalid UTF8String)";
#else
                oStream << "";
#endif
            }
            else
            {
#ifdef _DEBUG
                oStream << "(Empty UTF8String)";
#else
                oStream << "";
#endif
            }
        }
        return(oStream);
    }

    //! Method to clear the internal storage string
    void UTF8String::clear()
    {
        internalStorage_.clear();
        valid_ = true;
        empty_ = true;
        dirty_ = false;
        dirtySz_ = false;
        dirtyLen_ = false;
        crc_ = 0;
        characterLength_ = 0;
        sizeInBytes_ = 0;
    }        

    //! Returns true if the string is empty
    /*!
     *  If the string is not valid, the empty flag, regardless
     *  of value, holds no meaning
     */
    bool UTF8String::isEmpty() const
    {
        return(empty_);
    }

    //! Returns true if the string is valid
    bool UTF8String::isValid() const
    {
        return(valid_);
    }

    //! Returns true if the string is dirty in terms of crc value, length, or size    
    /*! Passing in true will compute values to clear the dirty flags
     */
    bool UTF8String::isDirty(bool reCalc)
    {
        if (reCalc)
        {
            this->dirty_ = true;
            this->computeCRC();
        }
        return(this->dirty_ || this->dirtySz_ || this->dirtyLen_);
    }

    //! Returns the crc value of the string
    /*! If the string is dirty, this method will auto
     *! compute the crc value
     */
    unsigned int UTF8String::crc()
    {
        if (dirty_)this->computeCRC();
        return(crc_);
    }

    //! Enables/disables the CRC computation that occurs upon any string modification
    /*! NOTE: Passing in true will force the crc computation 
     *!
     */
    void UTF8String::SetCRCOn(bool crcFlag)
    {
        this->crcOn_ = crcFlag;
        if (this->crcOn_)
        {
            this->dirty_ = true;
            this->computeCRC();
        }
    }

    //! Returns length of string in characters
    size_t UTF8String::length() const
    {
        if (!valid_ || empty_ || dirty_ || dirtyLen_)return(0);
        return(this->characterLength_);
    }

    //! Returns the size of the string in bytes
    size_t UTF8String::size() const
    {
        if (!valid_ || empty_ || dirty_ || dirtySz_)return(0);
        return(this->sizeInBytes_);
    }

    //! Const accessor returning the internal string storage object    
    const std::basic_string<rct::UTF8String::UTF8Char>& UTF8String::str() const
    {
        return(internalStorage_);
    }

    //! Const accessor returning the internal string buffer
    const rct::UTF8String::UTF8Char* UTF8String::c_str() const
    {
        return(internalStorage_.c_str());
    }

    //! Converter to std::string (narrowing operation)
    bool UTF8String::Narrow(std::string& output) const
    {
        if (!valid_ || empty_ || dirtyLen_)return(false);
        if (output.length() < this->length())
        {
            output.resize(this->characterLength_+1);
        }
        unsigned int idx = 0;
        char* outputPtr = &output[0];
        const wchar_t* inputPtr = &internalStorage_[0];        
        while(idx++ < this->characterLength_) { *outputPtr++ = (char)((*(inputPtr+idx) % 0xff)); }
        return(true);
    }

    //! Converter accessor for std::string
    std::string UTF8String::nstr() const
    {
        if (!valid_ || empty_ || dirtyLen_)return(std::string());
        std::string rt(this->length(), 0);
        wcstombs(&rt[0], internalStorage_.c_str(), rt.size());
        return(rt);
    }

    //! Converter accessor for std::string
    bool UTF8String::cnstr(std::string& output) const
    {
        if (!valid_ || empty_ || dirtyLen_)return(false);
        if (output.length() < this->length())
        {
            output.resize(this->characterLength_+1, 0);
        }
        size_t sZ = wcstombs(&output[0], internalStorage_.c_str(), output.size());
        return(sZ > 0);
    }

    //! Converter accessor for std::wstring
    std::wstring UTF8String::nwstr() const
    {
        if (!valid_ || empty_)return(std::wstring());
        return(std::wstring(internalStorage_.data()));
    }

} //namespace rct


//!  UTF8String Access Shim
/*!
 *  These methods allow our UTF8String class to be inserted into
 *  logging statements.
 *
 *  NOTE: This can be removed once the pantheos library is removed!!!
 */
namespace stlsoft
{
	inline stlsoft::basic_shim_string<rct::UTF8String::UTF8Char> c_str_data_w(const rct::UTF8String& utf8String)
	{
		return utf8String.c_str();
	}

	inline size_t c_str_len_w(const rct::UTF8String& utf8String)
	{
		return utf8String.length();
	}
}
