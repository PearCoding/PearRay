#define PR_RAY_CACHE_MOMENTUM
#include "Profiler.h"
#include "geometry/TriangleIntersection.h"
#include "geometry/TriangleIntersection_BW.h"
#include "image_io.h"
#include "ray/RayPackage.h"

#include <array>
#include <boost/filesystem.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

//#define _PR_SANDBOX_NO_THREADS

using namespace PR;
namespace bf				 = boost::filesystem;
static const std::string DIR = "results/triangles/";
constexpr size_t WIDTH		 = 500;
constexpr size_t HEIGHT		 = 500;
constexpr float TRI_HEIGHT	 = 1.0f;
constexpr float SCENE_SCALE	 = 1.0f;

constexpr std::array<float, 4> DEGREES = { 0, 30, 60, 90 };
constexpr std::array<size_t, 5> DEPTHS = { 0, 2, 8, 14, 15 };
constexpr std::array<int, 4> HITRATES  = { 0, 30, 60, 100 };

static const Vector3f P0 = SCENE_SCALE * Vector3f(0.5f, TRI_HEIGHT, 0);
static const Vector3f P1 = SCENE_SCALE * Vector3f(1, 0, 0);
static const Vector3f P2 = SCENE_SCALE * Vector3f(0, 0, 0);

using Vector3fS = Eigen::Matrix<float, 3, 1, Eigen::DontAlign>;
struct Triangle {
	Vector3fS P0;
	Vector3fS P1;
	Vector3fS P2;
	Vector3fS M0;
	Vector3fS M1;
	Vector3fS M2;
	float BW9Mat[9];
	int32 BW9FixedColumn;
	float BW12Mat[12];
};

static void addTriangle(std::vector<Triangle>& triangles,
						const Vector3f& p0, const Vector3f& p1, const Vector3f& p2)
{
	Triangle tri;
	tri.P0 = p0;
	tri.P1 = p1;
	tri.P2 = p2;
	tri.M0 = p0.cross(p1);
	tri.M1 = p1.cross(p2);
	tri.M2 = p2.cross(p0);

	TriangleIntersection::constructBW9Matrix(p0, p1, p2, tri.BW9Mat, tri.BW9FixedColumn);
	TriangleIntersection::constructBW12Matrix(p0, p1, p2, tri.BW12Mat);

	triangles.emplace_back(tri);
}

