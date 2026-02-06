#pragma once
#include <vector>
#include "dataType.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11ComputeShader;

#define BUFFER_HANDLE_OPERATOR(TYPE)							   \
operator bool() const { return m_Id != UINT_MAX; }				   \
bool operator ==(TYPE& other) const { return m_Id == other.m_Id; } \
bool operator !=(TYPE& other) const { return m_Id == other.m_Id; } \
operator unsigned int() const { return m_Id; }

namespace MG::Buffer {

	bool NewVertexBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer);
	bool NewBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& srv);
	bool NewBufferCopy(unsigned int stride, unsigned int capcity, ID3D11Buffer*& buffer, ID3D11ShaderResourceView*& srv, ID3D11UnorderedAccessView*& uav);
	
	// コンピュートシェーダでパディング
	// 戻り値：新しいデータ数
	unsigned int DivisionPadByCS(unsigned int stride, unsigned int dataCount,
		std::vector<BOOKMARK>& bookmarks,
		ID3D11Buffer* dataBuffer,
		ID3D11Buffer* returnBookmarkBuffer = nullptr
	);

	
	/*unsigned int DivisionPadByCS(unsigned int stride, unsigned int dataCount,
		std::vector<BOOKMARK>& bookmarks,
		ID3D11Buffer* bookmarkBuffer,
		ID3D11ShaderResourceView* bookmarkSRV,
		ID3D11Buffer* dataBuffer,
		ID3D11ShaderResourceView* dataSRV,
		ID3D11ComputeShader* padCS
	);*/

} // namespace MG::Buffer