using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using JetBrains.Annotations;
using UnityEditor;
using UnityEngine;

namespace Editor
{
    public class MaterialDataOption
    {
        public bool AdvancedFlg = false;
        public List<string> Textures = new List<string>();
        public List<string> Colors = new List<string>();

        public MaterialDataOption()
        {
            Textures.Add("_MainTex");
            Colors.Add("_Color");
        }
    }

    public class VertexDataOption
    {
        public bool AdvancedFlg = false;
        public bool Position = true;
        public bool Normal = true;
        public bool Tangent = false;
        public bool Uv1 = true;
        public bool Uv2 = false;
        public bool Uv3 = false;
        public bool Uv4 = false;
        public bool Uv5 = false;
        public bool Uv6 = false;
        public bool Uv7 = false;
        public bool Uv8 = false;
        public bool Color = false;
        public const bool BoneIndex = true;
        public const bool BoneWeight = true;

        [Flags]
        public enum Flg
        {
            Position = 0x0001,
            Normal = 0x0002,
            Tangent = 0x0004,
            Uv1 = 0x0008,
            Uv2 = 0x0010,
            Uv3 = 0x0020,
            Uv4 = 0x0040,
            Uv5 = 0x0080,
            Uv6 = 0x0100,
            Uv7 = 0x0200,
            Uv8 = 0x0400,
            Color = 0x0800,
        }

        public Flg GetFormatFlg()
        {
            Flg ret = 0;
            if (Position)
                ret |= Flg.Position;
            if (Normal)
                ret |= Flg.Normal;
            if (Tangent)
                ret |= Flg.Tangent;
            if (Uv1)
                ret |= Flg.Uv1;
            if (Uv2)
                ret |= Flg.Uv2;
            if (Uv3)
                ret |= Flg.Uv3;
            if (Uv4)
                ret |= Flg.Uv4;
            if (Uv5)
                ret |= Flg.Uv5;
            if (Uv6)
                ret |= Flg.Uv6;
            if (Uv7)
                ret |= Flg.Uv7;
            if (Uv8)
                ret |= Flg.Uv8;
            if (Color)
                ret |= Flg.Color;
            return ret;
        }
    }

    public partial class MeshExportWindow
    {
        private struct VertexData
        {
            public Vector3 Vertex;
            public Vector3 Normal;
            public Vector2 Uv;
            public Vector4 Color;
        }
    }

    public class NonSkinnedMesh
    {
        private readonly Mesh mesh;
        private readonly Vector3[] vertices;
        private readonly Vector3[] normals;
        private readonly Vector4[] tangents;
        private readonly Vector2[] uv1S;
        private readonly Vector2[] uv2S;
        private readonly Vector2[] uv3S;
        private readonly Vector2[] uv4S;
        private readonly Vector2[] uv5S;
        private readonly Vector2[] uv6S;
        private readonly Vector2[] uv7S;
        private readonly Vector2[] uv8S;
        private readonly Color[] colors;

