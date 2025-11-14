#pragma once

#ifndef MATERIAL_H
#define MATERIAL_H

#include "primitive.h"

class Material
{
	public:
		virtual ~Material() = default;

		virtual bool scatter(const Ray& r_in, const Hit_record& rec, Vec3& attenuation, Ray& scat) const
		{
			return false;
		}
};

class Lambertian : public Material
{
	public:
		Lambertian(const Vec3& albedo) : albedo(albedo) {}

		bool scatter(const Ray& r_in, const Hit_record& rec, Vec3& attentuation, Ray& scat) const override
		{
			auto scat_dir = rec.normal + random_unit_vector();

			if (scat_dir.near_zero())
				scat_dir = rec.normal;

			scat = Ray(rec.p, scat_dir);
			attentuation = albedo;
			return true;
		}

	private:
		Vec3 albedo;
};

class Metal : public Material
{
	public:
		Metal(const Vec3& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

		bool scatter(const Ray& r_in, const Hit_record& rec, Vec3& attenuation, Ray& scat) const override
		{
			Vec3 reflected = reflect(r_in.direction(), rec.normal);
			reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
			scat = Ray(rec.p, reflected);
			attenuation = albedo;
			return (dot(scat.direction(), rec.normal) > 0);
		}

	private:
		Vec3 albedo;
		double fuzz;
};

class Dielectric : public Material
{
	public:
		Dielectric(double index) : index(index) {}

		bool scatter(const Ray& r, const Hit_record& rec, Vec3& attenuation, Ray& scat) const override
		{
			attenuation = Vec3(1.0, 1.0, 1.0);
			double ri = rec.front_face ? (1.0 / index) : index;

			Vec3 dir = unit_vector(r.direction());
			double cos_theta = std::fmin(dot(-dir, rec.normal), 1.0);
			double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

			bool can_refract = ri * sin_theta <= 1.0;
			Vec3 direction;

			if (can_refract || reflectance(cos_theta, ri) <= random_double())
				direction = refract(dir, rec.normal, ri);
			else
				direction = reflect(dir, rec.normal);

			scat = Ray(rec.p, direction);
			return true;
		}

	private:
		double index;

		static double reflectance(double cos, double index)
		{
			auto r0 = (1 - index) / (1 + index);
			r0 *= r0;
			return r0 + (1 - r0) * std::pow((1 - cos), 5);
		}
};

#endif
