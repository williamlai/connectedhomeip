#include "MtCommand.h"

CHIP_ERROR MtCommand::Run()
{
    chip::System::Clock::Timeout duration = GetWaitDuration();

    mCommandExitStatus = RunCommand();
    if (mCommandExitStatus == CHIP_NO_ERROR)
    {
        mCommandExitStatus = StartWaiting(GetWaitDuration());
    }

    return mCommandExitStatus;
}

static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    (reinterpret_cast<MtCommand *>(appState))->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}

void MtCommand::SetCommandExitStatus(CHIP_ERROR status)
{
    mCommandExitStatus = status;
    if (CHIP_NO_ERROR != status)
    {
        ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(status));
    }
    StopWaiting();
}

CHIP_ERROR MtCommand::StartWaiting(chip::System::Clock::Timeout duration)
{
    if (duration.count() > 0)
    {
        {
            std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
            mWaitingForResponse = true;
        }

        ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));

        while (mWaitingForResponse)
        {
            chip::DeviceLayer::PlatformMgr().RunEventLoop();
        }
    }

    return mCommandExitStatus;
}

void MtCommand::StopWaiting()
{
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = false;
    }
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
}
