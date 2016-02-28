#pragma once
#include "Transform.h"

enum class CAMERA_MODE
{
	Orthographic,
	Perspective
};

class Camera
{
private:
	void UpdateProjectionMatrix() const;
	void UpdateInverseProjectionMatrix() const;

	CAMERA_MODE m_cameraMode;

	// Projection Parameters
	float m_vFoV;			// Vertical field of view.
	float m_aspectRatio;	// Aspect ratio
	// Orthographic Parameters
	float m_orthoWidth;		// Projection Width
	float m_orthoHeight;	// Projection Height
	// General Parameters
	float m_zNear;			// Near clip distance
	float m_zFar;			// Far clip distance.


	// This data must be aligned otherwise the SSE intrinsics fail and throw exceptions.
	__declspec(align(16)) struct AlignedData
	{
		// An orthographic or perspective projection matrix and its inverse
		DirectX::XMMATRIX m_projectionMatrix;
		DirectX::XMMATRIX m_inverseProjectionMatrix;
	};
	AlignedData* pData;

	// Flag to track whether the projection matrix needs to be recalculated
	mutable bool m_projectionDirty, m_inverseProjectionDirty;

public:

	Camera(CAMERA_MODE mode = CAMERA_MODE::Perspective);
	~Camera();

	Transform transform;
	
	void SetMode(CAMERA_MODE mode);
	void SetPerspective(float fovy, float aspect, float zNear, float zFar);
	void SetOrthographic(float width, float height, float zNear, float zFar);

	DirectX::XMMATRIX GetProjectionMatrix() const;
	DirectX::XMMATRIX GetInverseProjectionMatrix() const;
	DirectX::XMMATRIX GetViewMatrix() const;
};

