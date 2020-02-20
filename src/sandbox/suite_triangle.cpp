#define PR_RAY_CACHE_MOMENTUM
#include "Profiler.h"
#include "geometry/TriangleIntersection.h"
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

//#define _SANDBOX_NO_THREADS
//#define _SANDBOX_NO_SINGLE_TRACE
//#define _SANDBOX_NO_PACKAGE_TRACE

using namespace PR;
namespace bf				 = boost::filesystem;
static const std::string DIR = "results/triangles/";
constexpr size_t WIDTH		 = 500;
constexpr size_t HEIGHT		 = 500;
constexpr float TRI_HEIGHT	 = 1.0f;
constexpr float SCENE_SCALE	 = 1.0f;

constexpr char CSV_SEP = ';';

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

	inline Vector3fv P0_v() const { return promote(Vector3f(P0)); }
	inline Vector3fv P1_v() const { return promote(Vector3f(P1)); }
	inline Vector3fv P2_v() const { return promote(Vector3f(P2)); }
	inline Vector3fv M0_v() const { return promote(Vector3f(M0)); }
	inline Vector3fv M1_v() const { return promote(Vector3f(M1)); }
	inline Vector3fv M2_v() const { return promote(Vector3f(M2)); }
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

	inline void reset()
	{
		*this = HitStat();
	}
};

class CSVStream {
private:
	std::ofstream mStream;

public:
	inline explicit CSVStream(const std::string& file)
	{
		mStream.open(file);
	}

	inline void writeHeader()
	{
		mStream << "Name" << CSV_SEP << "PackageSize" << CSV_SEP << "ResultType" << CSV_SEP
				<< "Triangles" << CSV_SEP << "Angle" << CSV_SEP << "HitRate" << CSV_SEP
				<< "TriangleArea" << CSV_SEP << "TriangleCoverage" << CSV_SEP
				<< "TotalTimeNS" << CSV_SEP << "MinTimeNS" << CSV_SEP << "MaxTimeNS" << CSV_SEP
				<< "TotalHits" << CSV_SEP << "MinHits" << CSV_SEP << "MaxHits"
				<< std::endl;
	}

	inline void writeEntry(const std::string& name, size_t packageSize, int type, size_t triangleCount,
						   int degree, int hitrate,
						   float triangleArea, float coverage,
						   const HitStat& stat)
	{
		mStream << name << CSV_SEP << packageSize << CSV_SEP << type << CSV_SEP << triangleCount << CSV_SEP << degree << CSV_SEP << hitrate << CSV_SEP
				<< triangleArea << CSV_SEP << coverage << CSV_SEP
				<< stat.TotalTime << CSV_SEP << (stat.TotalTime > 0 ? stat.MinTime : 0) << CSV_SEP << stat.MaxTime << CSV_SEP
				<< stat.TotalHits << CSV_SEP << (stat.TotalHits > 0 ? stat.MinHits : 0) << CSV_SEP << stat.MaxHits
				<< std::endl;
	}
};

class TriangleCheck {
private:
	std::vector<float> mData;
	std::vector<float> mIds;
	std::vector<float> mHits;
	std::vector<float> mUvs;

	const std::vector<Triangle>& mTriangles;
	std::string mName;
	float mDegree;
	int mHitRate;

	Vector3f mRayDirection;

	HitStat mStat;
	size_t mPixelsWithHits;

	CSVStream& mCSV;

public:
	inline TriangleCheck(const std::string& name, const std::vector<Triangle>& triangles,
						 float degree, int hitrate,
						 CSVStream& csv)
		: mData(WIDTH * HEIGHT, 0.0f)
		, mIds(WIDTH * HEIGHT, 0.0f)
		, mHits(WIDTH * HEIGHT, 0.0f)
		, mUvs(3 * WIDTH * HEIGHT, 0.0f)
		, mTriangles(triangles)
		, mName(name)
		, mDegree(degree)
		, mHitRate(hitrate)
		, mRayDirection(0, std::sin(mDegree * PR_DEG2RAD), std::cos(mDegree * PR_DEG2RAD))
		, mPixelsWithHits(0)
		, mCSV(csv)
	{
	}

