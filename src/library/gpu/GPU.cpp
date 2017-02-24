#include "GPU.h"
#include "Logger.h"

#include "spectral/Spectrum.h"

#include <vector>
#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

namespace PR
{
#include "spectral/xyz.inl"

	GPU::GPU()
	{
	}

	GPU::~GPU()
	{
	}

#ifdef PR_WITH_GPU
	const char* GPU::error(cl_int err)
	{
		switch (err)
		{
		// run-time and JIT compiler errors
		case CL_SUCCESS:						return "CL_SUCCESS";
		case CL_DEVICE_NOT_FOUND:				return "CL_DEVICE_NOT_FOUND";
		case CL_DEVICE_NOT_AVAILABLE:			return "CL_DEVICE_NOT_AVAILABLE";
		case CL_COMPILER_NOT_AVAILABLE:			return "CL_COMPILER_NOT_AVAILABLE";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:	return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES:				return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY:				return "CL_OUT_OF_HOST_MEMORY";
		case CL_PROFILING_INFO_NOT_AVAILABLE:	return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case CL_MEM_COPY_OVERLAP:				return "CL_MEM_COPY_OVERLAP";
		case CL_IMAGE_FORMAT_MISMATCH:			return "CL_IMAGE_FORMAT_MISMATCH";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:		return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case CL_BUILD_PROGRAM_FAILURE:			return "CL_BUILD_PROGRAM_FAILURE";
		case CL_MAP_FAILURE:					return "CL_MAP_FAILURE";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:	return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
		case CL_COMPILE_PROGRAM_FAILURE:		return "CL_COMPILE_PROGRAM_FAILURE";
		case CL_LINKER_NOT_AVAILABLE:			return "CL_LINKER_NOT_AVAILABLE";
		case CL_LINK_PROGRAM_FAILURE:			return "CL_LINK_PROGRAM_FAILURE";
		case CL_DEVICE_PARTITION_FAILED:		return "CL_DEVICE_PARTITION_FAILED";
		case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:	return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

		case CL_INVALID_VALUE:					return "CL_INVALID_VALUE";
		case CL_INVALID_DEVICE_TYPE:			return "CL_INVALID_DEVICE_TYPE";
		case CL_INVALID_PLATFORM:				return "CL_INVALID_PLATFORM";
		case CL_INVALID_DEVICE:					return "CL_INVALID_DEVICE";
		case CL_INVALID_CONTEXT:				return "CL_INVALID_CONTEXT";
		case CL_INVALID_QUEUE_PROPERTIES:		return "CL_INVALID_QUEUE_PROPERTIES";
		case CL_INVALID_COMMAND_QUEUE:			return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_HOST_PTR:				return "CL_INVALID_HOST_PTR";
		case CL_INVALID_MEM_OBJECT:				return "CL_INVALID_MEM_OBJECT";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case CL_INVALID_IMAGE_SIZE:				return "CL_INVALID_IMAGE_SIZE";
		case CL_INVALID_SAMPLER:				return "CL_INVALID_SAMPLER";
		case CL_INVALID_BINARY:					return "CL_INVALID_BINARY";
		case CL_INVALID_BUILD_OPTIONS:			return "CL_INVALID_BUILD_OPTIONS";
		case CL_INVALID_PROGRAM:				return "CL_INVALID_PROGRAM";
		case CL_INVALID_PROGRAM_EXECUTABLE:		return "CL_INVALID_PROGRAM_EXECUTABLE";
		case CL_INVALID_KERNEL_NAME:			return "CL_INVALID_KERNEL_NAME";
		case CL_INVALID_KERNEL_DEFINITION:		return "CL_INVALID_KERNEL_DEFINITION";
		case CL_INVALID_KERNEL:					return "CL_INVALID_KERNEL";
		case CL_INVALID_ARG_INDEX:				return "CL_INVALID_ARG_INDEX";
		case CL_INVALID_ARG_VALUE:				return "CL_INVALID_ARG_VALUE";
		case CL_INVALID_ARG_SIZE:				return "CL_INVALID_ARG_SIZE";
		case CL_INVALID_KERNEL_ARGS:			return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_DIMENSION:			return "CL_INVALID_WORK_DIMENSION";
		case CL_INVALID_WORK_GROUP_SIZE:		return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE:			return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_OFFSET:			return "CL_INVALID_GLOBAL_OFFSET";
		case CL_INVALID_EVENT_WAIT_LIST:		return "CL_INVALID_EVENT_WAIT_LIST";
		case CL_INVALID_EVENT:					return "CL_INVALID_EVENT";
		case CL_INVALID_OPERATION:				return "CL_INVALID_OPERATION";
		case CL_INVALID_GL_OBJECT:				return "CL_INVALID_GL_OBJECT";
		case CL_INVALID_BUFFER_SIZE:			return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_MIP_LEVEL:				return "CL_INVALID_MIP_LEVEL";
		case CL_INVALID_GLOBAL_WORK_SIZE:		return "CL_INVALID_GLOBAL_WORK_SIZE";
		case CL_INVALID_PROPERTY:				return "CL_INVALID_PROPERTY";
		case CL_INVALID_IMAGE_DESCRIPTOR:		return "CL_INVALID_IMAGE_DESCRIPTOR";
		case CL_INVALID_COMPILER_OPTIONS:		return "CL_INVALID_COMPILER_OPTIONS";
		case CL_INVALID_LINKER_OPTIONS:			return "CL_INVALID_LINKER_OPTIONS";
		case CL_INVALID_DEVICE_PARTITION_COUNT: return "CL_INVALID_DEVICE_PARTITION_COUNT";

			// extension errors
		case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
		case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
		case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
		case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
		case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
		case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
		default: return "Unknown OpenCL error";
		}
	}
#endif//PR_WITH_GPU

//#define PR_GPU_DEBUG

#ifdef PR_GPU_DEBUG
# define PR_GPU_TYPES (CL_DEVICE_TYPE_CPU)
#else
# define PR_GPU_TYPES (CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU)
#endif

