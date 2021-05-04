#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"

class camera
{
public:
	camera
	(
		vec3 look_from,
		vec3 look_at,
		vec3 view_up,
		double vfov, // vertical field-of-view in degrees
		double aspect_ratio,
		double aperture,
		double focus_dist,
		double tm_start = 0.0,
		double tm_end = 0.0
	)
	{
		double theta = degrees_to_radians(vfov);
		double h = tan(theta / 2);
		double viewport_height = 2.0 * h;
		double viewport_width = aspect_ratio * viewport_height;

		w = normalize(look_from - look_at);
		u = normalize(cross(view_up, w));
		v = cross(w, u);

		origin = look_from;
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

		lens_radius = aperture / 2;
		time_start = tm_start;
		time_end = tm_end;
	}

	ray get_ray(double s, double t) const
	{
		vec3 dir = lens_radius * random_in_unit_disk();
		vec3 offset = u * dir.x() + v * dir.y();

		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, random_double(time_start, time_end));
	}

private:
	vec3 origin;
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;        
	double time_start, time_end;  // shutter open/close times
};

#endif