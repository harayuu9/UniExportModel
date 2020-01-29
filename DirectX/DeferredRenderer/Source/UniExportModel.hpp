#pragma once
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <memory>

namespace uem
{
	class FileStream
	{
	private:
		char* data = nullptr;
		char* activeData = nullptr;
	public:
		FileStream() {}
		FileStream(const char* filename) { Load(filename); }
		~FileStream() {
			if (data != nullptr)
				delete[] data;
		}

		void Load(const char* filename)
		{
			FILE* fp;
			assert(fopen_s(&fp, filename, "rb") == 0);
			fpos_t pos = 0;
			fseek(fp, 0L, SEEK_END);
			fgetpos(fp, &pos);
			fseek(fp, 0, SEEK_SET);

			data = new char[pos];
			fread(data, sizeof(char), pos, fp);
			activeData = data;
			fclose(fp);
		}

		void Read(void* data, int size)
		{
			memcpy(data, activeData, size);
			activeData += size;
		}
	};

	struct Transform
	{
		Transform* parent = nullptr;
		std::vector<Transform*> child;

		size_t hash;
		std::string name;
		DirectX::XMFLOAT3 position;
		DirectX::XMVECTOR rotation;
		DirectX::XMFLOAT3 scale;

		DirectX::XMMATRIX LocalToWorldMatrix()
		{
			auto localMtx = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				DirectX::XMMatrixRotationQuaternion(rotation) *
				DirectX::XMMatrixTranslation(position.x, position.y, position.z);
			if (parent == nullptr)
				return localMtx;
			else
				return localMtx * parent->LocalToWorldMatrix();
		}