static void setup_subdivision_triangles(std::vector<Triangle>& triangles,
										const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
										size_t depth)
{
	if (depth == 0) {
		addTriangle(triangles, p0, p1, p2);
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

static void setup_overlayed_triangles(std::vector<Triangle>& triangles,
									  const Vector3f& p0, const Vector3f& p1, const Vector3f& p2,
									  int hitrate)
{
	constexpr float SEP			 = 0.001f;
	constexpr int TRIS			 = 10000;
	const static Vector3f OFF[4] = { Vector3f(1000, 1000, 0),
									 Vector3f(-1000, 1000, 0),
									 Vector3f(1000, -1000, 0),
									 Vector3f(-1000, -1000, 0) };
	triangles.reserve(TRIS);

	int good = TRIS * (hitrate / 100.0f);
	int bad	 = TRIS - good;

	// Will be hit
	for (int i = 0; i < good; ++i) {
		Vector3f d0 = p0 + Vector3f(0, 0, i * SEP);
		Vector3f d1 = p1 + Vector3f(0, 0, i * SEP);
		Vector3f d2 = p2 + Vector3f(0, 0, i * SEP);
		addTriangle(triangles, d0, d1, d2);
	}

	// Will not be hit
	for (int i = 0; i < bad; ++i) {
		Vector3f d0 = p0 + Vector3f(0, 0, i * SEP) + OFF[i % 4];
		Vector3f d1 = p1 + Vector3f(0, 0, i * SEP) + OFF[i % 4];
		Vector3f d2 = p2 + Vector3f(0, 0, i * SEP) + OFF[i % 4];
		addTriangle(triangles, d0, d1, d2);
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

struct HitStat {
	size_t TotalTime = 0;
	size_t MinTime	 = std::numeric_limits<size_t>::max();
	size_t MaxTime	 = std::numeric_limits<size_t>::min();
	size_t TotalHits = 0;
	size_t MinHits	 = std::numeric_limits<size_t>::max();
	size_t MaxHits	 = std::numeric_limits<size_t>::min();

	inline void apply(size_t time, size_t hits)
	{
		TotalTime += time;
		MinTime = std::min(MinTime, time);
		MaxTime = std::max(MaxTime, time);
		TotalHits += hits;
		MinHits = std::min(MinHits, hits);
		MaxHits = std::max(MaxHits, hits);
	}
};

constexpr char CSV_SEP = ';';
template <typename Func>
static void check_triangles(const std::string& name, const std::vector<Triangle>& triangles,
							float degree, int hitrate, std::ofstream& csv, Func f)
{
	tbb::static_partitioner partitioner;

	HitStat total;
#ifndef _PR_SANDBOX_NO_THREADS
	std::mutex mutex;
#endif

	// Assuming all triangles are same size
	const float tri_area = (triangles[0].P1 - triangles[0].P0).cross(triangles[0].P2 - triangles[0].P0).norm() / 2;
	std::cout << name << "> " << triangles.size() << " [" << (int)degree << "°] TRI AREA: "
			  << std::setprecision(-1) << std::defaultfloat << tri_area;
	if (hitrate >= 0)
		std::cout << " HIT RATE: " << hitrate << "%";
	std::cout << std::endl;

	const float rad	 = degree * PR_PI / 180.0f;
	const Vector3f D = Vector3f(0, std::sin(rad), std::cos(rad));

	std::vector<float> data(WIDTH * HEIGHT, 0.0f);
	std::vector<float> ids(WIDTH * HEIGHT, 0.0f);
	std::vector<float> hits(WIDTH * HEIGHT, 0.0f);
	std::vector<float> uvs(3 * WIDTH * HEIGHT, 0.0f);

	// Barycentric projection
	const auto project = [&](double nx, double ny) -> Vector3f {
		const double sum = nx + ny;
		const double u	 = nx / sum;
		const double v	 = ny / sum;
		const double w	 = 1 - u - v;
		if (std::isinf(u) || std::isinf(v))
			return P0 - D;
		else
			return P0 * w + P1 * u + P2 * v - D;
	};

	size_t pixelsWithHits = 0;
	for (size_t y = 0; y < HEIGHT; ++y) {
		const double fy = 1 - y / static_cast<double>(HEIGHT - 1);

#ifdef _PR_SANDBOX_NO_THREADS
		tbb::blocked_range<size_t> r(0, WIDTH);
		{
#else
		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, WIDTH),
			[&](const tbb::blocked_range<size_t>& r) {
#endif
			for (size_t x = r.begin(); x != r.end(); ++x) {
				const double fx = x / static_cast<double>(WIDTH - 1);

				const Vector3f P = project(fx, fy);
				const Ray ray	 = Ray(P, D);

				size_t hitcount = 0;
				size_t id		= 0;
				Vector2f uv		= Vector2f(0, 0);
				float gt		= std::numeric_limits<float>::infinity();

				const auto start = std::chrono::high_resolution_clock::now();
				// Normally it is enough to check till found, but for the sake of benchmarking we check all
				for (size_t i = 0; i < triangles.size(); ++i) {
					float lt;
					Vector2f luv;
					bool hit = f(triangles[i], ray, lt, luv);
					if (hit) {
						++hitcount;

						if (lt < gt) {
							id = i;
							gt = lt;
							uv = luv;
						}
					}
				}
				const auto end	= std::chrono::high_resolution_clock::now();
				const auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

				if (hitcount > 0) {
					data[y * WIDTH + x]			 = gt;
					ids[y * WIDTH + x]			 = id;
					hits[y * WIDTH + x]			 = hitcount;
					uvs[(y * WIDTH + x) * 3 + 0] = uv[0];
					uvs[(y * WIDTH + x) * 3 + 1] = uv[1];
				}

#ifndef _PR_SANDBOX_NO_THREADS
				mutex.lock();
#endif
				total.apply(nano, hitcount);
				if (hitcount > 0)
					++pixelsWithHits;
#ifndef _PR_SANDBOX_NO_THREADS
				mutex.unlock();
#endif
			}
		}
#ifndef _PR_SANDBOX_NO_THREADS
			,
			partitioner);
#endif

			const double percentage = (1 - fy) * 100.0;
			std::cout << "\r" << std::setprecision(2) << std::setw(6) << std::fixed << percentage << "%" << std::flush;
	}
	std::cout << "\r";

	double avgHits = 0.0;
	if (pixelsWithHits > 0)
		avgHits = total.TotalHits / (double)pixelsWithHits;

	const auto avgNano = total.TotalTime / (WIDTH * HEIGHT * triangles.size());
	std::cout << "=> FullT: "
			  << prettytime(total.TotalTime) << std::endl
			  << "   AvgT:  "
			  << prettytime(avgNano) << std::endl
			  << "   AvgH:  "
			  << std::setprecision(8) << std::fixed << avgHits << std::endl
			  << "   AvgHR: "
			  << std::setprecision(8) << std::fixed << 100 * avgHits / triangles.size() << "%" << std::endl;

	// Add csv entry
	csv << name << CSV_SEP << triangles.size() << CSV_SEP << (int)degree << CSV_SEP << hitrate << CSV_SEP
		<< tri_area << CSV_SEP << pixelsWithHits / static_cast<float>(WIDTH * HEIGHT) << CSV_SEP
		<< total.TotalTime << CSV_SEP << (total.TotalTime > 0 ? total.MinTime : 0) << CSV_SEP << total.MaxTime << CSV_SEP
		<< total.TotalHits << CSV_SEP << (total.TotalHits > 0 ? total.MinHits : 0) << CSV_SEP << total.MaxHits
		<< std::endl;

	// Write images
	std::stringstream path_stream;
	path_stream << DIR << "/" << name << "_" << triangles.size() << "_" << (int)degree;
	if (hitrate >= 0)
		path_stream << "_" << hitrate;

	save_normalized_gray(path_stream.str() + "_t.png", WIDTH, HEIGHT, data);
	save_normalized_gray(path_stream.str() + "_id.png", WIDTH, HEIGHT, ids);
	save_normalized_gray(path_stream.str() + "_hits.png", WIDTH, HEIGHT, hits);
	save_image(path_stream.str() + "_uvs.png", WIDTH, HEIGHT, uvs, 3);
}

static void tri_mt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_mt", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectMT(ray, tri.P0, tri.P1, tri.P2, uv, t);
					});
}

