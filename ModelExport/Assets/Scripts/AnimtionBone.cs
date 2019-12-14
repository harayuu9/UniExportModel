using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEditor;
using UnityEngine;

public class AnimtionBone : MonoBehaviour
{
	[SerializeField] private AnimationClip clip = null;

	private class ClipToAnimation
	{
		private readonly Dictionary<string, int> PropetyToCurve = new Dictionary<string, int>()
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

		private class Curve
		{
			public readonly List<float> Time = new List<float>();
			public readonly List<float> Key = new List<float>();

			public float GetValue(float time)
			{
				//外
				if (Time.Count == 1)
					return Key[0];
				if (time < Time[0])
					return Key[0];
				if (time > Time.Last())
					return Key.Last();

				var index = 0;
				for (; index < Time.Count; index++)
				{
					if (Time[index] > time)
						break;
				}
				index--;

				return Mathf.Lerp(Key[index], Key[index + 1], (time - Time[index]) / (Time[index + 1] - Time[index]));
			}
		}

		public Transform Transform { get; set; }

		private readonly List<Curve> curves = new List<Curve>()
		{
			new Curve(), new Curve(), new Curve(),
			new Curve(), new Curve(), new Curve(), new Curve(),
			new Curve(), new Curve(), new Curve()
		};

		public void AddKey(string propertyName, float time, float key)
		{
			if (!PropetyToCurve.ContainsKey(propertyName))
			{
				Debug.LogWarning($"\"{propertyName}\" is not supported and is not output.");
				return;
			}
			curves[PropetyToCurve[propertyName]].Time.Add(time);
			curves[PropetyToCurve[propertyName]].Key.Add(key);
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
				if (curves[i].Key.Any()) continue;
				curves[i].Key.Add(floatList[i]);
				curves[i].Time.Add(0.0f);
			}
		}

		public void SetTransform(float time)
		{
			var position = new Vector3(curves[0].GetValue(time), curves[1].GetValue(time), curves[2].GetValue(time));
			var rotation = new Quaternion(curves[3].GetValue(time), curves[4].GetValue(time), curves[5].GetValue(time),
				curves[6].GetValue(time));
			var scale = new Vector3(curves[7].GetValue(time), curves[8].GetValue(time), curves[9].GetValue(time));

			Transform.localPosition = position;
			Transform.localRotation = rotation;
			Transform.localScale = scale;
		}
	}

	private readonly List<ClipToAnimation> animationList = new List<ClipToAnimation>();

	private void Awake()
	{
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

		foreach (var clipToAnimation in animationList)
		{
			clipToAnimation.InitKey();
		}
	}

	public float time = 0.0f;

	private void Update()
	{
		foreach (var clipToAnimation in animationList)
		{
//			time = Time.time * 0.5f;
			clipToAnimation.SetTransform(time);
		}
	}
}