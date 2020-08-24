#pragma once
#include <cstring>
#include <DirectXMath.h>
#include <memory>
#include <cstdio>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace uem {
using Float2 = DirectX::XMFLOAT2;
using Float3 = DirectX::XMFLOAT3;
using Float4 = DirectX::XMFLOAT4;
using Int4 = DirectX::XMINT4;
using Vector4 = DirectX::XMVECTOR;
using Matrix = DirectX::XMMATRIX;

inline Vector4 MakeQuaternion(const Float3& degree)
{
    return DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(degree.x),
        DirectX::XMConvertToRadians(degree.y),
        DirectX::XMConvertToRadians(degree.z));
}

inline Matrix Transpose(const Matrix& matrix)
{
    return XMMatrixTranspose(matrix);
}

class FileStream
{
private:
    char* m_data = nullptr;
    char* m_activeData = nullptr;
public:
    FileStream() = default;

    explicit FileStream(const char* filename)
    {
        Load( filename );
    }

    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;

    ~FileStream()
    {
        delete[] m_data;
    }

    void Load(const char* filename)
    {
        FILE* fp;
        assert( fopen_s(&fp, filename, "rb") == 0 );
        fpos_t pos = 0;
        fseek( fp, 0L, SEEK_END );
        fgetpos( fp, &pos );
        fseek( fp, 0, SEEK_SET );

        m_data = new char[pos];
        fread( m_data, sizeof( char ), pos, fp );
        m_activeData = m_data;
        fclose( fp );
    }

    void Read(void* data, const int size)
    {
        std::memcpy( data, m_activeData, size );
        m_activeData += size;
    }
};

struct Transform
{
    Transform* m_parent = nullptr;
    std::vector<Transform*> m_child;

    std::size_t m_hash;
    std::string m_name;
    Float3 m_position;
    Vector4 m_rotation;
    Float3 m_scale;

    Matrix LocalToWorldMatrix() const
    {
        auto localMtx = DirectX::XMMatrixScaling( m_scale.x, m_scale.y, m_scale.z ) *
            DirectX::XMMatrixRotationQuaternion( m_rotation ) *
            DirectX::XMMatrixTranslation( m_position.x, m_position.y, m_position.z );
        if ( m_parent == nullptr )
            return localMtx;
        return localMtx * m_parent->LocalToWorldMatrix();
    }

    Transform* Find(const std::string& str)
    {
        if ( m_name == str )
            return this;
        for ( auto* t : m_child )
        {
            if ( auto* ret = t->Find( str ) )
                return ret;
        }
        return nullptr;
    }
};

enum Flg
{
    POSITION = 0x0001,
    NORMAL = 0x0002,
    TANGENT = 0x0004,
    UV1 = 0x0008,
    UV2 = 0x0010,
    UV3 = 0x0020,
    UV4 = 0x0040,
    UV5 = 0x0080,
    UV6 = 0x0100,
    UV7 = 0x0200,
    UV8 = 0x0400,
    COLOR = 0x0800,
};

static const std::vector<std::pair<int, int>> VertexFormatSizes
{
    { POSITION, 4 * 3 },
    { NORMAL, 4 * 3 },
    { TANGENT, 4 * 3 },
    { UV1, 4 * 2 },
    { UV2, 4 * 2 },
    { UV3, 4 * 2 },
    { UV4, 4 * 2 },
    { UV5, 4 * 2 },
    { UV6, 4 * 2 },
    { UV7, 4 * 2 },
    { UV8, 4 * 2 },
    { COLOR, 4 * 4 },
};

struct Material
{
private:
    std::unordered_map<std::size_t, std::string> m_textureNames;
    std::unordered_map<std::size_t, Float4> m_colors;

public:
    std::string name;

    static std::size_t GetHash(const std::string& property)
    {
        return std::hash<std::string>()( property );
    }

    void AddColor(const std::string& property, Float4 color)
    {
        m_colors.insert( std::make_pair( GetHash( property ), color ) );
    }

    void AddTexture(const std::string& property, const std::string& textureName)
    {
        m_textureNames.insert( std::make_pair( GetHash( property ), textureName ) );
    }

    Float4 GetColor(const std::size_t propertyHash)
    {
        return m_colors[propertyHash];
    }

