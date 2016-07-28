#ifndef DATA_STORE_H_
#define DATA_STORE_H_

#include "UTF8String.h"
#include "BaseObject.h"
#include <map>
#include <iostream>
#include "FileReader.h"
#include "FileWriter.h"
#include "FileUtilities.h"
#include <boost/tokenizer.hpp>

namespace rct {

class DataStore : protected rct::Object<>
{
public:
    typedef unsigned long IndexType;
    typedef rct::Object<>::KeyType DataStr;
public:
    class DataStoreRecord : public rct::Object<>
    {
    public:
        class RecordResult
        {
        public:
            typedef struct Result
            {
                DataStr* dataStr_;
                void* dataObj_;
            };

        private:
            void operator=(const RecordResult& rhs);
            RecordResult(const RecordResult& rhs);
        private:
            DataStr stringData_;
            void* objectData_;
            bool isObject_;
        public:
            explicit RecordResult(const DataStr& data) : stringData_(data), objectData_(nullptr), isObject_(false) {}
            explicit RecordResult(void* data)              : stringData_(rct::EMPTY_STRING), objectData_(data), isObject_(true) {}
            RecordResult(RecordResult&& other) { *this = std::move(other); }
            const RecordResult& operator=(RecordResult&& other)
            {
                if (this != &other)
                {                  
                    this->stringData_ = std::move(other.stringData_);
                    this->objectData_ = other.objectData_;
                    this->isObject_   = other.isObject_;

                    //Clear other
                    other.objectData_ = nullptr;
                    other.isObject_ = false;
                }
                return(*this);
            }
            bool IsObject() const { return(this->isObject_); }

            RecordResult::Result&& GetResult(bool& isObj)
            {
                isObj = isObject_;
                RecordResult::Result r;
                if (isObject_)
                {
                    r.dataObj_ = objectData_;
                }
                else
                {
                    r.dataStr_ = &stringData_;
                }
                return(std::move(r)); 
            }
        };
    private:
        IndexType id_;
        IndexType nextColumnIdx_;
        IndexType nextObjectColumnIdx_;
        std::map<IndexType, DataStr> indexToColumnStringMap_;
        std::map<IndexType, DataStr> indexToColumnObjectMap_;
    public:
        explicit DataStoreRecord(IndexType id) : 
            rct::Object<DataStr>(rct::UTF8String(boost::str(boost::format("%d") % id)).c_str()),
            id_(id),
            nextColumnIdx_(0),
            nextObjectColumnIdx_(0)
        {}

        IndexType GetNumberPropertyColumns() const
        {
            return(nextColumnIdx_);
        }

        IndexType GetNumberObjectColumns() const
        {
            return(nextObjectColumnIdx_);
        }

        std::vector<DataStr>&& GetPropertyColumnNames() const
        {
            auto cIter = indexToColumnStringMap_.cbegin();
            auto eIter = indexToColumnStringMap_.cend();
            std::vector<DataStr> rt;
            for (; cIter != eIter; ++cIter)
            {
                rt.push_back(cIter->second);
            }
            return(std::move(rt));
        }

        std::vector<DataStr>&& GetObjectColumnNames() const
        {
            auto cIter = indexToColumnObjectMap_.cbegin();
            auto eIter = indexToColumnObjectMap_.cend();
            std::vector<DataStr> rt;
            for (; cIter != eIter; ++cIter)
            {
                rt.push_back(cIter->second);
            }
            return(std::move(rt));
        }

        bool AddColumn(const DataStr& name, const DataStr& val)
        {
            if (name.empty())return(false);
            if (this->Set(name, val))
            {
                this->indexToColumnStringMap_[this->nextColumnIdx_++] = name;
                return(true);
            }
            return(false);
        }

        bool AddColumn(const DataStr& name, rct::Object<>::UnknownObjValType val, rct::Object<>::UnknownObjSizeType size)
        {
            if (name.empty())return(false);
            if (this->SetObject(name, val, size))
            {
                this->indexToColumnObjectMap_[this->nextObjectColumnIdx_++] = name;
                return(true);
            }
            return(false);
        }

        bool GetColumn(const DataStr& name, RecordResult&& result)
        {
            if (name.empty())
            {
                result = std::move(RecordResult(nullptr));
                return(false);
            }
            auto stringMapFind = this->properties_.find(name);
            auto objectMapFind = this->objects_.find(name);
            bool notInStringIdx = (stringMapFind == this->properties_.end());
            bool notInObjectIdx = (objectMapFind == this->objects_.end());
            if (notInStringIdx && notInObjectIdx)
            {
                return(false);
            }
            else
            {
                if (!notInStringIdx)
                {
                    //Retrieve object from string property map
                    result = std::move(RecordResult(stringMapFind->second));
                }
                else
                {
                    //Retrieve object from object map
                    result = std::move(RecordResult(objectMapFind->second.GetData()));
                }
                return(true);
            }
            return(false);
        }

