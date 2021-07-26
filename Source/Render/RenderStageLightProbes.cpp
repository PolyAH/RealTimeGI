// Copyright (c) 2021 Ammar Herzallah
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.




#include "RenderStageLightProbes.h"
#include "Application.h"
#include "Renderer.h"
#include "Render/RendererPipeline.h"
#include "RenderData/RenderScene.h"
#include "RenderData/RenderShadow.h"
#include "RenderData/RenderLight.h"
#include "RenderData/Primitives/RenderSphere.h"
#include "RenderData/Shaders/RenderShader.h"
#include "RenderData/Shaders/RenderUniform.h"


#include "VKInterface/VKIDevice.h"
#include "VKInterface/VKIImage.h"
#include "VKInterface/VKIRenderPass.h"
#include "VKInterface/VKIFramebuffer.h"
#include "VKInterface/VKIDescriptor.h"
#include "VKInterface/VKICommandBuffer.h"
#include "VKInterface/VKISwapChain.h"
#include "VKInterface/VKIGraphicsPipeline.h"





	// Construct.
RenderStageLightProbes::RenderStageLightProbes()
{

}


RenderStageLightProbes::~RenderStageLightProbes()
{

}


void RenderStageLightProbes::Initialize(VKIDevice* device, StageRenderTarget hdrTargets[2], RenderUniform* commonUniform)
{
	mDevice = device;
	mCommon = commonUniform;
	mHDRTarget[0] = &hdrTargets[0];
	mHDRTarget[1] = &hdrTargets[1];

	// The Render Sphere.
	mSphere = Application::Get().GetRenderer()->GetSphere();


	SetupCaptureCubePass();
	SetupIrradianceFilter();
}


void RenderStageLightProbes::Destroy()
{

}


void RenderStageLightProbes::RenderCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame, uint32_t face,
	const glm::ivec4& viewport)
{
	mCaptureCubeRenderPass->Begin(cmdBuffer, mCaptureCubeFB.get(), viewport);
	mCaptureCubeShader->Bind(cmdBuffer);
	mCaptureCubeShader->GetDescriptorSet()->Bind(cmdBuffer, frame, mCaptureCubeShader->GetPipeline());

	uint32_t layer = face;

	vkCmdPushConstants(cmdBuffer->GetCurrent(),
		mCaptureCubeShader->GetPipeline()->GetLayout(),
		VK_SHADER_STAGE_GEOMETRY_BIT,
		0, sizeof(uint32_t), &layer);

	vkCmdDraw(cmdBuffer->GetCurrent(), 3, 1, 0, 0);
	mCaptureCubeRenderPass->End(cmdBuffer);
}


void RenderStageLightProbes::FilterCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame, 
	RenderLightProbe* lightProbe, const glm::ivec4& viewport)
{
	mIrradianceFilterPass->Begin(cmdBuffer, lightProbe->GetIrradianceFB(), viewport);
	mIrradianceFilter->Bind(cmdBuffer);
	mIrradianceFilter->GetDescriptorSet()->Bind(cmdBuffer, frame, mIrradianceFilter->GetPipeline());
	mSphere->Draw(cmdBuffer);
	mIrradianceFilterPass->End(cmdBuffer);
}


