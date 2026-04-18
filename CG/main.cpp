#include<Windows.h>
#pragma region インクルード

#include<cstdint>
#include<string>
#include<format>
#include<cmath>
#include<numbers>
#include<fstream>
#include<sstream>

	#pragma region DirectXインクルード・リンク

#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include<dxgidebug.h>
#pragma comment(lib,"dxguid.lib")



#include<dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")


#include"externals/DirectXTex-mar2023/DirectXTex/DirectXTex.h"



#pragma endregion

	#pragma region ImGuiインクルード

#ifdef USE_IMGUI
#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

	#pragma endregion

#pragma endregion

#pragma region 構造体_計算_関数


	#pragma region 構造体
	
	#pragma region Vector構造体

struct Vector2
{
	float x;
	float y;
};

struct Vector3 
{
	float x;
	float y;
	float z;
};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

#pragma endregion

	#pragma region Matrix構造体

	struct Matrix4x4
	{
		float m[4][4];
	};


	struct Transform
	{
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct TransformationMatrix
	{
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};
	//Transform変数を作る。
	Transform transform
	{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	#pragma endregion

	#pragma region Data構造体

	struct VertexData
	{
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct MaterialData
	{
		std::string textureFilePath;
	};

	struct ModelData
	{
		std::vector<VertexData> vertices;
		MaterialData material;
	};


	#pragma endregion


	struct Material
	{
		Vector4 color;
		int32_t enableLighting;
		float shininess;
	};

	struct DirectionalLight
	{
		Vector4 color;     //ライトの色
		Vector3 direction; //ライトの向き
		float intensity;   //輝度
	};

	

	struct CameraForGPU
	{
		Vector3 worldPosition;
	};

	//ブレンドモード
	enum BlendMode
	{
		//!<ブレンドなし
		kBlendModeNone,
		//!<　通常αブレンド デフォルト Src * SrcA + Dest * (1 - SrcA)
		kBlendModeNormal,
		//!<　加算 Src * SrcA + Dest * 1
		kBlendModeAdd,
		//!< 減算 Dest * 1 - Src * SrcA
		kBlendModeSubtract,
		//!< 乗算 Src * θ + Dest * Src
		kBlendModeMultiply,
		//!< スクリーン Src * (1 - Dest) + Dest * 1
		kBlendModeScreen,
		//利用してはいけない
		kCount0fBlendMode,
	};

#pragma endregion

	#pragma region ベクトルの計算
float Dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Length(const Vector3& v)
{
	return std::sqrt(Dot(v, v));
}

Vector3 Normalize(const Vector3& v)
{
	float length = Length(v);
	if (length == 0)
	{
		return v;
	}
	return { v.x / length, v.y / length, v.z / length };
}
#pragma endregion

	#pragma region 行列



// 単位行列
Matrix4x4 MakeIdentity4x4()
{
	Matrix4x4 identity;
	identity.m[0][0] = 1.0f;	identity.m[0][1] = 0.0f;	identity.m[0][2] = 0.0f;	identity.m[0][3] = 0.0f;
	identity.m[1][0] = 0.0f;	identity.m[1][1] = 1.0f;	identity.m[1][2] = 0.0f;	identity.m[1][3] = 0.0f;
	identity.m[2][0] = 0.0f;	identity.m[2][1] = 0.0f;	identity.m[2][2] = 1.0f;	identity.m[2][3] = 0.0f;
	identity.m[3][0] = 0.0f;	identity.m[3][1] = 0.0f;	identity.m[3][2] = 0.0f;	identity.m[3][3] = 1.0f;
	return identity;
}

// 4x4の掛け算
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;
	result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

	result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

	result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

	result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

	return result;
}

// X軸で回転
Matrix4x4 MakeRotateXMatrix(float radian)
{
	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);
	return 
	{
		1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cosTheta, sinTheta, 0.0f,
			0.0f, -sinTheta, cosTheta, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
	};
}

// Y軸で回転
Matrix4x4 MakeRotateYMatrix(float radian)
{
	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);
	return 
	{ 
		cosTheta, 0.0f, -sinTheta, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sinTheta, 0.0f, cosTheta, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f 
	};
}

// Z軸で回転
Matrix4x4 MakeRotateZMatrix(float radian) 
{
	float cosTheta = std::cos(radian);
	float sinTheta = std::sin(radian);
	return
	{ 
		cosTheta, sinTheta, 0.0f, 0.0f,
			-sinTheta, cosTheta, 0.0f , 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f 
	};
}

// Affine変換
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate)
{
	Matrix4x4 result = Multiply(Multiply(MakeRotateXMatrix(rotate.x), MakeRotateYMatrix(rotate.y)), MakeRotateZMatrix(rotate.z));
	result.m[0][0] *= scale.x;
	result.m[0][1] *= scale.x;
	result.m[0][2] *= scale.x;

	result.m[1][0] *= scale.y;
	result.m[1][1] *= scale.y;
	result.m[1][2] *= scale.y;

	result.m[2][0] *= scale.z;
	result.m[2][1] *= scale.z;
	result.m[2][2] *= scale.z;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	return result;
}



Matrix4x4 Inverse(const Matrix4x4& m)
{
	float determinant = +m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]

		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	Matrix4x4 result;
	float recpDeterminant = 1.0f / determinant;
	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] +
		m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
		m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
	result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] -
		m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
		m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] +
		m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
		m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
	result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] -
		m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
		m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

	result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] -
		m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
		m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] +
		m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
		m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]) * recpDeterminant;
	result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] -
		m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
		m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]) * recpDeterminant;
	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] +
		m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
		m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]) * recpDeterminant;

	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] +
		m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
		m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
	result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] -
		m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
		m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]) * recpDeterminant;
	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] +
		m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
		m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]) * recpDeterminant;
	result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] -
		m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
		m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]) * recpDeterminant;

	result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] -
		m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
		m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] +
		m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
		m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]) * recpDeterminant;
	result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] -
		m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
		m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]) * recpDeterminant;
	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] +
		m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
		m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]) * recpDeterminant;

	return result;
}

//透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip)
{
	float cotHalfFovV = 1.0f / std::tan(fovY / 2.0f);
	return
	{
		(cotHalfFovV / aspectRatio), 0.0f, 0.0f, 0.0f,
		0.0f, cotHalfFovV, 0.0f, 0.0f,
		0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f,
		0.0f, 0.0f, -(nearClip * farClip) / (farClip - nearClip), 0.0f
	};
}

//平行投影行列
Matrix4x4 MakeOethographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip)
{
	return
	{
		2.0f / (right - left),0.0f,0.0f,0.0f,
		0.0f,2.0f / (top - bottom),0.0f,0.0f,
		0.0f,0.0f,1.0f / (farClip - nearClip),0.0f,
		(left + right) / (left - right),
		(top + bottom) / (bottom - top),
		nearClip / (nearClip - farClip),1.0f,
	};
}

#pragma endregion

	#pragma region WinMain外関数


	#pragma region 画面生成(WinMain外)




//ウインドウプロシージャ
LRESULT CALLBACK WindowProc
(
	HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam
)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
	{
		return true;
	}

	//メッセージに応じてゲーム固有の処理を行う
	switch (msg)
	{
	case WM_DESTROY:

		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);

}

#pragma endregion

	#pragma region ログ(WinMain外)



void Log(const std::string& message)
{
	OutputDebugStringA(message.c_str());
}

//string->wstring
std::wstring ConvertString(const std::string& str);

//wstring->string
std::string ConvertString(const std::wstring& str);


