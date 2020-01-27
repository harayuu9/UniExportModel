using System.Collections.Generic;
using System.IO;
using System.Linq;
using JetBrains.Annotations;
using UnityEditor;
using UnityEngine;

namespace Editor
{
    public partial class MeshExportWindow : EditorWindow
    {
        [MenuItem("ModelExport/Mesh")]
        private static void Create()
        {
            var window = GetWindow<MeshExportWindow>("MeshExport");
        }

        private int meshCount = 1;
        private readonly List<GameObject> meshObjectList = new List<GameObject>();

        private float progress = 0.0f;
        private string progressStr;


        private readonly VertexDataOption vertexDataOption = new VertexDataOption();
        private readonly MaterialDataOption materialDataOption = new MaterialDataOption();

        private void OnGUI()
        {
            meshCount = EditorGUILayout.IntField("Mesh Count", meshCount);
            while (meshCount > meshObjectList.Count)
                meshObjectList.Add(null);
            while (meshCount < meshObjectList.Count)
                meshObjectList.RemoveAt(meshObjectList.Count - 1);
            for (var i = 0; i < meshCount; i++)
                meshObjectList[i] =
                    EditorGUILayout.ObjectField("Mesh", meshObjectList[i], typeof(GameObject), true) as GameObject;
            using (new GUILayout.HorizontalScope())
            {
                if (GUILayout.Button("Write"))
                {
                    WriteMesh();
                }

                if (GUILayout.Button("WriteBinary"))
                {
                    WriteMeshBinary();
                }
            }

            using (new GUILayout.HorizontalScope())
            {
                if (GUILayout.Button("Write Scene"))
                {
                    meshObjectList.Clear();
                    var meshRenderers = FindObjectsOfType<MeshRenderer>();
                    meshCount = meshRenderers.Length;
                    foreach (var meshRenderer in meshRenderers)
                    {
                        if (meshRenderer.gameObject.activeInHierarchy)
                            meshObjectList.Add(meshRenderer.gameObject);
                    }

                    WriteMesh();
                }

                if (GUILayout.Button("Write Scene Binary"))
                {
                    meshObjectList.Clear();
                    var meshRenderers = FindObjectsOfType<MeshRenderer>();
                    meshCount = meshRenderers.Length;
                    foreach (var meshRenderer in meshRenderers)
                    {
                        if (meshRenderer.gameObject.activeInHierarchy)
                            meshObjectList.Add(meshRenderer.gameObject);
                    }

                    WriteMeshBinary();
                }
            }

            //出力するマテリアルデータ
            materialDataOption.AdvancedFlg =
                EditorGUILayout.BeginFoldoutHeaderGroup(materialDataOption.AdvancedFlg, "Material Setting");
            if (materialDataOption.AdvancedFlg)
            {
                int newCount = EditorGUILayout.IntField("Texture", materialDataOption.Textures.Count);
                while (newCount > materialDataOption.Textures.Count)
                    materialDataOption.Textures.Add("null");
                while (newCount < materialDataOption.Textures.Count)
                    materialDataOption.Textures.RemoveAt(materialDataOption.Textures.Count - 1);
                for (var index = 0; index < materialDataOption.Textures.Count; index++)
                {
                    materialDataOption.Textures[index] = EditorGUILayout.TextField(materialDataOption.Textures[index]);
                }

                EditorGUILayout.Space();
                newCount = EditorGUILayout.IntField("Color", materialDataOption.Colors.Count);
                while (newCount > materialDataOption.Colors.Count)
                    materialDataOption.Colors.Add("null");
                while (newCount < materialDataOption.Colors.Count)
                    materialDataOption.Colors.RemoveAt(materialDataOption.Colors.Count - 1);
                for (var index = 0; index < materialDataOption.Colors.Count; index++)
                {
                    materialDataOption.Colors[index] = EditorGUILayout.TextField(materialDataOption.Colors[index]);
                }
            }

            EditorGUILayout.EndFoldoutHeaderGroup();

            //拡張頂点情報
            vertexDataOption.AdvancedFlg =
                EditorGUILayout.BeginFoldoutHeaderGroup(vertexDataOption.AdvancedFlg, "Advance Setting");
            if (vertexDataOption.AdvancedFlg)
            {
                vertexDataOption.Position = EditorGUILayout.Toggle("Position", vertexDataOption.Position);
                vertexDataOption.Normal = EditorGUILayout.Toggle("Normal", vertexDataOption.Normal);
                vertexDataOption.Tangent = EditorGUILayout.Toggle("Tangent", vertexDataOption.Tangent);
                vertexDataOption.Uv1 = EditorGUILayout.Toggle("UV1", vertexDataOption.Uv1);
                vertexDataOption.Uv2 = EditorGUILayout.Toggle("UV2", vertexDataOption.Uv2);
                vertexDataOption.Uv3 = EditorGUILayout.Toggle("UV3", vertexDataOption.Uv3);
                vertexDataOption.Uv4 = EditorGUILayout.Toggle("UV4", vertexDataOption.Uv4);
                vertexDataOption.Uv5 = EditorGUILayout.Toggle("UV5", vertexDataOption.Uv5);
                vertexDataOption.Uv6 = EditorGUILayout.Toggle("UV6", vertexDataOption.Uv6);
                vertexDataOption.Uv7 = EditorGUILayout.Toggle("UV7", vertexDataOption.Uv7);
                vertexDataOption.Uv8 = EditorGUILayout.Toggle("UV8", vertexDataOption.Uv8);
                vertexDataOption.Color = EditorGUILayout.Toggle("Color", vertexDataOption.Color);
            }

            EditorGUILayout.EndFoldoutHeaderGroup();
        }

