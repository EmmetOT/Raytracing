#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include "utils.h"
#include "colour.h"

struct hit_record;

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, colour& attenutation, ray& scattered) const = 0;
};

class lambertian : public material
{
public:
	lambertian(const colour& a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, colour& attenutation, ray& scattered) const override
	{
		vec3 scatter_direction = random_in_hemisphere(rec.normal);

		// Catch degenerate scatter direction
		if (scatter_direction.near_zero())
			scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction);
		attenutation = albedo;
		return true;
	}

public:
	colour albedo;
};

class metal : public material
{
public:
	metal(const colour& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered) const override
	{
		vec3 reflected = reflect(normalize(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

public:
	colour albedo;
	double fuzz;
};

class dielectric : public material
{
public:
	dielectric(double index_of_refraction) : ir(index_of_refraction) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, colour& attenuation, ray& scattered) const override
	{
		attenuation = colour(1, 1, 1);
		double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

		vec3 direction = normalize(r_in.direction());

		double cos_theta = fmin(dot(-direction, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		// no valid solution for refraction if (refraction_ratio * sin_theta > 1.0)
		// so we reflect. (total internal reflection)
		bool cannot_refract = refraction_ratio * sin_theta > 1.0;

		// we also sometimes reflect randomly, with the chance affected by angle of incidence
		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
			direction = reflect(direction, rec.normal);
		else
			direction = refract(direction, rec.normal, refraction_ratio);

		scattered = ray(rec.p, direction);
		return true;
	}

public:
	double ir; // index of refraction

private:
	static double reflectance(double cosine, double refraction_ratio)
	{
		// use Schlick's approximation for reflectance.
		double r0 = (1 - refraction_ratio) / (1 + refraction_ratio);
		r0 *= r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}
};

#endif