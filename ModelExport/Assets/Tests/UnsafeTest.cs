// using System.IO;
// using System.Runtime.CompilerServices;
// using NUnit.Framework;
// using UnityEngine;
// using Utility;
//
// public class UnsafeTest
// {
// 	private Vector4[] RandomData(int size)
// 	{
// 		var result = new Vector4[size];
// 		return result;
// 	}
//
// 	private const int arrayLength = 10000000;
// 	
// 	[Test]
// 	public void Test()
// 	{
// 		using (var writer = new UnsafeBinaryWriter("aaa.bin"))
// 		{
// 			var vector3 = new Vector3(50, 30, 10);
// 			writer.Write(ref vector3);
// 			writer.Write(RandomData(arrayLength));
// 		}
//
// 		{
// 			var reader = new UnsafeBinaryReader("aaa.bin");
// 			var work = Vector3.zero;
// 			reader.Read(ref work);
// 			reader.Read(out Vector4[] vector3s,arrayLength);
// 		}
// 	}
//
// 	[Test]
// 	public void Test2()
// 	{
// 		using (var writer = new BinaryWriter(new FileStream("bbb.bin",FileMode.Create)))
// 		{
// 			var vector3 = new Vector3(50, 30, 10);
// 			var vector4s = RandomData(arrayLength);
// 			writer.Write(vector3.x);
// 			writer.Write(vector3.y);
// 			writer.Write(vector3.z);
// 			foreach (var vector31 in vector4s)
// 			{
// 				writer.Write(vector31.x);
// 				writer.Write(vector31.y);
// 				writer.Write(vector31.z);
// 				writer.Write(vector31.w);
// 			}
// 		}
//
// 		using (var reader = new BinaryReader(new FileStream("bbb.bin", FileMode.Open)))
// 		{
// 			var work = new Vector3(reader.ReadSingle(),reader.ReadSingle(),reader.ReadSingle());
// 			var vector3s = new Vector4[arrayLength];
// 			for (var i = 0; i < arrayLength; i++)
// 			{
// 				vector3s[i] = new Vector4(reader.ReadSingle(),reader.ReadSingle(),reader.ReadSingle(),reader.ReadSingle());
// 			}
// 		}
// 	}
// }