        bool OutputToStream(std::wostream& output)
        {
            //Output id and counters
            output << id_ << '|' << nextColumnIdx_ << '|' << nextObjectColumnIdx_ << '|' << L"\r\n";
            //Output string objects
            if (!this->properties_.empty())
            {
                output << L"PROPERTIES" << '|' << this->properties_.size() << '|' << L"\r\n";
                std::map<DataStr, DataStr>::iterator pIter = this->properties_.begin();
                std::map<DataStr, DataStr>::iterator eIter = this->properties_.end();
                for (; pIter != eIter; ++pIter)
                {
                    rct::UTF8String firstS(pIter->first);
                    rct::UTF8String secondS(pIter->second);
                    output << L'\"' << firstS.str() << L'\"' << L':' << L'\"' << secondS.str() << L'\"' << L"\r\n";
                }
                output << "\r\n";
            }
            if (!this->objects_.empty())
            {
                output << L"OBJECTS" << L'|' << this->objects_.size() << L'|' << L"\r\n";
                std::map<DataStr, rct::Object<>::UnkObjType>::iterator pIter = this->objects_.begin();
                std::map<DataStr, rct::Object<>::UnkObjType>::iterator eIter = this->objects_.end();
                CryptoPP::HexEncoder hexEncoder;
                for (; pIter != eIter; ++pIter)
                {
                    rct::Object<>::UnkObjType& obj = pIter->second;
                    std::string outputResult;
                    if (obj.GetSize() > 0)
                    {
                        CryptoPP::StringSink* strSink = new CryptoPP::StringSink(outputResult);
                        hexEncoder.Attach(strSink);
                        hexEncoder.Put((unsigned char*)obj.GetData(), obj.GetSize(), true);
                        //End encoding
                        hexEncoder.MessageEnd();

                        rct::UTF8String firstS(pIter->first);
                        output << L'\"' << firstS.str() << L'\"' << L':' << L'\"' << obj.GetSize() << L'\"' << L':' <<  L'\"' << rct::UTF8String(outputResult).str() << L'\"' << L"\r\n";
                        output.flush();
                        outputResult.clear();
                        //Cleanup string sink
                        //delete strSink;
                    }
                }
                output << L"\r\n";
            }
            output.flush();
            return(true);
        }