    Float4 GetColor(const std::string& property)
    {
        return GetColor( GetHash( property ) );
    }

    std::string GetTexture(const std::size_t propertyHash)
    {
        return m_textureNames[propertyHash];
    }

    std::string GetTexture(const std::string& property)
    {
        return GetTexture( GetHash( property ) );
    }

    bool operator ==(const std::string& a) const
    {
        return name == a;
    }
};

template <class X>
struct Model
{
    struct Mesh
    {
        std::vector<X> vertexDatas;
        std::vector<uint32_t> indexes;
        int materialNo{};
    };

    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;

    void LoadAscii(std::string filename)
    {
        std::ifstream ifs( filename );
        auto lastSlash = filename.find_last_of( '/' );
        filename.erase( lastSlash );
        assert( ifs.is_open() );

        //頂点フォーマットを読み込み
        int vertexFormat;
        ifs >> vertexFormat;

        int modelCount;
        ifs >> modelCount;

        //フォーマットエラーチェック
        std::vector<bool> formatFlg;
        formatFlg.resize( 12 );
        auto totalByte = 0;
        for ( auto i = 0; i < VertexFormatSizes.size(); i++ )
            if ( vertexFormat & VertexFormatSizes[i].first )
                totalByte += VertexFormatSizes[i].second;
        if ( totalByte != sizeof( X ) )
        {
            auto errorLog = std::string( typeid( X ).name() ) + " is " + std::to_string( sizeof( X ) ) + "\n " +
                "The required size is " + std::to_string( totalByte ) + " bytes";
        }

        for ( auto i = 0; i < modelCount; i++ )
        {
            Mesh model;
            //頂点情報読み込み
            int vertexCount;
            ifs >> vertexCount;
            for ( auto j = 0; j < vertexCount; j++ )
            {
                uint8_t* rawData = new uint8_t[sizeof( X )];
                auto rawCnt = 0;

                for ( auto i2 = 0; i2 < VertexFormatSizes.size(); i2++ )
                {
                    if ( vertexFormat & VertexFormatSizes[i2].first )
                    {
                        float tmpData[4];
                        auto dataSize = VertexFormatSizes[i2].second / 4;
                        for ( auto i1 = 0; i1 < dataSize; i1++ )
                            ifs >> tmpData[i1];
                        memcpy( &rawData[rawCnt], tmpData, VertexFormatSizes[i2].second );
                        rawCnt += VertexFormatSizes[i2].second;
                    }
                }

                X data;
                memcpy( &data, rawData, sizeof( X ) );
                delete[] rawData;
                model.vertexDatas.push_back( data );
            }

            //インデックス読み込み
            int indexCount;
            ifs >> indexCount;
            model.indexes.resize( indexCount );
            for ( auto j = 0; j < indexCount; j++ )
            {
                ifs >> model.indexes[j];
            }

            //マテリアルの読み込み
            Material material;
            ifs >> material.name;
            int colorCount;
            ifs >> colorCount;
            for ( auto i1 = 0; i1 < colorCount; i1++ )
            {
                std::string propertyName;
                ifs >> propertyName;
                Float4 color;
                ifs >> color.x >> color.y >> color.z >> color.w;
                material.AddColor( propertyName, color );
            }

            int textureCount;
            ifs >> textureCount;
            for ( auto i1 = 0; i1 < textureCount; i1++ )
            {
                std::string propertyName;
                ifs >> propertyName;
                std::string textureName;
                ifs >> textureName;
                material.AddTexture( propertyName, filename + "/" + textureName );
            }

            auto materialNo = -1;
            for ( auto j = 0; j < static_cast<int>( m_materials.size() ); j++ )
            {
                if ( m_materials[j] == material.name )
                    materialNo = j;
            }
            if ( materialNo == -1 )
            {
                materialNo = static_cast<int>( m_materials.size() );
                m_materials.push_back( material );
            }
            model.materialNo = materialNo;
            m_meshes.push_back( model );
        }
    }

