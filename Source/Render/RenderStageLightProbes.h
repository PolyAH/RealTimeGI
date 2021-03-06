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



#pragma once



#include "Core/Core.h"
#include "RenderData/RenderTypes.h"
#include "glm/vec4.hpp"


#include <vector>



class RenderShader;
class RenderScene;
class RenderUniform;
class RenderLightProbe;
class RenderIrradianceVolume;


class VKIDevice;
class VKIImage;
class VKIFramebuffer;
class VKIRenderPass;
class VKIImageView;
class VKISampler;
class VKICommandBuffer;







// RenderStageLightProbes:
//    - a stage that is part of the pipeline for rendering and updating light probes.
//
class RenderStageLightProbes
{
public:
	// Construct.
	RenderStageLightProbes();

	// Destruct.
	~RenderStageLightProbes();

	// Initialize The Pipeline.
	void Initialize(VKIDevice* device, StageRenderTarget hdrTargets[2], StageRenderTarget* dephtTarget, RenderUniform* commonUniform);

	// Destroy The Pipeline.
	void Destroy();

	// Render light rrobe into the scene.
	void Render(VKICommandBuffer* cmdBuffer, uint32_t frame, const std::vector<RenderLightProbe*>& lightProbes);
	void Render(VKICommandBuffer* cmdBuffer, uint32_t frame, const std::vector<RenderIrradianceVolume*>& volumes);

	// Update the light probe by capturing.
	void RenderCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderLightProbe* lightProbe, uint32_t face, const glm::ivec4& viewport);
	void RenderCaptureCube(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderIrradianceVolume* volume, uint32_t layer, const glm::ivec4& viewport);

	// Pre-Filter Capture Cube map and store the result in lightProbe.
	void FilterIrradiance(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderLightProbe* lightProbe, const glm::ivec4& viewport);
	void FilterIrradianceVolume(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderIrradianceVolume* volume, uint32_t probe, const glm::ivec4& viewport);

	// Return the capture cube image.
	inline VKIImage* GetCaptureCube() { return mCaptureCubeTarget.image.get(); }

	// Return Irradiance Filter Render Pass.
	inline VKIRenderPass* GetIrradianceFilterPass() { return mIrradianceFilterPass.get(); }
	inline RenderShader* GetIrradianceFilterShader() { return mIrradianceFilter.get(); }
	inline RenderShader* GetIrradianceArrayFilterShader() { return mIrradianceArrayFilter.get(); }

	// Return the lighting shader used to render light probe.
	inline RenderShader* GetLightingShader() { return mLightingShader.get(); }
	inline RenderShader* GetLightingVolumeShader() { return mLightingVolumeShader.get(); }

	// 
	inline RenderShader* GetVisualizeShader() { return mVisualizeProbeShader.get(); }

	//
	void RenderVisualize(VKICommandBuffer* cmdBuffer, uint32_t frame, RenderLightProbe* lightProbe);

private:
	// Setup capture cube map pass for capturing the scene into a HDR cube map.
	void SetupCaptureCubePass();

	// Setup irradiance filter for filtering the the captured HDR cube map.
	void SetupIrradianceFilter();

	//  Setup the lighting pass.
	void SetupLightingPass();

	//
	void SetupVisualizePass();

private:
	// The Vulkan Device.
	VKIDevice* mDevice;

	// The Pipeline Common Unifrom.
	RenderUniform* mCommon;

	// The Pipeline HDR Target.
	StageRenderTarget* mHDRTarget[2];
	StageRenderTarget* mDepth;

	// The Render Sphere.
	class RenderSphere* mSphere;

	// Caputre Cube.
	StageRenderTarget mCaptureCubeTarget;
	UniquePtr<VKIRenderPass> mCaptureCubeRenderPass;
	UniquePtr<VKIFramebuffer> mCaptureCubeFB;
	UniquePtr<RenderShader> mCaptureCubeShader;

	// Irradiance Pass.
	UniquePtr<VKIRenderPass> mIrradianceFilterPass;
	UniquePtr<RenderShader> mIrradianceFilter;
	UniquePtr<RenderShader> mIrradianceArrayFilter;

	// Lighting Stage Pass.
	UniquePtr<RenderShader> mLightingShader;
	UniquePtr<RenderShader> mLightingVolumeShader;

	//
	UniquePtr<RenderShader> mVisualizeProbeShader;
};



