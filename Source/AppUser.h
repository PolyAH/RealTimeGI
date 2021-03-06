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
#include "Core/Transform.h"

#include <string>



class Scene;







// AppUser:
//    - 
//
class AppUser
{
public:
	// Construct.
	AppUser();

	// Construct.
	~AppUser();

	// Initialize User.
	void Initialize();

	// Destroy User.
	void Destroy();

	// Update User Interaction.
	void Update(float deltaTime, Scene* scene);

	// Called every frame to Update ImGui ui.
	void UpdateImGui();

private:
	// Update Input States.
	void UpdateInput(float deltaTime);

	// Update Navigation.
	void UpdateNav(float deltaTime, Scene* scene);

	// Perfrm Scene Select.
	void SceneSelect(Scene* scene);

	// Load a new scene from path.
	void LoadNewScene(const std::string& path);

	// Mark all light components in the scene dirty to get updated.
	void UpdateProbes();

};