    void LoadBinary(std::string filename)
    {
        FileStream fileStream( filename.c_str() );
        auto lastSlash = filename.find_last_of( '/' );
        filename.erase( lastSlash );

        short vertexFormat;
        fileStream.Read( &vertexFormat, sizeof( short ) );

        uint16_t modelCount;
        fileStream.Read( &modelCount, sizeof( uint16_t ) );

        //フォーマットエラーチェック
        std::vector<bool> formatFlg;
        formatFlg.resize( 12 );
        auto totalByte = 0; //BoneIndex & BoneWeight
        for ( auto i = 0; i < VertexFormatSizes.size(); i++ )
            if ( vertexFormat & VertexFormatSizes[i].first )
                totalByte += VertexFormatSizes[i].second;
        if ( totalByte != sizeof( X ) )
        {
            auto errorLog = std::string( typeid( X ).name() ) + " is " + std::to_string( sizeof( X ) ) + "\n " +
                "The required size is " + std::to_string( totalByte ) + " bytes";
        }

        for ( auto i = 0; i < modelCount; i++ )
        {
            Mesh model;
            //頂点情報読み込み
            uint32_t vertexCount;
            fileStream.Read( &vertexCount, sizeof( uint32_t ) );
            model.vertexDatas.resize( vertexCount );
            fileStream.Read( &model.vertexDatas[0], sizeof( X ) * vertexCount );

            //インデックス読み込み
            uint32_t indexCount;
            fileStream.Read( &indexCount, sizeof( uint32_t ) );
            model.indexes.resize( indexCount );
            fileStream.Read( &model.indexes[0], sizeof( uint32_t ) * indexCount );

            //マテリアルの読み込み
            Material material;
            uint16_t materialNameCount;
            fileStream.Read( &materialNameCount, sizeof( uint16_t ) );
            material.name.resize( materialNameCount );
            fileStream.Read( &material.name[0], sizeof( char ) * materialNameCount );

            uint16_t colorCount;
            fileStream.Read( &colorCount, sizeof( uint16_t ) );
            for ( auto i1 = 0; i1 < colorCount; i1++ )
            {
                std::string propertyName;
                uint16_t propertyNameCount;
                fileStream.Read( &propertyNameCount, sizeof( uint16_t ) );
                propertyName.resize( propertyNameCount );
                fileStream.Read( &propertyName[0], sizeof( char ) * propertyNameCount );

                Float4 color;
                fileStream.Read( &color, sizeof( float ) * 4 );
                material.AddColor( propertyName, color );
            }

            uint16_t textureCount;
            fileStream.Read( &textureCount, sizeof( uint16_t ) );
            for ( auto i1 = 0; i1 < textureCount; i1++ )
            {
                std::string propertyName;
                uint16_t propertyNameCount;
                fileStream.Read( &propertyNameCount, sizeof( uint16_t ) );
                propertyName.resize( propertyNameCount );
                fileStream.Read( &propertyName[0], sizeof( char ) * propertyNameCount );

                std::string textureName;
                uint16_t textureNameCount;
                fileStream.Read( &textureNameCount, sizeof( uint16_t ) );
                if ( textureNameCount == 0 )
                {
                    material.AddTexture( propertyName, "null" );
                    continue;
                }
                textureName.resize( textureNameCount );
                fileStream.Read( &textureName[0], sizeof( char ) * textureNameCount );
                material.AddTexture( propertyName, filename + "/" + textureName );
            }

            auto materialNo = -1;
            for ( auto j = 0; j < static_cast<int>( m_materials.size() ); j++ )
            {
                if ( m_materials[j] == material.name )
                    materialNo = j;
            }
            if ( materialNo == -1 )
            {
                materialNo = static_cast<int>( m_materials.size() );
                m_materials.push_back( material );
            }
            model.materialNo = materialNo;
            m_meshes.push_back( model );
        }
    }
};