        bool InputFromStream(std::wistream& input)
        {
            DataStr::value_type firstLineBuffer[2048];
            DataStr::value_type tempChar;
            std::wstring endLine;
            //std::istream& iStr = input.getline(firstLineBuffer, 2048, '|');
            //Read Id, next column idx, and next object column idx
            input >> id_ >> tempChar >> nextColumnIdx_ >> tempChar >> nextObjectColumnIdx_ >> tempChar;// >> endLine;
            IndexType columnSz = nextColumnIdx_;
            IndexType colObjSz = nextObjectColumnIdx_;
            //Reset next column index and next object column index
            nextColumnIdx_ = 0;
            nextObjectColumnIdx_ = 0;

            //Create the tokenizer
            typedef boost::tokenizer<boost::char_separator<DataStr::value_type>, DataStr::const_iterator, DataStr> TokenizerType;
            boost::char_separator<DataStr::value_type> sep(L":|\'\"");

            //See if we even have any properties
            if (columnSz > 0)
            {
                DataStr workstring;
                DataStr propertiesId;
                IndexType amountProps = 0;
                input >> workstring;
                TokenizerType tokens(workstring, sep);
                TokenizerType::const_iterator tokIter = tokens.begin();
                propertiesId = rct::UTF8String(*tokIter).c_str();
                ++tokIter;
                amountProps = boost::lexical_cast<IndexType, DataStr>(rct::UTF8String(*tokIter).c_str());
                
                //Amount props should match column size
                if (propertiesId == L"PROPERTIES" && columnSz == amountProps)
                {
                    rct::UTF8String propName;
                    rct::UTF8String propVal;
                    for (unsigned int i = 0; i < amountProps; ++i)
                    {
                        try
                        {
                            input >> workstring;
                            tokens.assign(workstring, sep);
                            tokIter = tokens.begin();
                            const DataStr pName = *tokIter;
                            ++tokIter;
                            const DataStr pVal = *tokIter; 
                            this->AddColumn(pName, pVal);
                            //std::string propNameS = propName.nstr();
                            //std::string propValS = propVal.nstr();
                        }
                        catch(std::runtime_error& rErr)
                        {
                        }
                        catch(std::exception& rEx)
                        {
                        }
                        catch(...)
                        {
                        }
                    }
                }
                else
                {
                    //Error
                    return(false);
                }
            }

            //See if we have any object properties
            if (colObjSz > 0)
            {
                DataStr workstring;
                DataStr objectsId;
                IndexType amountObjProps;
                input >> workstring;//objectsId >> tempChar >> amountObjProps >> tempChar >> endLine;
                TokenizerType tokens(workstring, sep);
                TokenizerType::const_iterator tokIter = tokens.begin();
                objectsId = rct::UTF8String(*tokIter).c_str();
                ++tokIter;
                amountObjProps = boost::lexical_cast<IndexType, DataStr>(rct::UTF8String(*tokIter).c_str());

                CryptoPP::HexDecoder hexDecoder;
                //Amount obj props should match column size
                if (objectsId == L"OBJECTS" && colObjSz == amountObjProps)
                {
                    for (unsigned int i = 0; i < colObjSz; ++i)
                    {
                        rct::UTF8String propName;
                        IndexType propSize;
                        rct::UTF8String propData;
                        input >> workstring; //quoteChar >> propName >> quoteChar >> semiColonChar >> quoteChar >> propSize >> quoteChar >> semiColonChar >> quoteChar >> propData >> quoteChar >> tempChar >> endLine;
                        tokens.assign(workstring, sep);
                        tokIter = tokens.begin();
                        propName.Set(*tokIter);
                        ++tokIter;
                        propSize = boost::lexical_cast<IndexType, DataStr>(rct::UTF8String(*tokIter).c_str());
                        ++tokIter;
                        propData.Set(*tokIter);
                        //Decode the hex if properly captured
                        std::string outputResult;
                        if (!propData.isEmpty())
                        {
                            //CryptoPP::StringSource* strSrc = new CryptoPP::StringSource(propData);
                            CryptoPP::StringSink* strSink = new CryptoPP::StringSink(outputResult);
                            //hexDecoder.Detach(strSrc);
                            hexDecoder.Attach(strSink);
                            unsigned char buffer[2];
                            unsigned int shiftIdx;
                            for (unsigned int i = 0; i < propSize; ++i)
                            {
                                shiftIdx = i<<1;
                                buffer[0] = propData[shiftIdx];
                                buffer[1] = propData[shiftIdx+1];
                                hexDecoder.Put(buffer, 2);
                            }
                            //End encoding
                            hexDecoder.MessageEnd();
                            //hexDecoder.

                            rct::UTF8String propS(outputResult);
                            AddColumn(propName.c_str(), (void*)outputResult.data(), propSize);
                            //outputResult.clear();
                            //Cleanup string sink
                            //delete strSink;
                        }
                        else
                        {
                            //Error occurred
                            return(false);
                        }
                    }
                    //Read next endline
//                    input >> endLine;
                }
                else
                {
                    //Error
                    return(false);
                }
            }
            return(true);
        }
    };
protected:
    bool AddRecord(DataStore::DataStoreRecord* record)
    {
        DataStr rowId = rct::UTF8String(boost::str(boost::format("%d") % nextRowIdx_)).c_str();
        bool rt = this->SetObjectProperty(rowId, static_cast<rct::Object<>::UnknownObjValType>(record), sizeof(DataStore::DataStoreRecord*));
        if (rt)nextRowIdx_++;
        return(rt);
    }

    bool GetRecord(IndexType id, DataStore::DataStoreRecord** record)
    {
        if (record == nullptr)return(false);
        std::wstring rowId = rct::UTF8String(boost::str(boost::format("%d") % id)).c_str();
        unsigned int size;
        void* vRec = this->GetObjectProperty(rowId, size);
        if (vRec != nullptr)
        {
            *record = static_cast<DataStore::DataStoreRecord*>(vRec);
            return(true);
        }
        return(false);
    }

    bool GetRecord(const DataStr& id, DataStore::DataStoreRecord** record)
    {
        if (record == nullptr)return(false);
        unsigned int size;
        void* vRec = this->GetObjectProperty(id, size);
        if (vRec != nullptr)
        {
            *record = static_cast<DataStore::DataStoreRecord*>(vRec);
            return(true);
        }
        return(false);
    }

    bool GetRecords(std::vector<DataStore::DataStoreRecord*>& result)
    {
        if (this->objects_.empty())return(false);
        result.reserve(this->objects_.size());
        auto bIter = this->objects_.begin();
        auto eIter = this->objects_.end();
        bool addedRecords = false;
        for (; bIter != eIter; ++bIter)
        {
            auto curVal = *bIter;
            void* curDataPtr = curVal.second.GetData();
            if (curDataPtr == nullptr)continue;
            result.push_back(static_cast<DataStore::DataStoreRecord*>(curDataPtr));
            addedRecords = true;
        }
        return(addedRecords && !result.empty());
    }

