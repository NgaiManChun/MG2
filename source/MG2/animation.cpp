#include "animation.h"
#include "renderer.h"
#include "MGUtility.h"
#include <fstream>

namespace MG {

	Animation Animation::Create(const char* filename)
	{
		Animation key{};

		// すでにロードしたかをチェック
		if (s_NameMap.count(filename) > 0) {
			key.m_Id = s_NameMap.at(filename);
			return key;
		}

		std::ifstream file(filename, std::ios::binary);

		// 頭文字とバージョンをチェック
		char prefix[4];
		file.read(prefix, 4);
		if (strcmp(prefix, ANIMATION_MAGIC)) {
			file.close();
			return {};
		}
		char version[8];
		file.read(version, 8);

		// データ読み込み
		ANIMATION_META animationMeta{};
		file.read((char*)&animationMeta, sizeof(ANIMATION_META));
		std::vector<ANIMATION_KEY_VECTOR> positionKeyArray(animationMeta.positionKeyCount);
		std::vector<ANIMATION_KEY_VECTOR> scaleKeyArray(animationMeta.scalingKeyCount);
		std::vector<ANIMATION_KEY_QUATERNION> rotationKeyArray(animationMeta.rotationKeyCount);
		std::vector<ANIMATION_CHANNEL> channelArray(animationMeta.channelCount);
		std::vector<char> nameArray(animationMeta.channelNameLength);
		file.read((char*)positionKeyArray.data(), sizeof(ANIMATION_KEY_VECTOR) * animationMeta.positionKeyCount);
		file.read((char*)scaleKeyArray.data(), sizeof(ANIMATION_KEY_VECTOR) * animationMeta.scalingKeyCount);
		file.read((char*)rotationKeyArray.data(), sizeof(ANIMATION_KEY_QUATERNION) * animationMeta.rotationKeyCount);
		file.read((char*)channelArray.data(), sizeof(ANIMATION_CHANNEL) * animationMeta.channelCount);
		file.read(nameArray.data(), sizeof(char) * animationMeta.channelNameLength);
		file.close();

		// ID確保
		DATA data{};
		data.frameCount = animationMeta.frames;
		data.duration = static_cast<float>(animationMeta.frames - 1) / animationMeta.frameRate * 1000;
		if (s_EmptyIds.empty()) {
			s_Data.push_back(data);
			key.m_Id = s_Data.size() - 1;
		}
		else {
			auto it = std::next(s_EmptyIds.begin(), 1);
			key.m_Id = *it;
			s_EmptyIds.erase(it);
			s_Data[key.m_Id] = data;
		}
		s_NameMap[filename] = key.m_Id;
		
		// チャンネル×フレームにTransformを展開
		auto& channels = s_Data[key.m_Id].channels;
		size_t nameOffset = 0;
		for (unsigned int i = 0; i < animationMeta.channelCount; i++) {
			std::string name = nameArray.data() + nameOffset;
			nameOffset += name.size() + 1;
			ANIMATION_CHANNEL& channel = channelArray[i];
			std::vector<TRANSFORM>& transforms = channels[name];
			transforms = std::vector<TRANSFORM>(animationMeta.frames);

			// position
			{
				unsigned int startKey = channel.positionKeyOffset;
				unsigned int endKey = channel.positionKeyOffset + channel.positionKeyCount;
				for (unsigned int p = startKey; p < endKey; p++) {
					if (p + 1 < endKey) {
						unsigned int startFrame = positionKeyArray[p].frame;
						unsigned int endFrame = positionKeyArray[p + 1].frame;
						XMVECTOR startValue = positionKeyArray[p].value;
						XMVECTOR endValue = positionKeyArray[p + 1].value;
						float invDivision = 1.0f / (endFrame - startFrame);
						for (unsigned int f = startFrame; f < endFrame; f++) {
							transforms[f].position = XMVectorLerp(startValue, endValue, static_cast<float>(f - startFrame) * invDivision);
						}
					}
					else {
						for (unsigned int f = positionKeyArray[p].frame; f < animationMeta.frames; f++) {
							transforms[f].position = positionKeyArray[p].value;
						}
					}
				}
			}

			// scale
			{
				unsigned int startKey = channel.scalingKeyOffset;
				unsigned int endKey = channel.scalingKeyOffset + channel.scalingKeyCount;
				for (unsigned int p = startKey; p < endKey; p++) {
					if (p + 1 < endKey) {
						unsigned int startFrame = scaleKeyArray[p].frame;
						unsigned int endFrame = scaleKeyArray[p + 1].frame;
						XMVECTOR startValue = scaleKeyArray[p].value;
						XMVECTOR endValue = scaleKeyArray[p + 1].value;
						float invDivision = 1.0f / (endFrame - startFrame);
						for (unsigned int f = startFrame; f < endFrame; f++) {
							transforms[f].scale = XMVectorLerp(startValue, endValue, static_cast<float>(f - startFrame) * invDivision);
						}
					}
					else {
						for (unsigned int f = scaleKeyArray[p].frame; f < animationMeta.frames; f++) {
							transforms[f].scale = scaleKeyArray[p].value;
						}
					}
				}
			}

			// rotation
			{
				unsigned int startKey = channel.rotationKeyOffset;
				unsigned int endKey = channel.rotationKeyOffset + channel.rotationKeyCount;
				for (unsigned int p = startKey; p < endKey; p++) {
					if (p + 1 < endKey) {
						unsigned int startFrame = rotationKeyArray[p].frame;
						unsigned int endFrame = rotationKeyArray[p + 1].frame;
						XMVECTOR startValue = rotationKeyArray[p].value;
						XMVECTOR endValue = rotationKeyArray[p + 1].value;
						float invDivision = 1.0f / (endFrame - startFrame);
						for (unsigned int f = startFrame; f < endFrame; f++) {
							transforms[f].rotation = XMQuaternionSlerp(startValue, endValue, static_cast<float>(f - startFrame) * invDivision);
						}
					}
					else {
						for (unsigned int f = rotationKeyArray[p].frame; f < animationMeta.frames; f++) {
							transforms[f].rotation = rotationKeyArray[p].value;
						}
					}
				}
			}
		}

		return key;
	}

	
} // namespace MG