template <class X>
struct SkinnedModel
{
    struct Mesh
    {
        std::vector<X> vertexDatas;
        std::vector<uint32_t> indexes;
        std::vector<std::pair<Matrix, Transform*>> bones;
        int materialNo;
    };

    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::unique_ptr<Transform> m_root;
    std::unordered_map<std::size_t, std::unique_ptr<Transform>> m_transformMap;

private:
    void LoadHierarchyAscii(std::ifstream& ifs)
    {
        //モデルの階層構造を読み込み
        auto active = m_root.get();
        auto transformCount = 0;
        {
            std::string tmp;
            ifs >> tmp;
            transformCount++;
            active->m_name = tmp;
            active->m_hash = std::hash<std::string>()( tmp );
            ifs >> active->m_position.x >> active->m_position.y >> active->m_position.z;
            Float3 euler;
            ifs >> euler.x >> euler.y >> euler.z;
            active->m_rotation = MakeQuaternion(euler);
            ifs >> active->m_scale.x >> active->m_scale.y >> active->m_scale.z;
        }
        while ( transformCount != 0 )
        {
            std::string tmp;
            ifs >> tmp;
            if ( tmp == "ChildEndTransform" )
            {
                transformCount--;
                active = active->m_parent;
                continue;
            }
            transformCount++;
            auto newTrans = std::make_unique<Transform>();
            newTrans->m_name = tmp;
            newTrans->m_hash = std::hash<std::string>()( tmp );
            ifs >> newTrans->m_position.x >> newTrans->m_position.y >> newTrans->m_position.z;

            Float3 euler;
            ifs >> euler.x >> euler.y >> euler.z;
            newTrans->m_rotation = MakeQuaternion(euler);
            ifs >> newTrans->m_scale.x >> newTrans->m_scale.y >> newTrans->m_scale.z;
            newTrans->m_parent = active;
            active->m_child.push_back( newTrans.get() );
            active = newTrans.get();
            m_transformMap.insert( std::make_pair( newTrans->m_hash, std::move( newTrans ) ) );
        }
    }

