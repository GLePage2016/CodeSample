#include "stdafx.h"
#include "Reactor.h"
#include "ReactorThread.h"

namespace rct
{

    bool function2()
    {
        return(false);
    }

    void functionTest()
    {
        TaskThread<bool>* testThread;
        testThread = new TaskThread<bool>(&function2);
        testThread->Start();
    }
}