		Transform* Find(std::string str)
		{
			if (name == str)
				return this;
			for (auto t : child)
			{
				auto ret = t->Find(str);
				if (ret != nullptr)
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

	static std::vector<std::pair<int, int>> VertexFormatSizes()
	{
		std::vector<std::pair<int, int>> result;
		result.push_back(std::make_pair(POSITION, 4 * 3));
		result.push_back(std::make_pair(NORMAL, 4 * 3));
		result.push_back(std::make_pair(TANGENT, 4 * 3));
		result.push_back(std::make_pair(UV1, 4 * 2));
		result.push_back(std::make_pair(UV2, 4 * 2));
		result.push_back(std::make_pair(UV3, 4 * 2));
		result.push_back(std::make_pair(UV4, 4 * 2));
		result.push_back(std::make_pair(UV5, 4 * 2));
		result.push_back(std::make_pair(UV6, 4 * 2));
		result.push_back(std::make_pair(UV7, 4 * 2));
		result.push_back(std::make_pair(UV8, 4 * 2));
		result.push_back(std::make_pair(COLOR, 4 * 4));
		return result;
	}

	struct Material
	{
	private:
		std::unordered_map<size_t, std::string> textureNames;
		std::unordered_map<size_t, DirectX::XMFLOAT4> colors;

	public:
		std::string name;

		static size_t GetHash(std::string property)
		{
			return std::hash<std::string>()(property);
		}

		void AddColor(std::string property, DirectX::XMFLOAT4 color)
		{
			colors.insert(std::make_pair(GetHash(property), color));
		}
		void AddTexture(std::string property, std::string textureName)
		{
			textureNames.insert(std::make_pair(GetHash(property), textureName));
		}

		DirectX::XMFLOAT4 GetColor(size_t propertyHash)
		{
			return colors[propertyHash];
		}
		DirectX::XMFLOAT4 GetColor(std::string property)
		{
			return GetColor(GetHash(property));
		}

		std::string GetTexture(size_t propertyHash)
		{
			return textureNames[propertyHash];
		}
		std::string GetTexture(std::string property)
		{
			return GetTexture(GetHash(property));
		}

		bool operator ==(const std::string& a)
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
			std::vector<uint32_t> indexs;
			int materialNo;
		};
		std::vector<Mesh> meshs;
		std::vector<Material> materials;

		void LoadAscii(std::string filename)
		{
			std::ifstream ifs(filename);
			auto lastSlash = filename.find_last_of('/');
			filename.erase(lastSlash);
			assert(ifs.is_open());

			//頂点フォーマットを読み込み
			int vertexFormat;
			ifs >> vertexFormat;

			int modelCount;
			ifs >> modelCount;

			//フォーマットエラーチェック
			std::vector<bool> formatFlg;
			formatFlg.resize(12);
			int totalByte = 0;
			auto formatSizes = VertexFormatSizes();
			for (int i = 0; i < formatSizes.size(); i++)
				if (vertexFormat & formatSizes[i].first)
					totalByte += formatSizes[i].second;
			if (totalByte != sizeof(X))
			{
				std::string errorLog = std::string(typeid(X).name()) + " is " + std::to_string(sizeof(X)) + "\n " +
					"The required size is " + std::to_string(totalByte) + " bytes";
			}

			for (int i = 0; i < modelCount; i++)
			{
				Mesh model;
				//頂点情報読み込み
				int vertexCount;
				ifs >> vertexCount;
				for (int j = 0; j < vertexCount; j++)
				{
					uint8_t* rawData = new uint8_t[sizeof(X)];
					int rawCnt = 0;

					for (int i = 0; i < formatSizes.size(); i++)
					{
						if (vertexFormat & formatSizes[i].first)
						{
							float tmpData[4];
							int dataSize = formatSizes[i].second / 4;
							for (int j = 0; j < dataSize; j++)
								ifs >> tmpData[j];
							memcpy(&rawData[rawCnt], tmpData, formatSizes[i].second);
							rawCnt += formatSizes[i].second;
						}
					}

					X data;
					memcpy(&data, rawData, sizeof(X));
					delete[] rawData;
					model.vertexDatas.push_back(data);
				}

				//インデックス読み込み
				int indexCount;
				ifs >> indexCount;
				model.indexs.resize(indexCount);
				for (int j = 0; j < indexCount; j++)
				{
					ifs >> model.indexs[j];
				}

				//マテリアルの読み込み
				Material material;
				ifs >> material.name;
				int colorCount;
				ifs >> colorCount;
				for (int i = 0; i < colorCount; i++)
				{
					std::string propertyName;
					ifs >> propertyName;
					DirectX::XMFLOAT4 color;
					ifs >> color.x >> color.y >> color.z >> color.w;
					material.AddColor(propertyName, color);
				}

				int textureCount;
				ifs >> textureCount;
				for (int i = 0; i < textureCount; i++)
				{
					std::string propertyName;
					ifs >> propertyName;
					std::string textureName;
					ifs >> textureName;
					material.AddTexture(propertyName, filename + "/" + textureName);
				}

				int materialNo = -1;
				for (int j = 0; j < static_cast<int>(materials.size()); j++)
				{
					if (materials[j] == material.name)
						materialNo = j;
				}
				if (materialNo == -1)
				{
					materialNo = static_cast<int>(materials.size());
					materials.push_back(material);
				}
				model.materialNo = materialNo;
				meshs.push_back(model);
			}
		}

		void LoadBinary(std::string filename)
		{
			FileStream fileStream(filename.c_str());
			auto lastSlash = filename.find_last_of('/');
			filename.erase(lastSlash);

			short vertexFormat;
			fileStream.Read(&vertexFormat, sizeof(short));

			uint16_t modelCount;
			fileStream.Read(&modelCount, sizeof(uint16_t));

			//フォーマットエラーチェック
			std::vector<bool> formatFlg;
			formatFlg.resize(12);
			int totalByte = 0; //BoneIndex & BoneWeight
			auto formatSizes = VertexFormatSizes();
			for (int i = 0; i < formatSizes.size(); i++)
				if (vertexFormat & formatSizes[i].first)
					totalByte += formatSizes[i].second;
			if (totalByte != sizeof(X))
			{
				std::string errorLog = std::string(typeid(X).name()) + " is " + std::to_string(sizeof(X)) + "\n " +
					"The required size is " + std::to_string(totalByte) + " bytes";
			}

			for (int i = 0; i < modelCount; i++)
			{
				Mesh model;
				//頂点情報読み込み
				uint32_t vertexCount;
				fileStream.Read(&vertexCount, sizeof(uint32_t));
				model.vertexDatas.resize(vertexCount);
				fileStream.Read(&model.vertexDatas[0], sizeof(X) * vertexCount);

				//インデックス読み込み
				uint32_t indexCount;
				fileStream.Read(&indexCount, sizeof(uint32_t));
				model.indexs.resize(indexCount);
				fileStream.Read(&model.indexs[0], sizeof(uint32_t) * indexCount);

				//マテリアルの読み込み
				Material material;
				uint16_t materialNameCount;
				fileStream.Read(&materialNameCount, sizeof(uint16_t));
				material.name.resize(materialNameCount);
				fileStream.Read(&material.name[0], sizeof(char) * materialNameCount);

				uint16_t colorCount;
				fileStream.Read(&colorCount, sizeof(uint16_t));
				for (int i = 0; i < colorCount; i++)
				{
					std::string propertyName;
					uint16_t propertyNameCount;
					fileStream.Read(&propertyNameCount, sizeof(uint16_t));
					propertyName.resize(propertyNameCount);
					fileStream.Read(&propertyName[0], sizeof(char) * propertyNameCount);

					DirectX::XMFLOAT4 color;
					fileStream.Read(&color, sizeof(float) * 4);
					material.AddColor(propertyName, color);
				}

				uint16_t textureCount;
				fileStream.Read(&textureCount, sizeof(uint16_t));
				for (int i = 0; i < textureCount; i++)
				{
					std::string propertyName;
					uint16_t propertyNameCount;
					fileStream.Read(&propertyNameCount, sizeof(uint16_t));
					propertyName.resize(propertyNameCount);
					fileStream.Read(&propertyName[0], sizeof(char) * propertyNameCount);

					std::string textureName;
					uint16_t textureNameCount;
					fileStream.Read(&textureNameCount, sizeof(uint16_t));
					if (textureNameCount == 0) {
						material.AddTexture(propertyName, "null");
						continue;
					}
					textureName.resize(textureNameCount);
					fileStream.Read(&textureName[0], sizeof(char) * textureNameCount);
					material.AddTexture(propertyName, filename + "/" + textureName);
				}

				int materialNo = -1;
				for (int j = 0; j < static_cast<int>(materials.size()); j++)
				{
					if (materials[j] == material.name)
						materialNo = j;
				}
				if (materialNo == -1)
				{
					materialNo = static_cast<int>(materials.size());
					materials.push_back(material);
				}
				model.materialNo = materialNo;
				meshs.push_back(model);
			}
		}
	};

	template <class X>
	struct SkinnedModel
	{
		struct Mesh
		{
			std::vector<X> vertexDatas;
			std::vector<uint32_t> indexs;
			std::vector<std::pair<DirectX::XMMATRIX, Transform*>> bones;
			int materialNo;
		};
		std::vector<Mesh> meshs;
		std::vector<Material> materials;
		std::unique_ptr<Transform> root;
		std::unordered_map<size_t, std::unique_ptr<Transform>> transformMap;

	private:
		void LoadHierarchyAscii(std::ifstream& ifs)
		{
			//モデルの階層構造を読み込み
			auto active = root.get();
			int transformCount = 0;
			{
				std::string tmp;
				ifs >> tmp;
				transformCount++;
				active->name = tmp;
				active->hash = std::hash<std::string>()(tmp);
				ifs >> active->position.x >> active->position.y >> active->position.z;
				DirectX::XMFLOAT3 euler;
				ifs >> euler.x >> euler.y >> euler.z;
				active->rotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(euler.x),
					DirectX::XMConvertToRadians(euler.y), DirectX::XMConvertToRadians(euler.z));
				ifs >> active->scale.x >> active->scale.y >> active->scale.z;
			}
			while (transformCount != 0)
			{
				std::string tmp;
				ifs >> tmp;
				if (tmp == "ChildEndTransform")
				{
					transformCount--;
					active = active->parent;
					continue;
				}
				else
					transformCount++;
				auto newTrans = std::unique_ptr<Transform>(new Transform());
				newTrans->name = tmp;
				newTrans->hash = std::hash<std::string>()(tmp);
				ifs >> newTrans->position.x >> newTrans->position.y >> newTrans->position.z;

				DirectX::XMFLOAT3 euler;
				ifs >> euler.x >> euler.y >> euler.z;
				newTrans->rotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(euler.x),
					DirectX::XMConvertToRadians(euler.y), DirectX::XMConvertToRadians(euler.z));
				ifs >> newTrans->scale.x >> newTrans->scale.y >> newTrans->scale.z;
				newTrans->parent = active;
				active->child.push_back(newTrans.get());
				active = newTrans.get();
				transformMap.insert(std::make_pair(newTrans->hash, std::move(newTrans)));
			}
		}