std::wstring ConvertString(const std::string& str)
{
	if (str.empty())
	{
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar
	(
		CP_UTF8, 0,
		reinterpret_cast<const char*>(&str[0]),
		static_cast<int>(str.size()),
		NULL, 0
	);

	if (sizeNeeded == 0)
	{
		return std::wstring();
	}

	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar
	(
		CP_UTF8, 0,
		reinterpret_cast<const char*>(&str[0]),
		static_cast<int>(str.size()),
		&result[0], sizeNeeded
	);
	return result;
}


std::string ConvertString(const std::wstring& str)
{
	if (str.empty())
	{
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte
	(
		CP_UTF8, 0,
		str.data(),
		static_cast<int>(str.size()),
		NULL, 0, NULL, NULL
	);

	if (sizeNeeded == 0)
	{
		return std::string();
	}

	std::string result(sizeNeeded, 0);
	WideCharToMultiByte
	(
		CP_UTF8, 0, str.data(),
		static_cast<int>(str.size()),
		result.data(), sizeNeeded, NULL, NULL
	);


	return result;

}



#pragma endregion

	#pragma region CompileShader関数

	IDxcBlob* CompileShader
	(
		//CompilerするShaderファイルへのパス
		const std::wstring& filePath,
		//Compilerに使用するProfile
		const wchar_t* profile,
		//初期化で生成したものを3つ
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler
	)
	{
		//1.hlslファイルを読む

		//これからシェーダーをコンパイルする旨をログに出す
		Log(ConvertString(std::format(L"Begin CompileShader, Path:{}, profile:{}\n", filePath, profile)));
		//hlslファイルを読む
		IDxcBlobEncoding* shaderSource = nullptr;
		HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
		//読めなかったら止める
		assert(SUCCEEDED(hr));
		//読み込んだファイルの内容を設定する	
		DxcBuffer shaderSourceBuffer;
		shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
		shaderSourceBuffer.Size = shaderSource->GetBufferSize();
		shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知







		//2.Compileする

		LPCWSTR arguments[] =
		{
			filePath.c_str(),//コンパイル対象のhlslファイル名
			L"-E",L"main",//エントリーポイントの指定。基本的にmain以外にはしない
			L"-T",profile,//ShaderProfileの設定
			L"-Zi",L"-Qembed_debug",//デバッグ用の情報を埋め込む
			L"-Od",//最適化を外しておく
			L"-Zpr",//メモリレイアウトは行優先
		};
		//実際にShaderをコンパイルする
		IDxcResult* shaderResult = nullptr;
		hr = dxcCompiler->Compile
		(
			&shaderSourceBuffer,//読み込んだファイル
			arguments,//コンパイルオプション
			_countof(arguments),//コンパイルオプションの数
			includeHandler,//includeが含まれた諸々
			IID_PPV_ARGS(&shaderResult)//コンパイル結果
		);
		//コンパイルエラーではなくdxcが起動できないなど致命的な状況
		assert(SUCCEEDED(hr));



		//3.警告・エラーがでていないか確認する
		IDxcBlobUtf8* shaderError = nullptr;
		shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
		if (shaderError != nullptr && shaderError->GetStringLength() != 0)
		{
			Log(shaderError->GetStringPointer());
			assert(false);
		}



		//4.Compile結果を受け取って返す
		//コンパイル結果から実行用のバイナリ部分を取得
		IDxcBlob* shaderBlob = nullptr;
		hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
		assert(SUCCEEDED(hr));
		//成功したログを出す
		Log(ConvertString(std::format(L"Compile Succeeded,Path:{},profile:{}\n", filePath, profile)));
		//もう使わないリソースを解放
		shaderSource->Release();
		shaderResult->Release();
		// 実行用のバイナリを返却
		return shaderBlob;
	}
	#pragma endregion

	#pragma region Resource作成関数


	ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
	{
		//頂点バッファビューを作成する
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;//UploadHeapを使う
		//頂点リーソスの設定
		D3D12_RESOURCE_DESC vertexResourceDesc{};
		//バッファリーソス。テクスチャの場合はまた別の設定をする
		vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexResourceDesc.Width = sizeInBytes;//リーソスのサイズ。こんかいはVector4を3頂点分
		//バッファの場合はこれらは1にする決まり
		vertexResourceDesc.Height = 1;
		vertexResourceDesc.DepthOrArraySize = 1;
		vertexResourceDesc.MipLevels = 1;
		vertexResourceDesc.SampleDesc.Count = 1;
		//バッファの場合はこれにする決まり
		vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		//ID3D12Resource* CreateBufferResource(ID3D12Device * device, size_t sizeInBytes);
		//実際に頂点リソースを作る
		ID3D12Resource* vertexResource = nullptr;
		HRESULT hr = device->CreateCommittedResource
		(
			&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
			&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, 
			nullptr,IID_PPV_ARGS(&vertexResource)
		);
		assert(SUCCEEDED(hr));

		return vertexResource;
	}




	#pragma endregion

	#pragma region DescriptorHeap作成関数


	ID3D12DescriptorHeap* CreateDescriptorHeap
	(
		ID3D12Device* device,
		D3D12_DESCRIPTOR_HEAP_TYPE heapType,
		UINT numDescriptors,
		bool shaderVisible
	)
	{
		//ディスククリプタヒープの生成
		ID3D12DescriptorHeap* descriptorHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Type = heapType;//レンダーターゲットビュー用
		descriptorHeapDesc.NumDescriptors = numDescriptors;//ダブルバッファように2つ。多くても構わない
		descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		//ディスクリプタヒープが作られなかったので起動できない
		assert(SUCCEEDED(hr));
		return descriptorHeap;
	}



	#pragma endregion

	#pragma region Textureデータ

	//1.Textureデータを読み込む
	DirectX::ScratchImage LoadTexture(const std::string& filePath)
	{
		//テクスチャファイルを読んでプログラムで扱えるようにする
		DirectX::ScratchImage image{};
		std::wstring filePathW = ConvertString(filePath);
		HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));

		//ミップマップの作成
		DirectX::ScratchImage mipImages{};
		hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
		assert(SUCCEEDED(hr));

		//ミップマップ付きのデータを返す
		return mipImages;
	}

	//2.DirectX12のTextureResourceを作る
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata)
	{
		//1.metadataを基にResourceの設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = UINT(metadata.width);
		resourceDesc.Height = UINT(metadata.height);
		resourceDesc.MipLevels = UINT16(metadata.mipLevels);
		resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
		resourceDesc.Format = metadata.format;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);


		//2.利用するHeapの設定
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

		//3.Resourceの生成
		ID3D12Resource* resource = nullptr;
		HRESULT hr = device->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource)
		);
		assert(SUCCEEDED(hr));
		return resource;
	}

	//3.TextureResourceにデータを転送する
	void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
	{
		//Meta	情報を取得
		const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
		//全MipMapについて
		for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; mipLevel++)
		{
			//MipMapLevelを指定して各Imageを取得
			const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
			//Textureに転送
			HRESULT hr = texture->WriteToSubresource
			(
				UINT(mipLevel),
				nullptr,
				img->pixels,
				UINT(img->rowPitch),
				UINT(img->slicePitch)
			);
			assert(SUCCEEDED(hr));
		}
	}



	#pragma endregion

	
	#pragma region DepthStencilTexure
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height)
	{
		//生成するResourceの設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.MipLevels = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		//利用するHeapの設定
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;


		//深度値のクリア設定
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;


		//Resourceの生成
		ID3D12Resource* resource = nullptr;
		HRESULT hr = device->CreateCommittedResource
		(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthClearValue,
			IID_PPV_ARGS(&resource)
		);



		assert(SUCCEEDED(hr));
		return resource;
	}
	#pragma endregion


	#pragma region CPUHandleとGPUHandleを取得する関数
	
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += (descriptorSize * index);
		return handleCPU;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr += (descriptorSize * index);
		return handleGPU;
	}

	#pragma endregion


	#pragma region MaterialData構造と読み込み関数
	
	//MaterialData構造と読み込み関数
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
	{
		//1.中で必要となる変数の宣言
		MaterialData materialData;//構築するMaterialData
		std::string line;//ファイルから読んだ1行を格納するもの


		//2.ファイルを開く

		std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
		assert(file.is_open());//開かなかったら止める



		//3.実際にファイルを読み、MAterialDataを構築していく
		while (std::getline(file, line))
		{
			std::string identifier;
			std::istringstream s(line);
			s >> identifier;


			//identifierに応じた処理
			if (identifier == "map_Kd")
			{
				std::string textureFilename;
				s >> textureFilename;
				//連結してファイルパスにする
				materialData.textureFilePath = directoryPath + "/" + textureFilename;
			}
		}

		//4.MAterialDataを返す

		return materialData;
	}


	#pragma endregion


	#pragma region ModelData構造体と読み込み関数

	//obj構造体と読み込み関数
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
	{
		//1.中で必要となる変数の宣言
		ModelData modelData;//構築するModelData
		std::vector<Vector4>positions;//位置
		std::vector<Vector3>normals;//法線
		std::vector<Vector2>texcoords;//テクスチャ座標
		std::string line;//ファイルから読んだ1行を格納するもの
		//2.ファイルを開く
		std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
		assert(file.is_open());//開かなかったら止める



		//3.実際にファイルを読み、ModelDataを構築していく
		while (std::getline(file, line))
		{
			std::string identifier;
			std::istringstream s(line);
			s >> identifier;//先頭の識別子を読む

			//identifier
			if (identifier == "v")
			{
				Vector4 position;
				s >> position.x >> position.y >> position.z;
				position.w = 1.0f;
				positions.push_back(position);
			}
			else if (identifier == "vt")
			{
				Vector2 texcoord;
				s >> texcoord.x >> texcoord.y;
				texcoords.push_back(texcoord);
			}
			else if (identifier == "vn")
			{
				Vector3 normal;
				s >> normal.x >> normal.y >> normal.z;
				normals.push_back(normal);
			}
			else if (identifier == "f")
			{
				//面は三角形限定。その他は未対応
				for (int32_t faceVertex = 0; faceVertex < 3; faceVertex++)
				{
					std::string vertexDefinition;
					s >> vertexDefinition;
					//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
					std::istringstream v(vertexDefinition);
					uint32_t elementIndices[3];
					for (int32_t element = 0; element < 3; element++)
					{
						std::string index;
						std::getline(v, index, '/');//区切りでインデックスを読んでいく
						elementIndices[element] = std::stoi(index);
					}
					//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
					Vector4 position = positions[elementIndices[0] - 1];
					Vector2 texcoord = texcoords[elementIndices[1] - 1];
					Vector3 normal = normals[elementIndices[2] - 1];
					VertexData vertex = { position,texcoord,normal };
					modelData.vertices.push_back(vertex);
				}
			}
			else if (identifier == "mtllib")
			{
				//materialTemplateLibraryファイルの名前を取得する
				std::string materialFilename;
				s >> materialFilename;
				//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
				modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);


			}


		}

		//4.ModelDataを返す
		return modelData;
	}





	#pragma endregion


