#include "modelAnimation.h"
#include "renderer.h"
#include "MGUtility.h"
#include "model.h"
#include "animation.h"

namespace MG {


	void ModelAnimation::Uninit()
	{
		SAFE_RELEASE(s_SRV);
		SAFE_RELEASE(s_Buffer);
		s_Data.clear();
		s_EmptyIds.clear();
	}

	void ModelAnimation::Update()
	{
		if (s_NeedUpdateBuffer) {
			if (s_Data.capacity() > s_Capcity) {
				SAFE_RELEASE(s_SRV);
				SAFE_RELEASE(s_Buffer);
				s_Buffer = Renderer::CreateStructuredBuffer(sizeof(DATA), s_Data.capacity(), s_Data.data());
				if (s_Buffer) {
					s_SRV = Renderer::CreateStructuredSRV(s_Buffer, s_Data.capacity());
					s_Capcity = s_Data.capacity();
					s_NeedUpdateBuffer = false;
				}
			}
		}
		if (s_NeedUpdateBuffer && s_SRV) {
			D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(DATA) * s_Data.size());
			Renderer::GetDeviceContext()->UpdateSubresource(s_Buffer, 0, &box, s_Data.data(), 0, 0);
			s_NeedUpdateBuffer = false;
		}
	}

	ModelAnimation ModelAnimation::Create(unsigned int modelId, unsigned int animationId, bool loop)
	{
		Model model = modelId;
		Animation animation = animationId;
		ModelAnimation key{};
		DATA data{};
		data.modelId = model;
		data.animationId = animation;
		data.nodeCount = model.GetNodeCount();
		data.frameCount = animation.GetFrameCount();
		data.duration = animation.GetDuration();
		data.loop = loop;

		std::vector<TRANSFORM> transforms(data.nodeCount * data.frameCount);
		auto& animationData = animation.GetData();
		auto& channels = animationData.channels;
		auto& namedNodeIndexes = model.GetData().namedNodeIndexes;

		for (unsigned int frame = 0; frame < data.frameCount; frame++) {
			unsigned int frameOffset = frame * data.nodeCount;
			for (auto& channel : channels) {
				if (namedNodeIndexes.count(channel.first)) {
					unsigned int nodeId = namedNodeIndexes.at(channel.first);
					transforms[frameOffset + nodeId] = channel.second[frame];
				}
			}
		}
		data.transformDivision = TransformDivision::Create(transforms.size(), transforms.data());

		if (s_EmptyIds.empty()) {
			s_Data.push_back(data);
			key.m_Id = s_Data.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Data[key.m_Id] = data;
		}

		s_NeedUpdateBuffer = true;

		return key;
	}

} // namespace MG