	bool parse_version(const std::string& str, uint32& major, uint32& minor)
	{
		if (str.size() < 10)
			return false;

		if (str.substr(0, 7) != "OpenCL ")
			return false;

		size_t s = str.find_first_of(' ', 7);
		std::string version = str.substr(7, s);
		size_t p = version.find('.');

		if (p == std::string::npos)
			return false;

		std::string majStr = version.substr(0, p);
		std::string minStr = version.substr(p+1);

		major = std::stoi(majStr);
		minor = std::stoi(minStr);

		return true;
	}

	const char* tonemapper_src=
		#include "cl/tonemapper.cl"

	bool GPU::init(const std::string& profile)
	{
#ifndef PR_WITH_GPU
		return false;
#else
		std::vector<cl::Platform> platforms;
		try
		{
			cl::Platform::get(&platforms);
		}
		catch(const cl::Error& err)
		{
			PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Exception: [%s] %s", err.what(), error(err.err()));
			return false;
		}

		if (platforms.empty())
		{
			PR_LOGGER.log(L_Error, M_GPU, "No OpenCL platform found.");
			return false;
		}

		// Select platform / device combo with a point system
		cl_uint maxUnits = 0;
		cl::Device maxUnitsDevice;
		cl_ulong maxAlloc = 0;
		cl::Device maxAllocDevice;
		cl_ulong maxSize = 0;
		cl::Device maxSizeDevice;

		std::vector<std::pair<cl::Platform, cl::Device> > selection;
		for (cl::Platform& plat : platforms)
		{
			std::string platVer = plat.getInfo<CL_PLATFORM_VERSION>();
			PR_LOGGER.logf(L_Info, M_GPU, "OpenCL: %s", platVer.c_str());
			PR_LOGGER.logf(L_Info, M_GPU, "        %s", plat.getInfo<CL_PLATFORM_VENDOR>().c_str());
			PR_LOGGER.logf(L_Debug, M_GPU, "        %s", plat.getInfo<CL_PLATFORM_EXTENSIONS>().c_str());

			uint32 platMaj = 0;
			uint32 platMin = 0;
			if (!parse_version(platVer, platMaj, platMin) || (platMaj < 1 && platMin < 2))
			{
				PR_LOGGER.logf(L_Info, M_GPU, "        NOT USEABLE");
			}
			else
			{
				std::vector<cl::Device> devices;
				plat.getDevices(CL_DEVICE_TYPE_ALL, &devices);
				if (devices.empty())
				{
					PR_LOGGER.logf(L_Info, M_GPU, "        NO DEVICES AVAILABLE");
				}
				else
				{
					for (cl::Device& dev : devices)
					{
						PR_LOGGER.logf(L_Info, M_GPU, "  -> Device: %s", dev.getInfo<CL_DEVICE_NAME>().c_str());

						if (dev.getInfo<CL_DEVICE_AVAILABLE>() == CL_TRUE)
						{
							selection.push_back(std::pair<cl::Platform, cl::Device>(plat, dev));

							cl_uint units = dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
							cl_ulong alloc = dev.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
							cl_ulong size = dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();

							if (maxUnits < units)
							{
								maxUnits = units;
								maxUnitsDevice = dev;
							}

							if (maxAlloc < alloc)
							{
								maxAlloc = alloc;
								maxAllocDevice = dev;
							}

							if (maxSize < size)
							{
								maxSize = size;
								maxSizeDevice = dev;
							}
						}
						else
						{
							PR_LOGGER.logf(L_Info, M_GPU, "             NOT SUPPORTED");
						}
					}
				}
			}
		}

		if (selection.empty())
		{
			PR_LOGGER.log(L_Error, M_GPU, "No valid OpenCL device found.");
			return false;
		}

		uint32 maxPoints = 0;
		for (auto pair : selection)
		{
			uint32 points = 0;
			if (pair.second() == maxUnitsDevice())
				points += 2;
			if (pair.second() == maxAllocDevice())
				points += 1;
			if (pair.second() == maxSizeDevice())
				points += 1;

			cl_device_type type = pair.second.getInfo<CL_DEVICE_TYPE>();
			if (type & CL_DEVICE_TYPE_GPU)
				points += 2;

			if (maxPoints < points)
			{
				mPlatform = pair.first;
				mMainDevice = pair.second;
			}
		}

		// Now create context
		PR_LOGGER.logf(L_Info, M_GPU, "Choosing: %s %s",
			mPlatform.getInfo<CL_PLATFORM_VERSION>().c_str(), mPlatform.getInfo<CL_PLATFORM_VENDOR>().c_str());
		PR_LOGGER.logf(L_Info, M_GPU, "       -> %s",
			mMainDevice.getInfo<CL_DEVICE_NAME>().c_str());

		cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(mPlatform)(), 0};
		try
		{
			cl::Context context(mMainDevice, properties);
			mMainContext = context;
		}
		catch (cl::Error err)
		{
			PR_LOGGER.logf(L_Error, M_GPU, "Couldn't create GPU context: %s (%s)", err.what(), error(err.err()));
			return false;
		}

