#ifndef REACTOR_THREAD_H
#define REACTOR_THREAD_H

#include "stdafx.h"
#include "UTF8String.h"
#include "Reactor.h"
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include "ReactorError.h"
#include "ReactorException.h"
#include "ReactorCriticalException.h"
#include "Logger.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "Base.h"
#include "BaseObject.h"

namespace rct
{
	template <typename T>
	class TaskThread
	{
    private:
        T resultObj_; 
        boost::packaged_task<T> threadTask_;
        boost::unique_future<T> threadFuture_;
        boost::thread* thread_;
        bool initialized_;
        bool hasException_;
        bool successful_;
	public:
		explicit TaskThread(T(*taskFunc)()) :
            resultObj_(),
            threadTask_(taskFunc),
            threadFuture_(threadTask_.get_future()),
            thread_(nullptr),
            initialized_(false),
            hasException_(false),
            successful_(false)
        {}

        virtual ~TaskThread()
        {
            this->Destroy();
        }

		bool Start()
        {
            if (initialized_)return(true);
            thread_ = new boost::thread(std::move(threadTask_));
            initialized_ = true;
            hasException_ = false;
            successful_ = false;
            return(true);
        }

        bool Sleep(unsigned int amount)
        {
            if (!initialized_)return(false);
            thread_->sleep(static_cast<boost::system_time>(amount));
            return(true);
        }

		bool Join()
        {
            if (!initialized_)return(false);
            thread_->join();
            return(true);
        }

        void Destroy()
        {
            try
            {
                if (thread_ != nullptr)
                {
                    delete thread_;
                    thread_ = nullptr;
                }
                initialized_ = false;
                successful_ = false;
                hasException_ = false;
            }
            catch(std::runtime_error& rErr)
            {
                Logger& log = LoggerSingleton::Instance();
                ReactorCriticalException ex(rct::ReactorError::RCT_ERR_THREAD, rErr);
                ReactorError& err = ex.GetErrorData();
                err.SetClassName("TaskThread");
                //err.SetErrorDetails(__FILE__, boost::lexical_cast<UTF8String::UTF8Char>(__LINE__));
                log.LogError(ex.ToString());
            }
            catch(std::exception& rEx)
            {
                Logger& log = LoggerSingleton::Instance();
                ReactorException ex(rct::ReactorError::RCT_ERR_THREAD, rEx);
                ReactorError& err = ex.GetErrorData();
                err.SetClassName("TaskThread");
                //err.SetErrorDetails(__FILE__, boost::lexical_cast<UTF8String::UTF8Char>(__LINE__));
                log.LogError(ex.ToString());
            }
        }

        bool IsComplete()
        {
            if (!initialized_)return(false);
            if (!this->threadFuture_.is_ready())return(false);
            if (this->threadFuture_.has_exception())
            {
                this->hasException_ = true;
                return(true);
            }
            else if (this->threadFuture_.has_value())
            {
                this->successful_ = true;
                return(true);
            }
            return(false);
        }

        bool AcquireResult()
        {
            bool rt = false;
            if (this->threadFuture_.is_ready())
            {
                if (this->threadFuture_.has_exception())
                {
                    this->hasException_ = true;
                    resultObj_ = this->threadFuture_.get();
                }
                else if (this->threadFuture_.has_value())
                {
                    this->successful_ = true;
                    resultObj_ = this->threadFuture_.get();
                    rt = true;
                }
            }
            return(rt);
        }

        bool IsSuccessful()
        {
            return(successful_);
        }

        bool HasException()
        {
            return(hasException_);
        }

        T& GetResultObj()
        {
            return(resultObj_);
        }
	};


    const static std::wstring STATUS = L"STATUS";
    class BackgroundWorkerThread
    {
    public:
        typedef boost::function<void (BackgroundWorkerThread*, rct::Object<>&)> ThreadTaskFunction;
        typedef boost::function<void (rct::Object<float>&)> ThreadProgressCallback;
        typedef boost::function<void (rct::Object<bool>&)> ThreadCallback;
        typedef rct::Object<> WorkItem;
        typedef std::vector<WorkItem> WorkItems;
    private:
        boost::thread* workerThread_;
        WorkItems threadTaskObjects_;
        unsigned int currentTask_;
        ThreadTaskFunction workTaskFxn_;        
        ThreadProgressCallback progressCallback_;
        ThreadCallback completedTaskCallback_;
        bool echoProgress_;
        bool threadDone_;
        bool killThread_;
    public:
        explicit BackgroundWorkerThread(
            WorkItems&& workerTasks,
            ThreadTaskFunction workTaskFxn,
            ThreadCallback completedCallback) : 
                workerThread_(nullptr),
                threadTaskObjects_(workerTasks),
                currentTask_(0),
                workTaskFxn_(workTaskFxn),
                progressCallback_(),
                completedTaskCallback_(completedCallback),
                echoProgress_(false) { };

            
        void AddProgressCallback(ThreadProgressCallback progCallback)
        {
            if (progCallback.empty())return;
            progressCallback_ = progCallback;
            echoProgress_ = true;
        }        

        void Start()
        {
            //TODO: Complete
        }

        bool IsComplete()
        {
            //TODO: Complete
        }

        bool Suspend()
        {
            //TODO: Complete
        }

        bool Kill()
        {
            //TODO: Complete
        }

        WorkItems&& GetWorkCompleted()
        {
        }

    };
}

#endif
