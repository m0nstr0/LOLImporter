#include "Factory/LOLAnimationReaderFactory.h"

#include "Misc/FileHelper.h"
#include "Reader/LOLANMReader.h"

namespace LOLImporter
{
	namespace LOLAnimationReaderFactory
	{
		template<class R, class T>
		TTuple<bool, FLOLLogMessages> Reader(T& Asset, TArray<uint8>& FileData)
		{
			R Reader(FileData);
			return MakeTuple(Reader.Read(Asset), Reader.GetLogMessages());
		}

		bool Read(FFeedbackContext* Logger, FLOLAnimationAsset& Asset, const FString& FilePath)
		{
			const FString FileExtension = FPaths::GetExtension(FilePath);

			TArray<uint8> FileData;
			if (!FFileHelper::LoadFileToArray(FileData, *FPaths::ConvertRelativePathToFull(FilePath)))
			{
				Logger->Logf(ELogVerbosity::Type::Error, TEXT("Unable to open file. File: %s"), *FilePath);
				return false;
			}

			TTuple<bool, FLOLLogMessages> Result;
			if (FileExtension.Equals("anm", ESearchCase::IgnoreCase))
			{
				Result = Reader<FLOLANMReader>(Asset.Animation, FileData);
			}
			else
			{
				Logger->Logf(ELogVerbosity::Type::Error, TEXT("Unsupported file format detected. File: %s"), *FilePath);
				return false;
			}

			for (FLOLLogMessage Message : Result.Get<1>())
			{
				Logger->Log(Message.Get<0>(), Message.Get<1>());
			}

			return Result.Get<0>();
		}
	}
}