		// Init files
		{
			std::stringstream sstream;
			sstream << "-D SAMPLING_COUNT=" << Spectrum::SAMPLING_COUNT;

			// Here we generate the constant parts on the fly...
			std::stringstream xyztable_stream;
			xyztable_stream << "constant float NM_TO_X[SAMPLING_COUNT] = {\n";
			for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT-1; ++i)
				xyztable_stream << NM_TO_X[i] << "f, ";
			xyztable_stream << NM_TO_X[Spectrum::SAMPLING_COUNT-1] << "\n};\n";

			xyztable_stream << "constant float NM_TO_Y[SAMPLING_COUNT] = {\n";
			for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT-1; ++i)
				xyztable_stream << NM_TO_Y[i] << "f, ";
			xyztable_stream << NM_TO_Y[Spectrum::SAMPLING_COUNT-1] << "\n};\n";

			xyztable_stream << "constant float NM_TO_Z[SAMPLING_COUNT] = {\n";
			for(uint32 i = 0; i < Spectrum::SAMPLING_COUNT-1; ++i)
				xyztable_stream << NM_TO_Z[i] << "f, ";
			xyztable_stream << NM_TO_Z[Spectrum::SAMPLING_COUNT-1] << "\n};\n";
			addSource("tonemapper", xyztable_stream.str() + tonemapper_src, sstream.str());
		}

		return true;
#endif
	}

#ifdef PR_WITH_GPU
	void GPU::addSource(const std::string& name, const std::string& source,
		const std::string& defs)
	{
		using namespace boost;
		PR_ASSERT(mPrograms.count(name) == 0, "Program with name does not exists");

		cl::Program program(mMainContext, source);
		try
		{
			std::string build_opts;
			build_opts += "-cl-std=CL1.2 ";
			build_opts += defs + " ";

#ifdef PR_GPU_DEBUG
			build_opts += "-g ";
#endif

			program.build(build_opts.c_str());

			mPrograms[name] = program;
		}
		catch (cl::Error err)
		{
			if (err.err() == CL_BUILD_PROGRAM_FAILURE)
			{
				std::string buildErr = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(mMainDevice);
				PR_LOGGER.logf(L_Error, M_GPU, "OpenCL Build Error:\n%s", buildErr.c_str());
			}
			else
			{
				PR_LOGGER.logf(L_Fatal, M_GPU, "OpenCL Error %s (%s)", err.what(), error(err.err()));
			}
		}
	}
#endif//PR_WITH_GPU
}
