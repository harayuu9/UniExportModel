using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using UnityEditor;
using UnityEditor.Experimental.AssetImporters;
using UnityEngine;

namespace Editor
{
	[ScriptedImporter(1, "umb")]
	public class UmbImporter : ScriptedImporter
	{
		public override void OnImportAsset(AssetImportContext ctx)
		{
			var materialMap = new Dictionary<string, Material>();
			
			var parent = new GameObject("UmbParent");
			
			ctx.AddObjectToAsset("main", parent);
			ctx.SetMainObject(parent);
			
			var reader = new UnsafeBinaryReader(ctx.assetPath);
			reader.Read(out short vertexFormat);
			reader.Read(out ushort modelCount);
			for (var i = 0; i < modelCount; i++)
			{
				var obj = new GameObject($"{i}");
				var filter = obj.AddComponent<MeshFilter>();
				var renderer = obj.AddComponent<MeshRenderer>();

				var mesh = new Mesh {name = obj.name + "_mesh"};
				mesh.MarkDynamic();

				var positions = new List<Vector3>();
				var normals = new List<Vector3>();
				var tangents = new List<Vector3>();
				var uv1S = new List<Vector2>();
				var uv2S = new List<Vector2>();
				var uv3S = new List<Vector2>();
				var uv4S = new List<Vector2>();
				var uv5S = new List<Vector2>();
				var uv6S = new List<Vector2>();
				var uv7S = new List<Vector2>();
				var uv8S = new List<Vector2>();
				var colors = new List<Color>();

				reader.Read(out uint vertexCount);
				for (var j = 0; j < vertexCount; j++)
				{
					if ((vertexFormat & 0x0001) != 0)
					{
						reader.Read(out Vector3 position);
						positions.Add(position);
					}

					if ((vertexFormat & 0x0002) != 0)
					{
						reader.Read(out Vector3 normal);
						normals.Add(normal);
					}

					if ((vertexFormat & 0x0004) != 0)
					{
						reader.Read(out Vector3 tangent);
						tangents.Add(tangent);
					}

					if ((vertexFormat & 0x0008) != 0)
					{
						reader.Read(out Vector2 uv1);
						uv1S.Add(new Vector2(uv1.x, 1 - uv1.y));
					}

					if ((vertexFormat & 0x0010) != 0)
					{
						reader.Read(out Vector2 uv2);
						uv2S.Add(new Vector2(uv2.x, 1 - uv2.y));
					}

					if ((vertexFormat & 0x0020) != 0)
					{
						reader.Read(out Vector2 uv3);
						uv3S.Add(new Vector2(uv3.x, 1 - uv3.y));
					}

					if ((vertexFormat & 0x0040) != 0)
					{
						reader.Read(out Vector2 uv4);
						uv4S.Add(new Vector2(uv4.x, 1 - uv4.y));
					}

					if ((vertexFormat & 0x0080) != 0)
					{
						reader.Read(out Vector2 uv5);
						uv5S.Add(new Vector2(uv5.x, 1 - uv5.y));
					}

					if ((vertexFormat & 0x0100) != 0)
					{
						reader.Read(out Vector2 uv6);
						uv6S.Add(new Vector2(uv6.x, 1 - uv6.y));
					}

					if ((vertexFormat & 0x0200) != 0)
					{
						reader.Read(out Vector2 uv7);
						uv7S.Add(new Vector2(uv7.x, 1 - uv7.y));
					}

					if ((vertexFormat & 0x0400) != 0)
					{
						reader.Read(out Vector2 uv8);
						uv8S.Add(new Vector2(uv8.x, 1 - uv8.y));
					}

					if ((vertexFormat & 0x0800) != 0)
					{
						reader.Read(out Color color);
						colors.Add(color);
					}
				}

				reader.Read(out uint indexCount);
				reader.Read(out uint[] indexes, (int) indexCount);

				mesh.SetVertices(positions);
				mesh.SetNormals(normals);
				mesh.SetTangents(tangents.Select(tangent => new Vector4(tangent.x, tangent.y, tangent.z, 1)).ToList());
				mesh.SetUVs(0, uv1S);
				mesh.SetUVs(1, uv2S);
				mesh.SetUVs(2, uv3S);
				mesh.SetUVs(3, uv4S);
				mesh.SetUVs(4, uv5S);
				mesh.SetUVs(5, uv6S);
				mesh.SetUVs(6, uv7S);
				mesh.SetUVs(7, uv8S);
				mesh.SetColors(colors);
				mesh.SetTriangles(indexes.Select(ui => (int) ui).ToArray(), 0);
				mesh.RecalculateBounds();
				filter.sharedMesh = mesh;

				reader.Read(out ushort materialNameCount);
				reader.Read(out byte[] materialName, materialNameCount);
				var materialNameA = Encoding.ASCII.GetString(materialName);
				var newMaterial = new Material(Shader.Find("Standard"));
				if (!materialMap.ContainsKey(materialNameA))
				{
					newMaterial.name = materialNameA;
					ctx.AddObjectToAsset(newMaterial.name, newMaterial);
					materialMap.Add(newMaterial.name,newMaterial);
				}
				renderer.material = materialMap[materialNameA];

				reader.Read(out ushort colorCount);
				for (var j = 0; j < colorCount; j++)
				{
					reader.Read(out ushort propertyNameCount);
					reader.Read(out byte[] propertyName, propertyNameCount);
					reader.Read(out Color color);
					newMaterial.SetColor(Encoding.ASCII.GetString(propertyName), color);
				}

				reader.Read(out ushort textureCount);
				for (var j = 0; j < textureCount; j++)
				{
					reader.Read(out ushort propertyNameCount);
					reader.Read(out byte[] propertyName, propertyNameCount);

					reader.Read(out ushort textureNameCount);
					reader.Read(out byte[] textureName, textureNameCount);
					var texture =
						AssetDatabase.LoadAssetAtPath(
							Path.GetDirectoryName(ctx.assetPath) + "\\" + Encoding.ASCII.GetString(textureName),
							typeof(Texture2D)) as Texture2D;
					newMaterial.SetTexture(Encoding.ASCII.GetString(propertyName), texture);
				}

				obj.transform.parent = parent.transform;
				ctx.AddObjectToAsset(mesh.name, mesh);
				ctx.AddObjectToAsset(obj.name, obj);
			}
		}
	}
}