    void LoadHierarchyBinary(FileStream& fileStream)
    {
        auto active = m_root.get();
        auto transformCount = 0;
        {
            std::string tmp;
            short tmpCount;
            fileStream.Read( &tmpCount, sizeof( short ) );
            tmp.resize( tmpCount );
            fileStream.Read( &tmp[0], sizeof( char ) * tmpCount );
            transformCount++;
            active->m_name = tmp;
            active->m_hash = std::hash<std::string>()( tmp );
            fileStream.Read( &active->m_position, sizeof( float ) * 3 );
            Float3 euler;
            fileStream.Read( &euler, sizeof( float ) * 3 );
            active->m_rotation = MakeQuaternion(euler);
            fileStream.Read( &active->m_scale, sizeof( float ) * 3 );
        }
        while ( transformCount != 0 )
        {
            std::string tmp;
            short tmpCount;
            fileStream.Read( &tmpCount, sizeof( short ) );
            if ( tmpCount == -1 )
            {
                transformCount--;
                active = active->m_parent;
                continue;
            }
            transformCount++;
            tmp.resize( tmpCount );
            fileStream.Read( &tmp[0], sizeof( char ) * tmpCount );

            auto newTrans = std::make_unique<Transform>();
            newTrans->m_name = tmp;
            newTrans->m_hash = std::hash<std::string>()( tmp );

            fileStream.Read( &newTrans->m_position, sizeof( float ) * 3 );
            Float3 euler;
            fileStream.Read( &euler, sizeof( float ) * 3 );
            newTrans->m_rotation = MakeQuaternion(euler);
            fileStream.Read( &newTrans->m_scale, sizeof( float ) * 3 );
            newTrans->m_parent = active;
            active->m_child.push_back( newTrans.get() );
            active = newTrans.get();
            m_transformMap.insert( std::make_pair( newTrans->m_hash, std::move( newTrans ) ) );
        }
    }

public:
    void LoadAscii(std::string filename)
    {
        m_root.reset( new Transform );

        std::ifstream ifs( filename );
        auto lastSlash = filename.find_last_of( '/' );
        filename.erase( lastSlash );
        assert( ifs.is_open() );

        LoadHierarchyAscii( ifs );

        //頂点フォーマットを読み込み
        int vertexFormat;
        ifs >> vertexFormat;

        int modelCount;
        ifs >> modelCount;

        //フォーマットエラーチェック
        std::vector<bool> formatFlg;
        formatFlg.resize( 12 );
        auto totalByte = 32; //BoneIndex & BoneWeight
        for ( auto i = 0; i < VertexFormatSizes.size(); i++ )
            if ( vertexFormat & VertexFormatSizes[i].first )
                totalByte += VertexFormatSizes[i].second;
        if ( totalByte != sizeof( X ) )
        {
            auto errorLog = std::string( typeid( X ).name() ) + " is " + std::to_string( sizeof( X ) ) + "\n " +
                "The required size is " + std::to_string( totalByte ) + " bytes";
        }

        for ( auto i = 0; i < modelCount; i++ )
        {
            Mesh model;
            //頂点情報読み込み
            int vertexCount;
            ifs >> vertexCount;
            for ( auto j = 0; j < vertexCount; j++ )
            {
                uint8_t* rawData = new uint8_t[sizeof( X )];
                auto rawCnt = 0;

                for ( auto i1 = 0; i1 < VertexFormatSizes.size(); i1++ )
                {
                    if ( vertexFormat & VertexFormatSizes[i1].first )
                    {
                        float tmpData[4];
                        auto dataSize = VertexFormatSizes[i1].second / 4;
                        for ( auto i2 = 0; i2 < dataSize; i2++ )
                            ifs >> tmpData[i2];
                        memcpy( &rawData[rawCnt], tmpData, VertexFormatSizes[i1].second );
                        rawCnt += VertexFormatSizes[i1].second;
                    }
                }
                Int4 boneIndex;
                Float4 boneWeight;
                ifs >> boneIndex.x >> boneIndex.y >> boneIndex.z >> boneIndex.w;
                ifs >> boneWeight.x >> boneWeight.y >> boneWeight.z >> boneWeight.w;
                memcpy( &rawData[rawCnt], &boneIndex, 16 );
                memcpy( &rawData[rawCnt + 16], &boneWeight, 16 );

                X data;
                memcpy( &data, rawData, sizeof( X ) );
                delete[] rawData;
                model.vertexDatas.push_back( data );
            }

            //インデックス読み込み
            int indexCount;
            ifs >> indexCount;
            model.indexes.resize( indexCount );
            for ( auto j = 0; j < indexCount; j++ )
            {
                ifs >> model.indexes[j];
            }

            //ベースポーズ読み込み
            int basePoseCount;
            ifs >> basePoseCount;
            for ( auto j = 0; j < basePoseCount; j++ )
            {
                std::string name;
                ifs >> name;

                float tmp[16];
                for (auto i = 0; i < 16; i++)
                    ifs >> tmp[i];

                auto trans = m_root->Find( name );
                model.bones.push_back( std::make_pair(  
                    Matrix {
                        tmp[0], tmp[4], tmp[8], tmp[12],
                        tmp[1], tmp[5], tmp[9], tmp[13],
                        tmp[2], tmp[6], tmp[10], tmp[14],
                        tmp[3], tmp[7], tmp[11], tmp[15]
                    } , trans ) );
            }

            //マテリアルの読み込み
            Material material;
            ifs >> material.name;
            int colorCount;
            ifs >> colorCount;
            for ( auto i1 = 0; i1 < colorCount; i1++ )
            {
                std::string propertyName;
                ifs >> propertyName;
                Float4 color;
                ifs >> color.x >> color.y >> color.z >> color.w;
                material.AddColor( propertyName, color );
            }

            int textureCount;
            ifs >> textureCount;
            for ( auto i1 = 0; i1 < textureCount; i1++ )
            {
                std::string propertyName;
                ifs >> propertyName;
                std::string textureName;
                ifs >> textureName;
                material.AddTexture( propertyName, filename + "/" + textureName );
            }

            auto materialNo = -1;
            for ( auto j = 0; j < static_cast<int>( m_materials.size() ); j++ )
            {
                if ( m_materials[j] == material.name )
                    materialNo = j;
            }
            if ( materialNo == -1 )
            {
                materialNo = static_cast<int>( m_materials.size() );
                m_materials.push_back( material );
            }
            model.materialNo = materialNo;
            m_meshes.push_back( model );
        }
    }

