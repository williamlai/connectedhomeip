/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "InteractiveCommands.h"

#include <platform/logging/LogV.h>

#include <editline.h>

#include <string>
#include <vector>

constexpr char kInteractiveModePrompt[]          = ">>> ";
constexpr char kInteractiveModeHistoryFileName[] = "chip_tool_history";
constexpr char kInteractiveModeStopCommand[]     = "quit()";
constexpr char kCategoryError[]                  = "Error";
constexpr char kCategoryProgress[]               = "Info";
constexpr char kCategoryDetail[]                 = "Debug";
constexpr char kCategoryAutomation[]             = "Automation";

namespace {

void ClearLine()
{
    printf("\r\x1B[0J"); // Move cursor to the beginning of the line and clear from cursor to end of the screen
}

void ENFORCE_FORMAT(3, 0) LoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    ClearLine();
    chip::Logging::Platform::LogV(module, category, msg, args);
    ClearLine();
}

class ScopedLock
{
public:
    ScopedLock(std::mutex & mutex) : mMutex(mutex) { mMutex.lock(); }

    ~ScopedLock() { mMutex.unlock(); }

private:
    std::mutex & mMutex;
};

struct InteractiveServerResultLog
{
    std::string module;
    std::string message;
    std::string messageType;
};

struct InteractiveServerResult
{
    bool mIsAsyncReport = true;
    int mStatus         = EXIT_SUCCESS;
    std::vector<std::string> mResults;
    std::vector<InteractiveServerResultLog> mLogs;

    std::mutex mMutex;

    void Setup(bool isAsyncReport)
    {
        auto lock      = ScopedLock(mMutex);
        mIsAsyncReport = isAsyncReport;
    }

    void Reset()
    {
        auto lock = ScopedLock(mMutex);

        mIsAsyncReport = true;
        mStatus        = EXIT_SUCCESS;
        mResults.clear();
        mLogs.clear();
    }

    bool IsAsyncReport()
    {
        auto lock = ScopedLock(mMutex);
        return mIsAsyncReport;
    }

    void MaybeAddLog(const char * module, uint8_t category, const char * base64Message)
    {
        auto lock = ScopedLock(mMutex);

        const char * messageType = nullptr;
        switch (category)
        {
        case chip::Logging::kLogCategory_Error:
            messageType = kCategoryError;
            break;
        case chip::Logging::kLogCategory_Progress:
            messageType = kCategoryProgress;
            break;
        case chip::Logging::kLogCategory_Detail:
            messageType = kCategoryDetail;
            break;
        case chip::Logging::kLogCategory_Automation:
            messageType = kCategoryAutomation;
            return;
        default:
            // This should not happen.
            chipDie();
            break;
        }

        mLogs.push_back(InteractiveServerResultLog({ module, base64Message, messageType }));
    }

    void MaybeAddResult(const char * result)
    {
        auto lock = ScopedLock(mMutex);

        mResults.push_back(result);
    }

    std::string AsJsonString()
    {
        auto lock = ScopedLock(mMutex);

        std::stringstream content;
        content << "{";

        content << "  \"results\": [";
        if (mResults.size())
        {
            for (const auto & result : mResults)
            {
                content << result << ",";
            }

            // Remove last comma.
            content.seekp(-1, std::ios_base::end);
        }

        if (mStatus != EXIT_SUCCESS)
        {
            if (mResults.size())
            {
                content << ",";
            }
            content << "{ \"error\": \"FAILURE\" }";
        }
        content << "],";

        content << "\"logs\": [";
        if (mLogs.size())
        {
            for (const auto & log : mLogs)
            {
                content << "{"
                           "  \"module\": \"" +
                        log.module +
                        "\","
                        "  \"category\": \"" +
                        log.messageType +
                        "\","
                        "  \"message\": \"" +
                        log.message +
                        "\""
                        "},";
            }

            // Remove last comma.
            content.seekp(-1, std::ios_base::end);
        }
        content << "]";

        content << "}";
        return content.str();
    }
};