        public NonSkinnedMesh(Mesh mesh, Matrix4x4 localToWorld)
        {
            this.mesh = mesh;
            vertices = mesh.vertices;
            normals = mesh.normals;
            tangents = mesh.tangents;
            uv1S = mesh.uv;
            uv2S = mesh.uv2;
            uv3S = mesh.uv3;
            uv4S = mesh.uv4;
            uv5S = mesh.uv5;
            uv6S = mesh.uv6;
            uv7S = mesh.uv7;
            uv8S = mesh.uv8;
            colors = mesh.colors;

            var tmpVertices = new List<Vector3>();
            var tmpNormals = new List<Vector3>();
            var tmpTangents = new List<Vector4>();
            var tmpUv1 = new List<Vector2>();
            var tmpUv2 = new List<Vector2>();
            var tmpUv3 = new List<Vector2>();
            var tmpUv4 = new List<Vector2>();
            var tmpUv5 = new List<Vector2>();
            var tmpUv6 = new List<Vector2>();
            var tmpUv7 = new List<Vector2>();
            var tmpUv8 = new List<Vector2>();
            var tmpColor = new List<Color>();
            for (var index = 0; index < vertices.Length; index++)
            {
                tmpVertices.Add(localToWorld.MultiplyPoint(vertices[index]));
                tmpNormals.Add(localToWorld.MultiplyVector(normals[index]));
                tmpTangents.Add(localToWorld.MultiplyVector(tangents[index]));
                
                var uv1 = uv1S.Length > index ? new Vector2(uv1S[index].x, 1.0f - uv1S[index].y) : Vector2.zero;
                var uv2 = uv2S.Length > index ? new Vector2(uv2S[index].x, 1.0f - uv2S[index].y) : Vector2.zero;
                var uv3 = uv3S.Length > index ? new Vector2(uv3S[index].x, 1.0f - uv3S[index].y) : Vector2.zero;
                var uv4 = uv4S.Length > index ? new Vector2(uv4S[index].x, 1.0f - uv4S[index].y) : Vector2.zero;
                var uv5 = uv5S.Length > index ? new Vector2(uv5S[index].x, 1.0f - uv5S[index].y) : Vector2.zero;
                var uv6 = uv6S.Length > index ? new Vector2(uv6S[index].x, 1.0f - uv6S[index].y) : Vector2.zero;
                var uv7 = uv7S.Length > index ? new Vector2(uv7S[index].x, 1.0f - uv7S[index].y) : Vector2.zero;
                var uv8 = uv8S.Length > index ? new Vector2(uv8S[index].x, 1.0f - uv8S[index].y) : Vector2.zero;
                var color = colors.Length > index ? (Vector4) colors[index] : Vector4.one;

                tmpUv1.Add(uv1);
                tmpUv2.Add(uv2);
                tmpUv3.Add(uv3);
                tmpUv4.Add(uv4);
                tmpUv5.Add(uv5);
                tmpUv6.Add(uv6);
                tmpUv7.Add(uv7);
                tmpUv8.Add(uv8);
                tmpColor.Add(color);
            }

            vertices = tmpVertices.ToArray();
            normals = tmpNormals.ToArray();
            tangents = tmpTangents.ToArray();
            uv1S = tmpUv1.ToArray();
            uv2S = tmpUv2.ToArray();
            uv3S = tmpUv3.ToArray();
            uv4S = tmpUv4.ToArray();
            uv5S = tmpUv5.ToArray();
            uv6S = tmpUv6.ToArray();
            uv7S = tmpUv7.ToArray();
            uv8S = tmpUv8.ToArray();
            colors = tmpColor.ToArray();
        }

        public void OutputAscii([NotNull] StreamWriter writer, VertexDataOption vertexDataOption)
        {
            writer.WriteLine(mesh.vertexCount);
            for (var index = 0; index < vertices.Length; index++)
            {
                if (vertexDataOption.Position)
                    writer.WriteLine($"{vertices[index].x:f8} {vertices[index].y:f8} {vertices[index].z:f8}");
                if (vertexDataOption.Normal)
                    writer.WriteLine($"{normals[index].x:f8} {normals[index].y:f8} {normals[index].z:f8}");
                if (vertexDataOption.Tangent)
                    writer.WriteLine($"{tangents[index].x:f8} {tangents[index].y:f8} {tangents[index].z:f8}");
                if (vertexDataOption.Uv1)
                    writer.WriteLine($"{uv1S[index].x:f8} {uv1S[index].y:f8}");
                if (vertexDataOption.Uv2)
                    writer.WriteLine($"{uv2S[index].x:f8} {uv2S[index].y:f8}");
                if (vertexDataOption.Uv3)
                    writer.WriteLine($"{uv3S[index].x:f8} {uv3S[index].y:f8}");
                if (vertexDataOption.Uv4)
                    writer.WriteLine($"{uv4S[index].x:f8} {uv4S[index].y:f8}");
                if (vertexDataOption.Uv5)
                    writer.WriteLine($"{uv5S[index].x:f8} {uv5S[index].y:f8}");
                if (vertexDataOption.Uv6)
                    writer.WriteLine($"{uv6S[index].x:f8} {uv6S[index].y:f8}");
                if (vertexDataOption.Uv7)
                    writer.WriteLine($"{uv7S[index].x:f8} {uv7S[index].y:f8}");
                if (vertexDataOption.Uv8)
                    writer.WriteLine($"{uv8S[index].x:f8} {uv8S[index].y:f8}");
                if (vertexDataOption.Color)
                    writer.WriteLine($"{colors[index].r:f8} {colors[index].g:f8} " +
                                     $"{colors[index].b:f8} {colors[index].a:f8}");
            }

            //インデックスデータ出力
            writer.WriteLine(mesh.triangles.Length);
            foreach (var triangle in mesh.triangles)
            {
                writer.Write($"{triangle} ");
            }
        }