		void LoadHierarchyBinary(FileStream& fileStream)
		{
			auto active = root.get();
			int transformCount = 0;
			{
				std::string tmp;
				short tmpCount;
				fileStream.Read(&tmpCount, sizeof(short));
				tmp.resize(tmpCount);
				fileStream.Read(&tmp[0], sizeof(char) * tmpCount);
				transformCount++;
				active->name = tmp;
				active->hash = std::hash<std::string>()(tmp);
				fileStream.Read(&active->position, sizeof(float) * 3);
				DirectX::XMFLOAT3 euler;
				fileStream.Read(&euler, sizeof(float) * 3);
				active->rotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(euler.x),
					DirectX::XMConvertToRadians(euler.y), DirectX::XMConvertToRadians(euler.z));
				fileStream.Read(&active->scale, sizeof(float) * 3);
			}
			while (transformCount != 0)
			{
				std::string tmp;
				short tmpCount;
				fileStream.Read(&tmpCount, sizeof(short));
				if (tmpCount == -1)
				{
					transformCount--;
					active = active->parent;
					continue;
				}
				else
					transformCount++;
				tmp.resize(tmpCount);
				fileStream.Read(&tmp[0], sizeof(char) * tmpCount);

				auto newTrans = std::unique_ptr<Transform>(new Transform());
				newTrans->name = tmp;
				newTrans->hash = std::hash<std::string>()(tmp);

				fileStream.Read(&newTrans->position, sizeof(float) * 3);
				DirectX::XMFLOAT3 euler;
				fileStream.Read(&euler, sizeof(float) * 3);
				newTrans->rotation = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(euler.x),
					DirectX::XMConvertToRadians(euler.y), DirectX::XMConvertToRadians(euler.z));
				fileStream.Read(&newTrans->scale, sizeof(float) * 3);
				newTrans->parent = active;
				active->child.push_back(newTrans.get());
				active = newTrans.get();
				transformMap.insert(std::make_pair(newTrans->hash, std::move(newTrans)));
			}
		}
	public:
		void LoadAscii(std::string filename)
		{
			root.reset(new Transform);

			std::ifstream ifs(filename);
			auto lastSlash = filename.find_last_of('/');
			filename.erase(lastSlash);
			assert(ifs.is_open());

			LoadHierarchyAscii(ifs);

			//頂点フォーマットを読み込み
			int vertexFormat;
			ifs >> vertexFormat;

			int modelCount;
			ifs >> modelCount;

			//フォーマットエラーチェック
			std::vector<bool> formatFlg;
			formatFlg.resize(12);
			int totalByte = 32; //BoneIndex & BoneWeight
			auto formatSizes = VertexFormatSizes();
			for (int i = 0; i < formatSizes.size(); i++)
				if (vertexFormat & formatSizes[i].first)
					totalByte += formatSizes[i].second;
			if (totalByte != sizeof(X))
			{
				std::string errorLog = std::string(typeid(X).name()) + " is " + std::to_string(sizeof(X)) + "\n " +
					"The required size is " + std::to_string(totalByte) + " bytes";
			}

			for (int i = 0; i < modelCount; i++)
			{
				Mesh model;
				//頂点情報読み込み
				int vertexCount;
				ifs >> vertexCount;
				for (int j = 0; j < vertexCount; j++)
				{
					uint8_t* rawData = new uint8_t[sizeof(X)];
					int rawCnt = 0;

					for (int i = 0; i < formatSizes.size(); i++)
					{
						if (vertexFormat & formatSizes[i].first)
						{
							float tmpData[4];
							int dataSize = formatSizes[i].second / 4;
							for (int j = 0; j < dataSize; j++)
								ifs >> tmpData[j];
							memcpy(&rawData[rawCnt], tmpData, formatSizes[i].second);
							rawCnt += formatSizes[i].second;
						}
					}
					DirectX::XMINT4 boneIndex;
					DirectX::XMFLOAT4 boneWeight;
					ifs >> boneIndex.x >> boneIndex.y >> boneIndex.z >> boneIndex.w;
					ifs >> boneWeight.x >> boneWeight.y >> boneWeight.z >> boneWeight.w;
					memcpy(&rawData[rawCnt], &boneIndex, 16);
					memcpy(&rawData[rawCnt + 16], &boneWeight, 16);

					X data;
					memcpy(&data, rawData, sizeof(X));
					delete[] rawData;
					model.vertexDatas.push_back(data);
				}

				//インデックス読み込み
				int indexCount;
				ifs >> indexCount;
				model.indexs.resize(indexCount);
				for (int j = 0; j < indexCount; j++)
				{
					ifs >> model.indexs[j];
				}

				//ベースポーズ読み込み
				int basePoseCount;
				ifs >> basePoseCount;
				for (int j = 0; j < basePoseCount; j++)
				{
					std::string name;
					ifs >> name;

					DirectX::XMMATRIX tmp;
					ifs >> tmp.r[0].m128_f32[0] >> tmp.r[0].m128_f32[1] >> tmp.r[0].m128_f32[2] >> tmp.r[0].m128_f32[3]
						>> tmp.r[1].m128_f32[0] >> tmp.r[1].m128_f32[1] >> tmp.r[1].m128_f32[2] >> tmp.r[1].m128_f32[3]
						>> tmp.r[2].m128_f32[0] >> tmp.r[2].m128_f32[1] >> tmp.r[2].m128_f32[2] >> tmp.r[2].m128_f32[3]
						>> tmp.r[3].m128_f32[0] >> tmp.r[3].m128_f32[1] >> tmp.r[3].m128_f32[2] >> tmp.r[3].m128_f32[3];

					auto trans = root->Find(name);
					model.bones.push_back(std::make_pair(XMMatrixTranspose(tmp), trans));
				}

				//マテリアルの読み込み
				Material material;
				ifs >> material.name;
				int colorCount;
				ifs >> colorCount;
				for (int i = 0; i < colorCount; i++)
				{
					std::string propertyName;
					ifs >> propertyName;
					DirectX::XMFLOAT4 color;
					ifs >> color.x >> color.y >> color.z >> color.w;
					material.AddColor(propertyName, color);
				}

				int textureCount;
				ifs >> textureCount;
				for (int i = 0; i < textureCount; i++)
				{
					std::string propertyName;
					ifs >> propertyName;
					std::string textureName;
					ifs >> textureName;
					material.AddTexture(propertyName, filename + "/" + textureName);
				}

				int materialNo = -1;
				for (int j = 0; j < static_cast<int>(materials.size()); j++)
				{
					if (materials[j] == material.name)
						materialNo = j;
				}
				if (materialNo == -1)
				{
					materialNo = static_cast<int>(materials.size());
					materials.push_back(material);
				}
				model.materialNo = materialNo;
				meshs.push_back(model);
			}
		}

		void LoadBinary(std::string filename)
		{
			root.reset(new Transform());

			FileStream fileStream(filename.c_str());
			auto lastSlash = filename.find_last_of('/');
			filename.erase(lastSlash);

			LoadHierarchyBinary(fileStream);

			short vertexFormat;
			fileStream.Read(&vertexFormat, sizeof(short));

			uint16_t modelCount;
			fileStream.Read(&modelCount, sizeof(uint16_t));

			//フォーマットエラーチェック
			std::vector<bool> formatFlg;
			formatFlg.resize(12);
			int totalByte = 32; //BoneIndex & BoneWeight
			auto formatSizes = VertexFormatSizes();
			for (int i = 0; i < formatSizes.size(); i++)
				if (vertexFormat & formatSizes[i].first)
					totalByte += formatSizes[i].second;
			if (totalByte != sizeof(X))
			{
				std::string errorLog = std::string(typeid(X).name()) + " is " + std::to_string(sizeof(X)) + "\n " +
					"The required size is " + std::to_string(totalByte) + " bytes";
			}

			for (int i = 0; i < modelCount; i++)
			{
				Mesh model;
				//頂点情報読み込み
				uint32_t vertexCount;
				fileStream.Read(&vertexCount, sizeof(uint32_t));
				model.vertexDatas.resize(vertexCount);
				fileStream.Read(&model.vertexDatas[0], sizeof(X) * vertexCount);

				//インデックス読み込み
				uint32_t indexCount;
				fileStream.Read(&indexCount, sizeof(uint32_t));
				model.indexs.resize(indexCount);
				fileStream.Read(&model.indexs[0], sizeof(uint32_t) * indexCount);

				//ベースポーズ読み込み
				uint16_t basePoseCount;
				fileStream.Read(&basePoseCount, sizeof(uint16_t));
				for (int j = 0; j < basePoseCount; j++)
				{
					std::string name;
					uint16_t nameCount;
					fileStream.Read(&nameCount, sizeof(uint16_t));
					name.resize(nameCount);
					fileStream.Read(&name[0], sizeof(char) * nameCount);

					DirectX::XMMATRIX tmp;
					fileStream.Read(&tmp, sizeof(float) * 16);

					auto trans = root->Find(name);
					model.bones.push_back(std::make_pair(XMMatrixTranspose(tmp), trans));
				}

				//マテリアルの読み込み
				Material material;
				uint16_t materialNameCount;
				fileStream.Read(&materialNameCount, sizeof(uint16_t));
				material.name.resize(materialNameCount);
				fileStream.Read(&material.name[0], sizeof(char) * materialNameCount);

				uint16_t colorCount;
				fileStream.Read(&colorCount, sizeof(uint16_t));
				for (int i = 0; i < colorCount; i++)
				{
					std::string propertyName;
					uint16_t propertyNameCount;
					fileStream.Read(&propertyNameCount, sizeof(uint16_t));
					propertyName.resize(propertyNameCount);
					fileStream.Read(&propertyName[0], sizeof(char) * propertyNameCount);

					DirectX::XMFLOAT4 color;
					fileStream.Read(&color, sizeof(float) * 4);
					material.AddColor(propertyName, color);
				}

				uint16_t textureCount;
				fileStream.Read(&textureCount, sizeof(uint16_t));
				for (int i = 0; i < textureCount; i++)
				{
					std::string propertyName;
					uint16_t propertyNameCount;
					fileStream.Read(&propertyNameCount, sizeof(uint16_t));
					propertyName.resize(propertyNameCount);
					fileStream.Read(&propertyName[0], sizeof(char) * propertyNameCount);

					std::string textureName;
					uint16_t textureNameCount;
					fileStream.Read(&textureNameCount, sizeof(uint16_t));
					if (textureNameCount == 0) {
						material.AddTexture(propertyName, "null");
						continue;
					}
					textureName.resize(textureNameCount);
					fileStream.Read(&textureName[0], sizeof(char) * textureNameCount);
					material.AddTexture(propertyName, filename + "/" + textureName);
				}

				int materialNo = -1;
				for (int j = 0; j < static_cast<int>(materials.size()); j++)
				{
					if (materials[j] == material.name)
						materialNo = j;
				}
				if (materialNo == -1)
				{
					materialNo = static_cast<int>(materials.size());
					materials.push_back(material);
				}
				model.materialNo = materialNo;
				meshs.push_back(model);
			}
		}
	};

	struct SkinnedAnimation
	{
		struct Curve
		{
			std::vector<float> times;
			std::vector<float> keys;

			float Lerp(float f1, float f2, float t)
			{
				return f1 + (f2 - f1) * t;
			}

			float GetValue(float time)
			{
				if (times.size() == 1)
					return keys[0];
				if (time < times[0])
					return keys[0];
				if (time > times[times.size() - 1])
					return keys[keys.size() - 1];

				int index = 0;
				for (; index < times.size() - 1; index++)
				{
					if (times[index] > time)
						break;
				}
				index--;

				return Lerp(keys[index], keys[index + 1], (time - times[index]) / (times[index + 1] - times[index]));
			}
		};

		struct Animation
		{
			uem::Transform* transform = nullptr;
			Curve curves[10];

			void SetTransform(float time)
			{
				DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(curves[0].GetValue(time), curves[1].GetValue(time), curves[2].GetValue(time));
				DirectX::XMVECTOR rotation = DirectX::XMVectorSet(curves[3].GetValue(time), curves[4].GetValue(time), curves[5].GetValue(time), curves[6].GetValue(time));
				DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(curves[7].GetValue(time), curves[8].GetValue(time), curves[9].GetValue(time));

				transform->position = position;
				transform->rotation = rotation;
				transform->scale = scale;
			}
		};
	private:
		std::vector<Animation> animationList;
		float maxAnimationTime = 0;
	public:
		void LoadAscii(std::string filename, Transform* root)
		{
			std::ifstream ifs(filename);
			auto lastSlash = filename.find_last_of('/');
			filename.erase(lastSlash);
			assert(ifs.is_open());

			int animationCount;
			ifs >> animationCount;
			animationList.resize(animationCount);
			for (int i = 0; i < animationCount; i++)
			{
				auto& anim = animationList[i];
				std::string transformName;
				ifs >> transformName;
				anim.transform = root->Find(transformName);
				for (int j = 0; j < 10; j++)
				{
					int keyCount;
					ifs >> keyCount;
					anim.curves[j].times.resize(keyCount);
					anim.curves[j].keys.resize(keyCount);
					for (int k = 0; k < keyCount; k++) 
					{
						ifs >> anim.curves[j].times[k];
						if (anim.curves[j].times[k] > maxAnimationTime)
							maxAnimationTime = anim.curves[j].times[k];
					}
					for (int k = 0; k < keyCount; k++)
						ifs >> anim.curves[j].keys[k];
				}
			}
		}

		void LoadBinary(std::string filename, Transform* root)
		{
			FileStream fileStream(filename.c_str());

			uint32_t animationCount;
			fileStream.Read(&animationCount, sizeof(uint32_t));

			animationList.resize(animationCount);
			for (uint16_t i = 0; i < animationCount; i++)
			{
				auto& anim = animationList[i];
				std::string transformName;
				uint16_t transformNameCount;
				fileStream.Read(&transformNameCount, sizeof(uint16_t));
				transformName.resize((size_t)transformNameCount);
				fileStream.Read(&transformName[0], sizeof(char) * transformNameCount);
				anim.transform = root->Find(transformName);
				for (int j = 0; j < 10; j++)
				{
					uint32_t keyCount;
					fileStream.Read(&keyCount, sizeof(uint32_t));
					anim.curves[j].times.resize(keyCount);
					anim.curves[j].keys.resize(keyCount);
					fileStream.Read(&anim.curves[j].times[0], sizeof(float) * keyCount);
					for (uint32_t t = 0; t < keyCount; t++)
					{
						if (anim.curves[j].times[t] > maxAnimationTime)
							maxAnimationTime = anim.curves[j].times[t];
					}
					fileStream.Read(&anim.curves[j].keys[0], sizeof(float) * keyCount);
				}
			}
		}

		void SetTransform(float time)
		{
			for (auto& animation : animationList)
				animation.SetTransform(time);
		};

		float GetMaxAnimationTime() const
		{
			return maxAnimationTime;
		}
	};
}