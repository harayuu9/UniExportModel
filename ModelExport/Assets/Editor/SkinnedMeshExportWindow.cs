using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using JetBrains.Annotations;
using UnityEditor;
using UnityEngine;
using Debug = UnityEngine.Debug;

namespace Editor
{
	public class SkinnedMeshExportWindow : EditorWindow
	{
		[MenuItem("ModelExport/SkinnedMesh & Animation")]
		private static void Create()
		{
			var window = GetWindow<SkinnedMeshExportWindow>("SkinnedMeshExport");
		}

		private readonly VertexDataOption vertexDataOption = new VertexDataOption();
		private readonly MaterialDataOption materialDataOption = new MaterialDataOption();

		private GameObject meshObject = null;

		private void OnGUI()
		{
			meshObject = EditorGUILayout.ObjectField("SkinnedMesh", meshObject, typeof(GameObject), true) as GameObject;
			if (meshObject != null)
			{
				using (new GUILayout.HorizontalScope())
				{
					if (GUILayout.Button("Write"))
						WriteSkinnedMesh(meshObject);
					if (GUILayout.Button("WriteBinary"))
						WriteSkinnedMeshBinary(meshObject);
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
				EditorGUILayout.Toggle("BoneIndex", VertexDataOption.BoneIndex);
				EditorGUILayout.Toggle("BoneWeight", VertexDataOption.BoneWeight);
			}

			EditorGUILayout.EndFoldoutHeaderGroup();
		}

		private void WriteSkinnedMesh(GameObject @object)
		{
			//オブジェクトに非対応のものが入ってきたときの処理
			var animator = @object.GetComponentInChildren<Animator>();
			if (animator == null)
			{
				EditorUtility.DisplayDialog("Model Export Error", "This object don't have Animator", "OK");
				return;
			}

			if (animator.isHuman)
			{
				EditorUtility.DisplayDialog("Model Export Error", "Humanoid is not support", "OK");
				return;
			}

			var filePath = EditorUtility.SaveFilePanel("Save", "Assets", "SkinnedMeshData", "usa");

			if (string.IsNullOrEmpty(filePath)) return;
			using (var writer = new StreamWriter(filePath))
			{
				var splitSlash = filePath.Split('/');
				var tmpFilePath = filePath.Remove(filePath.Length - splitSlash.Last().Length,
					splitSlash.Last().Length);
				WriteSkinnedMeshAscii(writer, @object, tmpFilePath);
			}

			//Animatorに登録されているアニメーション全て
			var clips = animator.runtimeAnimatorController.animationClips;
			foreach (var clip in clips)
			{
				var clipName = clip.name;
				var invalidChars = Path.GetInvalidFileNameChars();
				clipName = invalidChars.Aggregate(clipName, (current, c) => current.Replace(c.ToString(), ""));
				Debug.Log(filePath + clipName + "anim.usaa");

				using (var writer = new StreamWriter(filePath + clipName + "anim.usaa"))
				{
					WriteAnimationAscii(writer, clip, @object.transform);
				}
			}
		}

		private void WriteSkinnedMeshBinary(GameObject @object)
		{
			//オブジェクトに非対応のものが入ってきたときの処理
			var animator = @object.GetComponentInChildren<Animator>();
			if (animator == null)
			{
				EditorUtility.DisplayDialog("Model Export Error", "This object don't have Animator", "OK");
				return;
			}

			if (animator.isHuman)
			{
				EditorUtility.DisplayDialog("Model Export Error", "Humanoid is not support", "OK");
				return;
			}

			var filePath = EditorUtility.SaveFilePanel("Save", "Assets", "SkinnedMeshData", "usb");

			if (string.IsNullOrEmpty(filePath)) return;
			using (var writer = new BinaryWriter(new FileStream(filePath, FileMode.Create)))
			{
				var splitSlash = filePath.Split('/');
				filePath = filePath.Remove(filePath.Length - splitSlash.Last().Length, splitSlash.Last().Length);
				WriteSkinnedMeshBinary(writer, @object, filePath);
			}

			//Animatorに登録されているアニメーション全て
			var clips = animator.runtimeAnimatorController.animationClips;
			foreach (var clip in clips)
			{
				var clipName = clip.name;
				var invalidChars = Path.GetInvalidFileNameChars();
				clipName = invalidChars.Aggregate(clipName, (current, c) => current.Replace(c.ToString(), ""));
				Debug.Log(filePath + clipName + "anim.usab");

				using (var writer =
					new BinaryWriter(new FileStream(filePath + clipName + "anim.usab", FileMode.Create)))
				{
					WriteAnimationBinary(writer, clip, @object.transform);
				}
			}
		}

		#region SkinnedMeshAscii

		private void LoopParentChildWriteAscii([NotNull] StreamWriter writer, Transform root)
		{
			writer.WriteLine(root.name);
			var localPosition = root.localPosition;
			var localRotation = root.localRotation;
			var localScale = root.localScale;
			writer.WriteLine(
				$"{localPosition.x:f8} {localPosition.y:f8} {localPosition.z:f8} " +
				$"{localRotation.eulerAngles.x:f8} {localRotation.eulerAngles.y:f8} {localRotation.eulerAngles.z:f8} " +
				$"{localScale.x:f8} {localScale.y:f8} {localScale.z:f8}");
			foreach (Transform child in root)
			{
				LoopParentChildWriteAscii(writer, child);
			}

			writer.WriteLine("ChildEndTransform");
		}

		private void WriteSkinnedMeshAscii([NotNull] StreamWriter writer, GameObject @object, string filePath)
		{
			//階層情報を保存
			LoopParentChildWriteAscii(writer, @object.transform);

			//保存する頂点データのフォーマットを保存(BitOR
			var vertexFormat = vertexDataOption.GetFormatFlg();
			writer.WriteLine((int) vertexFormat);

			//Meshの情報を保存
			var smrs = @object.GetComponentsInChildren<SkinnedMeshRenderer>();
			//メッシュレンダラー混在のモデルもあるので
			var mrs = @object.GetComponentsInChildren<MeshRenderer>();

			writer.WriteLine(smrs.Length + mrs.Length);
			foreach (var smr in smrs)
			{
				var mesh = smr.sharedMesh;

				//頂点データ出力
				var skinnedMesh = new SkinnedMesh(mesh);
				skinnedMesh.OutputAscii(writer, vertexDataOption);

				writer.WriteLine("");

				if (mesh.bindposes.Length == 0)
				{
					//SkinnedMeshなのにスキンがついてない謎モデル
					writer.WriteLine(1);
					writer.WriteLine(smr.transform.name);
					writer.WriteLine($"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");
				}
				else
				{
					Debug.Assert(smr.bones.Length == mesh.bindposes.Length);
					//BasePose出力
					writer.WriteLine(mesh.bindposes.Length);
					for (var index = 0; index < mesh.bindposes.Length; index++)
					{
						var trans = smr.bones[index];
						var bindPose = smr.sharedMesh.bindposes[index];
						//Boneの名前と逆行列を保存
						writer.WriteLine(trans.name);
						writer.WriteLine(
							$"{bindPose.m00:f8} {bindPose.m01:f8} {bindPose.m02:f8} {bindPose.m03:f8} " +
							$"{bindPose.m10:f8} {bindPose.m11:f8} {bindPose.m12:f8} {bindPose.m13:f8} " +
							$"{bindPose.m20:f8} {bindPose.m21:f8} {bindPose.m22:f8} {bindPose.m23:f8} " +
							$"{bindPose.m30:f8} {bindPose.m31:f8} {bindPose.m32:f8} {bindPose.m33:f8}");
					}
				}

				//マテリアルデータを出力
				smr.sharedMaterial.OutputAscii(writer, materialDataOption, filePath);
			}

			foreach (var mr in mrs)
			{
				var mesh = mr.GetComponent<MeshFilter>().sharedMesh;
				var skinnedMesh = new SkinnedMesh(mesh);
				skinnedMesh.OutputAscii(writer, vertexDataOption);

				writer.WriteLine("");

				//SkinnedMeshなのにスキンがついてない謎モデル
				writer.WriteLine(1);
				writer.WriteLine(mr.transform.name);
				writer.WriteLine($"1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1");

				//マテリアルデータを出力
				mr.sharedMaterial.OutputAscii(writer, materialDataOption, filePath);
			}
		}

		#endregion

		#region SkinnedMeshBinary

		private void LoopParentChildWriteBinary([NotNull] BinaryWriter writer, Transform root)
		{
			var byteStr = Encoding.ASCII.GetBytes(root.name);
			writer.Write((short) byteStr.Length);
			writer.Write(byteStr, 0, byteStr.Length);

			var localPosition = root.localPosition;
			writer.Write(localPosition.x);
			writer.Write(localPosition.y);
			writer.Write(localPosition.z);
			var localRotation = root.localRotation;
			writer.Write(localRotation.eulerAngles.x);
			writer.Write(localRotation.eulerAngles.y);
			writer.Write(localRotation.eulerAngles.z);
			var localScale = root.localScale;
			writer.Write(localScale.x);
			writer.Write(localScale.y);
			writer.Write(localScale.z);

			foreach (Transform child in root)
			{
				LoopParentChildWriteBinary(writer, child);
			}

			writer.Write((short) -1);
		}

		private void WriteSkinnedMeshBinary([NotNull] BinaryWriter writer, GameObject @object, string filePath)
		{
			//階層情報を保存
			LoopParentChildWriteBinary(writer, @object.transform);

			//保存する頂点データのフォーマットを保存(BitOR 2byte
			var vertexFormat = vertexDataOption.GetFormatFlg();
			writer.Write((short) vertexFormat);

			//Meshの情報を保存
			var smrs = @object.GetComponentsInChildren<SkinnedMeshRenderer>();
			var mrs = @object.GetComponentsInChildren<MeshRenderer>();

			writer.Write((ushort) (smrs.Length + mrs.Length));
			foreach (var smr in smrs)
			{
				var mesh = smr.sharedMesh;

				//頂点データ出力
				var skinnedMesh = new SkinnedMesh(mesh);
				skinnedMesh.OutputBinary(writer, vertexDataOption);

				if (mesh.bindposes.Length == 0)
				{
					//SkinnedMeshなのにスキンがついてない謎モデル
					writer.Write((ushort) 1);

					var byteStr = Encoding.ASCII.GetBytes(smr.transform.name);
					writer.Write((ushort) byteStr.Length);
					writer.Write(byteStr, 0, byteStr.Length);

					writer.Write(1.0f);
					writer.Write(0.0f);
					writer.Write(0.0f);
					writer.Write(0.0f);

					writer.Write(0.0f);
					writer.Write(1.0f);
					writer.Write(0.0f);
					writer.Write(0.0f);

					writer.Write(0.0f);
					writer.Write(0.0f);
					writer.Write(1.0f);
					writer.Write(0.0f);

					writer.Write(0.0f);
					writer.Write(0.0f);
					writer.Write(0.0f);
					writer.Write(1.0f);
				}
				else
				{
					Debug.Assert(smr.bones.Length == mesh.bindposes.Length);
					//BasePose出力
					writer.Write((ushort) mesh.bindposes.Length);
					for (var index = 0; index < mesh.bindposes.Length; index++)
					{
						var trans = smr.bones[index];
						var bindPose = smr.sharedMesh.bindposes[index];
						//Boneの名前と逆行列を保存
						var byteStr = Encoding.ASCII.GetBytes(trans.name);
						writer.Write((ushort) byteStr.Length);
						writer.Write(byteStr, 0, byteStr.Length);

						writer.Write(bindPose.m00);
						writer.Write(bindPose.m01);
						writer.Write(bindPose.m02);
						writer.Write(bindPose.m03);

						writer.Write(bindPose.m10);
						writer.Write(bindPose.m11);
						writer.Write(bindPose.m12);
						writer.Write(bindPose.m13);

						writer.Write(bindPose.m20);
						writer.Write(bindPose.m21);
						writer.Write(bindPose.m22);
						writer.Write(bindPose.m23);

						writer.Write(bindPose.m30);
						writer.Write(bindPose.m31);
						writer.Write(bindPose.m32);
						writer.Write(bindPose.m33);
					}
				}

				//マテリアルデータの出力
				smr.sharedMaterial.OutputBinary(writer, materialDataOption, filePath);
			}

			foreach (var mr in mrs)
			{
				var mesh = mr.GetComponent<MeshFilter>().sharedMesh;

				//頂点データ出力
				var skinnedMesh = new SkinnedMesh(mesh);
				skinnedMesh.OutputBinary(writer, vertexDataOption);

				//SkinnedMeshなのにスキンがついてない謎モデル
				writer.Write((ushort) 1);

				var byteStr = Encoding.ASCII.GetBytes(mr.transform.name);
				writer.Write((ushort) byteStr.Length);
				writer.Write(byteStr, 0, byteStr.Length);

				writer.Write(1.0f);
				writer.Write(0.0f);
				writer.Write(0.0f);
				writer.Write(0.0f);

				writer.Write(0.0f);
				writer.Write(1.0f);
				writer.Write(0.0f);
				writer.Write(0.0f);

				writer.Write(0.0f);
				writer.Write(0.0f);
				writer.Write(1.0f);
				writer.Write(0.0f);

				writer.Write(0.0f);
				writer.Write(0.0f);
				writer.Write(0.0f);
				writer.Write(1.0f);

				//マテリアルデータの出力
				mr.sharedMaterial.OutputBinary(writer, materialDataOption, filePath);
			}
		}

		//TODO Next Version...

		#endregion

		private class ClipToAnimation
		{
			private readonly Dictionary<string, int> propertyToCurve = new Dictionary<string, int>()
			{
				{"m_LocalPosition.x", 0},
				{"m_LocalPosition.y", 1},
				{"m_LocalPosition.z", 2},
				{"m_LocalRotation.x", 3},
				{"m_LocalRotation.y", 4},
				{"m_LocalRotation.z", 5},
				{"m_LocalRotation.w", 6},
				{"m_LocalScale.x", 7},
				{"m_LocalScale.y", 8},
				{"m_LocalScale.z", 9},
			};

			public class Curve
			{
				public readonly List<float> Time = new List<float>();
				public readonly List<float> Key = new List<float>();
			}

			public Transform Transform { get; set; }

			public readonly List<Curve> Curves = new List<Curve>()
			{
				new Curve(), new Curve(), new Curve(),
				new Curve(), new Curve(), new Curve(), new Curve(),
				new Curve(), new Curve(), new Curve()
			};

			public void AddKey(string propertyName, float time, float key)
			{
				if (!propertyToCurve.ContainsKey(propertyName))
				{
					//Debug.LogWarning($"\"{propertyName}\" is not supported and is not output.");
					return;
				}

				Curves[propertyToCurve[propertyName]].Time.Add(time);
				Curves[propertyToCurve[propertyName]].Key.Add(key);
			}

			public void InitKey()
			{
				var localPosition = Transform.localPosition;
				var localRotation = Transform.localRotation;
				var localScale = Transform.localScale;
				var floatList = new List<float>
				{
					localPosition.x,
					localPosition.y,
					localPosition.z,
					localRotation.x,
					localRotation.y,
					localRotation.z,
					localRotation.w,
					localScale.x,
					localScale.y,
					localScale.z
				};

				for (var i = 0; i < 10; i++)
				{
					if (Curves[i].Key.Any()) continue;
					Curves[i].Key.Add(floatList[i]);
					Curves[i].Time.Add(0.0f);
				}
			}
		}

		#region AnimationAscii

		private void WriteAnimationAscii([NotNull] StreamWriter writer, AnimationClip clip, Transform transform)
		{
			var animationList = new List<ClipToAnimation>();
			var bindings = AnimationUtility.GetCurveBindings(clip);
			foreach (var binding in bindings)
			{
				var tmp = transform.Find(binding.path);
				var toAnimation = animationList.Find(anim => anim.Transform == tmp);
				ClipToAnimation clipToAnim;
				if (toAnimation == null)
				{
					clipToAnim = new ClipToAnimation {Transform = transform.Find(binding.path)};
					animationList.Add(clipToAnim);
				}
				else
				{
					clipToAnim = toAnimation;
				}

				var curve = AnimationUtility.GetEditorCurve(clip, binding);
				foreach (var key in curve.keys)
				{
					clipToAnim.AddKey(binding.propertyName, key.time, key.value);
				}
			}

			writer.Write($"{animationList.Count} ");
			foreach (var clipToAnimation in animationList)
			{
				clipToAnimation.InitKey();
				writer.WriteLine(clipToAnimation.Transform.name);
				foreach (var curve in clipToAnimation.Curves)
				{
					writer.WriteLine(curve.Time.Count);
					foreach (var f in curve.Time)
					{
						writer.WriteLine($"{f:f8}");
					}

					foreach (var f in curve.Key)
					{
						writer.WriteLine($"{f:f8}");
					}
				}
			}
		}

		#endregion

		#region AnimationBinary

		private void WriteAnimationBinary([NotNull] BinaryWriter writer, AnimationClip clip, Transform transform)
		{
			var animationList = new List<ClipToAnimation>();
			var bindings = AnimationUtility.GetCurveBindings(clip);
			foreach (var binding in bindings)
			{
				var tmp = transform.Find(binding.path);
				var toAnimation = animationList.Find(anim => anim.Transform == tmp);
				ClipToAnimation clipToAnim;
				if (toAnimation == null)
				{
					clipToAnim = new ClipToAnimation {Transform = transform.Find(binding.path)};
					animationList.Add(clipToAnim);
				}
				else
				{
					clipToAnim = toAnimation;
				}

				var curve = AnimationUtility.GetEditorCurve(clip, binding);
				foreach (var key in curve.keys)
				{
					clipToAnim.AddKey(binding.propertyName, key.time, key.value);
				}
			}

			writer.Write((uint) animationList.Count);
			foreach (var clipToAnimation in animationList)
			{
				clipToAnimation.InitKey();
				var byteStr = Encoding.ASCII.GetBytes(clipToAnimation.Transform.name);
				writer.Write((ushort) clipToAnimation.Transform.name.Length);
				writer.Write(byteStr, 0, byteStr.Length);
				foreach (var curve in clipToAnimation.Curves)
				{
					writer.Write((uint) curve.Time.Count);
					foreach (var f in curve.Time)
					{
						writer.Write(f);
					}

					foreach (var f in curve.Key)
					{
						writer.Write(f);
					}
				}
			}
		}

		#endregion
	}
}