	template <typename Func>
	inline void processSingle1(Func func)
	{
		processSingle(1, [&](size_t x, size_t y, const Ray& ray,
							 size_t& hitcount, uint64& nano) {
			hitcount		 = 0;
			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				bool hit = func(mTriangles[i], ray);
				if (hit)
					++hitcount;
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			if (hitcount > 0)
				setPoint(x, y, hitcount);
		});
	}

	template <typename Func>
	inline void processSingle2(Func func)
	{
		processSingle(2, [&](size_t x, size_t y, const Ray& ray,
							 size_t& hitcount, uint64& nano) {
			hitcount  = 0;
			size_t id = 0;
			float gt  = std::numeric_limits<float>::infinity();

			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				float lt;
				bool hit = func(mTriangles[i], ray, lt);
				if (hit) {
					++hitcount;

					if (lt < gt) {
						id = i;
						gt = lt;
					}
				}
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			if (hitcount > 0)
				setPoint(x, y, gt, id, hitcount);
		});
	}

	template <typename Func>
	inline void processSingle3(Func func)
	{
		processSingle(3, [&](size_t x, size_t y, const Ray& ray,
							 size_t& hitcount, uint64& nano) {
			hitcount	= 0;
			size_t id	= 0;
			Vector2f uv = Vector2f(0, 0);
			float gt	= std::numeric_limits<float>::infinity();

			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				float lt;
				Vector2f luv;
				bool hit = func(mTriangles[i], ray, lt, luv);
				if (hit) {
					++hitcount;

					if (lt < gt) {
						id = i;
						gt = lt;
						uv = luv;
					}
				}
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			if (hitcount > 0)
				setPoint(x, y, gt, id, hitcount, uv);
		});
	}

	template <typename Func>
	inline void processPackage1(Func func)
	{
		processPackage(1, [&](size_t x, size_t y, const RayPackage& package,
							  vuint32& hitcount, uint64& nano) {
			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				const bfloat hit = func(mTriangles[i], package);
				hitcount		 = blend(hitcount + vuint32(1), hitcount, hit);
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			for (size_t k = 0; k < PR_SIMD_BANDWIDTH && x * PR_SIMD_BANDWIDTH + k < WIDTH; ++k) {
				uint32 s = extract(k, hitcount);
				if (s > 0)
					setPoint(x * PR_SIMD_BANDWIDTH + k, y, s);
			}
		});
	}

	template <typename Func>
	inline void processPackage2(Func func)
	{
		processPackage(2, [&](size_t x, size_t y, const RayPackage& package,
							  vuint32& hitcount, uint64& nano) {
			vuint32 id = vuint32(0);
			vfloat gt  = vfloat(std::numeric_limits<float>::infinity());

			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				vfloat lt;
				Vector2fv luv;
				const bfloat hit   = func(mTriangles[i], package, lt);
				const bfloat close = hit & (lt < gt);

				hitcount = blend(hitcount + vuint32(1), hitcount, hit);
				id		 = blend(vuint32(i), id, close);
				gt		 = blend(lt, gt, close);
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			for (size_t k = 0; k < PR_SIMD_BANDWIDTH && x * PR_SIMD_BANDWIDTH + k < WIDTH; ++k) {
				uint32 s = extract(k, hitcount);
				if (s > 0)
					setPoint(x * PR_SIMD_BANDWIDTH + k, y,
							 extract(k, gt), extract(k, id), s);
			}
		});
	}