#pragma endregion

#pragma endregion 構造体_計算_関数

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	CoInitializeEx(0, COINIT_MULTITHREADED);
	//出力ウインドウへの文字出力
	//OutputDebugStringA("Hello,DirectX.\n");
	//wstring->string
	Log(ConvertString(std::format(L"--------------------WSTRING{}\n", L"abc")));

#pragma region 画面クリア_シェーダーコンパイル

		#pragma region =====画面生成=====

#pragma region ウインドウクラスを登録する

	WNDCLASS wc{};
	//ウィンドウブロシージャ
	wc.lpfnWndProc = WindowProc;

	//ウィンドウクラス(なんでも良い)
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);//スペル間違いを
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&wc);

#pragma endregion

#pragma region クライアント領域のサイズ	

	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

#pragma endregion

#pragma region ウインドウ生成

	HWND hwnd = CreateWindow
	(
		wc.lpszClassName,        //利用するクラス名
		L"CG2",                  //タイトルバーの文字(なんでも良い)
		WS_OVERLAPPEDWINDOW,	 //よく見るウィンドウスタイル
		CW_USEDEFAULT,			 //表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			 //表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left,	 //ウィンドウ横幅
		wrc.bottom - wrc.top,	 //ウィンドウ縦幅
		nullptr,				 //親ウィンドウハンドル
		nullptr,				 //メニューハンドル
		wc.hInstance,			 //インスタンスハンドル
		nullptr
	);				 //オプション

	//ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

#pragma endregion

#pragma endregion

		#pragma region =====DebugLayer(画面生成直後)=====


#ifdef _DEBUG//DEBUGはCreateWindowの直後

	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		//デバックレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG



#pragma endregion

		#pragma region =====DirectX12の初期化=====

	#pragma region DXGIFactoryの生成(ウインドウ後メインループ前)


	//DXGIファクトリーの作成
	IDXGIFactory7* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));




#pragma endregion

	#pragma region 使用するアダプター(GPU)を決定する
	IDXGIAdapter4* useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);
#pragma endregion

	#pragma region D3D12Deviceの生成
	ID3D12Device* device = nullptr;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* fertureLevelStrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr))
		{
			Log(std::format("FeatureLevel : {}\n", fertureLevelStrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");
#pragma endregion

#pragma endregion

		#pragma region =====DebugLayer(DirectX12初期化直後)=====

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
	{
		//ヤバいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] =
		{
			//Windows11でのDXGIデバックレイヤーとDX12デバックレイヤーの相互作用バグによるエラーメッセージ
			//https://stackoberflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		//解放
		infoQueue->Release();
	}
#endif // DEBUG


#pragma endregion

		#pragma region =====[画面の色を変える]から[完璧な画面クリア]まで=====

	#pragma region CommandQueueを生成
	//コマンドキューを生成する
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

#pragma endregion

	#pragma region CommandListを生成
		//コマンドアロケータを生成する
		ID3D12CommandAllocator* commandAllocator = nullptr;
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
		assert(SUCCEEDED(hr));

		//コマンドリストを生成する
		ID3D12GraphicsCommandList* commandList = nullptr;
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
			IID_PPV_ARGS(&commandList));
		assert(SUCCEEDED(hr));
	#pragma endregion

	#pragma region SwapChainを生成


		IDXGISwapChain4* swapChain = nullptr;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
		swapChainDesc.Width = kClientWidth;
		swapChainDesc.Height = kClientHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
		hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));

	#pragma endregion

	#pragma region DescriptorHeapの生成
	
		//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisbleはfalse
		ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,2, false);
		//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものではないので、ShaderVisbleはtrue
		ID3D12DescriptorHeap* srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);



	#pragma endregion

	#pragma region SwapChainからResourceを引っ張ってくる

	//SwapChainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));

	//うまく取得できなけらば起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

#pragma endregion

	#pragma region RTVを作る
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	
	
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定して上げる必要がある
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	
	
	//2つ目のディスクリプタハンドルを得る
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//2つ目を作る
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

#pragma endregion

#pragma endregion

		#pragma region FenceEventを生成(メインループ開始前)
	//初期値0でFenceを作る
	ID3D12Fence* fence = nullptr;

	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//FenceのSignalを持つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
#pragma endregion

		#pragma region =====シェーダーコンパイル=====

	#pragma region DXCの初期化

	//dxcCompilerを初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	//現時点でincludeはしないが、includeに対応するための設定を行っておく
	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));


#pragma endregion

	#pragma region レンダリングパイプライン

		#pragma region RootSignatureを生成する

		//RootSignature作成
		D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
		descriptionRootSignature.Flags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;



		D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
		descriptorRange[0].BaseShaderRegister = 0;
		descriptorRange[0].NumDescriptors = 1;
		descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;




		#pragma region Rootparameter(RootSignature部に追加)

		//RootParameter作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
		D3D12_ROOT_PARAMETER rootParameters[6] = {};
		//CBVを使う//b0のbと一致する--------------------↓
		//ConstantBuffer<Material> gMaterial : register(b0);
		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		//PixelShaderで使う
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		//レジスタ番号0とバインド//bというのはConstantBufferのこと
		//b0のbと一致する----------------------------↓
		rootParameters[0].Descriptor.ShaderRegister = 0;
	
	
		//RootParameterを追加する(三角形を動かす)
		//CBVを使う
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		//VertexShaderで使う
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		//レジスタ番号0を使う
		rootParameters[1].Descriptor.ShaderRegister = 0;
	

		rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;//DescriptorTableを使う
		rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
		rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;//Tableの中身の配列を指定
		rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);//Tableで利用する数

		//CBVを使う
		rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		//PixelShaderで使う
		rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		//レジスタ番号1を使う
		rootParameters[3].Descriptor.ShaderRegister = 1;

		//CBVを使う
		rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		//PixelShaderで使う
		rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		//レジスタ番号1を使う
		rootParameters[4].Descriptor.ShaderRegister = 2;


		//CBVを使う
		rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		//PixelShaderで使う
		rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		//レジスタ番号1を使う
		rootParameters[5].Descriptor.ShaderRegister = 3;








	
		//ルートパラメータ配列へのポインタ
		descriptionRootSignature.pParameters = rootParameters;
		//配列の長さ
		descriptionRootSignature.NumParameters = _countof(rootParameters);

		#pragma endregion

		//Samplerの設定
		D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
		staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;//バイナリニアフィルター
		staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//0～1の範囲外をリピート
		staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//比較しない
		staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;//Mipmapを使う
		staticSamplers[0].ShaderRegister = 0;//レジスタ番号0を使う
		staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//PixelShaderで使う
		descriptionRootSignature.pStaticSamplers = staticSamplers;
		descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);


	











		//シリアライズしてバイナリにする
		ID3DBlob* signatureBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		hr = D3D12SerializeRootSignature
		(
			&descriptionRootSignature,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&signatureBlob, &errorBlob
		);
		if (FAILED(hr))
		{
			Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
			assert(false);
		}

		//バイナリを元に作成
		ID3D12RootSignature* rootSignature = nullptr;
		hr = device->CreateRootSignature
		(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature)
		);
		assert(SUCCEEDED(hr));


	#pragma endregion

		#pragma region InputLayputの設定

		//InputLayout
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
		inputElementDescs[0].SemanticName = "POSITION";
		inputElementDescs[0].SemanticIndex = 0;
		inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		
		//テクスチャ用に拡張
		inputElementDescs[1].SemanticName = "TEXCOORD";
		inputElementDescs[1].SemanticIndex = 0;
		inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		//法線用に拡張
		inputElementDescs[2].SemanticName = "NORMAL";
		inputElementDescs[2].SemanticIndex = 0;
		inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;




		
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElementDescs;
		inputLayoutDesc.NumElements = _countof(inputElementDescs);



		#pragma endregion

		#pragma region BlendStateの設定

		//BlendStateの設定
		D3D12_BLEND_DESC blendDesc{};
		//すべての色要素を書き込む
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;