        public void OutputBinary([NotNull] BinaryWriter writer, VertexDataOption vertexDataOption)
        {
            writer.Write((uint) mesh.vertexCount);
            for (var index = 0; index < vertices.Length; index++)
            {
                if (vertexDataOption.Position)
                {
                    writer.Write(vertices[index].x);
                    writer.Write(vertices[index].y);
                    writer.Write(vertices[index].z);
                }

                if (vertexDataOption.Normal)
                {
                    writer.Write(normals[index].x);
                    writer.Write(normals[index].y);
                    writer.Write(normals[index].z);
                }

                if (vertexDataOption.Tangent)
                {
                    writer.Write(tangents[index].x);
                    writer.Write(tangents[index].y);
                    writer.Write(tangents[index].z);
                }

                if (vertexDataOption.Uv1)
                {
                    writer.Write(uv1S[index].x);
                    writer.Write(uv1S[index].y);
                }

                if (vertexDataOption.Uv2)
                {
                    writer.Write(uv2S[index].x);
                    writer.Write(uv2S[index].y);
                }

                if (vertexDataOption.Uv3)
                {
                    writer.Write(uv3S[index].x);
                    writer.Write(uv3S[index].y);
                }

                if (vertexDataOption.Uv4)
                {
                    writer.Write(uv4S[index].x);
                    writer.Write(uv4S[index].y);
                }

                if (vertexDataOption.Uv5)
                {
                    writer.Write(uv5S[index].x);
                    writer.Write(uv5S[index].y);
                }

                if (vertexDataOption.Uv6)
                {
                    writer.Write(uv6S[index].x);
                    writer.Write(uv6S[index].y);
                }

                if (vertexDataOption.Uv7)
                {
                    writer.Write(uv7S[index].x);
                    writer.Write(uv7S[index].y);
                }

                if (vertexDataOption.Uv8)
                {
                    writer.Write(uv8S[index].x);
                    writer.Write(uv8S[index].y);
                }

                if (vertexDataOption.Color)
                {
                    writer.Write(colors[index].r);
                    writer.Write(colors[index].g);
                    writer.Write(colors[index].b);
                    writer.Write(colors[index].a);
                }
            }

            //インデックスデータ出力
            writer.Write((uint) mesh.triangles.Length);
            foreach (var triangle in mesh.triangles)
            {
                writer.Write((uint) triangle);
            }
        }
    }

    public class SkinnedMesh
    {
        private readonly Mesh mesh;
        private readonly Vector3[] vertices;
        private readonly Vector3[] normals;
        private readonly Vector4[] tangents;
        private readonly Vector2[] uv1S;
        private readonly Vector2[] uv2S;
        private readonly Vector2[] uv3S;
        private readonly Vector2[] uv4S;
        private readonly Vector2[] uv5S;
        private readonly Vector2[] uv6S;
        private readonly Vector2[] uv7S;
        private readonly Vector2[] uv8S;
        private readonly Color[] colors;
        private readonly BoneWeight[] boneWeights;