	template <typename Func>
	inline void processPackage3(Func func)
	{
		processPackage(3, [&](size_t x, size_t y, const RayPackage& package,
							  vuint32& hitcount, uint64& nano) {
			vuint32 id	 = vuint32(0);
			Vector2fv uv = promote(Vector2f(0, 0));
			vfloat gt	 = vfloat(std::numeric_limits<float>::infinity());

			const auto start = std::chrono::high_resolution_clock::now();
			// Normally it is enough to check till found, but for the sake of benchmarking we check all
			for (size_t i = 0; i < mTriangles.size(); ++i) {
				vfloat lt;
				Vector2fv luv;
				const bfloat hit   = func(mTriangles[i], package, lt, luv);
				const bfloat close = hit & (lt < gt);

				hitcount = blend(hitcount + vuint32(1), hitcount, hit);
				id		 = blend(vuint32(i), id, close);
				gt		 = blend(lt, gt, close);
				uv[0]	 = blend(luv[0], uv[0], close);
				uv[1]	 = blend(luv[1], uv[1], close);
			}
			const auto end = std::chrono::high_resolution_clock::now();
			nano		   = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

			for (size_t k = 0; k < PR_SIMD_BANDWIDTH && x * PR_SIMD_BANDWIDTH + k < WIDTH; ++k) {
				uint32 s = extract(k, hitcount);
				if (s > 0)
					setPoint(x * PR_SIMD_BANDWIDTH + k, y,
							 extract(k, gt), extract(k, id), s,
							 Vector2f(extract(k, uv[0]), extract(k, uv[1])));
			}
		});
	}

private:
	template <typename Func>
	inline void processSingle(int type, Func func)
	{
#ifdef _SANDBOX_NO_SINGLE_TRACE
		return;
#endif
		mStat.reset();
		mPixelsWithHits = 0;

#ifndef _SANDBOX_NO_THREADS
		tbb::static_partitioner partitioner;
		std::mutex mutex;
#endif

		processInfoStart(1, type);

		for (size_t y = 0; y < HEIGHT; ++y) {
			const double fy = 1 - y / static_cast<double>(HEIGHT - 1);

#ifdef _SANDBOX_NO_THREADS
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
					const Ray ray	 = Ray(P, mRayDirection);

					size_t hitcount;
					uint64 nano;
					func(x, y, ray, hitcount, nano);

#ifndef _SANDBOX_NO_THREADS
					mutex.lock();
#endif
					mStat.apply(nano, hitcount);
					if (hitcount > 0)
						++mPixelsWithHits;
#ifndef _SANDBOX_NO_THREADS
					mutex.unlock();
#endif
				}
			}
#ifndef _SANDBOX_NO_THREADS
			,
			partitioner);