/*
		blendDesc.RenderTarget[0].BlendEnable = TRUE;

		//通常
		
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		//Result = SrcColor * SrcAlpha + DestColor * (1-SrcAlpha)



		//共通
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
*/



		//加算合成
		/*
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		*/


	    //減算合成(逆減算合成)
	    /*
	    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	    */
	    

	    //乗算合成
	    /*
	    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	    */


	    //スクリーン合成
	    /*
	    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	    */



		#pragma endregion



		#pragma region RasterizerStateの設定

	//RasiterzerStateの設定	
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


		#pragma endregion

		#pragma region ShaderをCompileする
	
	//Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob = CompileShader
	(
		L"Object3d.VS.hlsl", L"vs_6_0",
		dxcUtils, dxcCompiler, includeHandler
	);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader
	(
		L"Object3d.PS.hlsl", L"ps_6_0",
		dxcUtils, dxcCompiler, includeHandler
	);
	assert(pixelShaderBlob != nullptr);


#pragma endregion





		#pragma region PSOを生成する	

	//PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPiplineStateDesc{};
	graphicsPiplineStateDesc.pRootSignature = rootSignature;//RootSignature
	graphicsPiplineStateDesc.InputLayout = inputLayoutDesc;//InputLayout
	graphicsPiplineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };//VertexShader
	graphicsPiplineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };//PixelShader
	graphicsPiplineStateDesc.BlendState = blendDesc;//BlendState
	graphicsPiplineStateDesc.RasterizerState = rasterizerDesc;//RasterizerState
	//書き込むRTVの情報	
	graphicsPiplineStateDesc.NumRenderTargets = 1;
	graphicsPiplineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトポロジ(形状)のタイプ。三角形
	graphicsPiplineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//どのように画面に色を打ち込むかの設定
	graphicsPiplineStateDesc.SampleDesc.Count = 1;
	graphicsPiplineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//実際に生成
	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPiplineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));


		#pragma endregion






		#pragma region DepthStencilTextureを生成する
	//DepthStencilTextureをウインドウサイズで作成
	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);
		#pragma endregion
		
		#pragma region DepthStencilView
	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStencilResource, &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		#pragma endregion

		#pragma region DepthStencilState

	//DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	//Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	//書き込み
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//比較関数はLessEqual(近ければ描画)
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;



	//DepthStencilの設定
	graphicsPiplineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPiplineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		#pragma endregion




	
		
	

	#pragma endregion レンダリングパイプライン

#pragma endregion =====シェーダーコンパイル=====

	
#pragma endregion 画面クリア_シェーダーコンパイル








#pragma region ImGuiの初期化


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device,
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());



#pragma endregion




#pragma region Material用のResourceを作る


	//マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialData = nullptr;
	//書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//今回は赤を書き込む
	materialData->color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->shininess = 70.0f;
#pragma endregion












#pragma region Texture関連

	//DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	
	//記入例
	//GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, 0);//

	#pragma region 1枚目 UV-Checker

	//Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	//DirectX::ScratchImage mipImages = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
	UploadTextureData(textureResource, mipImages);


	//mtaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);


	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//先頭はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);

	#pragma endregion

	#pragma region 2枚目 MonsterBall

	//2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	ID3D12Resource* textureResource2 = CreateTextureResource(device, metadata2);
	UploadTextureData(textureResource2, mipImages2);

	//metadataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	//SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	//SRVの生成
	device->CreateShaderResourceView(textureResource2, &srvDesc2, textureSrvHandleCPU2);

	#pragma endregion

	//テクスチャ切り替え
	bool useMonsterBall = false;

	

#pragma endregion





#pragma region 設定


	//      Oo
	//   [>[二]
	//
	//
#pragma region カメラ用
	
	//カメラ位置を指定
	Transform cameraTransform
	{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,-15.0f}
	};

	//カメラ用のリソースを作る
	ID3D12Resource* cameraResource = CreateBufferResource(device, sizeof(CameraForGPU));
	//マテリアルにデータを書き込む
	CameraForGPU* cameraData = nullptr;
	//書き込むためのアドレスを取得
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraData));
	cameraData->worldPosition = cameraTransform.translate;


	bool cameraReset = false;

#pragma endregion







	//              *
	//          [] **
	//  [][][][][]***
	//  [][][][][]***
	//          [] **
	//              *
#pragma region ライト用


	//ライト用のリソースを作る
	ID3D12Resource* directionalLightResource = CreateBufferResource(device, sizeof(DirectionalLight));
	//マテリアルにデータを書き込む
	DirectionalLight* directionalLightData = nullptr;
	//書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	//デフォルト値
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;



	bool lightReset = false;



	

#pragma endregion





#pragma region =====頂点型=====




	//○[][][][][][][][]○
	//[]                []
	//[]                []
	//[]                []
	//○[][][][][][][][]○


	#pragma region 頂点インデックスのリソース


   

   //VertexRecource(関数化済)を生成する
	//関数から呼び出す
   ID3D12Resource* vertexResourceSprite2 = CreateBufferResource(device, sizeof(VertexData) * 4);


   //VertexBufferViewを生成する
   //頂点バッファビューを作成する
   D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite2{};
   //リソースの先頭のアドレスから使う
   vertexBufferViewSprite2.BufferLocation = vertexResourceSprite2->GetGPUVirtualAddress();
   //使用するリソースのサイズは頂点4つ分のサイズ
   vertexBufferViewSprite2.SizeInBytes = sizeof(VertexData) * 4;
   //1頂点あたりのサイズ
   vertexBufferViewSprite2.StrideInBytes = sizeof(VertexData);

   //Resourceにデータを書き込む
  //頂点リソースにデータを書き込む
   VertexData* vertexDataSprite2 = nullptr;
   //書き込むためのアドレスを取得
   vertexResourceSprite2->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite2));



   //1枚目の三角形

	//左下
   vertexDataSprite2[0].position = { 0.0f,360.0f,0.0f,1.0f };
   vertexDataSprite2[0].texcoord = { 0.0f,1.0f };
   vertexDataSprite2[0].normal = { 0.0f,0.0f,-1.0f };

   //左上
   vertexDataSprite2[1].position = { 0.0f,0.0f,0.0f,1.0f };
   vertexDataSprite2[1].texcoord = { 0.0f,0.0f };
   vertexDataSprite2[1].normal = { 0.0f,0.0f,-1.0f };

   //右下
   vertexDataSprite2[2].position = { 640.0f,360.0f,0.0f,1.0f };
   vertexDataSprite2[2].texcoord = { 1.0f,1.0f };
   vertexDataSprite2[2].normal = { 0.0f,0.0f,-1.0f };

   //右上
   vertexDataSprite2[3].position = { 640.0f,0.0f,0.0f,1.0f };
   vertexDataSprite2[3].texcoord = { 1.0f,0.0f };
   vertexDataSprite2[3].normal = { 0.0f,0.0f,-1.0f };




   //スプライト用のTransformationMatrix用のリソースを作る。
   ID3D12Resource* transformationMatrixResourceSprite2 = CreateBufferResource(device, sizeof(TransformationMatrix));
   //データを書き込む
   TransformationMatrix* transformationMatrixDataSprite2 = nullptr;
   //書き込むためのアドレスを取得
   transformationMatrixResourceSprite2->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite2));
   //単位行列書き込む
   transformationMatrixDataSprite2->WVP = MakeIdentity4x4();
   transformationMatrixDataSprite2->World = MakeIdentity4x4();
   transformationMatrixDataSprite2->WorldInverseTranspose = MakeIdentity4x4();




   //スプライトのTransform
   Transform transformSprite2
   {
	   {1.0f,1.0f,1.0f},
	   {0.0f,0.0f,0.0f},
	   {0.0f,0.0f,0.0f}
   };

   ID3D12Resource* IndexResourceSprite = CreateBufferResource(device, sizeof(uint32_t) * 6);

   D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
   //リソースの先頭アドレスから使う
   indexBufferViewSprite.BufferLocation = IndexResourceSprite->GetGPUVirtualAddress();
   //使用するリソースのサイズはインデックス6つ分のサイズ
   indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 4;
   //インデックスはuint32_tとする
   indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;


   uint32_t* indexDataSprite = nullptr;
   IndexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));

   indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
   indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;



   bool drawIndexSprite = false;





