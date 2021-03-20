#include "Reader/LOLSKNReader.h"

namespace LOLImporter
{
    bool FLOLSKNReader::Read(FLOLMesh& Mesh)
    {
        uint32 Magic = 0;
        uint16 MajorVersion = 0;
        uint16 MinorVersion = 0;
        uint32 IndexCount = 0;
        uint32 VertexCount = 0;
        uint32 VertexType = 0;
        uint32 VertexSize = 0;
        uint32 SubMeshCount = 0;

        //Magic
        GetReader().Serialize(&Magic, sizeof(Magic));

        if (Magic != 0x00112233 && !GetReader().IsError())
        {
            AddLogErrorMessage("File format or file extension is not valid");
            return false;
        }

        //Version
        GetReader().Serialize(&MajorVersion, sizeof(MajorVersion));
        GetReader().Serialize(&MinorVersion, sizeof(MinorVersion));

        if (GetReader().IsError() || (MajorVersion != 0 && MajorVersion != 2 && MajorVersion != 4 && MinorVersion != 1))
        {
            AddLogErrorMessage("Unsupported file version detected"); 
            return false;
        }
        
        //SubMeshes
        if (MajorVersion == 0)
        {
            FLOLSubMesh& SubMesh = Mesh.SubMeshes.Emplace_GetRef();

            GetReader().Serialize(&IndexCount, sizeof(IndexCount));
            GetReader().Serialize(&VertexCount, sizeof(VertexCount));

            SubMesh.Name = "Base";
            SubMesh.IndexCount = IndexCount;
            SubMesh.VertexCount = VertexCount;
        }
        
        if (MajorVersion != 0)
        {
            GetReader().Serialize(&SubMeshCount, sizeof(SubMeshCount));

            if (SubMeshCount == 0)
            {
                AddLogErrorMessage("File has 0 submeshes");
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
                    AddLogErrorMessage("File format is not valid");
                    return false;
                }

                if (SubMesh.VertexCount == 0 || SubMesh.IndexCount == 0)
                {
                    AddLogErrorMessage("Submesh has 0 Vertices or 0 Indices");
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

        if (MajorVersion == 4)
        {
            uint32 Flags;
            GetReader().Serialize(&Flags, sizeof(Flags));
        }

        if (MajorVersion != 0)
        {
            GetReader().Serialize(&IndexCount, sizeof(IndexCount));
            GetReader().Serialize(&VertexCount, sizeof(VertexCount));
        }

        if (MajorVersion == 4)
        {
            GetReader().Serialize(&VertexSize, sizeof(VertexSize));
            GetReader().Serialize(&VertexType, sizeof(VertexType));

            uint8 BBOX[40];
            GetReader().Serialize(BBOX, sizeof(BBOX));
        }

        if (GetReader().IsError())
        {
            AddLogErrorMessage("File format is not valid");
            return false;
        }

        if (IndexCount == 0 || VertexCount == 0)
        {
            AddLogErrorMessage("File has 0 Vertices or 0 Indices");
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
                AddLogErrorMessage("File format is not valid");
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
                uint8 Color[4];
                GetReader().Serialize(Color, sizeof(Color));
            }

            if (GetReader().IsError())
            {
                AddLogErrorMessage("File format is not valid");
                return false;
            }
        }

        return true;
    }
}