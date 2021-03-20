#pragma once

#include "CoreMinimal.h"

namespace LOLImporter
{
    typedef TTuple<ELogVerbosity::Type, FString> FLOLLogMessage;
    typedef TArray<FLOLLogMessage> FLOLLogMessages;

    class FLOLLogger
    {
    private:
        FLOLLogMessages LogMessages;
    public:
        FLOLLogger() {}
        virtual ~FLOLLogger();
        const FLOLLogMessages& GetLogMessages() const { return LogMessages; }
        void AddLogWarningMessage(const FString& Message);
        void AddLogErrorMessage(const FString& Message);
        void AddLogInfoMessage(const FString& Message);
    };
}