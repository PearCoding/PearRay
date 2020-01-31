#define PR_RAY_CACHE_MOMENTUM
#include "geometry/TriangleIntersection.h"
#include "image_io.h"
#include "ray/RayPackage.h"

#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

using namespace PR;
namespace bf				 = boost::filesystem;
static const std::string DIR = "results/triangles/";
constexpr size_t WIDTH		 = 500;
constexpr size_t HEIGHT		 = 500;
constexpr float TRI_HEIGHT	 = 1.0f;
constexpr float SCENE_SCALE	 = 1.0f;

constexpr std::array<float, 4> DEGREES = { 0, 30, 60, 90 };
constexpr std::array<size_t, 5> DEPTHS = { 0, 2, 8, 14, 15 };

struct Triangle {
	Vector3f P0;
	Vector3f P1;
	Vector3f P2;
	Vector3f M0;
	Vector3f M1;
	Vector3f M2;
};
static void setup_subdivision_triangles(std::vector<Triangle>& triangles,
										const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
										size_t depth)
{
	if (depth == 0) {
		Vector3f m0 = p0.cross(p1);
		Vector3f m1 = p1.cross(p2);
		Vector3f m2 = p2.cross(p0);
		triangles.emplace_back(Triangle{ p0, p1, p2, m0, m1, m2 });
	} else {
		Vector3f e0 = p1 - p0;
		Vector3f e1 = p2 - p1;
		Vector3f e2 = p0 - p2;

		// Determine largest edge
		float maxNorm = e0.squaredNorm();
		int dim		  = 0;
		if (maxNorm < e1.squaredNorm()) {
			dim		= 1;
			maxNorm = e1.squaredNorm();
		}
		if (maxNorm < e2.squaredNorm())
			dim = 2;

		// Subdivide
		switch (dim) {
		case 0: {
			Vector3f mid = p0 + 0.5f * e0;
			setup_subdivision_triangles(triangles, p0, mid, p2, depth - 1);
			setup_subdivision_triangles(triangles, mid, p1, p2, depth - 1);
		} break;
		case 1: {
			Vector3f mid = p1 + 0.5f * e1;
			setup_subdivision_triangles(triangles, p0, p1, mid, depth - 1);
			setup_subdivision_triangles(triangles, p0, mid, p2, depth - 1);
		} break;
		case 2: {
			Vector3f mid = p2 + 0.5f * e2;
			setup_subdivision_triangles(triangles, mid, p1, p2, depth - 1);
			setup_subdivision_triangles(triangles, p0, p1, mid, depth - 1);
		} break;
		}
	}
}

static void setup_overlay_triangles(std::vector<Triangle>& triangles,
									const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
									size_t depth)
{
	constexpr float SEP = 0.001f;
	size_t count		= std::pow(2, depth);
	triangles.reserve(count);

	for (size_t i = 0; i < count; ++i) {
		Vector3f d0 = p0 + Vector3f(0, 0, i * SEP);
		Vector3f d1 = p1 + Vector3f(0, 0, i * SEP);
		Vector3f d2 = p2 + Vector3f(0, 0, i * SEP);

		Vector3f m0 = d0.cross(d1);
		Vector3f m1 = d1.cross(d2);
		Vector3f m2 = d2.cross(d0);
		triangles.emplace_back(Triangle{ d0, d1, d2, m0, m1, m2 });
	}
}

static std::string prettytime(uint64 t)
{
	uint64 ns = t % 1000;
	t /= 1000;
	uint64 mis = t % 1000;
	t /= 1000;
	uint64 ms = t % 1000;
	t /= 1000;
	uint64 s = t % 60;
	t /= 60;
	uint64 min = t % 60;
	t /= 60;
	uint64 h = t;

	std::stringstream stream;
	stream << std::setfill('0');
	if (h > 0)
		stream << std::setw(2) << h << ":";
	if (min > 0)
		stream << std::setw(2) << min << ":";
	stream << std::setw(2) << s << "." << std::setw(4) << ms << "." << std::setw(4) << mis << "." << std::setw(4) << ns;
	return stream.str();
}

