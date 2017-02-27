#include "pymath.h"
#include "ray/Ray.h"

using namespace PR;
namespace PRPY
{
    class RayWrap : public Ray, public bpy::wrapper<Ray>
    {
    public:
        RayWrap() : Ray() {}
        RayWrap(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir, uint32 depth=0,
			float time=0, uint8 wavelength=0, uint16 flags=0) :
            Ray(px, py, to3D(pos), to3D(dir), depth, time, wavelength, flags) {}
        RayWrap(const boost::reference_wrapper<const PR::Ray>::type& other) : Ray(other) {}

        inline void setStartPosition_Py(const bpy::object& p) { setStartPosition(to3D(p)); }
		inline bpy::object startPosition_Py() const { return convert3D(startPosition()); }

		inline void setDirection_Py(const bpy::object& p) { setDirection(to3D(p)); }
		inline bpy::object direction_Py() const { return convert3D(direction()); }

        inline Ray next_Py(const bpy::object& pos, const bpy::object& dir) const {
            return next(to3D(pos), to3D(dir));
        }

		static inline Ray safe_Py(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir,
			uint32 depth, float time, uint8 wavelength, uint16 flags) {
            return safe(px, py, to3D(pos), to3D(dir), depth, time, wavelength, flags);
        }

		static inline Ray safe_Py(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir,
			uint32 depth, float time, uint8 wavelength) {
            return safe_Py(px, py, pos, dir, depth, time, wavelength, 0);
        }

		static inline Ray safe_Py(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir,
			uint32 depth, float time) {
            return safe_Py(px, py, pos, dir, depth, time, 0);
        }

		static inline Ray safe_Py(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir,
			uint32 depth) {
            return safe_Py(px, py, pos, dir, depth, 0);
        }

		static inline Ray safe_Py(uint32 px, uint32 py, const bpy::object& pos, const bpy::object& dir) {
            return safe_Py(px, py, pos, dir, 0);
        }
    };

    void setup_ray()
    {
        bpy::class_<RayWrap>("Ray",
            bpy::init<uint32,uint32, bpy::object, bpy::object, bpy::optional<uint32, float, uint8, uint16> >())
        .add_property("startPosition", &RayWrap::startPosition_Py, &RayWrap::setStartPosition_Py)
        .add_property("direction", &RayWrap::direction_Py, &RayWrap::setDirection_Py)
        .add_property("pixelX", &Ray::pixelX, &Ray::setPixelX)
        .add_property("pixelY", &Ray::pixelY, &Ray::setPixelY)
        .add_property("depth", &Ray::depth, &Ray::setDepth)
        .add_property("time", &Ray::time, &Ray::setTime)
        .add_property("wavelength", &Ray::wavelength, &Ray::setWavelength)
        .add_property("flags", &Ray::flags, &Ray::setFlags)

        .def("next", &RayWrap::next_Py)
        .add_property("safe", bpy::make_function(
            (Ray (*)(uint32, uint32, const bpy::object&, const bpy::object&, uint32, float, uint8, uint16))&RayWrap::safe_Py))
        .add_property("safe", bpy::make_function(
            (Ray (*)(uint32, uint32, const bpy::object&, const bpy::object&, uint32, float, uint8))&RayWrap::safe_Py))
        .add_property("safe", bpy::make_function(
            (Ray (*)(uint32, uint32, const bpy::object&, const bpy::object&, uint32, float))&RayWrap::safe_Py))
        .add_property("safe", bpy::make_function(
            (Ray (*)(uint32, uint32, const bpy::object&, const bpy::object&, uint32))&RayWrap::safe_Py))
        .add_property("safe", bpy::make_function(
            (Ray (*)(uint32, uint32, const bpy::object&, const bpy::object&))&RayWrap::safe_Py))
        ;

        bpy::enum_<RayFlags>("RayFlags")
        .value("Light", RF_Light)
        .value("Debug", RF_Debug)
        ;
    }
}