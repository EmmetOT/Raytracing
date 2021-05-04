#ifndef AABB_H
#define AABB_H

#include "utils.h"

class aabb
{
public:
	aabb() {}
	aabb(const vec3& a, const vec3& b) { minimum = a; maximum = b; }

	vec3 min() const { return minimum; }	// near bottom left corner
	vec3 max() const { return maximum; }	// far top right corner

	// determine whether the given ray intersects with this aabb between the 2 values for t
	bool hit(const ray& r, double t_min, double t_max) const
	{
		for (int a = 0; a < 3; a++)
		{
			double invD = 1.0 / r.direction()[a];
			double t0 = (minimum[a] - r.origin()[a]) * invD;
			double t1 = (maximum[a] - r.origin()[a]) * invD;

			if (invD < 0.0)
				std::swap(t0, t1);

			t_min = t0 > t_min ? t0 : t_min;
			t_max = t1 < t_max ? t1 : t_max;

			if (t_max <= t_min)
				return false;
		}

		return true;
	}

	vec3 minimum;
	vec3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1)
{
	vec3 small(	fmin(box0.min().x(), box1.min().x()),
				fmin(box0.min().y(), box1.min().y()),
				fmin(box0.min().z(), box1.min().z()));

	vec3 big(	fmax(box0.max().x(), box1.max().x()),
				fmax(box0.max().y(), box1.max().y()),
				fmax(box0.max().z(), box1.max().z()));

	return aabb(small, big);

}

#endif