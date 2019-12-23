#include "Environment.h"
#include "Logger.h"
#include "image_io.h"
#include "material/IMaterial.h"
#include "material/MaterialManager.h"
#include "math/Spherical.h"
#include "renderer/RenderTileSession.h"
#include "shader/ShadingPoint.h"
#include "spectral/SpectrumDescriptor.h"

#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>

using namespace PR;
namespace bf				 = boost::filesystem;
static const std::string DIR = "results/material/";
constexpr size_t WIDTH		 = 200;
constexpr size_t HEIGHT		 = 200;

constexpr float THETA_D = PR_PI / WIDTH;
constexpr float PHI_D   = 2 * PR_PI / HEIGHT;

static ShadingPoint standardSP(const Vector3f& V = Vector3f(0, 0, -1))
{
	ShadingPoint spt;
	spt.Geometry.N	= Vector3f(0, 0, 1);
	spt.Geometry.Nx   = Vector3f(1, 0, 0);
	spt.Geometry.Ny   = Vector3f(0, 1, 0);
	spt.Geometry.P	= Vector3f(0, 0, 0);
	spt.Geometry.UVW  = Vector3f(0.5f, 0.5f, 0);
	spt.N			  = spt.Geometry.N;
	spt.Nx			  = spt.Geometry.Nx;
	spt.Ny			  = spt.Geometry.Ny;
	spt.P			  = spt.Geometry.P;
	spt.Ray.Direction = V.normalized();
	spt.Ray.Origin	= Vector3f(0, 0, 1);
	spt.NdotV		  = spt.Ray.Direction.dot(spt.N);
	spt.Depth2		  = 1;
	return spt;
}

static void handle_material_eval_case(const std::string& name,
									  const std::shared_ptr<IMaterial>& material)
{
	ShadingPoint spt = standardSP();

	std::vector<float> rgb(WIDTH * HEIGHT * 3);
	std::vector<float> pdf(WIDTH * HEIGHT);
	float sum = 0.0f;
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			float theta = THETA_D * x - 0.5f * PR_PI;
			float phi   = PHI_D * y;

			const Vector3f L = Spherical::cartesian(theta, phi);

			MaterialEvalInput in;
			in.Outgoing = L;
			in.NdotL	= std::abs(L.dot(spt.N));
			in.Point	= spt;
			MaterialEvalOutput out;
			material->eval(in, out, RenderTileSession());

			pdf[y * WIDTH + x]			   = out.PDF_S;
			rgb[y * WIDTH * 3 + x * 3 + 0] = out.Weight[0];
			rgb[y * WIDTH * 3 + x * 3 + 1] = out.Weight[1];
			rgb[y * WIDTH * 3 + x * 3 + 2] = out.Weight[2];

			sum += out.Weight[0] * std::abs(std::sin(theta)) * 0.5f * (THETA_D * PHI_D);
		}
	}

	std::cout << "-> " << sum << std::endl;
	if (sum > 1.0f)
		std::cout << "WARNING: Not energy conserving!" << std::endl;

	save_image(DIR + name + ".exr", WIDTH, HEIGHT, rgb);
	save_gray(DIR + name + "_pdf.exr", WIDTH, HEIGHT, pdf);
}

static void handle_material_sample_case(const std::string& name,
										const std::shared_ptr<IMaterial>& material)
{
	ShadingPoint spt = standardSP();

	std::vector<float> rgb(WIDTH * HEIGHT * 3);
	std::vector<float> pdf(WIDTH * HEIGHT);
	std::vector<float> dir(WIDTH * HEIGHT);
	for (size_t y = 0; y < HEIGHT; ++y) {
		for (size_t x = 0; x < WIDTH; ++x) {
			MaterialSampleInput in;
			in.Point = spt;
			in.RND   = Vector2f(x / (float)WIDTH, y / (float)HEIGHT);
			MaterialSampleOutput out;
			material->sample(in, out, RenderTileSession());

			pdf[y * WIDTH + x]			   = out.PDF_S;
			rgb[y * WIDTH * 3 + x * 3 + 0] = out.Weight[0];
			rgb[y * WIDTH * 3 + x * 3 + 1] = out.Weight[1];
			rgb[y * WIDTH * 3 + x * 3 + 2] = out.Weight[2];

			size_t dx = std::max<int>(0,
									  std::min<int>(WIDTH - 1,
													(0.5f * out.Outgoing.x() + 0.5f) * WIDTH));
			size_t dy = std::max<int>(0,
									  std::min<int>(HEIGHT - 1,
													(0.5f * out.Outgoing.y() + 0.5f) * HEIGHT));

			dir[dy * WIDTH + dx] += 1;
		}
	}

	save_image(DIR + name + ".exr", WIDTH, HEIGHT, rgb);
	save_gray(DIR + name + "_pdf.exr", WIDTH, HEIGHT, pdf);
	save_normalized_gray(DIR + name + "_dir.exr", WIDTH, HEIGHT, dir);
}

