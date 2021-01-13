#include "Environment.h"
#include "Logger.h"
#include "Profiler.h"
#include "ResourceManager.h"
#include "SceneLoadContext.h"
#include "entity/GeometryDev.h"
#include "entity/GeometryRepr.h"
#include "entity/IEntity.h"
#include "entity/IEntityPlugin.h"
#include "geometry/GeometryPoint.h"
#include "math/Projection.h"
#include "math/SplitSample.h"
#include "math/Tangent.h"
#include "mesh/MeshBase.h"

#include <filesystem>

namespace PR {

class Mesh {
public:
	explicit Mesh(const std::shared_ptr<MeshBase>& mesh)
		: mScene()
		, mGeometry()
		, mBase(mesh)
		, mWasGenerated(false)
	{
		// Build mixed indices
		// TODO: Why not inside MeshBase?
		if (!mesh->isOnlyTriangular() && !mesh->isOnlyQuadrangular()) {
			const auto& origIndices = mesh->vertexComponentIndices(MeshComponent::Vertex);
			mMixedIndices.reserve(mesh->faceCount() * 4);
			size_t iind = 0;
			for (size_t i = 0; i < mesh->faceCount(); ++i) {
				const uint32 num = mesh->faceVertexCount(i);

				mMixedIndices.push_back(origIndices[iind++]);
				mMixedIndices.push_back(origIndices[iind++]);
				mMixedIndices.push_back(origIndices[iind++]);

				if (num == 4)
					mMixedIndices.push_back(origIndices[iind++]);
				else
					mMixedIndices.push_back(mMixedIndices.back()); // Repeat previous entry
			}
		}
	}

	~Mesh()
	{
		rtcReleaseGeometry(mGeometry);
		rtcReleaseScene(mScene);
	}

	inline std::tuple<Vector3f, Vector3f> interpolateTangent(uint32 primID, const Vector2f& param) const
	{
		RTCInterpolateArguments args;
		args.bufferType = RTC_BUFFER_TYPE_VERTEX;
		args.bufferSlot = 0;
		args.geometry	= mGeometry;
		args.primID		= primID;
		args.u			= param(0);
		args.v			= param(1);
		args.valueCount = 3;

		RTC_ALIGN(16)
		float Pu[3];
		RTC_ALIGN(16)
		float Pv[3];
		args.P		 = nullptr;
		args.dPdu	 = Pu;
		args.dPdv	 = Pv;
		args.ddPdudu = nullptr;
		args.ddPdudv = nullptr;
		args.ddPdvdv = nullptr;

		rtcInterpolate(&args);

		return { Vector3f(Pu[0], Pu[1], Pu[2]), Vector3f(Pv[0], Pv[1], Pv[2]) };
	}

	inline RTCScene generate(const RTCDevice& dev)
	{
		if (!mWasGenerated) {
			setupOriginal(dev);
			mWasGenerated = true;
		}

		return mScene;
	}

	inline MeshBase* base() const { return mBase.get(); }
	inline RTCScene scene() const { return mScene; }

private:
	inline void setupOriginal(const RTCDevice& dev)
	{
		mGeometry = rtcNewGeometry(dev, mBase->isOnlyTriangular() ? RTC_GEOMETRY_TYPE_TRIANGLE : RTC_GEOMETRY_TYPE_QUAD);

		// TODO: Make sure the internal mesh buffer is proper aligned at the end
		rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, mBase->vertexComponent(MeshComponent::Vertex).data(), 0, sizeof(float) * 3, mBase->vertexComponent(MeshComponent::Vertex).size() / 3);
		if (mBase->isOnlyTriangular()) {
			rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, mBase->vertexComponentIndices(MeshComponent::Vertex).data(), 0, sizeof(uint32) * 3, mBase->faceCount());
		} else {
			if (mBase->isOnlyQuadrangular())
				rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT4, mBase->vertexComponentIndices(MeshComponent::Vertex).data(), 0, sizeof(uint32) * 4, mBase->faceCount());
			else
				rtcSetSharedGeometryBuffer(mGeometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT4, mMixedIndices.data(), 0, sizeof(uint32) * 4, mBase->faceCount());
		}
		rtcCommitGeometry(mGeometry);

		mScene = rtcNewScene(dev);

		rtcAttachGeometry(mScene, mGeometry);

		rtcSetSceneFlags(mScene, RTC_SCENE_FLAG_COMPACT | RTC_SCENE_FLAG_ROBUST);
		rtcSetSceneBuildQuality(mScene, RTC_BUILD_QUALITY_HIGH);
		rtcCommitScene(mScene);
	}

	RTCScene mScene;
	RTCGeometry mGeometry;
	std::shared_ptr<MeshBase> mBase;
	bool mWasGenerated;

	// Following buffer will be used to construct mixed indices as used inside Embree
	std::vector<uint32> mMixedIndices;
};

