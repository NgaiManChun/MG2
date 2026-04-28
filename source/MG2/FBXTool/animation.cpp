// =======================================================
// アニメーションコンバータ
// -------------------------------------------------------
// 外部ファイル（FBX等）からアニメーションデータを読み込み、
// MG2専用フォーマット（.mga）へ変換する
//
// ■ 出力構造
// [MAGIC]
// [VERSION]
// [META]
// [position keys]
// [scale keys]
// [rotation keys]
// [channels]
// [node names]
// =======================================================
#include "animation.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

#include "dataType.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix4x4.h"

#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace MG;

void ReadAnimation(const char* fileName, const char* outputName)
{
	// Assimpでファイル読み込み
	const aiScene* scene = aiImportFile(
		fileName,
		aiProcessPreset_TargetRealtime_MaxQuality |
		aiProcess_ConvertToLeftHanded
	);
	assert(scene);

	// =======================================================
	// データ格納用
	// =======================================================
	ANIMATION_META animationMeta{};

	std::vector<ANIMATION_KEY_VECTOR> positionKeyArray;
	std::vector<ANIMATION_KEY_VECTOR> scaleKeyArray;
	std::vector<ANIMATION_KEY_QUATERNION> rotationKeyArray;
	std::vector<ANIMATION_CHANNEL> channelArray;
	std::vector<char> nodeNameArray;

	// 各配列のオフセット計算用
	size_t positionKeyOffset = 0;
	size_t scalingKeyOffset = 0;
	size_t rotationKeyOffset = 0;
	size_t nameLength = 0;

	// =======================================================
	// アニメーション読み込み
	// =======================================================
	if (scene->mNumAnimations > 0) {
		aiAnimation* aianimation = scene->mAnimations[0];

		// フレームレートと総フレーム数
		animationMeta.frameRate = static_cast<unsigned int>(aianimation->mTicksPerSecond);
		animationMeta.frames = static_cast<unsigned int>(aianimation->mDuration + 1);

		// =======================================================
		// チャンネル情報構築（ノードごと）
		// =======================================================
		channelArray.resize(aianimation->mNumChannels);

		for (unsigned int a = 0; a < aianimation->mNumChannels; a++) {
			aiNodeAnim* ainodeanim = aianimation->mChannels[a];

			// ノード名の長さを合計（後でまとめて確保）
			nameLength += strlen(ainodeanim->mNodeName.C_Str()) + 1;

			ANIMATION_CHANNEL& channel = channelArray[a];

			// 各キー配列内でのオフセット
			channel.positionKeyOffset = static_cast<unsigned int>(positionKeyOffset);
			channel.scalingKeyOffset = static_cast<unsigned int>(scalingKeyOffset);
			channel.rotationKeyOffset = static_cast<unsigned int>(rotationKeyOffset);

			channel.positionKeyCount = ainodeanim->mNumPositionKeys;
			channel.scalingKeyCount = ainodeanim->mNumScalingKeys;
			channel.rotationKeyCount = ainodeanim->mNumRotationKeys;

			// 次のチャンネルのためにオフセットを進める
			positionKeyOffset += channel.positionKeyCount;
			scalingKeyOffset += channel.scalingKeyCount;
			rotationKeyOffset += channel.rotationKeyCount;
		}

		// =======================================================
		// メモリ確保
		// =======================================================
		nodeNameArray.resize(nameLength);

		positionKeyArray.reserve(positionKeyOffset);
		scaleKeyArray.reserve(scalingKeyOffset);
		rotationKeyArray.reserve(rotationKeyOffset);

		unsigned int nameOffset = 0;

		// =======================================================
		// 実データ格納
		// =======================================================
		for (unsigned int a = 0; a < aianimation->mNumChannels; a++) {
			aiNodeAnim* ainodeanim = aianimation->mChannels[a];

			// -----------------------
			// ノード名（連続メモリ）
			// -----------------------
			std::string name = ainodeanim->mNodeName.C_Str();
			memcpy(nodeNameArray.data() + nameOffset, name.data(), name.size());
			nameOffset += static_cast<unsigned int>(name.size());
			nodeNameArray[nameOffset++] = '\0';

			// -----------------------
			// 位置キー
			// -----------------------
			for (unsigned int k = 0; k < ainodeanim->mNumPositionKeys; k++) {
				aiVector3D& vector = ainodeanim->mPositionKeys[k].mValue;

				positionKeyArray.push_back({
					static_cast<unsigned int>(ainodeanim->mPositionKeys[k].mTime),
					{ vector.x, vector.y, vector.z }
					});
			}

			// -----------------------
			// スケールキー
			// -----------------------
			for (unsigned int k = 0; k < ainodeanim->mNumScalingKeys; k++) {
				aiVector3D& vector = ainodeanim->mScalingKeys[k].mValue;

				scaleKeyArray.push_back({
					static_cast<unsigned int>(ainodeanim->mScalingKeys[k].mTime),
					{ vector.x, vector.y, vector.z }
					});
			}

			// -----------------------
			// 回転キー（クォータニオン）
			// -----------------------
			for (unsigned int k = 0; k < ainodeanim->mNumRotationKeys; k++) {
				aiQuaternion& quaternion = ainodeanim->mRotationKeys[k].mValue;

				rotationKeyArray.push_back({
					static_cast<unsigned int>(ainodeanim->mRotationKeys[k].mTime),
					{ quaternion.x, quaternion.y, quaternion.z, quaternion.w }
					});
			}
		}

		// =======================================================
		// メタ情報構築
		// =======================================================
		animationMeta.positionKeyCount = static_cast<unsigned int>(positionKeyArray.size());
		animationMeta.scalingKeyCount = static_cast<unsigned int>(scaleKeyArray.size());
		animationMeta.rotationKeyCount = static_cast<unsigned int>(rotationKeyArray.size());
		animationMeta.channelCount = static_cast<unsigned int>(channelArray.size());
		animationMeta.channelNameLength = sizeof(char) * nodeNameArray.size();

		// 総データサイズ
		animationMeta.dataSize =
			sizeof(ANIMATION_KEY_VECTOR) * animationMeta.positionKeyCount +
			sizeof(ANIMATION_KEY_VECTOR) * animationMeta.scalingKeyCount +
			sizeof(ANIMATION_KEY_QUATERNION) * animationMeta.rotationKeyCount +
			sizeof(ANIMATION_CHANNEL) * animationMeta.channelCount +
			sizeof(char) * nodeNameArray.size();

		// =======================================================
		// 出力ファイル名決定
		// =======================================================
		std::string ouputFileName;
		if (outputName) {
			ouputFileName = outputName;
		}
		else {
			ouputFileName = fileName;
			ouputFileName += ".mga";
		}

		// =======================================================
		// バイナリ書き出し
		// =======================================================
		std::ofstream file(ouputFileName, std::ios::binary);

		file.write((const char*)ANIMATION_MAGIC, sizeof(char) * 4);
		file.write((const char*)ANIMATION_VERSION, sizeof(char) * 8);
		file.write((const char*)&animationMeta, sizeof(ANIMATION_META);

		file.write((const char*)positionKeyArray.data(), sizeof(ANIMATION_KEY_VECTOR) * positionKeyArray.size());
		file.write((const char*)scaleKeyArray.data(), sizeof(ANIMATION_KEY_VECTOR) * scaleKeyArray.size());
		file.write((const char*)rotationKeyArray.data(), sizeof(ANIMATION_KEY_QUATERNION) * rotationKeyArray.size());
		file.write((const char*)channelArray.data(), sizeof(ANIMATION_CHANNEL) * channelArray.size());
		file.write((const char*)nodeNameArray.data(), sizeof(char) * nodeNameArray.size());

		file.close();
	}

	// Assimp解放
	aiReleaseImport(scene);
}