InteractiveServerResult gInteractiveServerResult;

void ENFORCE_FORMAT(3, 0) InteractiveServerLoggingCallback(const char * module, uint8_t category, const char * msg, va_list args)
{
    va_list args_copy;
    va_copy(args_copy, args);

    chip::Logging::Platform::LogV(module, category, msg, args);

    char message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    vsnprintf(message, sizeof(message), msg, args_copy);
    va_end(args_copy);

    char base64Message[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE * 2] = {};
    chip::Base64Encode(chip::Uint8::from_char(message), static_cast<uint16_t>(strlen(message)), base64Message);

    gInteractiveServerResult.MaybeAddLog(module, category, base64Message);
}

} // namespace

char * InteractiveStartCommand::GetCommand(char * command)
{
    if (command != nullptr)
    {
        free(command);
        command = nullptr;
    }

    command = readline(kInteractiveModePrompt);

    // Do not save empty lines
    if (command != nullptr && *command)
    {
        add_history(command);
        write_history(GetHistoryFilePath().c_str());
    }

    return command;
}

std::string InteractiveStartCommand::GetHistoryFilePath() const
{
    std::string storageDir;
    if (GetStorageDirectory().HasValue())
    {
        storageDir = GetStorageDirectory().Value();
    }
    else
    {
        // Match what GetFilename in ExamplePersistentStorage.cpp does.
        const char * dir = getenv("TMPDIR");
        if (dir == nullptr)
        {
            dir = "/tmp";
        }
        storageDir = dir;
    }

    return storageDir + "/" + kInteractiveModeHistoryFileName;
}

CHIP_ERROR InteractiveServerCommand::RunCommand()
{
    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(InteractiveServerLoggingCallback);

    RemoteDataModelLogger::SetDelegate(this);
    ReturnErrorOnFailure(mWebSocketServer.Run(mPort, this));

    gInteractiveServerResult.Reset();
    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

bool InteractiveServerCommand::OnWebSocketMessageReceived(char * msg)
{
    bool isAsyncReport = false;

    gInteractiveServerResult.Setup(isAsyncReport);

    auto shouldStop = ParseCommand(msg, &gInteractiveServerResult.mStatus);
    mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
    gInteractiveServerResult.Reset();
    return shouldStop;
}

CHIP_ERROR InteractiveServerCommand::LogJSON(const char * json)
{
    gInteractiveServerResult.MaybeAddResult(json);
    if (gInteractiveServerResult.IsAsyncReport())
    {
        mWebSocketServer.Send(gInteractiveServerResult.AsJsonString().c_str());
        gInteractiveServerResult.Reset();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractiveStartCommand::RunCommand()
{
    read_history(GetHistoryFilePath().c_str());

    // Logs needs to be redirected in order to refresh the screen appropriately when something
    // is dumped to stdout while the user is typing a command.
    chip::Logging::SetLogRedirectCallback(LoggingCallback);

    char * command = nullptr;
    int status;
    while (true)
    {
        command = GetCommand(command);
        if (command != nullptr && !ParseCommand(command, &status))
        {
            break;
        }
    }

    if (command != nullptr)
    {
        free(command);
        command = nullptr;
    }

    SetCommandExitStatus(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

bool InteractiveCommand::ParseCommand(char * command, int * status)
{
    if (strcmp(command, kInteractiveModeStopCommand) == 0)
    {
        // If scheduling the cleanup fails, there is not much we can do.
        // But if something went wrong while the application is leaving it could be because things have
        // not been cleaned up properly, so it is still useful to log the failure.
        LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().ScheduleWork(ExecuteDeferredCleanups, 0));
        return false;
    }

    ClearLine();

    *status = mHandler->RunInteractive(command, GetStorageDirectory(), NeedsOperationalAdvertising());

    return true;
}

bool InteractiveCommand::NeedsOperationalAdvertising()
{
    return mAdvertiseOperational.ValueOr(true);
}