#pragma endregion







	//    []
	//   [][]
	//  [][][]   +     []
	// [][][][]      [][][]
	//[][][][][]   [][][][][]
	#pragma region 三角形のTransformationMatrix用のResorceを作る

	//VertexRecource(関数化済)を生成する
	//関数から呼び出す
	ID3D12Resource* vertexResourceTriangle = CreateBufferResource(device, sizeof(VertexData) * 6);
	
	
	//VertexBufferViewを生成する
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewTriangle{};
	//リソースの先頭のアドレスから使う
	vertexBufferViewTriangle.BufferLocation = vertexResourceTriangle->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViewTriangle.SizeInBytes = sizeof(VertexData) * 6;
	//1頂点あたりのサイズ
	vertexBufferViewTriangle.StrideInBytes = sizeof(VertexData);


		
		
    //Resourceにデータを書き込む
	//頂点リソースにデータを書き込む
	VertexData* vertexDataTriangle = nullptr;
	//書き込むためのアドレスを取得
	vertexResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTriangle));
	//左下
	vertexDataTriangle[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexDataTriangle[0].texcoord = { 0.0f,1.0f };
	vertexDataTriangle[0].normal = { 0.0f,0.0f,-1.0f };
	//上
	vertexDataTriangle[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexDataTriangle[1].texcoord = { 0.5f,0.0f };
	vertexDataTriangle[1].normal = { 0.0f,0.0f,-1.0f };
	//右下
	vertexDataTriangle[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexDataTriangle[2].texcoord = { 1.0f,1.0f };
	vertexDataTriangle[2].normal = { 0.0f,0.0f,-1.0f };
	
	
	//左下2
	vertexDataTriangle[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexDataTriangle[3].texcoord = { 0.0f,1.0f };
	vertexDataTriangle[3].normal = { 0.0f,0.0f,-1.0f };
	//上2
	vertexDataTriangle[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDataTriangle[4].texcoord = { 0.5f,0.0f };
	vertexDataTriangle[4].normal = { 0.0f,0.0f,-1.0f };
	//右下2
	vertexDataTriangle[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexDataTriangle[5].texcoord = { 1.0f,1.0f };
	vertexDataTriangle[5].normal = { 0.0f,0.0f,-1.0f };


	//三角形マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	ID3D12Resource* materialResourceTriangle = CreateBufferResource(device, sizeof(Material));
	//マテリアルにデータを書き込む
	Material* materialDataTriangle = nullptr;
	//書き込むためのアドレスを取得
	materialResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTriangle));
	//今回は赤を書き込む
	materialDataTriangle->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataTriangle->enableLighting = true;
	materialDataTriangle->shininess = 70.0f;



	//三角形WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	ID3D12Resource* wvpResourceTriangle = CreateBufferResource(device, sizeof(TransformationMatrix));
	//データを書き込む
	TransformationMatrix* wvpDataTriangle = nullptr;
	//書き込むためにアドレスを取得
	wvpResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataTriangle));
	//単位行列を書き込んでおく
	wvpDataTriangle->WVP = MakeIdentity4x4();
	wvpDataTriangle->World = MakeIdentity4x4();
	wvpDataTriangle->WorldInverseTranspose = MakeIdentity4x4();
	
	//三角形のTransform
	Transform transformTriangle
	{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};



	#pragma region 三角形の設定

	bool drawTriangle = false;
	bool triangleReset = false;
	bool triangleRotateX = false;
	bool triangleRotateY = false;
	bool triangleRotateZ = false;

	#pragma endregion


	
#pragma endregion



	//[][][][][][][][][][]
	//[][][][][][][][][][]
	//[][][][][][][][][][]
	//[][][][][][][][][][]
	//[][][][][][][][][][]

	#pragma region Spriteのリソース

		//VertexRecource(関数化済)を生成する
		//関数から呼び出す
		ID3D12Resource* vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);


		//VertexBufferViewを生成する
		//頂点バッファビューを作成する
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
		//リソースの先頭のアドレスから使う
		vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
		//使用するリソースのサイズは頂点3つ分のサイズ
		vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
		//1頂点あたりのサイズ
		vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

		//Resourceにデータを書き込む
	   //頂点リソースにデータを書き込む
		VertexData* vertexDataSprite = nullptr;
		//書き込むためのアドレスを取得
		vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	
	
		//[]
		//[][][]
		//[][][][][]
		//[][][][][][][]
		//[][][][][][][][][]


		//1枚目の三角形
	
		//左下
		vertexDataSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
		vertexDataSprite[0].texcoord = { 0.0f,1.0f };
		vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	
		//左上
		vertexDataSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
		vertexDataSprite[1].texcoord = { 0.0f,0.0f };
		vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	
		//右下
		vertexDataSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
		vertexDataSprite[2].texcoord = { 1.0f,1.0f };
		vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };




	
	
	
	
		//  [][][][][][][][][]
		//	    [][][][][][][]
		//          [][][][][]
		//              [][][]
		//                  []
		//2枚目の三角形

		//左上
		vertexDataSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };
		vertexDataSprite[3].texcoord = { 0.0f,0.0f };
		vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };

		//右上
		vertexDataSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
		vertexDataSprite[4].texcoord = { 1.0f,0.0f };
		vertexDataSprite[4].normal = { 0.0f,0.0f,-1.0f };

		//右下
		vertexDataSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
		vertexDataSprite[5].texcoord = { 1.0f,1.0f };
		vertexDataSprite[5].normal = { 0.0f,0.0f,-1.0f };

		//スプライト用のTransformationMatrix用のリソースを作る。
		ID3D12Resource* transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(TransformationMatrix));
		//データを書き込む
		TransformationMatrix* transformationMatrixDataSprite = nullptr;
		//書き込むためのアドレスを取得
		transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
		//単位行列書き込む
		transformationMatrixDataSprite->WVP = MakeIdentity4x4();
		transformationMatrixDataSprite->World = MakeIdentity4x4();
		transformationMatrixDataSprite->WorldInverseTranspose = MakeIdentity4x4();

	


		//スプライトのTransform
		Transform transformSprite
		{
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};



		#pragma region スプライトの設定

		bool drawSprite = false;
		bool spriteReset = false;
		bool spriteRotateX = false;
		bool spriteRotateY = false;
		bool spriteRotateZ = false;

		#pragma endregion




	#pragma endregion


	//  [][][]
	//[]//[][][]
	//[/[][][][]
	//[][][][][]
	//  [][][]

	#pragma region 球のリソース


		//球表示用
		const uint32_t kSubdivision = 12;
		const uint32_t kNumSphereVertices = kSubdivision * kSubdivision * 6;
		float pi = std::numbers::pi_v<float>;


		ID3D12Resource* vertexResourceSphere = CreateBufferResource(device, sizeof(VertexData) * kNumSphereVertices);


		//頂点バッファビューを作成する
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
		//リソースの先頭のアドレスから使う
		vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
		//使用するリソースのサイズは頂点3つ分のサイズ
		vertexBufferViewSphere.SizeInBytes = sizeof(VertexData) * kNumSphereVertices;
		//1頂点あたりのサイズ
		vertexBufferViewSphere.StrideInBytes = sizeof(VertexData);

		//頂点リソースにデータを書き込む
		VertexData* vertexDataSphere = nullptr;
		//書き込むためのアドレスを取得
		vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSphere));


		//経度分割1つ分の角度φ
		const float kLonEvery = pi * 2.0f / float(kSubdivision);
		//緯度分割1つ分の角度θ
		const float kLatEvery = pi / float(kSubdivision);

		//緯度の方向に分割
		for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++)
		{
			float lat = -pi / 2.0f + kLatEvery * latIndex;
			//経度の方向に分割しながら線を描く
			for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++)
			{
				uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
				float lon = lonIndex * kLonEvery;
				//頂点にデータを入力する
				vertexDataSphere[start].position.x = std::cos(lat) * std::cos(lon);
				vertexDataSphere[start].position.y = std::sin(lat);
				vertexDataSphere[start].position.z = std::cos(lat) * std::sin(lon);
				vertexDataSphere[start].position.w = 1.0f;
				vertexDataSphere[start].texcoord = { float(lonIndex) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision) };


				vertexDataSphere[start].normal.x = vertexDataSphere[start].position.x;
				vertexDataSphere[start].normal.y = vertexDataSphere[start].position.y;
				vertexDataSphere[start].normal.z = vertexDataSphere[start].position.z;







				vertexDataSphere[start + 1].position.x = std::cos(lat + kLatEvery) * std::cos(lon);
				vertexDataSphere[start + 1].position.y = std::sin(lat + kLatEvery);
				vertexDataSphere[start + 1].position.z = std::cos(lat + kLatEvery) * std::sin(lon);
				vertexDataSphere[start + 1].position.w = 1.0f;
				vertexDataSphere[start + 1].texcoord = { float(lonIndex) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision) };

				vertexDataSphere[start + 1].normal.x = vertexDataSphere[start + 1].position.x;
				vertexDataSphere[start + 1].normal.y = vertexDataSphere[start + 1].position.y;
				vertexDataSphere[start + 1].normal.z = vertexDataSphere[start + 1].position.z;





				vertexDataSphere[start + 2].position.x = std::cos(lat) * std::cos(lon + kLonEvery);
				vertexDataSphere[start + 2].position.y = std::sin(lat);
				vertexDataSphere[start + 2].position.z = std::cos(lat) * std::sin(lon + kLonEvery);
				vertexDataSphere[start + 2].position.w = 1.0f;
				vertexDataSphere[start + 2].texcoord = { float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex) / float(kSubdivision) };

				vertexDataSphere[start + 2].normal.x = vertexDataSphere[start + 2].position.x;
				vertexDataSphere[start + 2].normal.y = vertexDataSphere[start + 2].position.y;
				vertexDataSphere[start + 2].normal.z = vertexDataSphere[start + 2].position.z;


				vertexDataSphere[start + 3] = vertexDataSphere[start + 2];
				vertexDataSphere[start + 4] = vertexDataSphere[start + 1];

				vertexDataSphere[start + 5].position.x = std::cos(lat + kLatEvery) * std::cos(lon + kLonEvery);
				vertexDataSphere[start + 5].position.y = std::sin(lat + kLatEvery);
				vertexDataSphere[start + 5].position.z = std::cos(lat + kLatEvery) * std::sin(lon + kLonEvery);
				vertexDataSphere[start + 5].position.w = 1.0f;
				vertexDataSphere[start + 5].texcoord = { float(lonIndex + 1) / float(kSubdivision),1.0f - float(latIndex + 1) / float(kSubdivision) };
		
				vertexDataSphere[start + 5].normal.x = vertexDataSphere[start + 5].position.x;
				vertexDataSphere[start + 5].normal.y = vertexDataSphere[start + 5].position.y;
				vertexDataSphere[start + 5].normal.z = vertexDataSphere[start + 5].position.z;

			}
		}


		//球マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
		ID3D12Resource* materialResourceSphere = CreateBufferResource(device, sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialDataSphere = nullptr;
		//書き込むためのアドレスを取得
		materialResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSphere));
		//今回は赤を書き込む
		materialDataSphere->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		materialDataSphere->enableLighting = true;
		materialDataSphere->shininess = 70.0f;



		//球WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
		ID3D12Resource* wvpResourceSphere = CreateBufferResource(device, sizeof(TransformationMatrix));
		//データを書き込む
		TransformationMatrix* wvpDataSphere = nullptr;
		//書き込むためにアドレスを取得
		wvpResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSphere));
		//単位行列を書き込んでおく
		wvpDataSphere->WVP = MakeIdentity4x4();
		wvpDataSphere->World = MakeIdentity4x4();
		wvpDataSphere->WorldInverseTranspose = MakeIdentity4x4();




		//スプライトのTransform
		Transform transformSphere
		{
			{1.0f,1.0f,1.0f},
			{0.0f,0.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};

		#pragma region 球の設定

		bool drawSphere = false;
		bool sphereReset = false;
		bool sphereRotateX = false;
		bool sphereRotateY = false;
		bool sphereRotateZ = false;

		#pragma endregion




	#pragma endregion


#pragma endregion


#pragma region =====モデルデータ型=====



      //[][][][]
      //[][][][]
      //[][][][]
      //[][][][]
	#pragma region Plane
		//モデル読み込み
		ModelData modelDataPlane = LoadObjFile("resources", "plane.obj");
	   


		//頂点リソースを作る
		ID3D12Resource* vertexResourcePlane = CreateBufferResource(device, sizeof(VertexData) * modelDataPlane.vertices.size());
		
		//頂点バッファービューを作成する
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViewPlane{};
		vertexBufferViewPlane.BufferLocation = vertexResourcePlane->GetGPUVirtualAddress();
		vertexBufferViewPlane.SizeInBytes = UINT(sizeof(VertexData) * modelDataPlane.vertices.size());
		vertexBufferViewPlane.StrideInBytes = sizeof(VertexData);
		
		
		//頂点リソースにデータを書き込む
		VertexData* vertexDataPlane = nullptr;
		vertexResourcePlane->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPlane));
		
		std::memcpy(vertexDataPlane, modelDataPlane.vertices.data(), sizeof(VertexData)* modelDataPlane.vertices.size());

		//モデルのマテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
		ID3D12Resource* materialResourcePlane = CreateBufferResource(device, sizeof(Material));
		//マテリアルにデータを書き込む
		Material* materialDataPlane = nullptr;
		//書き込むためのアドレスを取得
		materialResourcePlane->Map(0, nullptr, reinterpret_cast<void**>(&materialDataPlane));
		//今回は赤を書き込む
		materialDataPlane->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		materialDataPlane->enableLighting = true;
		materialDataPlane->shininess = 70.0f;


		//モデルWVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
		ID3D12Resource* wvpResourcePlane = CreateBufferResource(device, sizeof(TransformationMatrix));
		//データを書き込む
		TransformationMatrix* wvpDataPlane = nullptr;
		//書き込むためにアドレスを取得
		wvpResourcePlane->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataPlane));
		//単位行列を書き込んでおく
		wvpDataPlane->WVP = MakeIdentity4x4();
		wvpDataPlane->World = MakeIdentity4x4();
		wvpDataPlane->WorldInverseTranspose = MakeIdentity4x4();



		//プランのTransform
		Transform transformPlane
		{
			{1.0f,1.0f,1.0f},
			{0.0f,3.0f,0.0f},
			{0.0f,0.0f,0.0f}
		};



		bool drawPlane = false;
		bool planeReset = false;
		bool planeRotateX = false;
		bool planeRotateY = false;
		bool planeRotateZ = false;


	#pragma endregion










