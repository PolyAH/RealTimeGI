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





#version 450
#extension GL_ARB_separate_shader_objects : enable




#include "Common.glsl"




// Input Attributes...
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;




#if defined(SPHERE_HELPER_MESH)
layout(push_constant) uniform Constants
{
	vec4 Position;
	vec4 Scale;
	vec4 Color;
} inConstant;
#endif





// VERTEX OUTPUT...
layout(location = 0) out VERTEX_OUT
{
	vec3 Position;
} outVert;








void main()
{

#if defined(SPHERE_HELPER_MESH)
	gl_Position = inCommon.ViewProjMatrix * vec4(inPosition * inConstant.Scale.xyz + inConstant.Position.xyz, 1.0);
#else
	gl_Position = vec4(inPosition, 1.0);
#endif

	outVert.Position = inPosition;
}