        private void WriteMesh()
        {
            var filePath = EditorUtility.SaveFilePanel("Save", "Assets", "MeshData", "uma");

            if (string.IsNullOrEmpty(filePath)) return;
            using (var writer = new StreamWriter(filePath))
            {
                var splitSlash = filePath.Split('/');
                filePath = filePath.Remove(filePath.Length - splitSlash.Last().Length, splitSlash.Last().Length);
                WriteMeshAscii(writer, filePath);
            }
        }

        private void WriteMeshBinary()
        {
            var filePath = EditorUtility.SaveFilePanel("Save", "Assets", "MeshData", "umb");

            if (string.IsNullOrEmpty(filePath)) return;
            using (var writer = new BinaryWriter(new FileStream(filePath, FileMode.Create)))
            {
                var splitSlash = filePath.Split('/');
                filePath = filePath.Remove(filePath.Length - splitSlash.Last().Length, splitSlash.Last().Length);
                WriteMeshBinary(writer, filePath);
            }
        }

        private void WriteMeshAscii([NotNull] StreamWriter writer, string filePath)
        {
            //保存する頂点データのフォーマットを保存(BitOR
            var vertexFormat = vertexDataOption.GetFormatFlg();
            writer.WriteLine((int) vertexFormat);
            
            writer.WriteLine(meshObjectList.Count);
            foreach (var meshObject in meshObjectList)
            {
                //ProgressView
                var matchIndex = meshObjectList.FindIndex(o => o == meshObject);
                progress = (float) matchIndex / meshObjectList.Count;
                var baseProgressStr = meshObject.name + "...(" + matchIndex + "/" + meshObjectList.Count + ")";
                EditorUtility.DisplayProgressBar("Write", progressStr = baseProgressStr, progress);

                var mesh = meshObject.GetComponent<MeshFilter>().sharedMesh;

                var nonSkinnedMesh = new NonSkinnedMesh(mesh,meshObject.transform.localToWorldMatrix);
                nonSkinnedMesh.OutputAscii(writer, vertexDataOption);

                writer.WriteLine("");
                //マテリアルデータを出力
                meshObject.GetComponent<MeshRenderer>().sharedMaterial
                    .OutputAscii(writer, materialDataOption, filePath);
            }
            EditorUtility.ClearProgressBar();
        }

        private void WriteMeshBinary([NotNull] BinaryWriter writer, string filePath)
        {
            //保存する頂点データのフォーマットを保存(BitOR 2byte
            var vertexFormat = vertexDataOption.GetFormatFlg();
            writer.Write((short) vertexFormat);
            
            writer.Write((ushort)meshObjectList.Count);
            foreach (var meshObject in meshObjectList)
            {
                //ProgressView
                var matchIndex = meshObjectList.FindIndex(o => o == meshObject);
                progress = (float) matchIndex / meshObjectList.Count;
                var baseProgressStr = meshObject.name + "...(" + matchIndex + "/" + meshObjectList.Count + ")";
                EditorUtility.DisplayProgressBar("Write", progressStr = baseProgressStr, progress);

                var mesh = meshObject.GetComponent<MeshFilter>().sharedMesh;
                
                var nonSkinnedMesh = new NonSkinnedMesh(mesh,meshObject.transform.localToWorldMatrix);
                nonSkinnedMesh.OutputBinary(writer,vertexDataOption);

                //マテリアルデータを出力
                meshObject.GetComponent<MeshRenderer>().sharedMaterial.OutputBinary(writer,materialDataOption,filePath);
            }

            EditorUtility.ClearProgressBar();
        }
    }
}