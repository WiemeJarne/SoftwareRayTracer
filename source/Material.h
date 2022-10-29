#pragma once
#include "Math.h"
#include "DataTypes.h"
#include "BRDFs.h"

namespace dae
{
#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final
	{
	public:
		Material_SolidColor(const ColorRGB& color): m_Color(color)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) const
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{colors::White};
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance){}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) const
		{
			return{ BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor)};
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{1.f}; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, float kd, float ks, float phongExponent):
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) const
		{
			return{  BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor)
				   + BRDF::Phong(m_SpecularReflectance, m_PhongExponent, l, -v, hitRecord.normal)};
		}

	private:
		ColorRGB m_DiffuseColor{colors::White};
		float m_DiffuseReflectance{0.5f}; //kd
		float m_SpecularReflectance{0.5f}; //ks
		float m_PhongExponent{1.f}; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, float metalness, float roughness):
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) const
		{
			const Vector3 normal{ hitRecord.normal };

			const Vector3 halfVector{ (-v + l).Normalized() };

			const ColorRGB fersnel{ BRDF::FresnelFunction_Schlick(halfVector, -v, (m_Metalness == 0.f) ? ColorRGB{0.04f, 0.04f, 0.04f} : m_Albedo) };

			ColorRGB fng{ fersnel * BRDF::NormalDistribution_GGX(normal, halfVector, m_Roughness) * BRDF::GeometryFunction_Smith(normal, -v, l, m_Roughness) };
			 
			const ColorRGB kd = (m_Metalness == 0.f) ? ColorRGB{ 1.f, 1.f, 1.f } - fersnel : ColorRGB{0.f, 0.f, 0.f};

			return { fng / 4 * (Vector3::Dot(-v, normal) * Vector3::Dot(l, normal)) + BRDF::Lambert(kd, m_Albedo) };
		}
		
	private:
		ColorRGB m_Albedo{0.955f, 0.637f, 0.538f}; //Copper
		float m_Metalness{1.0f};
		float m_Roughness{0.1f}; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
	};
#pragma endregion
}