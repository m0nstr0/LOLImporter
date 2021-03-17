#include "Misc/FileHelper.h"
#include "Reader/LOLReaderFactory.h"
#include "Reader/LOLSKLReader.h"
#include "Reader/LOLSKNReader.h"

namespace LOLImporter
{
    bool FLOLReaderFactory::ReadFile(FFeedbackContext* Logger, FLOLAsset& Asset, const FString& FilePath)
    {
		const FString FileExtension = FPaths::GetExtension(FilePath);

		TArray<uint8> FileData; 
		if (!FFileHelper::LoadFileToArray(FileData, *FPaths::ConvertRelativePathToFull(FilePath)))
		{
			return false;
		}

		TSharedPtr<FLOLReader> Reader;
		if (FileExtension.Equals("skn", ESearchCase::IgnoreCase))
		{
			Reader = MakeShareable(new FLOLSKNReader(FileData));
		}
		else if (FileExtension.Equals("skl", ESearchCase::IgnoreCase))
		{
			Reader = MakeShareable(new FLOLSKLReader(FileData));
		}
		else
		{
			Logger->Logf(ELogVerbosity::Type::Error, TEXT("Unsupported file format detected. File: %s"), *FilePath);
			return false;
		}

		if (!Reader->Read(Asset))
		{
			for(const FLOLLogMessage& Message : Reader->GetLogMessages())
			{
				Logger->Log(Message.Get<0>(), Message.Get<1>());
			}
			return false;
		}

		return true;
    }
}