template <typename Func>
static void check_triangles(const std::string& name, const std::vector<Triangle>& triangles,
							float degree, Func f)
{
	tbb::static_partitioner partitioner;

	// Assuming all triangles are same size
	const float tri_area = (triangles[0].P1 - triangles[0].P0).cross(triangles[0].P2 - triangles[0].P0).norm() / 2;
	std::cout << name << "> " << triangles.size() << " [" << (int)degree << "°] TRI AREA: "
			  << std::setprecision(-1) << std::defaultfloat << tri_area << std::endl;

	const float rad	 = degree * PR_PI / 180.0f;
	const Vector3f D = Vector3f(0, std::sin(rad), std::cos(rad));

	std::vector<float> data(WIDTH * HEIGHT, 0.0f);
	std::vector<float> ids(WIDTH * HEIGHT, 0.0f);
	std::vector<float> hits(WIDTH * HEIGHT, 0.0f);

	size_t totalHits	  = 0.0;
	size_t pixelsWithHits = 0;
	const auto start	  = std::chrono::high_resolution_clock::now();
	for (size_t y = 0; y < HEIGHT; ++y) {
		const float fy = 1 - y / static_cast<float>(HEIGHT - 1);

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, WIDTH),
			[&](const tbb::blocked_range<size_t>& r) {
				for (size_t x = r.begin(); x != r.end(); ++x) {
					const float fx = x / static_cast<float>(WIDTH - 1);

					const Vector3f P = SCENE_SCALE * (Vector3f(fx, fy, 0) - D);
					const Ray ray	 = Ray(P, D);

					size_t hitcount = 0;
					size_t id		= 0;
					float gt		= std::numeric_limits<float>::infinity();

					// Normally it is enough to check till found, but for the sake of benchmarking we check all
					for (size_t i = 0; i < triangles.size(); ++i) {
						float lt = f(triangles[i], ray);
						if (lt < std::numeric_limits<float>::infinity())
							++hitcount;

						if (lt < gt) {
							id = i;
							gt = lt;
						}
					}

					if (gt < std::numeric_limits<float>::infinity()) {
						data[y * WIDTH + x] = gt;
						ids[y * WIDTH + x]	= id;
						hits[y * WIDTH + x] = hitcount;
						totalHits += hitcount;
						++pixelsWithHits;
					}
				}
			},
			partitioner);

		const float percentage = (1 - fy) * 100.0f;
		std::cout << "\r" << std::setprecision(2) << std::setw(6) << std::fixed << percentage << "%" << std::flush;
	}
	std::cout << "\r";
	const auto end = std::chrono::high_resolution_clock::now();

	double avgHits = 0.0;
	if (pixelsWithHits > 0)
		avgHits = totalHits / (double)pixelsWithHits;

	const auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	std::cout << "=> FullT: "
			  << prettytime(nano) << std::endl
			  << "   AvgT:  "
			  << prettytime(nano / (WIDTH * HEIGHT * triangles.size())) << std::endl
			  << "   AvgH:  "
			  << std::setprecision(8) << std::fixed << avgHits << std::endl
			  << "   AvgHR: "
			  << std::setprecision(8) << std::fixed << 100 * avgHits / triangles.size() << "%" << std::endl;

	std::stringstream path_stream;
	path_stream << DIR << "/" << name << "_" << triangles.size() << "_" << (int)degree;
	save_normalized_gray(path_stream.str() + "_t.png", WIDTH, HEIGHT, data);
	save_normalized_gray(path_stream.str() + "_id.png", WIDTH, HEIGHT, ids);
	save_normalized_gray(path_stream.str() + "_hits.png", WIDTH, HEIGHT, hits);
}

static void tri_mt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree)
{
	check_triangles(suffix + "_mt", triangles, degree,
					[](const Triangle& tri, const Ray& ray) {
						float t = std::numeric_limits<float>::infinity();
						Vector2f uv;
						TriangleIntersection::intersectMT(ray, tri.P0, tri.P1, tri.P2, uv, t);
						return t;
					});
}

static void tri_pi(const std::string& suffix, const std::vector<Triangle>& triangles, float degree)
{
	check_triangles(suffix + "_pi", triangles, degree,
					[](const Triangle& tri, const Ray& ray) {
						float t = std::numeric_limits<float>::infinity();
						Vector2f uv;
						TriangleIntersection::intersectPI_NonOpt(ray, tri.P0, tri.P1, tri.P2, uv, t);
						return t;
					});
}

static void tri_pi_opt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree)
{
	check_triangles(suffix + "_pi_opt", triangles, degree,
					[](const Triangle& tri, const Ray& ray) {
						float t = std::numeric_limits<float>::infinity();
						Vector2f uv;
						TriangleIntersection::intersectPI_Opt(ray, tri.P0, tri.P1, tri.P2, tri.M0, tri.M1, tri.M2, uv, t);
						return t;
					});
}

typedef void (*MAT_FUNC)(const std::string&, const std::vector<Triangle>&, float);
static MAT_FUNC s_funcs[] = {
	tri_mt,
	tri_pi,
	tri_pi_opt,
	nullptr
};

void suite_triangle()
{
	bf::create_directory(DIR);

	for (int i = 0; s_funcs[i]; ++i) {
		for (auto depth : DEPTHS) {
			{
				std::vector<Triangle> triangles;
				setup_subdivision_triangles(triangles,
											SCENE_SCALE * Vector3f(0.5f, TRI_HEIGHT, 0),
											SCENE_SCALE * Vector3f(1, 0, 0),
											SCENE_SCALE * Vector3f(0, 0, 0),
											depth);
				for (auto degree : DEGREES)
					s_funcs[i]("sub", triangles, degree);
			}
			{
				std::vector<Triangle> triangles;
				setup_overlay_triangles(triangles,
										SCENE_SCALE * Vector3f(0.5f, TRI_HEIGHT, 0),
										SCENE_SCALE * Vector3f(1, 0, 0),
										SCENE_SCALE * Vector3f(0, 0, 0),
										depth);
				for (auto degree : DEGREES)
					s_funcs[i]("ovl", triangles, degree);
			}
		}
		std::cout << "--------------------------------------------------------" << std::endl;
	}
}