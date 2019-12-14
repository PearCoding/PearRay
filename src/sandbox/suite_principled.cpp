#include "Logger.h"
#include "image_io.h"

#include "math/Microfacet.h"

#include <boost/filesystem.hpp>

using namespace PR;
namespace bf = boost::filesystem;

constexpr size_t IRES = 500;

Vector2f toSlopeSpace(const Vector3f& v)
{
	/*if (v(2) > PR_EPSILON)
		return Vector2f(-v(0) / v(2), -v(1) / v(2));
	else*/
	return Vector2f(v(0), v(1));
}

// V in tangent space
void produce_vndf_image(const std::string& path, const Vector3f& V, float alpha_x, float alpha_y)
{
	alpha_x = std::max(0.0001f, alpha_x);
	alpha_y = std::max(0.0001f, alpha_y);

	/*const Vector3f N  = Vector3f(0, 0, 1);
	const Vector3f Nx = Vector3f(1, 0, 0);
	const Vector3f Ny = Vector3f(0, 1, 0);*/

	std::vector<float> data(IRES * IRES, 0.0f);
	std::vector<float> pdfs(IRES * IRES, 0.0f);
	std::vector<float> dots(IRES * IRES, 0.0f);

	float du = 1.0f / (IRES - 1);
	for (size_t y = 0; y < IRES; ++y) {
		for (size_t x = 0; x < IRES; ++x) {
			float u = x * du;
			float v = y * du;

			float pdf;
			Vector3f Nh = Microfacet::sample_ggx_vndf(u, v, V, alpha_x, alpha_y, pdf);

			// Slope space
			Vector2f slope = toSlopeSpace(Nh);
			//std::cout << slope(0) << " " << slope(1) << std::endl;
			float fx = std::max(0.0f, std::min(1.0f, (1 + slope(0)) / 2));
			float fy = std::max(0.0f, std::min(1.0f, (1 + slope(1)) / 2));

			size_t px = fx * (IRES - 1);
			size_t py = fy * (IRES - 1);

			data[py * IRES + px] += 1.0f;

			// Pdf
			pdfs[y * IRES + x] = pdf;

			// Nh dot V
			dots[y * IRES + x] = Nh.dot(V);
		}
	}

	save_normalized_gray(path + ".exr", IRES, IRES, data);
	save_gray(path + "_pdf.exr", IRES, IRES, pdfs);
	save_gray(path + "_dot.exr", IRES, IRES, dots);
}

void suite_principled()
{
	bf::create_directory("results/principled");

	produce_vndf_image("results/principled/n_x0_0_y0_0", Vector3f(0, 0, 1), 0.0f, 0.0f);
	produce_vndf_image("results/principled/n_x0_5_y0_5", Vector3f(0, 0, 1), 0.5f, 0.5f);
	produce_vndf_image("results/principled/n_x1_0_y1_0", Vector3f(0, 0, 1), 1.0f, 1.0f);
	produce_vndf_image("results/principled/n_x0_0_y0_5", Vector3f(0, 0, 1), 0.0f, 0.5f);
	produce_vndf_image("results/principled/n_x0_5_y1_0", Vector3f(0, 0, 1), 0.5f, 1.0f);
	produce_vndf_image("results/principled/n45_x0_0_y0_0", Vector3f(0, 1, 1).normalized(), 0.0f, 0.0f);
	produce_vndf_image("results/principled/n45_x0_5_y0_5", Vector3f(0, 1, 1).normalized(), 0.5f, 0.5f);
	produce_vndf_image("results/principled/n45_x1_0_y1_0", Vector3f(0, 1, 1).normalized(), 1.0f, 1.0f);
	produce_vndf_image("results/principled/n45_x0_0_y0_5", Vector3f(0, 1, 1).normalized(), 0.0f, 0.5f);
	produce_vndf_image("results/principled/n45_x0_5_y1_0", Vector3f(0, 1, 1).normalized(), 0.5f, 1.0f);
	produce_vndf_image("results/principled/n60_x0_0_y0_0", Vector3f(0.75f, 0, 0.5f).normalized(), 0.0f, 0.0f);
	produce_vndf_image("results/principled/n60_x0_5_y0_5", Vector3f(0.75f, 0, 0.5f).normalized(), 0.5f, 0.5f);
	produce_vndf_image("results/principled/n60_x1_0_y1_0", Vector3f(0.75f, 0, 0.5f).normalized(), 1.0f, 1.0f);
	produce_vndf_image("results/principled/n60_x0_0_y0_5", Vector3f(0.75f, 0, 0.5f).normalized(), 0.0f, 0.5f);
	produce_vndf_image("results/principled/n60_x0_5_y1_0", Vector3f(0.75f, 0, 0.5f).normalized(), 0.5f, 1.0f);
}