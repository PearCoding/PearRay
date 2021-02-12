#include "Environment.h"
#include "Logger.h"
#include "Random.h"
#include "SceneLoadContext.h"
#include "math/Sampling.h"
#include "shader/INodePlugin.h"

namespace PR {

[[maybe_unused]] static inline float smoothstep(float x)
{
	return x * x * (3 - 2 * x);
}

[[maybe_unused]] static inline float smootherstep(float x)
{
	return x * x * x * (x * (x * 6 - 15) + 10);
}

static inline float lerp(float a, float b, float t)
{
	return a * (1 - t) + b * t;
}

// Based on http://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2
class PerlinNode : public FloatScalarNode {
public:
	PerlinNode(float scale, size_t seed, size_t size)
		: FloatScalarNode(0 /*Or all?*/)
		, mScale(scale)
		, mSeed(seed)
	{
		buildTables(size);
	}

	float eval(const ShadingContext& ctx) const override
	{
		const uint32 mask = mGradients.size() - 1;

		// Calculate grid position
		const int xi0 = ((int)std::floor(mScale * ctx.UV(0))) & mask;
		const int yi0 = ((int)std::floor(mScale * ctx.UV(1))) & mask;
		const int zi0 = 0; //((int)std::floor(p.z)) & mask; TODO

		// Calculate nex neighbor
		const int xi1 = (xi0 + 1) & mask;
		const int yi1 = (yi0 + 1) & mask;
		const int zi1 = (zi0 + 1) & mask;

		// Calculate fractional part
		const float tx = mScale * ctx.UV(0) - ((int)std::floor(mScale * ctx.UV(0)));
		const float ty = mScale * ctx.UV(1) - ((int)std::floor(mScale * ctx.UV(1)));
		const float tz = 0; //ctx.UV(2) - ((int)std::floor(ctx.UV(2)));

		// Interpolate
		const float u = smoothstep(tx);
		const float v = smoothstep(ty);
		const float w = smoothstep(tz);

		// Gradients at the corner of the cell
		const Vector3f& c000 = mGradients[permute(xi0, yi0, zi0)];
		const Vector3f& c100 = mGradients[permute(xi1, yi0, zi0)];
		const Vector3f& c010 = mGradients[permute(xi0, yi1, zi0)];
		const Vector3f& c110 = mGradients[permute(xi1, yi1, zi0)];

		const Vector3f& c001 = mGradients[permute(xi0, yi0, zi1)];
		const Vector3f& c101 = mGradients[permute(xi1, yi0, zi1)];
		const Vector3f& c011 = mGradients[permute(xi0, yi1, zi1)];
		const Vector3f& c111 = mGradients[permute(xi1, yi1, zi1)];

		// Generate vectors going from the grid points to p
		const float x0 = tx, x1 = tx - 1;
		const float y0 = ty, y1 = ty - 1;
		const float z0 = tz, z1 = tz - 1;

		const Vector3f p000 = Vector3f(x0, y0, z0);
		const Vector3f p100 = Vector3f(x1, y0, z0);
		const Vector3f p010 = Vector3f(x0, y1, z0);
		const Vector3f p110 = Vector3f(x1, y1, z0);

		const Vector3f p001 = Vector3f(x0, y0, z1);
		const Vector3f p101 = Vector3f(x1, y0, z1);
		const Vector3f p011 = Vector3f(x0, y1, z1);
		const Vector3f p111 = Vector3f(x1, y1, z1);

		// Linear interpolation
		const float a = lerp(c000.dot(p000), c100.dot(p100), u);
		const float b = lerp(c010.dot(p010), c110.dot(p110), u);
		const float c = lerp(c001.dot(p001), c101.dot(p101), u);
		const float d = lerp(c011.dot(p011), c111.dot(p111), u);

		const float e = lerp(a, b, v);
		const float f = lerp(c, d, v);

		return lerp(e, f, w);
	}

	std::string dumpInformation() const override
	{
		std::stringstream sstream;
		sstream << "PerlinNoise (" << mScale << "," << mSeed << ", " << mGradients.size() << ")";
		return sstream.str();
	}

private:
	inline uint32 permute(int x, int y, int z) const
	{
		return mPermutations[mPermutations[mPermutations[x] + y] + z];
	}

	void buildTables(size_t size)
	{
		Random rnd(mSeed);

		mGradients.reserve(size);
		mPermutations.reserve(size * 2);

		// Construct random gradients
		for (size_t i = 0; i < size; ++i) {
			mGradients.emplace_back(Sampling::sphere(rnd.getFloat(), rnd.getFloat()));
			mPermutations.emplace_back(i);
		}

		// Apply permutations
		for (unsigned i = 0; i < size; ++i)
			std::swap(mPermutations[i], mPermutations[rnd.get32(0, size)]);

		// Extend the permutation table in the index range [s:2s]
		for (unsigned i = 0; i < size; ++i)
			mPermutations[size + i] = mPermutations[i];
	}

	const float mScale; // ~ Frequency

	const size_t mSeed;

	std::vector<Vector3f> mGradients;
	std::vector<uint32> mPermutations;
};

constexpr size_t DEFAULT_SEED = 4207337;
constexpr size_t DEFAULT_SIZE = 256;

class PerlinNodePlugin : public INodePlugin {
public:
	std::shared_ptr<INode> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const float scale = ctx.parameters().getNumber(0, 1.0f);
		const size_t seed = ctx.parameters().getUInt(1, DEFAULT_SEED);
		const size_t size = ctx.parameters().getUInt(2, DEFAULT_SIZE);
		return std::make_shared<PerlinNode>(scale, seed, size);
	}

	const std::vector<std::string>& getNames() const override
	{
		const static std::vector<std::string> names({ "perlin" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Perlin Noise Node", "A node generating random numbers based on the perlin noise function")
			.Identifiers(getNames())
			.Inputs()
			.Number("scale", "Scale or frequency of the perlin noise", 1.0f)
			.Int("seed", "Seed", DEFAULT_SEED)
			.Int("size", "Size of the perlin table", DEFAULT_SIZE)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::PerlinNodePlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)