    void LoadBinary(std::string filename)
    {
        m_root.reset( new Transform() );

        FileStream fileStream( filename.c_str() );
        auto lastSlash = filename.find_last_of( '/' );
        filename.erase( lastSlash );

        LoadHierarchyBinary( fileStream );

        short vertexFormat;
        fileStream.Read( &vertexFormat, sizeof( short ) );

        uint16_t modelCount;
        fileStream.Read( &modelCount, sizeof( uint16_t ) );

        //フォーマットエラーチェック
        std::vector<bool> formatFlg;
        formatFlg.resize( 12 );
        auto totalByte = 32; //BoneIndex & BoneWeight
        for ( auto i = 0; i < VertexFormatSizes.size(); i++ )
            if ( vertexFormat & VertexFormatSizes[i].first )
                totalByte += VertexFormatSizes[i].second;
        if ( totalByte != sizeof( X ) )
        {
            auto errorLog = std::string( typeid( X ).name() ) + " is " + std::to_string( sizeof( X ) ) + "\n " +
                "The required size is " + std::to_string( totalByte ) + " bytes";
        }

        for ( auto i = 0; i < modelCount; i++ )
        {
            Mesh model;
            //頂点情報読み込み
            uint32_t vertexCount;
            fileStream.Read( &vertexCount, sizeof( uint32_t ) );
            model.vertexDatas.resize( vertexCount );
            fileStream.Read( &model.vertexDatas[0], sizeof( X ) * vertexCount );

            //インデックス読み込み
            uint32_t indexCount;
            fileStream.Read( &indexCount, sizeof( uint32_t ) );
            model.indexes.resize( indexCount );
            fileStream.Read( &model.indexes[0], sizeof( uint32_t ) * indexCount );

            //ベースポーズ読み込み
            uint16_t basePoseCount;
            fileStream.Read( &basePoseCount, sizeof( uint16_t ) );
            for ( auto j = 0; j < basePoseCount; j++ )
            {
                std::string name;
                uint16_t nameCount;
                fileStream.Read( &nameCount, sizeof( uint16_t ) );
                name.resize( nameCount );
                fileStream.Read( &name[0], sizeof( char ) * nameCount );

                Matrix tmp;
                fileStream.Read( &tmp, sizeof( float ) * 16 );


                auto trans = m_root->Find( name );
                model.bones.push_back( std::make_pair( Transpose( tmp ), trans ) );
            }

            //マテリアルの読み込み
            Material material;
            uint16_t materialNameCount;
            fileStream.Read( &materialNameCount, sizeof( uint16_t ) );
            material.name.resize( materialNameCount );
            fileStream.Read( &material.name[0], sizeof( char ) * materialNameCount );

            uint16_t colorCount;
            fileStream.Read( &colorCount, sizeof( uint16_t ) );
            for ( auto i1 = 0; i1 < colorCount; i1++ )
            {
                std::string propertyName;
                uint16_t propertyNameCount;
                fileStream.Read( &propertyNameCount, sizeof( uint16_t ) );
                propertyName.resize( propertyNameCount );
                fileStream.Read( &propertyName[0], sizeof( char ) * propertyNameCount );

                Float4 color;
                fileStream.Read( &color, sizeof( float ) * 4 );
                material.AddColor( propertyName, color );
            }

            uint16_t textureCount;
            fileStream.Read( &textureCount, sizeof( uint16_t ) );
            for ( auto i1 = 0; i1 < textureCount; i1++ )
            {
                std::string propertyName;
                uint16_t propertyNameCount;
                fileStream.Read( &propertyNameCount, sizeof( uint16_t ) );
                propertyName.resize( propertyNameCount );
                fileStream.Read( &propertyName[0], sizeof( char ) * propertyNameCount );

                std::string textureName;
                uint16_t textureNameCount;
                fileStream.Read( &textureNameCount, sizeof( uint16_t ) );
                if ( textureNameCount == 0 )
                {
                    material.AddTexture( propertyName, "null" );
                    continue;
                }
                textureName.resize( textureNameCount );
                fileStream.Read( &textureName[0], sizeof( char ) * textureNameCount );
                material.AddTexture( propertyName, filename + "/" + textureName );
            }

            auto materialNo = -1;
            for ( auto j = 0; j < static_cast<int>( m_materials.size() ); j++ )
            {
                if ( m_materials[j] == material.name )
                    materialNo = j;
            }
            if ( materialNo == -1 )
            {
                materialNo = static_cast<int>( m_materials.size() );
                m_materials.push_back( material );
            }
            model.materialNo = materialNo;
            m_meshes.push_back( model );
        }
    }
};

