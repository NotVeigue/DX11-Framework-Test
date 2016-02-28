#include "Camera.h"

using namespace DirectX;

Camera::Camera(CAMERA_MODE mode)
	: m_vFoV(45.0f)
	, m_aspectRatio(1.0f)
	, m_orthoWidth(10.0f)
	, m_orthoHeight(10.0f)
	, m_zNear(0.1f)
	, m_zFar(1000.0f)
	, m_projectionDirty(true)
	, m_cameraMode(mode)
{
	pData = (AlignedData*)_aligned_malloc(sizeof(AlignedData), 16);
	if (pData == NULL)
	{
		MessageBoxA(nullptr, "The data is NULL?!", "Error", MB_OK | MB_ICONERROR);
	}
}

Camera::~Camera()
{
	_aligned_free(pData);
}

// ------------------------------------------------------------------------------------------------------------------------------
// Public Member Functions
// ------------------------------------------------------------------------------------------------------------------------------

void Camera::SetMode(CAMERA_MODE mode)
{
	m_cameraMode = mode;
	m_projectionDirty = true;
}

void Camera::SetPerspective(float fovy, float aspect, float zNear, float zFar)
{
	m_vFoV = fovy;
	m_aspectRatio = aspect;
	m_zNear = zNear;
	m_zFar = zFar;

	m_projectionDirty = m_inverseProjectionDirty = true;
}

void Camera::SetOrthographic(float width, float height, float zNear, float zFar)
{
	m_orthoWidth = width;
	m_orthoHeight = height;
	m_zNear = zNear;
	m_zFar = zFar;

	m_projectionDirty = m_inverseProjectionDirty = true;
}

XMMATRIX Camera::GetProjectionMatrix() const
{
	if (m_projectionDirty)
	{
		UpdateProjectionMatrix();
	}

	return pData->m_projectionMatrix;
}

XMMATRIX Camera::GetInverseProjectionMatrix() const
{
	if (m_inverseProjectionDirty)
	{
		UpdateInverseProjectionMatrix();
	}

	return pData->m_inverseProjectionMatrix;
}

XMMATRIX Camera::GetViewMatrix() const
{
	// The view matrix is just the inverse of the
	// camera's world transform.
	return transform.GetInverseWorldMatrix();
}

// ------------------------------------------------------------------------------------------------------------------------------
// Private Member Functions
// ------------------------------------------------------------------------------------------------------------------------------

void Camera::UpdateProjectionMatrix() const
{
	if (m_cameraMode == CAMERA_MODE::Perspective)
	{
		pData->m_projectionMatrix = XMMatrixPerspectiveFovLH( XMConvertToRadians(m_vFoV), m_aspectRatio, m_zNear, m_zFar);
	}
	else
	{
		pData->m_projectionMatrix = XMMatrixOrthographicLH(m_orthoWidth, m_orthoHeight, m_zNear, m_zFar);
	}

	m_projectionDirty = false;
	m_inverseProjectionDirty = true;
}

void Camera::UpdateInverseProjectionMatrix() const
{
	if (m_projectionDirty)
	{
		UpdateProjectionMatrix();
	}

	pData->m_inverseProjectionMatrix = XMMatrixInverse(nullptr, pData->m_projectionMatrix);
	m_inverseProjectionDirty = false;
}