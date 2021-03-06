#include "pch.h"
#include "utils.h"
#include "colour.h"
#include "hittable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "camera.h"
#include "beautify_duration.h"
#include "bvh.h"

#include <iostream>
#include <chrono>

using namespace std::chrono;
using std::string;

hittable_list showcase_scene()
{
	hittable_list balls;

	shared_ptr<material> ground_mat = make_shared<lambertian>(colour(0.5, 0.5, 0.5));
	shared_ptr<material> main_light_mat = make_shared<light>(colour(0.9568, 0.90588, 0.6431) * 3.0);
	shared_ptr<material> refractor_mat = make_shared<dielectric>(1.4);

	// add ground
	balls.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, ground_mat));
	balls.add(make_shared<sphere>(vec3(0, 1, 0), 1, main_light_mat));
	balls.add(make_shared<sphere>(vec3(0, 2, 0), 1, refractor_mat));

	hittable_list world;
	world.add(make_shared<bvh_node>(balls, 0, 1));

	return world;
}

hittable_list random_scene()
{
	hittable_list balls;

	shared_ptr<material> ground_material = make_shared<lambertian>(colour(0.5, 0.5, 0.5));
	balls.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			double choose_mat = random_double();
			vec3 centre(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((centre - vec3(4, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.4)
				{
					// diffuse
					colour albedo = colour::random() * colour::random();
					sphere_material = make_shared<lambertian>(albedo);

					vec3 centre_end = centre + vec3(0, random_double(0, 0.5), 0);

					balls.add(make_shared<sphere>(centre, 0.2, sphere_material));
				}
				else if (choose_mat < 0.8)
				{
					// light
					colour light_colour = colour::random() * 1.2;

					sphere_material = make_shared<light>(light_colour);
					balls.add(make_shared<sphere>(centre, 0.2, sphere_material));
				}
				else if (choose_mat < 0.9)
				{
					// metal
					colour albedo = colour::random(0.5, 1);
					double fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					balls.add(make_shared<sphere>(centre, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					balls.add(make_shared<sphere>(centre, 0.2, sphere_material));
				}
			}
		}
	}

	// add some big boys

	shared_ptr<material> sun = make_shared<light>(colour(0.9568, 0.90588, 0.6431) * 3.0);
	balls.add(make_shared<sphere>(vec3(-8, 1, 0), 1.0, sun));

	shared_ptr<material> material1 = make_shared<dielectric>(1.5);
	balls.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, material1));

	shared_ptr<material> material2 = make_shared<lambertian>(colour(0.4, 0.2, 0.1));
	balls.add(make_shared<sphere>(vec3(-4, 1, 0), 1.0, material2));

	shared_ptr<material> material3 = make_shared<metal>(colour(0.7, 0.6, 0.5), 0.0);
	balls.add(make_shared<sphere>(vec3(4, 1, 0), 1.0, material3));

	hittable_list world;
	world.add(make_shared<bvh_node>(balls, 0, 1));

	return world;
}

colour ray_colour(const ray& r, const colour& background_colour, const hittable& world, int depth)
{
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return colour(0, 0, 0);

	// If the ray hits nothing, return the background color.
	// the small min distance prevents shadow acne, caused by the resulting hit distance not being
	// at exactly zero due to floating point error
	if (!world.hit(r, 0.000001, infinity, rec))
		return background_colour;

	ray scattered;
	colour attenuation;
	colour emitted = rec.mat_ptr->emitted(0, 0, rec.p);

	// ray has hit a light source and terminates
	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return attenuation * ray_colour(scattered, colour(0, 0, 0), world, depth - 1);
}

int main(int argc, char *argv[])
{
	int image_width = 600;
	int samples_per_pixel = 20;

	if (argc == 3)
	{
		int arg_0 = atoi(argv[1]);
		int arg_1 = atoi(argv[2]);

		image_width = arg_0 > 0 ? arg_0 : image_width;
		samples_per_pixel = arg_1 > 0 ? arg_1 : samples_per_pixel;
	}
	else if (argc == 2)
	{
		int arg_0 = atoi(argv[1]);
		image_width = arg_0 > 0 ? arg_0 : image_width;
	}

	// Image
	const double aspect_ratio = 3.0 / 2.0;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int max_depth = 50;

	std::cerr << "Rendering image of width " << image_width << " and height " << image_height << ", with " << samples_per_pixel << " samples per pixel.\n";

	// World

	//auto world = random_scene();
	auto world = showcase_scene();

	// Camera
	//vec3 look_from(13, 2, 3);
	vec3 look_from(15, 6, 3);
	vec3 look_at(0, 0, 0);
	vec3 view_up(0, 1, 0);
	double dist_to_focus = 10.0;
	double aperture = 0.1;
	colour background_colour(0.0352, 0.0627, 0.2078);

	camera cam(look_from, look_at, view_up, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

	// Render

	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	// pixels are written out in rows with pixels left to right
	// rows are written out from top to bottom

	int milliseconds_total = 0;
	int elapsed = 0;

	for (int j = image_height - 1; j >= 0; --j) 
	{
		string time_remaining_text = " ";

		if (elapsed > 0)
		{
			double average_time = double(milliseconds_total) / elapsed;
			int time_remaining_estimate = int(average_time) * (j + 1);
			time_remaining_text = " (Estimated time remaining = " + beautify_duration(time_remaining_estimate) + ")";
		}

		std::cerr << "\33[2K\rScanlines remaining: " << j << time_remaining_text;// << std::flush;

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for (int i = 0; i < image_width; ++i)
		{
			colour pixel_colour(0, 0, 0);

			for (int s = 0; s < samples_per_pixel; ++s)
			{
				double u = double(i + random_double()) / (image_width - 1);
				double v = double(j + random_double()) / (image_height - 1);

				ray r = cam.get_ray(u, v);
				pixel_colour += ray_colour(r, background_colour, world, max_depth);
			}

			write_colour(std::cout, pixel_colour, samples_per_pixel);
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

		milliseconds_total += int(milliseconds);
		elapsed++;
	}

	string total_time_text = beautify_duration(milliseconds_total);
	total_time_text = total_time_text.length() == 0 ? "Finished!" : "Finished in " + total_time_text + "!";
	std::cerr << "\33[2K\r" << total_time_text;
}