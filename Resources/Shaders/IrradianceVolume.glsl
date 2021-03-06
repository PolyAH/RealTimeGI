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









// Volume Data for Irradiance Volume
struct IrradianceVolumeData
{
	// The start of the volume.
	vec3 Start;

	// The Extent of the volume.
	vec3 Extent;

	// The number of probes in each axis of the volume.
	ivec3 Count;

	//
	vec3 GridSize;

	//
	float GridLen;
};




ivec3 GetGridCoord(in vec3 Pos, in IrradianceVolumeData IrVolume)
{
	return ivec3((Pos - IrVolume.Start + IrVolume.GridSize) / IrVolume.GridSize) - 1;
}



int GetProbeIndex(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return GridCoord.x + GridCoord.y * IrVolume.Count.x + GridCoord.z * (IrVolume.Count.x * IrVolume.Count.y);
}


vec3 GetProbePos(ivec3 GridCoord, in IrradianceVolumeData IrVolume)
{
	return vec3(GridCoord) * IrVolume.GridSize + IrVolume.GridSize * 0.5 + IrVolume.Start;
}



float SampleIrOcclusion(vec3 v, float ld, float layer, in samplerCubeArray Radiance) 
{ 
	float bias = 0.016; 
	
	float offset = 2.5;
	float numSamples = 3.0;
	float ShadowValue = 0.0;
	
	for(float x = -offset; x < offset; x += offset / (numSamples * 0.5)) 
	{ 
		for(float y = -offset; y < offset; y += offset / (numSamples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (numSamples * 0.5)) 
			{
				float s_depth = texture(Radiance, vec4(v + vec3(x, y, z), layer)).a; 
				ShadowValue += s_depth > (ld - bias) ? 1.0 : 0.0;
			} 
		} 
	}

	ShadowValue /= 27.0;

	return 0.0;
}




//
//
float SampleIrOcclusion(vec3 v, float ld, int layer, in samplerCubeArray Radiance) 
{ 
	float bias = 0.005; 
	
	float offset = 2.5;
	float numSamples = 3.0;
	float M1 = 0.0;
	float M2 = 0.0;
	
	for(float x = -offset; x < offset; x += offset / (numSamples * 0.5)) 
	{ 
		for(float y = -offset; y < offset; y += offset / (numSamples * 0.5))
		{
			for(float z = -offset; z < offset; z += offset / (numSamples * 0.5)) 
			{
				float s_depth = texture(Radiance, vec4(v + vec3(x, y, z), layer)).a; 

				M1 += s_depth;
				M2 += s_depth * s_depth;
			} 
		} 
	}

	M1 /= 27.0;
	M2 /= 27.0;


	float Ver = M2 - M1 * M1;
	ld -= bias;
	float g = ld - M1;

	return  g > 0.0 ? (Ver / (Ver + g * g)) : 1.0;
}


vec4 SampleIrVolumeLayer(ivec3 GridCoord, in SurfaceData Surface, 
	in IrradianceVolumeData IrVolume, in samplerCubeArray Irradiance, in samplerCubeArray Radiance)
{
	GridCoord = clamp(GridCoord, ivec3(0), IrVolume.Count - 1);
	vec3 Probe0Pos = GetProbePos(GridCoord, IrVolume);
	int Probe0Index = GetProbeIndex(GridCoord, IrVolume);
	float SampleRadius = IrVolume.GridLen * 2.0;
	vec3 Sample0 = LightProbeSampleRay(Probe0Pos, SampleRadius, Surface.P, Surface.N);
	vec4 Irradiance0 = texture(Irradiance, vec4(Sample0, Probe0Index));
	
	vec3 L = Surface.P - Probe0Pos;
	float Dist = length(L);
	//float Occlusion = SampleIrOcclusion(L, Dist * 0.001, Probe0Index, Radiance);
	float Occlusion = 1.0;
	
	L /= Dist;
	float NDotL = clamp(dot(-L, Surface.N), 0.0, 1.0);
	Occlusion *= NDotL;

	return vec4(Irradiance0.rgb, Occlusion);
}
