#ifndef MOVING_SPHERE_H
#define MOVING_SPHERE_H

#include "material.h"
#include "hittable.h"
#include "vec3.h"

class moving_sphere : public hittable
{
public:
	moving_sphere() {}
	moving_sphere(vec3 cen_start, vec3 cen_end, double tm_start, double tm_end, double r, shared_ptr<material> m) : 
		centre_start(cen_start), centre_end(cen_end), time_start(tm_start), time_end(tm_end), radius(r), mat_ptr(m) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

	vec3 centre(double time) const;

public:
	vec3 centre_start, centre_end;
	double radius;
	shared_ptr<material> mat_ptr;
	double time_start, time_end;
};

bool moving_sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 sphere_centre = centre(r.time());
	vec3 oc = r.origin() - sphere_centre;
	double a = r.direction().length_squared();
	double half_b = dot(oc, r.direction());
	double c = oc.length_squared() - radius * radius;
	double discriminant = half_b * half_b - a * c;

	if (discriminant < 0) 
		return false;

	double sqrtd = sqrt(discriminant);

	// Find the nearest root that lies in the acceptable range.
	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root)
	{
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
			return false;
	}

	rec.t = root;
	rec.p = r.at(rec.t);
	vec3 outward_normal = (rec.p - sphere_centre) / radius;
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat_ptr;

	return true;
}

vec3 moving_sphere::centre(double time) const
{
	return centre_start + ((time - time_start) / (time_end - time_start)) * (centre_end - centre_start);
}

bool moving_sphere::bounding_box(double time0, double time1, aabb& output_box) const
{
	vec3 centre0 = centre(time0);
	vec3 centre1 = centre(time1);

	aabb box0(centre0 - vec3(radius, radius, radius), centre0 + vec3(radius, radius, radius));
	aabb box1(centre1 - vec3(radius, radius, radius), centre1 + vec3(radius, radius, radius));
	output_box = surrounding_box(box0, box1);

	return true;
}

#endif