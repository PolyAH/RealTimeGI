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




#include "Node.h"
#include "Scene.h"





Node::Node()
	: mTransform(Transform::IDENTITY)
	, mName("NO_NAME")
	, mIndexInScene(INVALID_UINDEX)
	, mType(ENodeType::Node)
{

}


Node::~Node()
{

}


void Node::SetTransform(const Transform& mtx)
{
	mTransform = mtx;
}


void Node::SetTranslate(const glm::vec3& translate)
{
	mTransform.SetTranslate(translate);
}


void Node::SetScale(const glm::vec3& scale)
{
	mTransform.SetScale(scale);
}


void Node::SetRotate(const glm::quat& rotate)
{
	mTransform.SetRotate(rotate);
}


void Node::OnAdd(Scene* scene)
{

}


void Node::OnRemove(Scene* scene)
{

}


void Node::OnTransform()
{

}


Box Node::GetBounds() const
{
	return Box();
}