    bool WriteToFile(const rct::UTF8String& fileName)
    {
        if (nextRowIdx_ == 0)return(false);
        rct::FileWriter fWriter;
        if (fWriter.OpenFile(fileName, false, true))
        {           
            fWriter.WriteLine(boost::str(boost::format("%d") % nextRowIdx_));
            std::vector<DataStore::DataStoreRecord*> records;
            bool failure = false;
            if (GetRecords(records))
            {
                std::vector<DataStore::DataStoreRecord*>::iterator cIter = records.begin();
                std::vector<DataStore::DataStoreRecord*>::iterator eIter = records.end();
                for (; cIter != eIter; ++cIter)
                {
                    std::wstringstream buffer;
                    DataStore::DataStoreRecord* curRecord = static_cast<DataStore::DataStoreRecord*>(*cIter);
                    if (curRecord == nullptr)continue;
                    curRecord->OutputToStream(buffer);
                    if (!fWriter.Write(buffer.str()))
                    {
                        failure = true;
                        break;
                    }
                }
            }
            if (!failure)
            {
                failure = !fWriter.CloseFile();
            }
            return(!failure);
        }
        return(false);
    }

    bool ReadFromFile(const rct::UTF8String& fileName)
    {
        //Reset data
        this->nextRowIdx_ = 0;
        this->objects_.clear();
        rct::FileReader fReader;
        if (fReader.OpenFile(fileName))
        {
            rct::UTF8String firstLine;
            if (!fReader.ReadLine(firstLine))
            {
                //Could not read first line
                return(false);
            }
            this->nextRowIdx_ = boost::lexical_cast<IndexType, DataStr>(firstLine.str());
            if (this->nextRowIdx_ > 0)
            {
                //Set next row index into counter and reset next row index
                IndexType recordCount = this->nextRowIdx_;
                this->nextRowIdx_ = 0;
                rct::UTF8String buffer;
                if (fReader.Read(buffer))
                {
                    //std::wstring bufferStr = buffer
                    std::wstringbuf sBuff(buffer.c_str());
                    std::wistream iStr(&sBuff, true);
                    for (unsigned int i = 0; i < recordCount; ++i)
                    {
                        //Create new data record with id of 0, id's are read in by the data store record clss
                        DataStore::DataStoreRecord* dStoreRec = new DataStore::DataStoreRecord(0);
                        if (dStoreRec != nullptr)
                        {
                            if (dStoreRec->InputFromStream(iStr))
                            {
                                if (!this->AddRecord(dStoreRec))
                                {
                                    //Error occurred - could not add record to the data store
                                    return(false);
                                }
                            }
                            else
                            {
                                //Error occurred - could not read from the input stream
                                return(false);
                            }
                        }
                        else
                        {
                            //Error occurred - could not create new data record
                            return(false);
                        }
                    }
                    //Clear the input stream - we are finished
                    iStr.clear();
                }
                else
                {
                    //Error occurred while reading
                    return(false);
                }
            }
            else
            {
                //Error occurred - next row index read is zero
                return(false);
            }
        }
        else
        {
            //Error occured - could not open file
            return(false);
        }
        //If we get here, we are successful
        return(true);
    }
public:
    DataStore(const DataStr& name) : 
        rct::Object<>(name), 
        nextRowIdx_(0)
    {
    }

    bool AddDataRecord(DataStore::DataStoreRecord* record)
    {
        return(this->AddRecord(record));
    }

    bool GetDataRecord(IndexType id, DataStore::DataStoreRecord** record)
    {
        return(this->GetRecord(id, record));
    }

    bool GetDataRecord(const DataStr& name, DataStore::DataStoreRecord** record)
    {
        return(this->GetRecord(name, record));
    }

    bool Save(const rct::UTF8String& fileName)
    {
        return(this->WriteToFile(fileName));
    }

    bool Load(const rct::UTF8String& fileName)
    {
        return(this->ReadFromFile(fileName));
    }

    IndexType GetNumberRecords() const
    {
        return(nextRowIdx_);
    }

    std::vector<DataStr>&& GetRecordIds() const
    {
        auto cIter = this->objects_.cbegin();
        auto eIter = this->objects_.cend();
        std::vector<DataStr> rt;
        for (; cIter != eIter; ++cIter)
        {
            rt.push_back(cIter->first);
        }
        return(std::move(rt));
    }

private:
    IndexType nextRowIdx_;
};

} //namespace rct

#endif //DATA_STORE_H_