template <bool HasUV, bool CustomNormal>
class MeshEntity : public IEntity {
public:
	ENTITY_CLASS

	MeshEntity(const std::string& name, const Transformf& transform,
			   const std::shared_ptr<Mesh>& mesh,
			   const std::vector<uint32>& materials,
			   uint32 lightID)
		: IEntity(lightID, name, transform)
		, mMaterials(materials)
		, mMesh(mesh)
		, mBoundingBox(mesh->base()->constructBoundingBox())
	{
	}
	virtual ~MeshEntity() {}

	std::string type() const override
	{
		return "mesh";
	}

	virtual float localSurfaceArea(uint32 id) const override
	{
		return mMesh->base()->surfaceArea(id, Eigen::Affine3f::Identity());
	}

	bool isCollidable() const override
	{
		return true;
	}

	float collisionCost() const override
	{
		return (float)mMesh->base()->faceCount();
	}

	virtual BoundingBox localBoundingBox() const override
	{
		return mBoundingBox;
	}

	GeometryRepr constructGeometryRepresentation(const GeometryDev& dev) const override
	{
		RTCScene original = mMesh->generate(dev);

		RTCGeometry geom = rtcNewGeometry(dev, RTC_GEOMETRY_TYPE_INSTANCE);
		rtcSetGeometryInstancedScene(geom, original);

		const Transformf& M = transform();
		rtcSetGeometryTransform(geom, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, M.data());
		rtcCommitGeometry(geom);

		return GeometryRepr(geom);
	}

	// TODO: Better sampling pdf for sampleParameterPointPDF
	EntitySamplePoint sampleParameterPoint(const Vector2f& rnd) const override
	{
		PR_PROFILE_THIS;
		const SplitSample2D split(rnd, 0, mMesh->base()->faceCount());
		const uint32 faceID = split.integral1();

		const Face face	  = mMesh->base()->getFace(faceID);
		const float pdf_a = 1.0f / (mMesh->base()->faceCount() * face.surfaceArea() * volumeScalefactor());

		Vector2f uv;
		if (!face.IsQuad)
			uv = Triangle::sample(Vector2f(split.uniform1(), split.uniform2()));
		else
			uv = Vector2f(split.uniform1(), split.uniform2());

		return EntitySamplePoint(transform() * face.interpolateVertices(uv), uv, faceID, pdf_a);
	}

	void provideGeometryPointLocal(const EntityGeometryQueryPoint& query,
								   GeometryPoint& pt) const
	{
		Face face = mMesh->base()->getFace(query.PrimitiveID);
		if constexpr (CustomNormal) {
			pt.N = face.interpolateNormals(query.UV);
			if constexpr (HasUV)
				face.tangentFromUV(pt.N, pt.Nx, pt.Ny);
			else
				Tangent::unnormalized_frame(pt.N, pt.Nx, pt.Ny);
		} else {
			auto tuple = mMesh->interpolateTangent(query.PrimitiveID, query.UV);
			pt.Nx	   = std::get<0>(tuple);
			pt.Ny	   = std::get<1>(tuple);
			pt.N	   = pt.Nx.cross(pt.Ny);
		}

		if constexpr (HasUV)
			pt.UV = face.interpolateUVs(query.UV);
		else
			pt.UV = query.UV;

		pt.MaterialID = face.MaterialSlot < mMaterials.size() ? mMaterials.at(face.MaterialSlot) : PR_INVALID_ID;
	}

