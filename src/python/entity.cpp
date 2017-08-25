#include "entity/Entity.h"
#include "entity/RenderEntity.h"
#include "material/Material.h"
#include "ray/Ray.h"
#include "sampler/Sampler.h"
#include "shader/FacePoint.h"

#include "pypearray.h"

using namespace PR;

namespace PRPY {
class EntityWrap : public Entity {
public:
	using Entity::Entity;

	inline virtual void onFreeze() override
	{
		PYBIND11_OVERLOAD(void, Entity, onFreeze);
	}
};

class RenderEntityWrap : public RenderEntity {
public:
	using RenderEntity::RenderEntity;

	bool isLight() const override
	{
		PYBIND11_OVERLOAD_PURE(bool, RenderEntity, isLight);
	}

	float surfaceArea(Material* m) const override
	{
		PYBIND11_OVERLOAD_PURE(float, RenderEntity, surfaceArea, m);
	}

	bool isCollidable() const override
	{
		PYBIND11_OVERLOAD(bool, RenderEntity, isCollidable);
	}

	float collisionCost() const override
	{
		PYBIND11_OVERLOAD(float, RenderEntity, collisionCost);
	}

	BoundingBox localBoundingBox() const override
	{
		PYBIND11_OVERLOAD_PURE(BoundingBox, RenderEntity, localBoundingBox);
	}

    RenderEntity::Collision checkCollision(const Ray& ray) const override
    {
        PYBIND11_OVERLOAD_PURE(RenderEntity::Collision, RenderEntity, checkCollision, ray);
    }

    FacePointSample sampleFacePoint(const Eigen::Vector3f& rnd) const override
    {
        PYBIND11_OVERLOAD_PURE(FacePointSample, RenderEntity, sampleFacePoint, rnd);
    }
};

void setup_entity(py::module& m)
{
	py::class_<Entity, EntityWrap, std::shared_ptr<Entity>>(m, "Entity")
		.def(py::init<uint32, std::string>())
		.def_property_readonly("id", &Entity::id)
		.def_property("name", &Entity::name, &Entity::setName)
		.def_property_readonly("type", &Entity::type)
		.def_property("flags", &Entity::flags, &Entity::setFlags)
		.def_property("position", &Entity::position, &Entity::setPosition)
		.def_property("scale", &Entity::scale, &Entity::setScale)
		.def_property("rotation", &Entity::rotation, &Entity::setRotation)
		.def_property_readonly("transform", &Entity::transform)
		.def_property_readonly("invTransform", &Entity::invTransform)
		.def_property_readonly("directionMatrix", &Entity::directionMatrix)
		.def_property_readonly("invDirectionMatrix", &Entity::invDirectionMatrix)
		.def("__str__", &Entity::toString)
		.def("freeze", &Entity::freeze)
		.def_property_readonly("frozen", &Entity::isFrozen)
		.def("onFreeze", &Entity::onFreeze)
		.def("invalidateCache", &Entity::invalidateCache)
		.def("dumpInformation", &Entity::dumpInformation);

	py::enum_<EntityFlags>(m, "EntityFlags")
		.value("DEBUG", EF_Debug)
		.value("LOCALAREA", EF_LocalArea);

	auto re = py::class_<RenderEntity, RenderEntityWrap, std::shared_ptr<RenderEntity>, Entity>(m, "RenderEntity");
	re.def(py::init<uint32, std::string>())
		.def("isLight", &RenderEntity::isLight)
		.def("surfaceArea", &RenderEntity::surfaceArea)
		.def("isCollidable", &RenderEntity::isCollidable)
		.def("collisionCost", &RenderEntity::collisionCost)
		.def("localBoundingBox", &RenderEntity::localBoundingBox)
		.def("worldBoundingBox", &RenderEntity::worldBoundingBox)
		.def("checkCollision", &RenderEntity::checkCollision)
		.def("sampleFacePoint", &RenderEntity::sampleFacePoint);
	
	py::class_<RenderEntity::Collision>(re, "Collision")
		.def_readwrite("Successful", &RenderEntity::Collision::Successful)
		.def_readwrite("Point", &RenderEntity::Collision::Point);

	py::class_<RenderEntity::FacePointSample>(re, "FacePointSample")
		.def_readwrite("PDF_A", &RenderEntity::FacePointSample::PDF_A)
		.def_readwrite("Point", &RenderEntity::FacePointSample::Point);
}
}