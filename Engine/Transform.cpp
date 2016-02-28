#include "Transform.h"

using namespace DirectX;

// Initialize world axes. Clearly designed for use with a left-handed coordinate system only.
const XMVECTOR Transform::WORLD_AXES::UP		= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
const XMVECTOR Transform::WORLD_AXES::RIGHT		= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
const XMVECTOR Transform::WORLD_AXES::FORWARD	= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

Transform::Transform()
	: m_parent(nullptr)
	, m_dirtyFlags(INT_MAX)
{
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
	if (pData == NULL)
	{
		MessageBoxA(nullptr, "The data is NULL?!", "Error", MB_OK | MB_ICONERROR);
	}
	pData->m_translation = XMVectorZero();
	pData->m_rotation = XMQuaternionIdentity();
	pData->m_scale = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
}

Transform::~Transform()
{
	_aligned_free(pData);
}

// ------------------------------------------------------------------------------------------------------------------------------
// Public Member Functions
// ------------------------------------------------------------------------------------------------------------------------------

// Child/Parent Management
//-----------------------------------------------------------------------------------------
void Transform::SetParent(Transform* other)
{
	// Why would anyone attempt this?
	if (!m_parent && !other)
		return;

	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}

	m_parent = other;

	if (other)
	{
		other->AddChild(this);
	}

	SetAllDirtyRecursive();
}

Transform* Transform::GetParent() const
{
	return m_parent;
}

void Transform::AddChild(Transform* child)
{
	m_children.push_back(child);
}

void Transform::RemoveChild(Transform* child)
{
	m_children.remove(child);
}

bool Transform::HasChild(Transform* child) const
{
	for (Transform* c: m_children)
	{
		if (c == child)
			return true;
	}

	return false;
}

// Position/Translation
//-----------------------------------------------------------------------------------------
void XM_CALLCONV Transform::SetPosition(FXMVECTOR position)
{
	pData->m_translation = position;
	SetWorldDirtyRecursive();
}

void Transform::SetPosition(float x, float y, float z)
{
	SetPosition(XMVectorSet(x, y, z, 1.0f));
}

XMVECTOR Transform::GetPosition() const
{
	return pData->m_translation;
}

void XM_CALLCONV Transform::Translate(FXMVECTOR translation, Space space)
{
	switch (space)
	{
		case Space::LocalSpace:
		{
			pData->m_translation += XMVector3Rotate(translation, pData->m_rotation);
		}
		break;
		case Space::WorldSpace:
		{
			pData->m_translation += translation;
		}
		break;
	}

	pData->m_translation = XMVectorSetW(pData->m_translation, 1.0f);

	SetWorldDirtyRecursive();
}

// Rotation
//-----------------------------------------------------------------------------------------

void XM_CALLCONV Transform::SetRotation(DirectX::FXMVECTOR quaternion)
{
	pData->m_rotation = quaternion;
	SetAllDirtyRecursive();
}

XMVECTOR Transform::GetRotation() const
{
	return pData->m_rotation;
}

void XM_CALLCONV Transform::Rotate(DirectX::FXMVECTOR quaternion)
{
	pData->m_rotation = XMQuaternionMultiply(pData->m_rotation, quaternion);
	SetAllDirtyRecursive();
}

// Scaling
//-----------------------------------------------------------------------------------------
void XM_CALLCONV Transform::SetScale(DirectX::FXMVECTOR scale)
{
	pData->m_scale = scale;
	SetWorldDirtyRecursive();
}

void Transform::SetScale(float x, float y, float z)
{
	SetScale(XMVectorSet(x, y, z, 1.0f));
}

DirectX::XMVECTOR Transform::GetScale() const
{
	return pData->m_scale;
}

void XM_CALLCONV Transform::Scale(DirectX::FXMVECTOR scale)
{
	pData->m_scale += scale;
	SetWorldDirtyRecursive();
}

// World Tansform Values
//-----------------------------------------------------------------------------------------
XMVECTOR Transform::GetWorldPosition() const
{
	if (!m_parent)
	{
		return pData->m_translation;
	}

	if(IsDirty(WORLD_POS))
	{
		pData->m_translationWorld = m_parent->GetWorldPosition() + XMVector3Rotate(pData->m_translation, m_parent->GetRotation());
		SetDirtyFlag(WORLD_POS, false);
	}
	
	return pData->m_translationWorld;
}

XMVECTOR Transform::GetWorldRotation() const
{
	if (!m_parent)
	{
		return pData->m_rotation;
	}

	if(IsDirty(WORLD_ROT))
	{
		pData->m_rotationWorld = XMQuaternionMultiply(m_parent->GetRotation(), pData->m_rotation);
		SetDirtyFlag(WORLD_ROT, false);
	}

	return pData->m_rotationWorld;
}

XMVECTOR Transform::GetWorldScale() const
{
	if (!m_parent)
	{
		return pData->m_scale;
	}

	if(IsDirty(WORLD_SCALE))
	{
		pData->m_scaleWorld = m_parent->GetWorldScale() * pData->m_scale;
		SetDirtyFlag(WORLD_SCALE, false);
	}

	return pData->m_scaleWorld;
}

