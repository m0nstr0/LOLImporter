#include "Factory/LOLSkeletalMeshReaderFactory.h"

#include "Misc/FileHelper.h"
#include "Reader/LOLSKLReader.h"
#include "Reader/LOLSKNReader.h"

namespace LOLImporter
{
	namespace LOLSkeletalMeshReaderFactory
	{
		template<class R, class T>
		TTuple<bool, FLOLLogMessages> Reader(T& Asset, TArray<uint8>& FileData)
		{
			R Reader(FileData);
			return MakeTuple(Reader.Read(Asset), Reader.GetLogMessages());
		}

		bool Read(FFeedbackContext* Logger, FLOLSkeletalMeshAsset& Asset, const FString& FilePath)
		{
			const FString FileExtension = FPaths::GetExtension(FilePath);

			TArray<uint8> FileData;
			if (!FFileHelper::LoadFileToArray(FileData, *FPaths::ConvertRelativePathToFull(FilePath)))
			{
				Logger->Logf(ELogVerbosity::Type::Error, TEXT("Unable to open file. File: %s"), *FilePath);
				return false;
			}

			TTuple<bool, FLOLLogMessages> Result;
			if (FileExtension.Equals("skn", ESearchCase::IgnoreCase))
			{
				Result = Reader<FLOLSKNReader>(Asset.Mesh, FileData);
			}
			else if (FileExtension.Equals("skl", ESearchCase::IgnoreCase))
			{
				Result = Reader<FLOLSKLReader>(Asset.Skeleton, FileData);
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