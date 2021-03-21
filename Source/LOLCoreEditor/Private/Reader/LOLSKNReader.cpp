#include "Reader/LOLSKNReader.h"

namespace LOLImporter
{
    bool FLOLSKNReader::Read(FLOLMesh& Mesh)
    {
        AddLogInfoMessage("[FLOLSKNReader]: Starting file parsing");

        uint32 Magic = 0;
        uint32 Version = 0;
        uint32 IndexCount = 0;
        uint32 VertexCount = 0;
        uint32 VertexType = 0;
        uint32 SubMeshCount = 0;

        //Magic
        GetReader().Serialize(&Magic, sizeof(Magic));

		AddLogInfoMessage(FString::Printf(TEXT("[FLOLSKNReader]: File magic: %u"), Magic));


        if (Magic != 0x00112233 && !GetReader().IsError())
        {
            AddLogErrorMessage("[FLOLSKNReader] File format or file extension is not valid");
            return false;
        }

        //Version
        GetReader().Serialize(&Version, sizeof(Version));

        AddLogInfoMessage(FString::Printf(TEXT("[FLOLSKNReader]: File version: %u"), Version));

        if (GetReader().IsError() || (Version != 0x00010000 && Version != 0x00010002 && Version != 0x00010004))
        {
            AddLogErrorMessage("[FLOLSKNReader] Unsupported file version detected"); 
            return false;
        }
        
        //SubMeshes
        if (Version == 0x00010000)
        {
            FLOLSubMesh& SubMesh = Mesh.SubMeshes.Emplace_GetRef();

            GetReader().Serialize(&IndexCount, sizeof(IndexCount));
            GetReader().Serialize(&VertexCount, sizeof(VertexCount));

            SubMesh.Name = "Base";
            SubMesh.IndexCount = IndexCount;
            SubMesh.VertexCount = VertexCount;
        }
        
        if (Version != 0x00010000)
        {
            GetReader().Serialize(&SubMeshCount, sizeof(SubMeshCount));

            if (SubMeshCount == 0)
            {
                AddLogErrorMessage("[FLOLSKNReader] File has 0 submeshes");
                return false;
            }

            Mesh.SubMeshes.Empty(SubMeshCount);

            for (uint32 i = 0; i < SubMeshCount; i++)
            {
                FLOLSubMesh& SubMesh = Mesh.SubMeshes.Emplace_GetRef();

                uint8 Name[64];
                GetReader().Serialize(Name, sizeof(Name));
                GetReader().Serialize(&SubMesh.StartVertex, sizeof(SubMesh.StartVertex));
                GetReader().Serialize(&SubMesh.VertexCount, sizeof(SubMesh.VertexCount));
                GetReader().Serialize(&SubMesh.StartIndex, sizeof(SubMesh.StartIndex));
                GetReader().Serialize(&SubMesh.IndexCount, sizeof(SubMesh.IndexCount));

                if (GetReader().IsError())
                {
                    AddLogErrorMessage("[FLOLSKNReader] File format is not valid");
                    return false;
                }

                if (SubMesh.VertexCount == 0 || SubMesh.IndexCount == 0)
                {
                    AddLogErrorMessage("[FLOLSKNReader] Submesh has 0 Vertices or 0 Indices");
                    return false;
                }

                for (uint8 j = 0; j < 64; j++) 
                {
                    if (Name[j] == '\0')
                    {
                        break;
                    }

                    SubMesh.Name.AppendChar(TCHAR(Name[j]));
                }
            }
        }

        if (Version == 0x00010004)
        {
			GetReader().Seek(GetReader().Tell() + 4);
        }

        if (Version != 0x00010000)
        {
            GetReader().Serialize(&IndexCount, sizeof(IndexCount));
            GetReader().Serialize(&VertexCount, sizeof(VertexCount));
        }

        if (Version == 0x00010004)
        {
            GetReader().Seek(GetReader().Tell() + 4);
            GetReader().Serialize(&VertexType, sizeof(VertexType));
			GetReader().Seek(GetReader().Tell() + 40);
        }

        if (GetReader().IsError())
        {
            AddLogErrorMessage("[FLOLSKNReader] File format is not valid");
            return false;
        }

        if (IndexCount == 0 || VertexCount == 0)
        {
            AddLogErrorMessage("[FLOLSKNReader] File has 0 Vertices or 0 Indices");
            return false;
        }

        // Indices
        Mesh.Indices.Empty(IndexCount);
        for (uint32 Idx = 0; Idx < IndexCount; Idx++)
        {
            uint16& Index = Mesh.Indices.Emplace_GetRef();
            GetReader().Serialize(&Index, sizeof(Index));

            if (GetReader().IsError())
            {
                AddLogErrorMessage("[FLOLSKNReader] File format is not valid");
                return false;
            }
        }

        //Vertices
        Mesh.Vertices.Empty(VertexCount);

        for (uint32 Idx = 0; Idx < VertexCount; Idx++) 
        {
            FLOLVertex& Vertex = Mesh.Vertices.Emplace_GetRef();

            GetReader().Serialize(&Vertex.Position.X, sizeof(Vertex.Position.X));
            GetReader().Serialize(&Vertex.Position.Y, sizeof(Vertex.Position.Y));
            GetReader().Serialize(&Vertex.Position.Z, sizeof(Vertex.Position.Z));

            GetReader().Serialize(Vertex.BoneIndex, sizeof(Vertex.BoneIndex));
            GetReader().Serialize(Vertex.Weight, sizeof(Vertex.Weight));

            GetReader().Serialize(&Vertex.Normal.X, sizeof(Vertex.Normal.X));
            GetReader().Serialize(&Vertex.Normal.Y, sizeof(Vertex.Normal.Y));
            GetReader().Serialize(&Vertex.Normal.Z, sizeof(Vertex.Normal.Z));

            GetReader().Serialize(&Vertex.TexCoord.X, sizeof(Vertex.TexCoord.X));
            GetReader().Serialize(&Vertex.TexCoord.Y, sizeof(Vertex.TexCoord.Y));

            if (VertexType == 56) 
            {
				GetReader().Seek(GetReader().Tell() + 4);
            }

            if (GetReader().IsError())
            {
                AddLogErrorMessage("[FLOLSKNReader] File format is not valid");
                return false;
            }
        }

		AddLogInfoMessage("[FLOLSKNReader]: End file parsing");

        return true;
    }
}