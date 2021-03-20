// Fill out your copyright notice in the Description page of Project Settings.


#include "Reader/LOLSKLReader.h"

namespace LOLImporter
{
    bool FLOLSKLReader::ReadNew(FLOLSkeleton& Skeleton)
    {
        uint16 Flags;
        GetReader().Serialize(&Flags, sizeof(Flags));

        uint16 JointCount = 0;
        GetReader().Serialize(&JointCount, sizeof(JointCount));

        uint32 InfluencesCount = 0;
        GetReader().Serialize(&InfluencesCount, sizeof(InfluencesCount));

        int32 JointOffset = 0;
        GetReader().Serialize(&JointOffset, sizeof(JointOffset));

        int32 JointIndicesOffset = 0;
        GetReader().Serialize(&JointIndicesOffset, sizeof(JointIndicesOffset));

        int32 InfluencesOffset = 0;
        GetReader().Serialize(&InfluencesOffset, sizeof(InfluencesOffset));

        if (JointOffset <= 0 || JointCount == 0)
        {
            AddLogErrorMessage("File has 0 joints");
            return false;
        }

        if (InfluencesOffset <= 0 || InfluencesCount == 0) 
        {
            AddLogErrorMessage("File has 0 influences");
            return false;
        }

        GetReader().Seek(JointOffset);

        bool IsHasRoot = false;
        Skeleton.IsMultiRoot = false;
        Skeleton.Joints.Empty(JointCount);

        for (uint16 i = 0; i < JointCount; i++)
        {
            int16 Padding;
            float Radius;
            float GlobalTransform[10];
            int32 NameOffset;

            FLOLJoint& Joint = Skeleton.Joints.Emplace_GetRef();

            GetReader().Serialize(&Padding, sizeof(Padding));
            GetReader().Serialize(&Joint.ID, sizeof(Joint.ID));
            GetReader().Serialize(&Joint.ParentID, sizeof(Joint.ParentID));
            GetReader().Serialize(&Padding, sizeof(Padding));
            GetReader().Serialize(&Joint.NameHash, sizeof(Joint.NameHash));
            GetReader().Serialize(&Radius, sizeof(Radius));
            GetReader().Serialize(&Joint.LocalTranslation.X, sizeof(Joint.LocalTranslation.X));
            GetReader().Serialize(&Joint.LocalTranslation.Y, sizeof(Joint.LocalTranslation.Y));
            GetReader().Serialize(&Joint.LocalTranslation.Z, sizeof(Joint.LocalTranslation.Z));
            GetReader().Serialize(&Joint.LocalScale.X, sizeof(Joint.LocalScale.X));
            GetReader().Serialize(&Joint.LocalScale.Y, sizeof(Joint.LocalScale.Y));
            GetReader().Serialize(&Joint.LocalScale.Z, sizeof(Joint.LocalScale.Z));
            GetReader().Serialize(&Joint.LocalRotation.X, sizeof(Joint.LocalRotation.X));
            GetReader().Serialize(&Joint.LocalRotation.Y, sizeof(Joint.LocalRotation.Y));
            GetReader().Serialize(&Joint.LocalRotation.Z, sizeof(Joint.LocalRotation.Z));
            GetReader().Serialize(&Joint.LocalRotation.W, sizeof(Joint.LocalRotation.W));
            GetReader().Serialize(GlobalTransform, sizeof(GlobalTransform));
            GetReader().Serialize(&NameOffset, sizeof(NameOffset));

            if (GetReader().IsError())
            {
                AddLogErrorMessage("File format is not valid");
                return false;
            }

            Joint.LocalRotation.Normalize();

            if (Joint.ParentID == -1 && !IsHasRoot)
            {
                IsHasRoot = true;
            }

            if (Joint.ParentID == -1 && IsHasRoot)
            {
                Skeleton.IsMultiRoot = true;
            }

            int64 ReturnPosition = GetReader().Tell();
            GetReader().Seek(ReturnPosition - 4 + NameOffset);

            while (true)
            {
                char Letter;
                GetReader().Serialize(&Letter, sizeof(Letter));

                if (GetReader().IsError())
                {
                    AddLogErrorMessage("File format is not valid");
                    return false;
                }

                if (Letter == '\0')
                {
                    break;
                };

                Joint.Name.AppendChar(TCHAR(Letter));
            }

            GetReader().Seek(ReturnPosition);
        }

        if (GetReader().IsError())
        {
            AddLogErrorMessage("File format is not valid");
            return false;
        }

        GetReader().Seek(InfluencesOffset);

        Skeleton.JointInfluences.Empty(InfluencesCount);

        for (uint32 i = 0; i < InfluencesCount; i++) 
        {
            uint16& Influence = Skeleton.JointInfluences.Emplace_GetRef();
            GetReader().Serialize(&Influence, sizeof(Influence));

            if (GetReader().IsError())
            {
                AddLogErrorMessage("File format is not valid");
                return false;
            }
        }

        return true;
    }

