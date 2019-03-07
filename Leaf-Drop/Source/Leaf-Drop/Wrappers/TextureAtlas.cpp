#include "CorePCH.h"
#include "TextureAtlas.h"
#include "../Objects/Texture.h"


TextureAtlas::TextureAtlas()
{
	m_coreRender = CoreRender::GetInstance();

}


TextureAtlas::~TextureAtlas()
{
}

TextureAtlas * TextureAtlas::GetInstance()
{
	static TextureAtlas textureAtlas;
	return &textureAtlas;
}


void TextureAtlas::CopyBindless(Texture* texture)
{
	if (m_nrOfTextures == 0)
		m_bindlessStartHandle = { m_coreRender->GetGPUDescriptorHeap()->GetGPUDescriptorHandleForHeapStart().ptr + m_coreRender->CopyToGPUDescriptorHeap(texture->GetCpuHandle(), texture->GetResource()->GetDesc().DepthOrArraySize) };
	else
		m_coreRender->CopyToGPUDescriptorHeap(texture->GetCpuHandle(), texture->GetResource()->GetDesc().DepthOrArraySize);
	m_nrOfTextures++;
}

void TextureAtlas::BindlessGraphicsSetGraphicsRootDescriptorTable(const UINT& rootParameterIndex,
	ID3D12GraphicsCommandList* commandList) const
{
	commandList->SetGraphicsRootDescriptorTable(rootParameterIndex, m_bindlessStartHandle);
}

void TextureAtlas::BindlessComputeSetGraphicsRootDescriptorTable(const UINT& rootParameterIndex,
	ID3D12GraphicsCommandList* commandList) const
{
	commandList->SetComputeRootDescriptorTable(rootParameterIndex, m_bindlessStartHandle);
}

void TextureAtlas::Reset()
{
	m_nrOfTextures = 0;
}

void TextureAtlas::Release()
{
}