#pragma endregion








#pragma endregion



	MSG msg{};
	//ウィンドウのxボタンが押されるまでループ
	while (msg.message != WM_QUIT)
	{
		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}else
		{
			//ゲーム処理.
			#pragma region ImGui


			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			



			#pragma region ImGui 機材

			ImGui::Begin("Light | Camera | Texture | BlendMode");
			

			//ライト
			ImGui::Checkbox("LightReset", &lightReset);
			ImGui::DragFloat3("Light", &directionalLightData->direction.x, 0.01f, -1.0f, 1.0f);

			

			//カメラ
			ImGui::Checkbox("CameraReset", &cameraReset);
			ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("Transform", &cameraTransform.translate.x, 0.01f);
		

			//テクスチャ切り替え
			ImGui::Checkbox("UseMonsterBall", &useMonsterBall);

			//ブレンドモード
			ImGui::ColorEdit4("materialTriangle | Sprite", &materialDataTriangle->color.x, ImGuiColorEditFlags_AlphaPreview);
			ImGui::ColorEdit4("materialSphere", &materialDataSphere->color.x, ImGuiColorEditFlags_AlphaPreview);
			ImGui::ColorEdit4("materialPlane", &materialDataPlane->color.x, ImGuiColorEditFlags_AlphaPreview);

			ImGui::End();

			#pragma endregion

			#pragma region ライト
			
			if (lightReset)
			{
				directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
				directionalLightData->direction = { 0.0f,-1.0f,0.0f };
				directionalLightData->intensity = 1.0f;
				lightReset = false;
			}
			
			//方向を正規化
			directionalLightData->direction = Normalize(directionalLightData->direction);

			#pragma endregion

            #pragma region カメラ

			if (cameraReset)
			{
				cameraTransform.rotate = { 0.0f,0.0f,0.0f };
				cameraTransform.translate = { 0.0f,0.0f,-15.0f };
				cameraReset = false;
			}
			

			#pragma endregion
		

			#pragma region 三角形

			ImGui::Begin("Triangle");

			ImGui::Checkbox("Draw", &drawTriangle);

			ImGui::Checkbox("Reset", &triangleReset);

			//ImGui::ColorEdit4("material", &materialDataTriangle->color.x, ImGuiColorEditFlags_AlphaPreview);

			ImGui::DragFloat3("TriangleScale", &transformTriangle.scale.x, 0.1f);
			ImGui::DragFloat3("TriangleRotate", &transformTriangle.rotate.x, 0.1f);


			ImGui::Checkbox("TriangleAutoRotate-X", &triangleRotateX);
			ImGui::Checkbox("TriangleAutoRotate-Y", &triangleRotateY);
			ImGui::Checkbox("TriangleAutoRotate-Z", &triangleRotateZ);

			ImGui::DragFloat3("TriangleTranslate", &transformTriangle.translate.x, 0.1f);

			ImGui::End();


			
			

			if (triangleReset)
			{
				transformTriangle.scale = { 1.0f,1.0f,1.0f };
				transformTriangle.rotate = { 0.0f,0.0f,0.0f };
				transformTriangle.translate = { 0.0f,0.0f,0.0f };
				triangleRotateX = false;
				triangleRotateY = false;
				triangleRotateZ = false;
				triangleReset = false;
			}

			if (triangleRotateX)
			{
				transformTriangle.rotate.x += 0.03f;
			}
			if (triangleRotateY)
			{
				transformTriangle.rotate.y += 0.03f;
			}
			if (triangleRotateZ)
			{
				transformTriangle.rotate.z += 0.03f;
			}
			#pragma endregion

			#pragma region 頂点インデックス

			ImGui::Begin("IndexSprite");

			ImGui::Checkbox("Draw", &drawIndexSprite);

			ImGui::End();

			

			#pragma endregion

			#pragma region スプライト


			ImGui::Begin("Sprite");

			ImGui::Checkbox("Draw", &drawSprite);

			ImGui::Checkbox("Reset", &spriteReset);

			//ImGui::ColorEdit4("material", &materialDataTriangle->color.x, ImGuiColorEditFlags_AlphaPreview);

			ImGui::DragFloat3("SpriteScale", &transformSprite.scale.x, 0.1f);
			ImGui::DragFloat3("SpriteRotate", &transformSprite.rotate.x, 0.1f);

			ImGui::Checkbox("SpriteAutoRotate-X", &spriteRotateX);
			ImGui::Checkbox("SpriteAutoRotate-Y", &spriteRotateY);
			ImGui::Checkbox("SpriteAutoRotate-Z", &spriteRotateZ);

			ImGui::DragFloat3("SpriteTranslate", &transformSprite.translate.x, 0.1f);
			
			ImGui::End();




			if (spriteReset)
			{
				transformSprite.scale = { 1.0f,1.0f,1.0f };
				transformSprite.rotate = { 0.0f,0.0f,0.0f };
				transformSprite.translate = { 0.0f,0.0f,0.0f };
				spriteRotateX = false;
				spriteRotateY = false;
				spriteRotateZ = false;
				spriteReset = false;
			}

			if (spriteRotateX)
			{
				transformSprite.rotate.x += 0.03f;
			}
			if (spriteRotateY)
			{
				transformSprite.rotate.y += 0.03f;
			}
			if (spriteRotateZ)
			{
				transformSprite.rotate.z += 0.03f;
			}
			#pragma endregion

			#pragma region 球


			ImGui::Begin("Sphere");

			ImGui::Checkbox("Draw", &drawSphere);

			ImGui::Checkbox("Reset", &sphereReset);

			
			ImGui::DragFloat3("SphereScale", &transformSphere.scale.x, 0.1f);
			
			ImGui::DragFloat3("SphereRotate", &transformSphere.rotate.x, 0.1f);

			ImGui::Checkbox("SphereAutoRotate-X", &sphereRotateX);
			ImGui::Checkbox("SphereAutoRotate-Y", &sphereRotateY);
			ImGui::Checkbox("SphereAutoRotate-Z", &sphereRotateZ);

			ImGui::DragFloat3("SphereTranslate", &transformSphere.translate.x, 0.1f);
			
			ImGui::End();




			if (sphereReset)
			{
				transformSphere.scale = { 1.0f,1.0f,1.0f };
				transformSphere.rotate = { 0.0f,0.0f,0.0f };
				transformSphere.translate = { 0.0f,0.0f,0.0f };
				sphereRotateX = false;
				sphereRotateY = false;
				sphereRotateZ = false;
				sphereReset = false;
			}

			if (sphereRotateX)
			{
				transformSphere.rotate.x += 0.03f;
			}
			if (sphereRotateY)
			{
				transformSphere.rotate.y += 0.03f;
			}
			if (sphereRotateZ)
			{
				transformSphere.rotate.z += 0.03f;
			}

			#pragma endregion


			#pragma region プラン


			ImGui::Begin("Plane");

			ImGui::Checkbox("Draw", &drawPlane);

			ImGui::Checkbox("Reset", &planeReset);


			ImGui::DragFloat3("SphereScale", &transformPlane.scale.x, 0.1f);

			ImGui::DragFloat3("SphereRotate", &transformPlane.rotate.x, 0.1f);

			ImGui::Checkbox("SphereAutoRotate-X", &planeRotateX);
			ImGui::Checkbox("SphereAutoRotate-Y", &planeRotateY);
			ImGui::Checkbox("SphereAutoRotate-Z", &planeRotateZ);

			ImGui::DragFloat3("PlaneTranslate", &transformPlane.translate.x, 0.1f);

			ImGui::End();

			if (planeReset)
			{
				transformPlane.scale = { 1.0f,1.0f,1.0f };
				transformPlane.rotate = { 0.0f,3.0f,0.0f };
				transformPlane.translate = { 0.0f,0.0f,0.0f };
				planeRotateX = false;
				planeRotateY = false;
				planeRotateZ = false;
				planeReset = false;
			}

			if (planeRotateX)
			{
				transformPlane.rotate.x += 0.03f;
			}
			if (planeRotateY)
			{
				transformPlane.rotate.y += 0.03f;
			}
			if (planeRotateZ)
			{
				transformPlane.rotate.z += 0.03f;
			}

			#pragma endregion


			
			//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			ImGui::ShowDemoWindow();
			//ImGuiの内部コマンドを生成する
			ImGui::Render();
			

			#pragma endregion

			

#pragma region コマンドを積み込んで確定させる

			//これから書き込むバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();



			#pragma region TransitionBarrierを張るコード(BackBufferIndex取得直後RenderTarget設定前)

			//TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを張る対象のリソース。現在のバックバッファに対して行う
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			//遷移前(現在)のResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//遷移後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			#pragma endregion



			//描画先のRTVを設定する
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			//指定した色で画面全体をクリアする
			//青系統の色
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);


			//描画先のRTVとDSVを設定する
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
			//指定した深度で画面全体をクリアする
			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);



			//描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);



			#pragma region ViewportとScissor


			//ビューポート
			D3D12_VIEWPORT viewport{};
			//クライアント領域のサイズと一緒にして画面全体に表示
			viewport.Width = kClientWidth;
			viewport.Height = kClientHeight;
			viewport.TopLeftX = 0;
			viewport.TopLeftY = 0;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			//シザー矩形
			D3D12_RECT scissorRect{};
			//基本的にビューポートと同じ矩形が構成されるようにする
			scissorRect.left = 0;
			scissorRect.right = kClientWidth;
			scissorRect.top = 0;
			scissorRect.bottom = kClientHeight;



			#pragma endregion




			#pragma region コマンドを積む

			//描画のコマンドを積む
			commandList->RSSetViewports(1, &viewport);//Viewportを設定
			commandList->RSSetScissorRects(1, &scissorRect);//DcissorRectを設定
			//RootSignatureを設定。PSOに設定しているが別途設定が必要
			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelineState);//PSOを設定
			
			//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			


			#pragma endregion


			#pragma region CBuffer

			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

			//ライトのCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			
			//カメラのCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
			
			#pragma endregion	



			
			#pragma region テクスチャ切り替え

			//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);


			if (useMonsterBall)
			{
				commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU2);
			}


			#pragma endregion










			//    []
			//   [][]
			//  [][][]
			// [][][][]
			//[][][][][]
			#pragma region 三角形の更新と描画





			//三角形用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixTriangle = MakeAffineMatrix(transformTriangle.scale, transformTriangle.rotate, transformTriangle.translate);
			Matrix4x4 cameraMatrixTriangle = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrixTriangle = Inverse(cameraMatrixTriangle);
			Matrix4x4 projectionMatrixTriangle = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixTriangle = Multiply(worldMatrixTriangle, Multiply(viewMatrixTriangle, projectionMatrixTriangle));
			wvpDataTriangle->WVP = worldViewProjectionMatrixTriangle;
			wvpDataTriangle->World = worldMatrixTriangle;



			//三角形を更新
			//transformTriangle.rotate.y += 0.03f;
			
			//*wvpDataTriangle = worldMatrixTriangle;

			//VBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewTriangle);

			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceTriangle->GetGPUVirtualAddress());

			//wvp用のCBufferの場所を設定
			//これをいれないと描画ができない
			commandList->SetGraphicsRootConstantBufferView(1, wvpResourceTriangle->GetGPUVirtualAddress());


			//描画
			if (drawTriangle)
			{
				commandList->DrawInstanced(6, 1, 0, 0);
			}
			

			#pragma endregion



			//○[][][][][][][][]○
	        //[]                []
	        //[]                []
	        //[]                []
	        //○[][][][][][][][]○

            #pragma region 頂点インデックスの更新と描画
			//Sprite用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSprite2 = MakeAffineMatrix(transformSprite2.scale, transformSprite2.rotate, transformSprite2.translate);
			Matrix4x4 viewMatrixSprite2 = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite2 = MakeOethographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite2 = Multiply(worldMatrixSprite2, Multiply(viewMatrixSprite2, projectionMatrixSprite2));
			transformationMatrixDataSprite2->WVP = worldViewProjectionMatrixSprite2;
			transformationMatrixDataSprite2->World = worldMatrixSprite2;


			commandList->IASetIndexBuffer(&indexBufferViewSprite);
			if (drawIndexSprite)
			{
				commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

			}
			


			#pragma endregion




			//[][][][][][][][][][]
	        //[][][][][][][][][][]
	        //[][][][][][][][][][]
	        //[][][][][][][][][][]
	        //[][][][][][][][][][]
			#pragma region スプライトの更新と描画
			
			//Sprite用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			Matrix4x4 projectionMatrixSprite = MakeOethographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
			transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
			transformationMatrixDataSprite->World = worldMatrixSprite;

			//Spriteの描画
			//VBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			//TransformationMatrixBufferの場所を指定
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

			//描画
			if (drawSprite)
			{
				commandList->DrawInstanced(6, 1, 0, 0);
			}

			#pragma endregion



			//  [][][]
	        //[]//[][][]
	        //[/[][][][]
	        //[][][][][]
	        //  [][][]
			#pragma region 球の更新と描画

			//球用のWorldViewProjectionMatrixを作る
			Matrix4x4 worldMatrixSphere = MakeAffineMatrix(transformSphere.scale, transformSphere.rotate, transformSphere.translate);
			Matrix4x4 cameraMatrixSphere = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrixSphere = Inverse(cameraMatrixSphere);
			Matrix4x4 projectionMatrixSphere = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSphere = Multiply(worldMatrixSphere, Multiply(viewMatrixSphere, projectionMatrixSphere));
			wvpDataSphere->WVP = worldViewProjectionMatrixSphere;
			wvpDataSphere->World = worldMatrixSphere;

			//VBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSphere);

			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourceSphere->GetGPUVirtualAddress());

			//wvp用のCBufferの場所を設定
			//これをいれないと描画ができない
			commandList->SetGraphicsRootConstantBufferView(1, wvpResourceSphere->GetGPUVirtualAddress());


			//描画
			if (drawSphere)
			{
				commandList->DrawInstanced(kNumSphereVertices, 1, 0, 0);
			}

			#pragma endregion












			Matrix4x4 worldMatrixPlane = MakeAffineMatrix(transformPlane.scale, transformPlane.rotate, transformPlane.translate);
			Matrix4x4 cameraMatrixPlane = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrixPlane = Inverse(cameraMatrixPlane);
			Matrix4x4 projectionMatrixPlane = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixPlane = Multiply(worldMatrixPlane, Multiply(viewMatrixPlane, projectionMatrixPlane));
			wvpDataPlane->WVP = worldViewProjectionMatrixPlane;
			wvpDataPlane->World = worldMatrixPlane;

			//VBVを設定
			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewPlane);

			//マテリアルCBufferの場所を設定
			commandList->SetGraphicsRootConstantBufferView(0, materialResourcePlane->GetGPUVirtualAddress());

			//wvp用のCBufferの場所を設定
			//これをいれないと描画ができない
			commandList->SetGraphicsRootConstantBufferView(1, wvpResourcePlane->GetGPUVirtualAddress());


				
			if (drawPlane)
			{
				commandList->DrawInstanced(UINT(modelDataPlane.vertices.size()), 1, 0, 0);
			}




















			//実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

			#pragma region 画面表示できるようにする(CrearRenderTargetView直後commandList->Close()前)
			
			//画面に描く処理は全ての終わり、画面に映すので、状態を遷移
			//今回はREnderTargetからPresentを張る
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);

			#pragma endregion

			//コマンドリストの内容を確定させる。全てのコマンドを積んでからcloseすること
			hr = commandList->Close();
			assert(SUCCEEDED(hr));
