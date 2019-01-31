#pragma once
class Camera
{
public:
	static Camera * GetActiveCamera();

public:
	Camera();
	~Camera();

	// FOV == Field Of View (degrees, not radians)
	void CreateProjectionMatrix(const float & nearPlane = 0.01f, const float & farPlane = 100.0f, const float & fov = 45.0f);

	void Update();

	void SetPosition(const DirectX::XMFLOAT3 & position);
	void SetPosition(const float & x, const float & y, const float & z);

	void Translate(const DirectX::XMFLOAT3 & translation, bool useForward = true);
	void Translate(const float & x, const float & y, const float & z, bool useForward = true);

	void SetDirection(const DirectX::XMFLOAT3 & direction);
	void SetDirection(const float & x, const float & y, const float & z);

	void Rotate(const DirectX::XMFLOAT3 & rotation);
	void Rotate(const float & x, const float & y, const float & z);

	void SetAsActiveCamera();

	const DirectX::XMFLOAT4 GetPosition() const;
	const DirectX::XMFLOAT4 GetUpVector() const;
	const DirectX::XMFLOAT4 GetDirectionVector() const;

	const DirectX::XMFLOAT4X4A & GetViewMatrix() const;
	const DirectX::XMFLOAT4X4A & GetProjectionMatrix() const;
	const DirectX::XMFLOAT4X4A & GetViewProjectionMatrix() const;

private:
	DirectX::XMFLOAT4 m_position =	DirectX::XMFLOAT4(0, 0, 0, 1);
	DirectX::XMFLOAT4 m_up =		DirectX::XMFLOAT4(0, 1, 0, 0);
	DirectX::XMFLOAT4 m_direction = DirectX::XMFLOAT4(0, 0, 1, 0);

	DirectX::XMFLOAT4 m_forward = DirectX::XMFLOAT4(0, 0, 1, 0);
	DirectX::XMFLOAT4 m_right = DirectX::XMFLOAT4(1, 0, 0, 0);

	DirectX::XMFLOAT4X4A m_viewMatrix;
	DirectX::XMFLOAT4X4A m_projectionMatrix;
	DirectX::XMFLOAT4X4A m_viewProjectionMatrix;

private:
	void _calcMatrices();
	void _calcForwardRightAndUp();


	static Camera * s_activeCamera;

};