        public SkinnedMesh(Mesh mesh)
        {
            this.mesh = mesh;
            vertices = mesh.vertices;
            normals = mesh.normals;
            tangents = mesh.tangents;
            uv1S = mesh.uv;
            uv2S = mesh.uv2;
            uv3S = mesh.uv3;
            uv4S = mesh.uv4;
            uv5S = mesh.uv5;
            uv6S = mesh.uv6;
            uv7S = mesh.uv7;
            uv8S = mesh.uv8;
            colors = mesh.colors;

            boneWeights = mesh.boneWeights;
            if (!boneWeights.Any())
            {
                boneWeights = new BoneWeight[vertices.Length];
                for (var index = 0; index < boneWeights.Length; index++)
                {
                    boneWeights[index] = new BoneWeight
                    {
                        weight0 = 1,
                        weight1 = 0,
                        weight2 = 0,
                        weight3 = 0,
                        boneIndex0 = 0,
                        boneIndex1 = 0,
                        boneIndex2 = 0,
                        boneIndex3 = 0
                    };
                }
            }

            var tmpUv1 = new List<Vector2>();
            var tmpUv2 = new List<Vector2>();
            var tmpUv3 = new List<Vector2>();
            var tmpUv4 = new List<Vector2>();
            var tmpUv5 = new List<Vector2>();
            var tmpUv6 = new List<Vector2>();
            var tmpUv7 = new List<Vector2>();
            var tmpUv8 = new List<Vector2>();
            var tmpColor = new List<Color>();
            for (var index = 0; index < vertices.Length; index++)
            {
                var uv1 = uv1S.Length > index ? new Vector2(uv1S[index].x, 1.0f - uv1S[index].y) : Vector2.zero;
                var uv2 = uv2S.Length > index ? new Vector2(uv2S[index].x, 1.0f - uv2S[index].y) : Vector2.zero;
                var uv3 = uv3S.Length > index ? new Vector2(uv3S[index].x, 1.0f - uv3S[index].y) : Vector2.zero;
                var uv4 = uv4S.Length > index ? new Vector2(uv4S[index].x, 1.0f - uv4S[index].y) : Vector2.zero;
                var uv5 = uv5S.Length > index ? new Vector2(uv5S[index].x, 1.0f - uv5S[index].y) : Vector2.zero;
                var uv6 = uv6S.Length > index ? new Vector2(uv6S[index].x, 1.0f - uv6S[index].y) : Vector2.zero;
                var uv7 = uv7S.Length > index ? new Vector2(uv7S[index].x, 1.0f - uv7S[index].y) : Vector2.zero;
                var uv8 = uv8S.Length > index ? new Vector2(uv8S[index].x, 1.0f - uv8S[index].y) : Vector2.zero;
                var color = colors.Length > index ? (Vector4) colors[index] : Vector4.one;

                tmpUv1.Add(uv1);
                tmpUv2.Add(uv2);
                tmpUv3.Add(uv3);
                tmpUv4.Add(uv4);
                tmpUv5.Add(uv5);
                tmpUv6.Add(uv6);
                tmpUv7.Add(uv7);
                tmpUv8.Add(uv8);
                tmpColor.Add(color);
            }

            uv1S = tmpUv1.ToArray();
            uv2S = tmpUv2.ToArray();
            uv3S = tmpUv3.ToArray();
            uv4S = tmpUv4.ToArray();
            uv5S = tmpUv5.ToArray();
            uv6S = tmpUv6.ToArray();
            uv7S = tmpUv7.ToArray();
            uv8S = tmpUv8.ToArray();
            colors = tmpColor.ToArray();
        }