#pragma endregion

#pragma region コマンドをキックする

			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うよう通知する
			swapChain->Present(1, 0);


#pragma region GPUにSignal(シグナル)を送る(swapChain->Present(1, 0);直後)
			//Fenceの値を更新
			fenceValue++;
			//GPUがここまでたどり着いた時に、Fenceの値を指定した値に代入するようにSignalを送る
			commandQueue->Signal(fence, fenceValue);
#pragma endregion

#pragma region Fenceの値を確認してGPUを待つ
			//Fenceの値が指定したSignal値にたどり着いているか確認する
			//GetCompiletedValueの初期化値はFence作成時に渡した初期値
			if (fence->GetCompletedValue() < fenceValue)
			{
				//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				//イベントを待つ
				WaitForSingleObject(fenceEvent, INFINITE);
			}

#pragma endregion

			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			//assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			//assert(SUCCEEDED(hr));

#pragma endregion



		}
	}



	

#pragma region =====DirectX12のオブジェクトを解放=====

	#pragma region 解放処理(メインループ終了後ReportLiveObjects前)

	CloseHandle(fenceEvent);
	
	//ImGuiの終了処理。詳細はさして重要ではないので解説は省略する
	//初期化と逆順に行う
	#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	#endif
	
	
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
#ifdef _DEBUG
	debugController->Release();
#endif

	#pragma region レンダリングパイプラインの解放
	
	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob)
	{
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	#pragma endregion 

	//マテリアルの解放
	materialResource->Release();
	



	#pragma region 素材
	//三角形
	vertexResourceTriangle->Release();
	wvpResourceTriangle->Release();
	
	//頂点インデックス
	vertexResourceSprite2->Release();
	transformationMatrixResourceSprite->Release();

	//スプライト
	vertexResourceSprite->Release();
	transformationMatrixResourceSprite->Release();
	
	//球
	vertexResourceSphere->Release();
	wvpResourceSphere->Release();


	//ライト
	directionalLightResource->Release();
	

	//カメラ
	cameraResource->Release();


	//プラン
	vertexResourcePlane->Release();



	#pragma endregion 




	CloseWindow(hwnd);
#pragma endregion 

	#pragma region ReportLiveObjects(メインループ終了後return前)
	IDXGIDebug* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
	{
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	#pragma endregion 

#pragma endregion 
	CoUninitialize();
	return 0;
}