    bool FLOLSKLReader::ReadLegacy(FLOLSkeleton& Skeleton, uint32 Version)
    {
        uint32 SkeletonID = 0;
        GetReader().Serialize(&SkeletonID, sizeof(SkeletonID));

        uint32 JointCount = 0;
        GetReader().Serialize(&JointCount, sizeof(JointCount));

        if (JointCount == 0)
        {
            AddLogErrorMessage("File has 0 joints");
            return false;
        }

        Skeleton.Joints.Empty(JointCount);
        for (uint32 Idx = 0; Idx < JointCount; Idx++)
        {
            FLOLJoint& Joint = Skeleton.Joints.Emplace_GetRef();

            Joint.ID = Idx;

            //Name
            while (true)
            {
                char Letter;
                GetReader().Serialize(&Letter, sizeof(Letter));

                if (GetReader().IsError())
                {
                    AddLogErrorMessage("File format is not valid");
                    return false;
                }

                if (Letter == '\0') 
                { 
                    break;
                };

                Joint.Name.AppendChar(TCHAR(Letter));
            }

            int32 ParentID;
            GetReader().Serialize(&ParentID, sizeof(ParentID));
            Joint.ParentID = ParentID;

            float Scale;
            GetReader().Serialize(&Scale, sizeof(Scale));

            FMatrix Matrix = FMatrix::Identity;
            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    GetReader().Serialize(&Matrix.M[j][i], sizeof(float));
                }
            }

            FTransform Transform(Matrix);
            Joint.LocalTranslation = Transform.GetTranslation();
            Joint.LocalRotation = Transform.GetRotation();

            if (GetReader().IsError())
            {
                AddLogErrorMessage("File format is not valid");
                return false;
            }
        }

        if (Version == 2)
        {
            uint32 InfluencesCount = 0;

            if (InfluencesCount == 0)
            {
                AddLogErrorMessage("File has 0 influences");
                return false;
            }

            GetReader().Serialize(&InfluencesCount, sizeof(InfluencesCount));

            Skeleton.JointInfluences.Empty(InfluencesCount);
            for (uint32 i = 0; i < InfluencesCount; i++)
            {
                uint32 Influence;
                GetReader().Serialize(&Influence, sizeof(Influence));

                if (GetReader().IsError())
                {
                    AddLogErrorMessage("File format is not valid");
                    return false;
                }

                Skeleton.JointInfluences.Add(Influence);
            }
        }

        if (Version == 1)
        {
            Skeleton.JointInfluences.Empty(JointCount);
            for (uint32 Idx = 0; Idx < JointCount; Idx++)
            {
                Skeleton.JointInfluences.Add(Idx);
            }
        }

        return true;
    }

	bool FLOLSKLReader::Read(FLOLSkeleton& Skeleton)
	{
        uint32 Magic = 0;
        uint32 Magic2 = 0;
        uint32 Version = 0;
        bool IsLegacy = false;

        //Magic
        GetReader().Serialize(&Magic, sizeof(Magic));
        GetReader().Serialize(&Magic2, sizeof(Magic2));

        if (Magic == 0x72336432 && Magic2 == 0x736B6C74)
        {
            IsLegacy = true;
        }
        else if (Magic2 != 0x22FD4FC3)
        {
            AddLogErrorMessage("File format or file extension is not valid");
            return false;
        }

        //Version
        GetReader().Serialize(&Version, sizeof(Version));

        if (GetReader().IsError() || (!IsLegacy && Version != 0))
        {
            AddLogErrorMessage("Unsupported file version detected");
            return false;
        }

        if (IsLegacy && Version != 1 && Version != 2)
        {
            AddLogErrorMessage("Unsupported file version detected");
            return false;
        }     
        
        if (IsLegacy)
        {
            return ReadLegacy(Skeleton, Version);
        }

        return ReadNew(Skeleton);
	}
}