using System;
using System.Collections.Generic;
using System.IO;

namespace Editor
{
	public class UnsafeBinaryReader
	{
		private byte[] readBuffer;
		private int currentNum = 0;

		public UnsafeBinaryReader(string filename)
		{
			readBuffer = File.ReadAllBytes(filename);
		}

		public unsafe void Read<T>(out T data)
		where T :unmanaged
		{
			fixed (byte* readDataRaw = &readBuffer[currentNum])
			{
				data = *(T*) readDataRaw;
			}
			currentNum += sizeof(T);
		}

		public unsafe void Read<T>(out T[] dates,int size)
			where T :unmanaged
		{
			var dataSize = sizeof(T);
			dates = new T[size];
			fixed (byte* readDataRaw = &readBuffer[currentNum])
			{
				for (var i = 0; i < size; i++)
				{
					dates[i] = *(T*) (readDataRaw + dataSize * i);
				}
			}

			currentNum += dataSize * size;
		}
	}

	public class UnsafeBinaryWriter : IDisposable
	{
		private readonly BinaryWriter writer;
		private readonly List<byte> writeBuffer = new List<byte>();

		public UnsafeBinaryWriter(string filename)
		{
			writer = new BinaryWriter(new FileStream(filename, FileMode.Create));
		}

		public unsafe void Write<T>(ref T data)
			where T : unmanaged
		{
			var dataSize = sizeof(T);
			fixed (T* writeData = &data)
			{
				var writeDataRaw = (byte*) writeData;
				for (var i = 0; i < dataSize; i++)
				{
					writeBuffer.Add(writeDataRaw[i]);
				}
			}
		}

		public unsafe void Write<T>(T[] dates)
			where T : unmanaged
		{
			var dataSize = sizeof(T);
			var writeSize = dates.Length;
			fixed (T* writeData = &dates[0])
			{
				var writeDataRaw = (byte*) writeData;
				for (var i = 0; i < writeSize; i++)
				{
					for (var j = 0; j < dataSize; j++)
					{
						writeBuffer.Add(writeDataRaw[i * dataSize + j]);
					}
				}
			}
		}
	
		public void Dispose()
		{
			writer.Write(writeBuffer.ToArray());
			writer?.Dispose();
		}
	}
}