// Utility
//-----------------------------------------------------------------------------------------

// ToDo: Modify this function to work properly with a parent transform
void XM_CALLCONV Transform::LookAt(DirectX::FXMVECTOR target, DirectX::FXMVECTOR up)
{
	// I am taking the inverse here because it appears that the XMMatrixLookAt functions return a view
	// matrix suitable for use by a camera (i.e. the inverse of the matrix we want). So to get the matrix
	// we need in order to transform an object to look at the given spot, we need to take the inverse of 
	// the matrix we recieve from the function call.
	pData->m_worldMatrix = XMMatrixInverse(nullptr, XMMatrixLookAtLH(pData->m_translation, target, up));
	pData->m_rotation = XMQuaternionRotationMatrix(pData->m_worldMatrix);

	SetAllDirtyRecursive();
	SetDirtyFlag(WORLD_MATRIX, false);
}

XMVECTOR Transform::Up() const
{
	if (IsDirty(WORLD_MATRIX)) UpdateWorldMatrix();
	if (IsDirty(UP)) UpdateUp();

	return pData->m_upVector;
}

XMVECTOR Transform::Right() const
{
	if (IsDirty(WORLD_MATRIX)) UpdateWorldMatrix();
	if (IsDirty(RIGHT)) UpdateRight();

	return pData->m_rightVector;
}

XMVECTOR Transform::Forward() const
{
	if (IsDirty(WORLD_MATRIX)) UpdateWorldMatrix();
	if (IsDirty(FORWARD)) UpdateForward();

	return pData->m_forwardVector;
}

XMMATRIX Transform::GetWorldMatrix() const
{
	if(IsDirty(WORLD_MATRIX))
	{
		UpdateWorldMatrix();
	}

	return pData->m_worldMatrix;
}

XMMATRIX Transform::GetInverseWorldMatrix() const
{
	if(IsDirty(INVERSE_WORLD))
	{
		UpdateInverseWorldMatrix();
	}

	return pData->m_inverseWorldMatrix;
}

// ------------------------------------------------------------------------------------------------------------------------------
// Private Member Functions
// ------------------------------------------------------------------------------------------------------------------------------

// For when the thing rotated and everything needs to be recalculated
void Transform::SetAllDirtyRecursive(bool value) const
{
	m_dirtyFlags = value ? INT_MAX : 0;

	for (Transform* child : m_children)
	{
		child->SetAllDirtyRecursive(value);
	}
}

// For when no rotation has taken place
void Transform::SetWorldDirtyRecursive(bool value) const
{
	if(value)
		m_dirtyFlags |= (WORLD_MATRIX | INVERSE_WORLD | WORLD_POS | WORLD_SCALE);
	else
		m_dirtyFlags &= ~(WORLD_MATRIX | INVERSE_WORLD | WORLD_POS | WORLD_SCALE);

	for (Transform* child : m_children)
	{
		child->SetWorldDirtyRecursive(value);
	}
}

void Transform::SetDirtyFlag(DIRTY_FLAG flag, bool value) const
{
	if (value) 
		m_dirtyFlags |= flag;
	else 
		m_dirtyFlags &= ~flag;
}

bool Transform::IsDirty(DIRTY_FLAG flag) const
{
	return (m_dirtyFlags & flag) != 0;
}

void Transform::UpdateWorldMatrix() const
{
	XMMATRIX translation = XMMatrixTranslationFromVector(pData->m_translation); //XMMatrixTranslationFromVector(-pData->m_translation);
	XMMATRIX rotation = XMMatrixRotationQuaternion(pData->m_rotation); //XMMatrixTranspose(XMMatrixRotationQuaternion(pData->m_rotation));
	XMMATRIX scaling = XMMatrixScalingFromVector(pData->m_scale);

	pData->m_worldMatrix = scaling * rotation * translation;
	
	if (m_parent)
	{
		pData->m_worldMatrix = pData->m_worldMatrix * m_parent->GetWorldMatrix();
	}

	SetDirtyFlag(WORLD_MATRIX, false);
	SetDirtyFlag(INVERSE_WORLD, true);
}

void Transform::UpdateInverseWorldMatrix() const
{
	if(IsDirty(WORLD_MATRIX))
	{
		UpdateWorldMatrix();
	}

	pData->m_inverseWorldMatrix = XMMatrixInverse(nullptr, pData->m_worldMatrix);
	SetDirtyFlag(INVERSE_WORLD, false);
}

void Transform::UpdateUp() const
{
	pData->m_upVector = XMVector3Rotate(WORLD_AXES::UP, GetWorldRotation());
	SetDirtyFlag(UP, false);
}

void Transform::UpdateRight() const
{
	pData->m_rightVector = XMVector3Rotate(WORLD_AXES::RIGHT, GetWorldRotation());
	SetDirtyFlag(RIGHT, false);
}

void Transform::UpdateForward() const
{
	pData->m_forwardVector = XMVector3Rotate(WORLD_AXES::FORWARD, GetWorldRotation());
	SetDirtyFlag(FORWARD, false);
}