        public void OutputAscii([NotNull] StreamWriter writer, VertexDataOption vertexDataOption)
        {
            writer.WriteLine(mesh.vertexCount);
            for (var index = 0; index < vertices.Length; index++)
            {
                if (vertexDataOption.Position)
                    writer.WriteLine($"{vertices[index].x:f8} {vertices[index].y:f8} {vertices[index].z:f8}");
                if (vertexDataOption.Normal)
                    writer.WriteLine($"{normals[index].x:f8} {normals[index].y:f8} {normals[index].z:f8}");
                if (vertexDataOption.Tangent)
                    writer.WriteLine($"{tangents[index].x:f8} {tangents[index].y:f8} {tangents[index].z:f8}");
                if (vertexDataOption.Uv1)
                    writer.WriteLine($"{uv1S[index].x:f8} {uv1S[index].y:f8}");
                if (vertexDataOption.Uv2)
                    writer.WriteLine($"{uv2S[index].x:f8} {uv2S[index].y:f8}");
                if (vertexDataOption.Uv3)
                    writer.WriteLine($"{uv3S[index].x:f8} {uv3S[index].y:f8}");
                if (vertexDataOption.Uv4)
                    writer.WriteLine($"{uv4S[index].x:f8} {uv4S[index].y:f8}");
                if (vertexDataOption.Uv5)
                    writer.WriteLine($"{uv5S[index].x:f8} {uv5S[index].y:f8}");
                if (vertexDataOption.Uv6)
                    writer.WriteLine($"{uv6S[index].x:f8} {uv6S[index].y:f8}");
                if (vertexDataOption.Uv7)
                    writer.WriteLine($"{uv7S[index].x:f8} {uv7S[index].y:f8}");
                if (vertexDataOption.Uv8)
                    writer.WriteLine($"{uv8S[index].x:f8} {uv8S[index].y:f8}");
                if (vertexDataOption.Color)
                    writer.WriteLine($"{colors[index].r:f8} {colors[index].g:f8} " +
                                     $"{colors[index].b:f8} {colors[index].a:f8}");
                writer.WriteLine($"{(uint) boneWeights[index].boneIndex0} {(uint) boneWeights[index].boneIndex1} " +
                                 $"{(uint) boneWeights[index].boneIndex2} {(uint) boneWeights[index].boneIndex3}");
                writer.WriteLine(
                    $"{boneWeights[index].weight0:f8} {boneWeights[index].weight1:f8} " +
                    $"{boneWeights[index].weight2:f8} {boneWeights[index].weight3:f8}");
            }

            //インデックスデータ出力
            writer.WriteLine(mesh.triangles.Length);
            foreach (var triangle in mesh.triangles)
            {
                writer.Write($"{triangle} ");
            }
        }

        public void OutputBinary([NotNull] BinaryWriter writer, VertexDataOption vertexDataOption)
        {
            writer.Write((uint) mesh.vertexCount);
            for (var index = 0; index < vertices.Length; index++)
            {
                if (vertexDataOption.Position)
                {
                    writer.Write(vertices[index].x);
                    writer.Write(vertices[index].y);
                    writer.Write(vertices[index].z);
                }

                if (vertexDataOption.Normal)
                {
                    writer.Write(normals[index].x);
                    writer.Write(normals[index].y);
                    writer.Write(normals[index].z);
                }

                if (vertexDataOption.Tangent)
                {
                    writer.Write(tangents[index].x);
                    writer.Write(tangents[index].y);
                    writer.Write(tangents[index].z);
                }

                if (vertexDataOption.Uv1)
                {
                    writer.Write(uv1S[index].x);
                    writer.Write(uv1S[index].y);
                }

                if (vertexDataOption.Uv2)
                {
                    writer.Write(uv2S[index].x);
                    writer.Write(uv2S[index].y);
                }

                if (vertexDataOption.Uv3)
                {
                    writer.Write(uv3S[index].x);
                    writer.Write(uv3S[index].y);
                }

                if (vertexDataOption.Uv4)
                {
                    writer.Write(uv4S[index].x);
                    writer.Write(uv4S[index].y);
                }

                if (vertexDataOption.Uv5)
                {
                    writer.Write(uv5S[index].x);
                    writer.Write(uv5S[index].y);
                }

                if (vertexDataOption.Uv6)
                {
                    writer.Write(uv6S[index].x);
                    writer.Write(uv6S[index].y);
                }

                if (vertexDataOption.Uv7)
                {
                    writer.Write(uv7S[index].x);
                    writer.Write(uv7S[index].y);
                }

                if (vertexDataOption.Uv8)
                {
                    writer.Write(uv8S[index].x);
                    writer.Write(uv8S[index].y);
                }

                if (vertexDataOption.Color)
                {
                    writer.Write(colors[index].r);
                    writer.Write(colors[index].g);
                    writer.Write(colors[index].b);
                    writer.Write(colors[index].a);
                }

                writer.Write((uint) boneWeights[index].boneIndex0);
                writer.Write((uint) boneWeights[index].boneIndex1);
                writer.Write((uint) boneWeights[index].boneIndex2);
                writer.Write((uint) boneWeights[index].boneIndex3);
                writer.Write(boneWeights[index].weight0);
                writer.Write(boneWeights[index].weight1);
                writer.Write(boneWeights[index].weight2);
                writer.Write(boneWeights[index].weight3);
            }

            //インデックスデータ出力
            writer.Write((uint) mesh.triangles.Length);
            foreach (var triangle in mesh.triangles)
            {
                writer.Write((uint) triangle);
            }
        }
    }