#endif
			processInfoUpdate(fy);
		}
		processInfoEnd();

		writeCSVEntry(1, type);
		writeImages(1, type);
	}

	template <typename Func>
	inline void processPackage(int type, Func func)
	{
#ifdef _SANDBOX_NO_PACKAGE_TRACE
		return;
#endif

		mStat.reset();
		mPixelsWithHits = 0;

#ifndef _SANDBOX_NO_THREADS
		tbb::static_partitioner partitioner;
		std::mutex mutex;
#endif

		processInfoStart(PR_SIMD_BANDWIDTH, type);

		const size_t W = WIDTH / PR_SIMD_BANDWIDTH + (WIDTH % PR_SIMD_BANDWIDTH != 0 ? 1 : 0);
		for (size_t y = 0; y < HEIGHT; ++y) {
			const double fy = 1 - y / static_cast<double>(HEIGHT - 1);

#ifdef _SANDBOX_NO_THREADS
			tbb::blocked_range<size_t> r(0, W);
			{
#else
		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, W),
			[&](const tbb::blocked_range<size_t>& r) {
#endif
				for (size_t tx = r.begin(); tx != r.end(); ++tx) {
					RayPackage package;
					for (size_t k = 0; k < PR_SIMD_BANDWIDTH && tx * PR_SIMD_BANDWIDTH + k < WIDTH; ++k) {
						const double fx = (tx * PR_SIMD_BANDWIDTH + k) / static_cast<double>(WIDTH - 1);

						const Vector3f P = project(fx, fy);
						const Ray ray	 = Ray(P, mRayDirection);
						insertIntoRayPackage(k, package, ray);
					}

					vuint32 hitcount = vuint32(0);
					uint64 nano		 = 0;
					func(tx, y, package, hitcount, nano);

#ifndef _SANDBOX_NO_THREADS
					mutex.lock();
#endif
					uint32 fullhitcount = 0;
					for (size_t k = 0; k < PR_SIMD_BANDWIDTH && tx * PR_SIMD_BANDWIDTH + k < WIDTH; ++k) {
						const uint32 hit = extract(k, hitcount);
						if (hit > 0)
							++mPixelsWithHits;
						fullhitcount += hit;
					}
					mStat.apply(nano, fullhitcount);
#ifndef _SANDBOX_NO_THREADS
					mutex.unlock();
#endif
				}
			}
#ifndef _SANDBOX_NO_THREADS
			,
			partitioner);
#endif
			processInfoUpdate(fy);
		}
		processInfoEnd();

		writeCSVEntry(PR_SIMD_BANDWIDTH, type);
		writeImages(PR_SIMD_BANDWIDTH, type);
	}

	inline void processInfoStart(size_t packageSize, int type) const
	{
		std::string typeString;
		switch (type) {
		default:
		case 1:
			typeString = "Line";
			break;
		case 2:
			typeString = "Segment";
			break;
		case 3:
			typeString = "Extra";
			break;
		}
		std::cout << mName << "[" << packageSize << "](" << typeString << ")> " << mTriangles.size() << " [" << (int)mDegree << "°] TRI AREA: "
				  << std::setprecision(-1) << std::defaultfloat << triArea();
		if (mHitRate >= 0)
			std::cout << " HIT RATE: " << mHitRate << "%";
		std::cout << std::endl;
	}

	inline void processInfoUpdate(float y) const
	{
		const double percentage = (1 - y) * 100.0;
		std::cout << "\r" << std::setprecision(2) << std::setw(6) << std::fixed << percentage << "%" << std::flush;
	}

	inline void processInfoEnd() const
	{
		std::cout << "\r";
		double avgHits = 0.0;
		if (mPixelsWithHits > 0)
			avgHits = mStat.TotalHits / (double)mPixelsWithHits;

		const auto avgNano = mStat.TotalTime / (WIDTH * HEIGHT * mTriangles.size());
		std::cout << "=> FullT: "
				  << prettytime(mStat.TotalTime) << std::endl
				  << "   AvgT:  "
				  << prettytime(avgNano) << std::endl
				  << "   AvgH:  "
				  << std::setprecision(8) << std::fixed << avgHits << std::endl
				  << "   AvgHR: "
				  << std::setprecision(8) << std::fixed << 100 * avgHits / mTriangles.size() << "%" << std::endl;
	}

	inline void writeCSVEntry(size_t packageSize, int type) const
	{
		mCSV.writeEntry(mName, packageSize, type,
						mTriangles.size(), (int)mDegree, mHitRate,
						triArea(), mPixelsWithHits / static_cast<float>(WIDTH * HEIGHT),
						mStat);
	}

	inline void writeImages(size_t packageSize, int type) const
	{
		std::stringstream path_stream;
		path_stream << DIR << "/" << mName << "_" << packageSize << "_" << type << "_" << mTriangles.size() << "_" << (int)mDegree;
		if (mHitRate >= 0)
			path_stream << "_" << mHitRate;

		save_normalized_gray(path_stream.str() + "_hits.png", WIDTH, HEIGHT, mHits);
		if (type >= 2) {
			save_normalized_gray(path_stream.str() + "_t.png", WIDTH, HEIGHT, mData);
			save_normalized_gray(path_stream.str() + "_id.png", WIDTH, HEIGHT, mIds);
		}
		if (type >= 3)
			save_image(path_stream.str() + "_uvs.png", WIDTH, HEIGHT, mUvs, 3);
	}

	// Assuming all triangles are same size
	inline float triArea() const
	{
		if (mTriangles.empty())
			return 0;
		else
			return (mTriangles[0].P1 - mTriangles[0].P0).cross(mTriangles[0].P2 - mTriangles[0].P0).norm() / 2;
	}

	inline Vector3f project(double nx, double ny) const
	{
		const double sum = nx + ny;
		const double u	 = nx / sum;
		const double v	 = ny / sum;
		const double w	 = 1 - u - v;
		if (std::isinf(u) || std::isinf(v))
			return P0 - mRayDirection;
		else
			return P0 * w + P1 * u + P2 * v - mRayDirection;
	};

	inline void setPoint(size_t x, size_t y,
						 size_t hitcount)
	{
		mHits[y * WIDTH + x] = hitcount;
	}

	inline void setPoint(size_t x, size_t y,
						 float t, size_t id, size_t hitcount)
	{
		setPoint(x, y, hitcount);
		mData[y * WIDTH + x] = t;
		mIds[y * WIDTH + x]	 = id;
	}

	inline void setPoint(size_t x, size_t y,
						 float t, size_t id, size_t hitcount, const Vector2f& uv)
	{
		setPoint(x, y, t, id, hitcount);
		mUvs[(y * WIDTH + x) * 3 + 0] = uv[0];
		mUvs[(y * WIDTH + x) * 3 + 1] = uv[1];
	}
};

static void tri_mt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_mt", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLineMT(ray, tri.P0, tri.P1, tri.P2);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectMT(ray, tri.P0, tri.P1, tri.P2, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectMT(ray, tri.P0, tri.P1, tri.P2, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLineMT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v());
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectMT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectMT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t, uv);
	});
}

