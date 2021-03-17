#include "Types/LOLLogger.h"

namespace LOLImporter
{
    FLOLLogger::~FLOLLogger()
    {
        LogMessages.Empty();
    }

    void FLOLLogger::AddLogWarningMessage(const FString& Message)
    {
        LogMessages.Emplace(ELogVerbosity::Type::Warning, Message);
    }

    void FLOLLogger::AddLogErrorMessage(const FString& Message)
    {
        LogMessages.Emplace(ELogVerbosity::Type::Error, Message);
    }

    void FLOLLogger::AddLogInfoMessage(const FString& Message)
    {
        LogMessages.Emplace(ELogVerbosity::Type::Log, Message);
    }
}