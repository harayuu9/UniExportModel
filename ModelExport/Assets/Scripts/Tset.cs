using System;
using UnityEditor;
using UnityEngine;

namespace DefaultNamespace
{
	public class Tset : MonoBehaviour
	{
		[SerializeField] private Material mat;
		
		private void Start()
		{
			var count = ShaderUtil.GetPropertyCount(mat.shader);
			for (int i = 0; i < count; i++)
			{
				var pt = ShaderUtil.GetPropertyType(mat.shader, i);
				if (pt == ShaderUtil.ShaderPropertyType.TexEnv)
				{
					var pn = ShaderUtil.GetPropertyName(mat.shader, i);
					Debug.Log(pn);
					var tex = mat.GetTexture(pn);
					if (tex)
					{
						Debug.Log(tex.name);
					}
				}
				//else if(pt == ShaderUtil.ShaderPropertyType.Color)
			}
		}
	}
}