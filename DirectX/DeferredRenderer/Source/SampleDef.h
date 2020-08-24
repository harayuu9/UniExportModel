#pragma once
#include <DirectXMath.h>

// DirectXMath以外を使い独自のTransformを定義して使う場合はこれと同等以上を定義すると使えます
namespace uem {
using Float2 = DirectX::XMFLOAT2;
using Float3 = DirectX::XMFLOAT3;
using Float4 = DirectX::XMFLOAT4;
using Int4 = DirectX::XMINT4;

struct Vector4
{
    Vector4() = default;
    Vector4(DirectX::XMVECTOR& vec)
    {
        v = vec;
    }
    Vector4(DirectX::XMVECTOR&& vec)
    {
        v = vec;
    }
    operator DirectX::XMVECTOR& ()
    {
        return v;
    }
    operator const DirectX::XMVECTOR& () const
    {
        return v;
    }
    union
    {
        struct
        {
            float x, y, z, w;
        };
        DirectX::XMVECTOR v;
    };
};

using Matrix = DirectX::XMMATRIX;

inline Vector4 MakeQuaternion(const Float3& degree)
{
    return DirectX::XMQuaternionRotationRollPitchYaw( DirectX::XMConvertToRadians( degree.x ),
                                                      DirectX::XMConvertToRadians( degree.y ),
                                                      DirectX::XMConvertToRadians( degree.z ) );
}

inline Matrix Transpose(const Matrix& matrix)
{
    return XMMatrixTranspose( matrix );
}

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
        const auto localMtx = DirectX::XMMatrixScaling( m_scale.x, m_scale.y, m_scale.z ) *
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
}