	void provideGeometryPoint(const EntityGeometryQueryPoint& query,
							  GeometryPoint& pt) const override
	{
		PR_PROFILE_THIS;

		// Local
		provideGeometryPointLocal(query, pt);

		// Global
		pt.N  = normalMatrix() * pt.N;
		pt.Nx = normalMatrix() * pt.Nx;
		pt.Ny = normalMatrix() * pt.Ny;

		pt.N.normalize();
		pt.Nx.normalize();
		pt.Ny.normalize();

		pt.PrimitiveID = query.PrimitiveID;
		pt.EmissionID  = emissionID();
		pt.DisplaceID  = PR_INVALID_ID;
	}

private:
	const std::vector<uint32> mMaterials;
	const std::shared_ptr<Mesh> mMesh;
	const BoundingBox mBoundingBox;
};

class MeshEntityPlugin : public IEntityPlugin {
public:
	std::unordered_map<MeshBase*, std::shared_ptr<Mesh>> mOriginalMesh;

	std::shared_ptr<IEntity> create(const std::string&, const SceneLoadContext& ctx) override
	{
		const ParameterGroup& params = ctx.parameters();

		std::string name	  = params.getString("name", "__unnamed__");
		std::string mesh_name = params.getString("mesh", "");

		const std::vector<uint32> materials = ctx.lookupMaterialIDArray(params.getParameter("materials"));
		const uint32 emsID					= ctx.lookupEmissionID(params.getParameter("emission"));
		const bool customNormal				= !params.getBool("ignore_normals", false);

		if (!ctx.hasMesh(mesh_name)) {
			PR_LOG(L_ERROR) << "Could not find a mesh named " << mesh_name << std::endl;
			return nullptr;
		} else {
			auto mesh = ctx.getMesh(mesh_name);
			std::shared_ptr<Mesh> mesh_p;
			if (mOriginalMesh.count(mesh.get()) > 0) {
				mesh_p = mOriginalMesh.at(mesh.get());
			} else {
				mesh_p					  = std::make_shared<Mesh>(mesh);
				mOriginalMesh[mesh.get()] = mesh_p;
			}

			if (customNormal && mesh->features() & MeshFeature::Normal) {
				if (mesh->features() & MeshFeature::Texture)
					return std::make_shared<MeshEntity<true, true>>(name, ctx.transform(),
																	mesh_p,
																	materials, emsID);
				else
					return std::make_shared<MeshEntity<false, true>>(name, ctx.transform(),
																	 mesh_p,
																	 materials, emsID);
			} else {
				if (mesh->features() & MeshFeature::Texture)
					return std::make_shared<MeshEntity<true, false>>(name, ctx.transform(),
																	 mesh_p,
																	 materials, emsID);
				else
					return std::make_shared<MeshEntity<false, false>>(name, ctx.transform(),
																	  mesh_p,
																	  materials, emsID);
			}
		}
	}

	const std::vector<std::string>& getNames() const override
	{
		static std::vector<std::string> names({ "mesh" });
		return names;
	}

	PluginSpecification specification(const std::string&) const override
	{
		return PluginSpecificationBuilder("Mesh Entity", "A mesh of triangles and quads")
			.Identifiers(getNames())
			.Inputs()
			.MeshReference("mesh", "Mesh")
			.MaterialReferenceV({ "material", "materials" }, "Material")
			.EmissionReference("emission", "Emission", true)
			.Bool("ignore_normals", "Ignore user given normals", false)
			.Specification()
			.get();
	}
};
} // namespace PR

PR_PLUGIN_INIT(PR::MeshEntityPlugin, _PR_PLUGIN_NAME, PR_PLUGIN_VERSION)