void RenderStageLightProbes::SetupCaptureCubePass()
{
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };

	// Cube Image...
	mCaptureCubeTarget.image = UniquePtr<VKIImage>(new VKIImage());
	mCaptureCubeTarget.image->SetUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	mCaptureCubeTarget.image->SetImageInfo(VK_IMAGE_TYPE_2D, VK_FORMAT_R16G16B16A16_SFLOAT, size,
		VK_IMAGE_LAYOUT_UNDEFINED);
	mCaptureCubeTarget.image->SetLayers(6, true);
	mCaptureCubeTarget.image->Create(mDevice);

	mCaptureCubeTarget.view = UniquePtr<VKIImageView>(new VKIImageView());
	mCaptureCubeTarget.view->SetType(VK_IMAGE_VIEW_TYPE_CUBE);
	mCaptureCubeTarget.view->SetViewInfo(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6);
	mCaptureCubeTarget.view->Create(mDevice, mCaptureCubeTarget.image.get());

	mCaptureCubeTarget.sampler = UniquePtr<VKISampler>(new VKISampler());
	mCaptureCubeTarget.sampler->SetFilter(VK_FILTER_LINEAR, VK_FILTER_LINEAR);
	mCaptureCubeTarget.sampler->SetAddressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
	mCaptureCubeTarget.sampler->CreateSampler(mDevice);


	// RenderPass...
	mCaptureCubeRenderPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());
	mCaptureCubeRenderPass->SetColorAttachment(0, mCaptureCubeTarget.image->GetFormat(),
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_LOAD, true);


	mCaptureCubeRenderPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	mCaptureCubeRenderPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT);

	mCaptureCubeRenderPass->CreateRenderPass(mDevice);


	// Framebuffer...
	mCaptureCubeFB = UniquePtr<VKIFramebuffer>(new VKIFramebuffer());
	mCaptureCubeFB->SetSize(size);
	mCaptureCubeFB->SetLayers(6);
	mCaptureCubeFB->SetImgView(0, mCaptureCubeTarget.view.get());
	mCaptureCubeFB->CreateFrameBuffer(mDevice, mCaptureCubeRenderPass.get());


	// Shader...
	mCaptureCubeShader = UniquePtr<RenderShader>(new RenderShader());
	mCaptureCubeShader->SetDomain(ERenderShaderDomain::Screen);
	mCaptureCubeShader->SetRenderPass(mCaptureCubeRenderPass.get());
	mCaptureCubeShader->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "ScreenVert.spv");
	mCaptureCubeShader->SetShader(ERenderShaderStage::Geometry, SHADERS_DIRECTORY "CubeCaptureGeom.spv");
	mCaptureCubeShader->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "CubeCaptureFrag.spv");
	mCaptureCubeShader->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mCaptureCubeShader->SetViewportDynamic(true);
	mCaptureCubeShader->SetBlendingEnabled(0, false);

	mCaptureCubeShader->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
		ERenderShaderStage::AllStages);

	mCaptureCubeShader->AddInput(1, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mCaptureCubeShader->AddPushConstant(0, 0, sizeof(int32_t), ERenderShaderStage::Geometry);

	mCaptureCubeShader->Create();

	// DescriptorSet...
	VKIDescriptorSet* descriptorSet = mCaptureCubeShader->CreateDescriptorSet();
	descriptorSet->SetLayout(mCaptureCubeShader->GetLayout());
	descriptorSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	descriptorSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_ALL, mCommon->GetBuffers());

	descriptorSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mHDRTarget[0]->view.get(), mHDRTarget[0]->sampler.get());

	descriptorSet->UpdateSets();
}


void RenderStageLightProbes::SetupIrradianceFilter()
{
	VkExtent2D size = { LIGHT_PROBES_TARGET_SIZE, LIGHT_PROBES_TARGET_SIZE };


	// RenderPass...
	mIrradianceFilterPass = UniquePtr<VKIRenderPass>(new VKIRenderPass());
	mIrradianceFilterPass->SetColorAttachment(0, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE, true);


	mIrradianceFilterPass->AddDependency(VK_SUBPASS_EXTERNAL, 0,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);

	mIrradianceFilterPass->AddDependency(0, VK_SUBPASS_EXTERNAL,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT);

	mIrradianceFilterPass->CreateRenderPass(mDevice);


	// Shader...
	mIrradianceFilter = UniquePtr<RenderShader>(new RenderShader());
	mIrradianceFilter->SetDomain(ERenderShaderDomain::Mesh);
	mIrradianceFilter->SetRenderPass(mIrradianceFilterPass.get());
	mIrradianceFilter->SetShader(ERenderShaderStage::Vertex, SHADERS_DIRECTORY "SphereVert.spv");
	mIrradianceFilter->SetShader(ERenderShaderStage::Geometry, SHADERS_DIRECTORY "SphereGeom.spv");
	mIrradianceFilter->SetShader(ERenderShaderStage::Fragment, SHADERS_DIRECTORY "IBLFilter_Irradiance.spv");
	mIrradianceFilter->SetViewport(glm::ivec4(0, 0, size.width, size.height));
	mIrradianceFilter->SetViewportDynamic(false);
	mIrradianceFilter->SetBlendingEnabled(0, false);

	mIrradianceFilter->AddInput(RenderShader::COMMON_BLOCK_BINDING, ERenderShaderInputType::Uniform,
		ERenderShaderStage::AllStages);

	mIrradianceFilter->AddInput(1, ERenderShaderInputType::Uniform,
		ERenderShaderStage::Geometry);

	mIrradianceFilter->AddInput(2, ERenderShaderInputType::ImageSampler,
		ERenderShaderStage::Fragment);

	mIrradianceFilter->Create();

	// Descriptor Set
	VKIDescriptorSet* descSet = mIrradianceFilter->CreateDescriptorSet();
	descSet->SetLayout(mIrradianceFilter->GetLayout());
	descSet->CreateDescriptorSet(mDevice, Renderer::NUM_CONCURRENT_FRAMES);

	descSet->AddDescriptor(RenderShader::COMMON_BLOCK_BINDING, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		VK_SHADER_STAGE_ALL, mCommon->GetBuffers());

	descSet->AddDescriptor(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT,
		mSphere->GetSphereUnifrom()->GetBuffers());

	descSet->AddDescriptor(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
		mCaptureCubeTarget.view.get(), mCaptureCubeTarget.sampler.get());

	descSet->UpdateSets();

}
