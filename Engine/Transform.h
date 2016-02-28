#pragma once
#include "DirectXIncludes.h"
#include <list>

class Transform
{
private:
	// The parent coordinate space of this transform
	Transform* m_parent;

	// A list of transforms childed to this transform's coordinate space.
	// I tried to think of other ways to ensure that all child transforms 
	// could be notified when a parent transform changes so that they can 
	// update their respective world matrices, but this seems like the 
	// simplest and lowest-impact solution.
	std::list<Transform*> m_children;

	// This data must be aligned otherwise the SSE intrinsics fail and throw exceptions.
	__declspec(align(16)) struct AlignedData
	{
		// Local/World-space position.
		DirectX::XMVECTOR m_translation;
		DirectX::XMVECTOR m_translationWorld;

		// Quaternion representing the local/world-space rotation.
		DirectX::XMVECTOR m_rotation;
		DirectX::XMVECTOR m_rotationWorld;

		// Local/World-space scale.
		DirectX::XMVECTOR m_scale;
		DirectX::XMVECTOR m_scaleWorld;

		// Vectors representing the up, right, and forward vectors of this transform's
		// local coordinate space.
		DirectX::XMVECTOR m_upVector, m_rightVector, m_forwardVector;

		// Matrices for transforming objects into and out of this transform's local coordinate space
		DirectX::XMMATRIX m_worldMatrix, m_inverseWorldMatrix;
	};
	AlignedData* pData;

	// Flags to keep track of which properties have changed and need to be recalculated before 
	// being polled.
	enum DIRTY_FLAG
	{
		WORLD_MATRIX	= 0x01,
		INVERSE_WORLD	= 0x02,
		UP				= 0x04,
		RIGHT			= 0x08,
		FORWARD			= 0x10,
		WORLD_POS		= 0x20,
		WORLD_ROT		= 0x40,
		WORLD_SCALE		= 0x80
	};

	// Keeps track of which dirty flags are set
	mutable unsigned int m_dirtyFlags;

	// Functions for setting and polling dirty flags
	void SetAllDirtyRecursive(bool value = true) const;	// Sets this transform's flags as well as those of its children
	void SetWorldDirtyRecursive(bool value = true) const; // Sets this transform's flags as well as those of its children
	void SetDirtyFlag(DIRTY_FLAG flag, bool value = true) const;
	bool IsDirty(DIRTY_FLAG flag) const;

	// Functions for updating members that require more than just setting a value
	void UpdateWorldMatrix() const;
	void UpdateInverseWorldMatrix() const;
	void UpdateUp() const;
	void UpdateRight() const;
	void UpdateForward() const;

public:

	// I felt it might be useful to have some statically available world axes instead of having to call 
	// XMVectorSet each time I need them.
	__declspec(align(16)) struct WORLD_AXES
	{
		static const DirectX::XMVECTOR UP;
		static const DirectX::XMVECTOR RIGHT;
		static const DirectX::XMVECTOR FORWARD;
	};

	// When performing transformations, it is sometimes useful to express the space in which 
	// the transformation should be applied.
	enum class Space
	{
		LocalSpace,
		WorldSpace,
	};

	Transform();
	~Transform();

	// Child/Parent Management Functions
	void SetParent(Transform* other);
	Transform* GetParent() const;
	void AddChild(Transform* child);
	void RemoveChild(Transform* child);
	bool HasChild(Transform* child) const;

	// Position/Translation Functions
	void XM_CALLCONV SetPosition(DirectX::FXMVECTOR position);
	void SetPosition(float x, float y, float z);
	DirectX::XMVECTOR GetPosition() const;
	void XM_CALLCONV Translate(DirectX::FXMVECTOR translation, Space space = Space::WorldSpace);

	// Rotation Functions
	void XM_CALLCONV SetRotation(DirectX::FXMVECTOR quaternion);
	DirectX::XMVECTOR GetRotation() const;
	void XM_CALLCONV Rotate(DirectX::FXMVECTOR quaternion);
	// ToDo: Implement some additional rotation functions that make it easier to do things like rotate
	// around a given axis.

	// Scaling Functions
	void XM_CALLCONV SetScale(DirectX::FXMVECTOR scale);
	void SetScale(float x, float y, float z);
	DirectX::XMVECTOR GetScale() const;
	void XM_CALLCONV Scale(DirectX::FXMVECTOR scale);

	// World Space Position/Rotation/Scale Functions
	// These calculate the true world-space position/rotation/scale 
	// of the transform, taking into consideration any parent transforms.
	DirectX::XMVECTOR GetWorldPosition() const;
	DirectX::XMVECTOR GetWorldRotation() const;
	DirectX::XMVECTOR GetWorldScale() const;

	// Utility Functions
	void XM_CALLCONV LookAt(DirectX::FXMVECTOR target, DirectX::FXMVECTOR up);

	DirectX::XMVECTOR Up() const;
	DirectX::XMVECTOR Right() const;
	DirectX::XMVECTOR Forward() const;
	DirectX::XMMATRIX GetWorldMatrix() const;
	DirectX::XMMATRIX GetInverseWorldMatrix() const;
};

