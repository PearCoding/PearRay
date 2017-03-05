#include <boost/python.hpp>
#include <boost/python/numeric.hpp>

#include "PearMath.h"

#include <iostream>
namespace bpy = boost::python;
namespace PRPY
{
    /* Non-aligned wrapper */
    template<int D>
    class pyvec
    {
    public:
        pyvec()
        {
            for(int i = 0; i < D; ++i)
                mVec[i] = 0;
        }

        pyvec(const PM::vec<D>& v)
        {
            for(int i = 0; i < D; ++i)
                mVec[i] = PM::pm_GetIndex(v, i);
        }

        pyvec(const bpy::tuple& tpl)
        {
            for(int i = 0; i < D; ++i)
                mVec[i] = bpy::extract<float>(tpl[i]);
        }

        operator PM::vec<D>() const
        {
            PM::vec<D> v;
            for(int i = 0; i < D; ++i)
                v = PM::pm_SetIndex(v, i, mVec[i]);
            return v;
        }

        bpy::tuple toTuple() const
        {
            if(D == 2)
                return bpy::make_tuple(mVec[0],mVec[1]);
            else if(D == 3)
                return bpy::make_tuple(mVec[0],mVec[1],mVec[2]);
            else if(D == 4)
                return bpy::make_tuple(mVec[0],mVec[1],mVec[2],mVec[3]);
            else
                return bpy::tuple();
        }

        bpy::object toNumPy() const
        {
            return bpy::numeric::array(toTuple());
        }

        int len() const { return D; }

        std::string repr() const
        { 
            std::stringstream stream;
            stream << "vec" << D << "(";
            for(int i = 0; i < D-1; ++i)
                stream << mVec[i] << ",";
            stream << mVec[D-1]  << ")";
            return stream.str();
        }

        std::string str() const
        { 
            std::stringstream stream;
            stream << "(";
            for(int i = 0; i < D-1; ++i)
                stream << mVec[i] << ",";
            stream << mVec[D-1]  << ")";
            return stream.str();
        }

        void set(int i, float f) { mVec[i] = f; }
        float get(int i) const { return mVec[i]; }

        float getX() const { return get(0); }
        float getY() const { return get(1); }
        float getZ() const { return get(2); }
        float getW() const { return get(3); }
        void setX(float f) { set(0,f); }
        void setY(float f) { set(1,f); }
        void setZ(float f) { set(2,f); }
        void setW(float f) { set(3,f); }

        float dot(const pyvec<D>& v) const {
            float f=0;
            for(int i = 0; i < D; ++i)
                f += mVec[i]*v.mVec[i];
            return f;
        }
        float magSqr() const { return dot(*this); }
        float mag() const { return std::sqrt(magSqr()); }
        void normalize()
        {
            const float iF = 1/mag();
            for(int i = 0; i < D; ++i)
                mVec[i] *= iF;
        }
    private:
        float mVec[D];
    };

    template<int D>
    bool eq(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        for(int i = 0; i < D; ++i)
        {
            if(v1.get(i) != v2.get(i))
                return false;
        }
        return true;
    }

    template<int D>
    bool neq(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        return !eq(v1,v2);
    }

    template<int D>
    pyvec<D> operator +(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) + v2.get(i));
        return r;
    }

    template<int D>
    pyvec<D> operator -(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) - v2.get(i));
        return r;
    }

    template<int D>
    pyvec<D> operator *(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) * v2.get(i));
        return r;
    }

    template<int D>
    pyvec<D> operator /(const pyvec<D>& v1, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) / v2.get(i));
        return r;
    }

    template<int D>
    pyvec<D> operator *(const pyvec<D>& v1, float f)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) * f);
        return r;
    }

    template<int D>
    pyvec<D> operator /(const pyvec<D>& v1, float f)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, v1.get(i) / f);
        return r;
    }

    template<int D>
    pyvec<D> operator *(float f, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, f * v2.get(i));
        return r;
    }

    template<int D>
    pyvec<D> operator /(float f, const pyvec<D>& v2)
    {
        pyvec<D> r;
        for(int i = 0; i < D; ++i)
            r.set(i, f / v2.get(i));
        return r;
    }

    boost::shared_ptr<pyvec<2>> init2(float f1, float f2) {
        auto ptr = boost::shared_ptr<pyvec<2> >(new pyvec<2>());
        ptr->setX(f1); ptr->setY(f2);
        return ptr;
    }
    boost::shared_ptr<pyvec<3>> init3(float f1, float f2, float f3) { 
        auto ptr = boost::shared_ptr<pyvec<3> >(new pyvec<3>());
        ptr->setX(f1); ptr->setY(f2); ptr->setZ(f3);
        return ptr;
    }
    boost::shared_ptr<pyvec<4>> init4(float f1, float f2, float f3, float f4) { 
        auto ptr = boost::shared_ptr<pyvec<4> >(new pyvec<4>());
        ptr->setX(f1); ptr->setY(f2); ptr->setZ(f3); ptr->setW(f4);
        return ptr;
    }

    template<int D>
    struct vec_to_pyvec
    {
        static PyObject* convert(PM::vec<D> const& x)
        {
            return bpy::incref(bpy::object(pyvec<D>(x)).ptr());
        }
    };

    template<int D>
    void setup_vec_internal(const char* name)
    {
        auto cls = bpy::class_<pyvec<D>, boost::shared_ptr<pyvec<D> > >(name)
            .def(bpy::init<bpy::tuple>())
            .add_property("x", &pyvec<D>::getX, &pyvec<D>::setX)
            .add_property("y", &pyvec<D>::getY, &pyvec<D>::setY)
            .def("__len__", &pyvec<D>::len)
            .def("toTuple", &pyvec<D>::toTuple)
            .def("toNumPy", &pyvec<D>::toNumPy)
            .def("__str__", &pyvec<D>::str)
            .def("__repr__", &pyvec<D>::repr)
            .def("__eq__", &eq<D>)
            .def("__neq__", &neq<D>)
            .def("__getitem__", &pyvec<D>::get)
            .def("__setitem__", &pyvec<D>::set)
            .def(bpy::self + bpy::self)
            .def(bpy::self - bpy::self)
            .def(bpy::self * bpy::self)
            .def(bpy::self / bpy::self)
            .def(bpy::self * float())
            .def(bpy::self / float())
            .def(float() * bpy::self)
            .def(float() / bpy::self)
            .def("dot", &pyvec<D>::dot)
            .def("mag", &pyvec<D>::mag)
            .def("magSqr", &pyvec<D>::magSqr)
            .def("normalize", &pyvec<D>::normalize)
        ;

        if(D == 2)
        {
            cls.def("__init__", bpy::make_constructor(&init2));
        }
        else if(D == 3)
        {
            cls.def("__init__", bpy::make_constructor(&init3))
                .add_property("z", &pyvec<D>::getZ, &pyvec<D>::setZ);
        }
        else if(D == 4)
        {
            cls.def("__init__", bpy::make_constructor(&init4))
                .add_property("z", &pyvec<D>::getZ, &pyvec<D>::setZ)
                .add_property("w", &pyvec<D>::getW, &pyvec<D>::setW);
        }

        bpy::to_python_converter<PM::vec<D>, vec_to_pyvec<D> >();
        //bpy::implicitly_convertible<PM::vec<D>,pyvec<D> >();
        bpy::implicitly_convertible<pyvec<D>,PM::vec<D> >();
    }

    void setup_vec()
    {
        setup_vec_internal<2>("vec2");
        setup_vec_internal<3>("vec3");
        setup_vec_internal<4>("vec4");
    }
}