static void handle_material_case(const std::string& name,
								 const std::shared_ptr<IMaterial>& material)
{
	std::cout << ">> " << name << std::endl;
	std::cout << "[EVAL]" << std::endl;
	handle_material_eval_case(name + "_eval", material);
	std::cout << "[SAMPLE]" << std::endl;
	handle_material_sample_case(name + "_samp", material);
}

// Specific materials
static void mat_lambert(Environment& env)
{
	Registry& reg   = env.registry();
	auto manag		= env.materialManager();
	const uint32 id = manag->nextID();
	auto fac		= manag->getFactory("lambert");

	reg.setForObject(RG_MATERIAL, id, "albedo", std::string("white"));
	auto mat = fac->create(id, id, env);
	if (!mat) {
		std::cout << "ERROR: Can not instantiate lambert material!" << std::endl;
		return;
	}
	handle_material_case("lambert", mat);
}

static void mat_orennayar(Environment& env)
{
	Registry& reg   = env.registry();
	auto manag		= env.materialManager();
	const uint32 id = manag->nextID();
	auto fac		= manag->getFactory("orennayar");

	reg.setForObject(RG_MATERIAL, id, "albedo", std::string("white"));
	{
		reg.setForObject(RG_MATERIAL, id, "roughness", 1.0f);
		auto mat = fac->create(id, id, env);
		if (!mat) {
			std::cout << "ERROR: Can not instantiate orennayar material!" << std::endl;
			return;
		}
		handle_material_case("orennayar_1_0", mat);
	}
	{
		reg.setForObject(RG_MATERIAL, id, "roughness", 0.5f);
		auto mat = fac->create(id, id, env);
		if (!mat) {
			std::cout << "ERROR: Can not instantiate orennayar material!" << std::endl;
			return;
		}
		handle_material_case("orennayar_0_5", mat);
	}
	{
		reg.setForObject(RG_MATERIAL, id, "roughness", 0.0f);
		auto mat = fac->create(id, id, env);
		if (!mat) {
			std::cout << "ERROR: Can not instantiate orennayar material!" << std::endl;
			return;
		}
		handle_material_case("orennayar_0_0", mat);
	}
}

static void mat_principled_p(Environment& env, float roughness, float specular)
{
	Registry& reg   = env.registry();
	auto manag		= env.materialManager();
	const uint32 id = manag->nextID();
	auto fac		= manag->getFactory("principled");

	reg.setForObject(RG_MATERIAL, id, "base", std::string("white"));
	reg.setForObject(RG_MATERIAL, id, "roughness", roughness);
	reg.setForObject(RG_MATERIAL, id, "specular", specular);

	auto mat = fac->create(id, id, env);
	if (!mat) {
		std::cout << "ERROR: Can not instantiate orennayar material!" << std::endl;
		return;
	}

	std::stringstream stream;
	stream << "principled_"
		   << (int)roughness << "_" << static_cast<int>((roughness - (int)roughness) * 10) << "_"
		   << (int)specular << "_" << static_cast<int>((specular - (int)specular) * 10);

	handle_material_case(stream.str(), mat);
}

static void mat_principled(Environment& env)
{
	mat_principled_p(env, 0.0f, 0.0f);
	mat_principled_p(env, 0.5f, 0.0f);
	mat_principled_p(env, 1.0f, 0.0f);
	mat_principled_p(env, 0.0f, 0.5f);
	mat_principled_p(env, 0.0f, 1.0f);
	mat_principled_p(env, 0.5f, 0.5f);
	mat_principled_p(env, 1.0f, 0.5f);
	mat_principled_p(env, 0.5f, 1.0f);
	mat_principled_p(env, 1.0f, 1.0f);
}

typedef void (*MAT_FUNC)(Environment&);
static MAT_FUNC s_funcs[] = {
	mat_lambert,
	mat_orennayar,
	mat_principled,
	nullptr
};

void suite_material()
{
	bf::create_directory(DIR);

	Environment env(L"./", SpectrumDescriptor::createSRGBTriplet(), L"./", true);
	for (int i = 0; s_funcs[i]; ++i)
		s_funcs[i](env);
}