static void tri_wt(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_wt", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLineWT(ray, tri.P0, tri.P1, tri.P2);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectWT(ray, tri.P0, tri.P1, tri.P2, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectWT(ray, tri.P0, tri.P1, tri.P2, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLineWT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v());
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectWT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectWT(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t, uv);
	});
}

static void tri_bw9(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_bw9", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLineBW9(ray, tri.BW9Mat, tri.BW9FixedColumn);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectBW9(ray, tri.BW9Mat, tri.BW9FixedColumn, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectBW9(ray, tri.BW9Mat, tri.BW9FixedColumn, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLineBW9(ray, tri.BW9Mat, tri.BW9FixedColumn);
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectBW9(ray, tri.BW9Mat, tri.BW9FixedColumn, t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectBW9(ray, tri.BW9Mat, tri.BW9FixedColumn, t, uv);
	});
}

static void tri_bw12(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_bw12", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLineBW12(ray, tri.BW12Mat);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectBW12(ray, tri.BW12Mat, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectBW12(ray, tri.BW12Mat, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLineBW12(ray, tri.BW12Mat);
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectBW12(ray, tri.BW12Mat, t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectBW12(ray, tri.BW12Mat, t, uv);
	});
}

static void tri_pi(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_pi", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLinePI(ray, tri.P0, tri.P1, tri.P2);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI(ray, tri.P0, tri.P1, tri.P2, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI(ray, tri.P0, tri.P1, tri.P2, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLinePI(ray, tri.P0_v(), tri.P1_v(), tri.P2_v());
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t, uv);
	});
}

static void tri_pi_mem(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_pi_mem", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLinePI_Mem(ray, tri.P0, tri.P1, tri.P2, tri.M0, tri.M1, tri.M2);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI_Mem(ray, tri.P0, tri.P1, tri.P2, tri.M0, tri.M1, tri.M2, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI_Mem(ray, tri.P0, tri.P1, tri.P2, tri.M0, tri.M1, tri.M2, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLinePI_Mem(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), tri.M0_v(), tri.M1_v(), tri.M2_v());
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI_Mem(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), tri.M0_v(), tri.M1_v(), tri.M2_v(), t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI_Mem(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), tri.M0_v(), tri.M1_v(), tri.M2_v(), t, uv);
	});
}

static void tri_pi_off(const std::string& suffix, const std::vector<Triangle>& triangles, float degree, int hitrate, CSVStream& csv)
{
	PR_PROFILE_THIS;
	TriangleCheck check(suffix + "_pi_off", triangles, degree, hitrate, csv);
	check.processSingle1([](const Triangle& tri, const Ray& ray) {
		return TriangleIntersection::intersectLinePI_Off(ray, tri.P0, tri.P1, tri.P2);
	});
	check.processSingle2([](const Triangle& tri, const Ray& ray, float& t) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI_Off(ray, tri.P0, tri.P1, tri.P2, t);
	});
	check.processSingle3([](const Triangle& tri, const Ray& ray, float& t, Vector2f& uv) {
		t = std::numeric_limits<float>::infinity();
		return TriangleIntersection::intersectPI_Off(ray, tri.P0, tri.P1, tri.P2, t, uv);
	});
	check.processPackage1([](const Triangle& tri, const RayPackage& ray) {
		return TriangleIntersection::intersectLinePI_Off(ray, tri.P0_v(), tri.P1_v(), tri.P2_v());
	});
	check.processPackage2([](const Triangle& tri, const RayPackage& ray, vfloat& t) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI_Off(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t);
	});
	check.processPackage3([](const Triangle& tri, const RayPackage& ray, vfloat& t, Vector2fv& uv) {
		t = vfloat(std::numeric_limits<float>::infinity());
		return TriangleIntersection::intersectPI_Off(ray, tri.P0_v(), tri.P1_v(), tri.P2_v(), t, uv);
	});
}

typedef void (*MAT_FUNC)(const std::string&, const std::vector<Triangle>&, float, int, CSVStream& csv);
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

void precision_test(CSVStream& csv)
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

void hitrate_test(CSVStream& csv)
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

	CSVStream csv(DIR + "/output.csv");
	csv.writeHeader();

	precision_test(csv);
	hitrate_test(csv);
}