struct SkinnedAnimation
{
    struct Curve
    {
        std::vector<float> times;
        std::vector<float> keys;

        static float Lerp(const float f1, const float f2, const float t)
        {
            return f1 + ( f2 - f1 ) * t;
        }

        float GetValue(const float time)
        {
            if ( times.size() == 1 )
                return keys[0];
            if ( time < times[0] )
                return keys[0];
            if ( time > times[times.size() - 1] )
                return keys[keys.size() - 1];

            auto index = 0UL;
            for ( ; index < times.size() - 1; index++ )
            {
                if ( times[index] > time )
                    break;
            }
            index--;

            return Lerp( keys[index], keys[index + 1], ( time - times[index] ) / ( times[index + 1] - times[index] ) );
        }
    };

    struct Animation
    {
        Transform* transform = nullptr;
        Curve curves[10];

        void SetTransform(const float time)
        {
            const Float3 position = { curves[0].GetValue(time), curves[1].GetValue(time),
                                                     curves[2].GetValue(time) };
            const Vector4 rotation = { curves[3].GetValue(time), curves[4].GetValue(time),
                                                        curves[5].GetValue(time), curves[6].GetValue(time) };
            const Float3 scale = { curves[7].GetValue(time), curves[8].GetValue(time),
                                                  curves[9].GetValue(time) };

            transform->m_position = position;
            transform->m_rotation = rotation;
            transform->m_scale = scale;
        }
    };

private:
    std::vector<Animation> animationList;
    float maxAnimationTime = 0;
public:
    void LoadAscii(std::string filename, Transform* root)
    {
        std::ifstream ifs( filename );
        const auto lastSlash = filename.find_last_of( '/' );
        filename.erase( lastSlash );
        assert( ifs.is_open() );

        int animationCount;
        ifs >> animationCount;
        animationList.resize( animationCount );
        for ( auto i = 0; i < animationCount; i++ )
        {
            auto& anim = animationList[i];
            std::string transformName;
            ifs >> transformName;
            anim.transform = root->Find( transformName );
            for ( auto& curve : anim.curves )
            {
                int keyCount;
                ifs >> keyCount;
                curve.times.resize( keyCount );
                curve.keys.resize( keyCount );
                for ( auto k = 0; k < keyCount; k++ )
                {
                    ifs >> curve.times[k];
                    if ( curve.times[k] > maxAnimationTime )
                        maxAnimationTime = curve.times[k];
                }
                for ( auto k = 0; k < keyCount; k++ )
                    ifs >> curve.keys[k];
            }
        }
    }

    void LoadBinary(const std::string& filename, Transform* root)
    {
        FileStream fileStream( filename.c_str() );

        uint32_t animationCount;
        fileStream.Read( &animationCount, sizeof( uint32_t ) );

        animationList.resize( animationCount );
        for ( uint32_t i = 0; i < animationCount; i++ )
        {
            auto& anim = animationList[i];
            std::string transformName;
            uint16_t transformNameCount;
            fileStream.Read( &transformNameCount, sizeof( uint16_t ) );
            transformName.resize( static_cast<std::size_t>( transformNameCount ) );
            fileStream.Read( &transformName[0], sizeof( char ) * transformNameCount );
            anim.transform = root->Find( transformName );
            for ( auto& curve : anim.curves )
            {
                uint32_t keyCount;
                fileStream.Read( &keyCount, sizeof( uint32_t ) );
                curve.times.resize( keyCount );
                curve.keys.resize( keyCount );
                fileStream.Read( &curve.times[0], sizeof( float ) * keyCount );
                for ( uint32_t t = 0; t < keyCount; t++ )
                {
                    if ( curve.times[t] > maxAnimationTime )
                        maxAnimationTime = curve.times[t];
                }
                fileStream.Read( &curve.keys[0], sizeof( float ) * keyCount );
            }
        }
    }

    void SetTransform(const float time)
    {
        for ( auto& animation : animationList )
            animation.SetTransform( time );
    };

    float GetMaxAnimationTime() const
    {
        return maxAnimationTime;
    }
};
}