static void tri_wt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_wt", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectWT(ray, tri.P0, tri.P1, tri.P2, uv, t);
					});
}

static void tri_bw9(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_bw9", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectBW9(ray, tri.BW9Mat, tri.BW9FixedColumn, uv, t);
					});
}

static void tri_bw12(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_bw12", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectBW12(ray, tri.BW12Mat, uv, t);
					});
}

static void tri_pi(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_pi", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectPI(ray, tri.P0, tri.P1, tri.P2, uv, t);
					});
}

static void tri_pi_mem(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_pi_mem", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectPI_Mem(ray, tri.P0, tri.P1, tri.P2, tri.M0, tri.M1, tri.M2, uv, t);
					});
}

static void tri_pi_off(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, std::ofstream& csv)
{
	PR_PROFILE_THIS;
	check_triangles(suffix + "_pi_off", triangles, degree, hitrate, csv,
					[](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
						t = std::numeric_limits<float>::infinity();
						return TriangleIntersection::intersectPI_Off(ray, tri.P0, tri.P1, tri.P2, uv, t);
					});
}

typedef void (*MAT_FUNC)(const std::string&, const std::vector<Triangle>&, float, int, std::ofstream& csv);
static MAT_FUNC s_funcs[] = {
	tri_bw12,
	tri_bw9,
	tri_mt,
	tri_pi,
	tri_pi_mem,
	tri_pi_off,
	tri_wt,
	nullptr
};

void precision_test(std::ofstream& csv)
{
	PR_PROFILE_THIS;

	for (int i = 0; s_funcs[i]; ++i) {
		for (auto depth : DEPTHS) {

			std::vector<Triangle> triangles;
			setup_subdivision_triangles(triangles,
										P0, P1, P2,
										depth);
			for (auto degree : DEGREES)
				s_funcs[i]("sub", triangles, degree, -1, csv);
		}
		std::cout << "--------------------------------------------------------" << std::endl;
	}
}

void hitrate_test(std::ofstream& csv)
{
	PR_PROFILE_THIS;

	PR_UNUSED(csv);
	for (int i = 0; s_funcs[i]; ++i) {
		for (auto hitrate : HITRATES) {
			std::vector<Triangle> triangles;
			setup_overlayed_triangles(triangles,
									  P0, P1, P2,
									  hitrate);
			s_funcs[i]("hr", triangles, 0, hitrate, csv);
		}
		std::cout << "--------------------------------------------------------" << std::endl;
	}
}

void suite_triangle()
{
	PR_PROFILE_THIS;

	bf::create_directory(DIR);

	std::ofstream csv;
	csv.open(DIR + "/output.csv");

	// Setup CSV header
	csv << "Name" << CSV_SEP
		<< "Triangles" << CSV_SEP << "Angle" << CSV_SEP << "HitRate" << CSV_SEP
		<< "TriangleArea" << CSV_SEP << "TriangleCoverage" << CSV_SEP
		<< "TotalTimeNS" << CSV_SEP << "MinTimeNS" << CSV_SEP << "MaxTimeNS" << CSV_SEP
		<< "TotalHits" << CSV_SEP << "MinHits" << CSV_SEP << "MaxHits"
		<< std::endl;

	precision_test(csv);
	hitrate_test(csv);
}