    public static class ExtensionClass
    {
        public static void OutputAscii(this Material material, [NotNull] TextWriter writer, MaterialDataOption option,
            string filepath)
        {
            writer.WriteLine(material.name);
            //Color出力
            writer.WriteLine(option.Colors.Count);
            foreach (var colorProperty in option.Colors)
            {
                var color = material.GetColor(colorProperty);
                writer.WriteLine(colorProperty);
                writer.WriteLine($"{color.r:f8} {color.g:f8} {color.b:f8} {color.a:f8}");
            }

            //Texture出力
            writer.WriteLine(option.Textures.Count);
            foreach (var textureProperty in option.Textures)
            {
                var texture = (Texture2D) material.GetTexture(textureProperty);
                writer.WriteLine(textureProperty);
                if (texture != null)
                {
                    if (texture.format == TextureFormat.DXT1 || !texture.isReadable)
                    {
                        Debug.LogError(texture.name +
                                       "のインポートセッティングが不正値です。Read/WriteEnableにチェック、FormatをRGBA 32bitに変更");
                    }

                    writer.WriteLine(texture.name + textureProperty + ".png");
                    if (!File.Exists(filepath + texture.name + textureProperty + ".png"))
                    {
                        var pngData = texture.EncodeToPNG(); // pngのバイト情報を取得
                        File.WriteAllBytes(filepath + texture.name + textureProperty + ".png", pngData);
                    }
                }
                else
                    writer.WriteLine("null");
            }
        }

        public static void OutputBinary(this Material material, [NotNull] BinaryWriter writer,
            MaterialDataOption option, string filepath)
        {
            var byteStr = Encoding.ASCII.GetBytes(material.name);
            writer.Write((ushort) byteStr.Length);
            writer.Write(byteStr, 0, byteStr.Length);

            //Color出力
            writer.Write((ushort) option.Colors.Count);
            foreach (var colorProperty in option.Colors)
            {
                var color = material.GetColor(colorProperty);

                var byteStr2 = Encoding.ASCII.GetBytes(colorProperty);
                writer.Write((ushort) byteStr2.Length);
                writer.Write(byteStr2, 0, byteStr2.Length);

                writer.Write(color.r);
                writer.Write(color.g);
                writer.Write(color.b);
                writer.Write(color.a);
            }

            //Texture出力
            writer.Write((ushort) option.Textures.Count);
            foreach (var textureProperty in option.Textures)
            {
                var texture = (Texture2D) material.GetTexture(textureProperty);
                
                var byteStr2 = Encoding.ASCII.GetBytes(textureProperty);
                writer.Write((ushort) byteStr2.Length);
                writer.Write(byteStr2, 0, byteStr2.Length);

                if (texture != null)
                {
                    if (texture.format == TextureFormat.DXT1 || !texture.isReadable)
                    {
                        Debug.LogError(texture.name +
                                       "のインポートセッティングが不正値です。Read/WriteEnableにチェック、FormatをRGBA 32bitに変更");
                    }

                    var byteStr3 = Encoding.ASCII.GetBytes(texture.name + textureProperty + ".png");
                    writer.Write((ushort) byteStr3.Length);
                    writer.Write(byteStr3, 0, byteStr3.Length);

                    if (!File.Exists(filepath + texture.name + textureProperty + ".png"))
                    {
                        var pngData = texture.EncodeToPNG(); // pngのバイト情報を取得
                        File.WriteAllBytes(filepath + texture.name + textureProperty + ".png", pngData);
                    }
                }
                else
                    writer.Write((ushort) 0);
            }
        }
    }
}