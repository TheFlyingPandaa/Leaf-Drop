#include "CorePCH.h"
#include "Camera.h"

Camera * Camera::s_activeCamera = nullptr;

Camera * Camera::GetActiveCamera()
{
	return s_activeCamera;
}

Camera::Camera()
{
	DirectX::XMStoreFloat4x4A(&m_viewMatrix, DirectX::XMMatrixIdentity());
	m_viewProjectionMatrix = m_projectionMatrix = m_viewMatrix;

	this->CreateProjectionMatrix();
	_calcForwardRightAndUp();	
	_calcMatrices();
}


Camera::~Camera()
{
	if (this == s_activeCamera)
		s_activeCamera = nullptr;
}

void Camera::CreateProjectionMatrix(const float & nearPlane, const float & farPlane, const float & fov)
{
	POINT wndSize = Window::GetInstance()->GetWindowSize();

	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	m_FOV = DirectX::XMConvertToRadians(fov);

	DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(m_FOV,
		static_cast<float>(wndSize.x) / static_cast<float>(wndSize.y), nearPlane, farPlane);

	DirectX::XMStoreFloat4x4A(&m_projectionMatrix, proj);

}

void Camera::Update()
{
	_calcMatrices();
}

void Camera::SetPosition(const DirectX::XMFLOAT3 & position)
{
	m_position = { position.x, position.y, position.z, 1.0f };
}

void Camera::SetPosition(const float & x, const float & y, const float & z)
{
	SetPosition({ x, y, z});
}

void Camera::Translate(const DirectX::XMFLOAT3 & translation, bool useForward)
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(&m_position);

	DirectX::XMVECTOR right = DirectX::XMLoadFloat4(&m_right);
	DirectX::XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	DirectX::XMVECTOR forward;

	if (useForward)
		forward = DirectX::XMLoadFloat4(&m_forward);
	else
		forward = DirectX::XMLoadFloat4(&m_direction);


	using namespace DirectX;

	forward = XMVectorMultiply(forward, XMVECTOR{translation.z, translation.z , translation.z});
	right = XMVectorMultiply(right, XMVECTOR{ translation.x, translation.x , translation.x });
	up = XMVectorMultiply(up, XMVECTOR{ translation.y, translation.y , translation.y });

	pos += forward + right + up;
	XMStoreFloat4(&m_position, pos);
}

void Camera::Translate(const float & x, const float & y, const float & z, bool useForward)
{
	Translate({ x, y, z }, useForward);
}

void Camera::SetDirection(const DirectX::XMFLOAT3 & direction)
{
	m_direction = { direction.x,direction.y, direction.z, 0.0f };

	DirectX::XMStoreFloat4(&m_direction, DirectX::XMVector3Normalize(DirectX::XMLoadFloat4(&m_direction)));

	_calcForwardRightAndUp();
}

void Camera::SetDirection(const float & x, const float & y, const float & z)
{
	SetDirection({ x, y, z });
}

void Camera::Rotate(const DirectX::XMFLOAT3 & rotation)
{
	using namespace DirectX;
	static const float MAX_TILT = 0.99f;
	static const XMVECTOR UP = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMVECTOR right = XMLoadFloat4(&m_right);
	XMVECTOR up = UP;
	XMVECTOR forward = XMLoadFloat4(&m_forward);
	XMVECTOR dir = XMLoadFloat4(&m_direction);

	forward = XMVectorMultiply(forward, XMVECTOR{ rotation.z, rotation.z , rotation.z });
	right = XMVectorMultiply(right, XMVECTOR{ rotation.x, rotation.x , rotation.x });
	up = XMVectorMultiply(up, XMVECTOR{ rotation.y, rotation.y , rotation.y });

	XMVECTOR AXIS = forward + right + up;
	XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(AXIS);

	dir = XMVector3Normalize(XMVector3Transform(dir, rot));

	XMStoreFloat4(&m_direction, dir);
	m_direction.w = 0.0f;

	const float dot = XMVectorGetX(XMVector3Dot(UP, dir));
	const float fDot = fabs(dot);

	if (fDot > MAX_TILT)
	{
		float angle = (fDot - MAX_TILT) * (dot < 0.0 ? -1.0f : 1.0f); // DETTA ÄR FEEEL

		this->Rotate(angle, 0.0f, 0.0f);
		return;
	}

	_calcForwardRightAndUp();
}

void Camera::Rotate(const float & x, const float & y, const float & z)
{
	Rotate({ x, y, z });
}

void Camera::SetAsActiveCamera()
{
	s_activeCamera = this;
}

const DirectX::XMFLOAT4 Camera::GetPosition() const
{
	return m_position;
}

const DirectX::XMFLOAT4 Camera::GetUpVector() const
{
	return m_up;
}

const DirectX::XMFLOAT4 Camera::GetDirectionVector() const
{
	return m_up;
}

const DirectX::XMFLOAT4X4A & Camera::GetViewMatrix() const
{
	return m_viewMatrix;
}

const DirectX::XMFLOAT4X4A & Camera::GetProjectionMatrix() const
{
	return m_projectionMatrix;
}

const DirectX::XMFLOAT4X4A & Camera::GetViewProjectionMatrix() const
{
	return m_viewProjectionMatrix;
}

const float & Camera::GetNearPlaneDistance() const
{
	return m_NearPlane;
}

const float & Camera::GetFarPlaneDistance() const
{
	return m_FarPlane;
}

const float & Camera::GetFOV() const
{
	return m_FOV;
}

void Camera::_calcMatrices()
{
	DirectX::XMVECTOR pos = DirectX::XMLoadFloat4(&m_position);
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat4(&m_direction);
	DirectX::XMVECTOR up = DirectX::XMLoadFloat4(&m_up);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(pos, dir, up);
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4A(&m_projectionMatrix);

	DirectX::XMMATRIX viewProj = view * proj;

	DirectX::XMStoreFloat4x4A(&m_viewMatrix, DirectX::XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4A(&m_viewProjectionMatrix, DirectX::XMMatrixTranspose(viewProj));
}

void Camera::_calcForwardRightAndUp()
{
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat4(&m_direction);
	DirectX::XMVECTOR up = { 0,1,0,0 };
	DirectX::XMVECTOR forward, right;

	right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(up, dir));
	forward = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(right, up));
	up = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(dir, right));


	DirectX::XMStoreFloat4(&m_up, up);
	DirectX::XMStoreFloat4(&m_forward, forward);
	DirectX::XMStoreFloat4(&m_right, right);
}