#include <boost/python.hpp>
#include "PearMath.h"

#include <iostream>
namespace bpy = boost::python;
namespace PRPY
{
    /* Non-aligned wrapper */
    template<int D1,int D2>
    class pymat
    {
    public:
        pymat()
        {
            for(int i = 0; i < D1*D2; ++i)
                mMat[i] = 0;
        }

        pymat(const PM::mat<D1,D2>& v)
        {
            assign(v);
        }

        pymat(const bpy::tuple& tpl)
        {
            for(int i = 0; i < D1; ++i)
            {
                bpy::tuple tpl2 = bpy::extract<bpy::tuple>(tpl[i]);
                for(int j = 0; j < D2; ++j)
                    mMat[i*D2 + j] = bpy::extract<float>(tpl2[j]);
            }
        }

        operator PM::mat<D1,D2>() const
        {
            PM::mat<D1,D2> m;
            for(int i = 0; i < D1; ++i)
                for(int j = 0; j < D2; ++j)
                    m.v[i] = PM::pm_SetIndex(m.v[i], j, mMat[i*D2+j]);
            
            return m;
        }

        int len() const { return D1*D2; }

        std::string str() const
        { 
            std::stringstream stream;
            stream << "(";
            for(int i = 0; i < D1; ++i)
            {
                for(int j = 0; j < D2-1; ++j)
                    stream << mMat[i*D2+j] << ",";
                
                stream << mMat[i*D2 + D2-1];

                if(i != D1-1)
                    stream << ";";
            }
            stream << ")";
            return stream.str();
        }

        void set(const bpy::tuple& index, float f) {
            const int r = bpy::extract<int>(index[0]);
            const int c = bpy::extract<int>(index[1]);
            _set(r,c,f);
        }
        
        float get(const bpy::tuple& index) const {
            const int r = bpy::extract<int>(index[0]);
            const int c = bpy::extract<int>(index[1]);
            return _get(r,c);
        }

        void _set(int i, int j, float f) {
            if(i < 0 || i >= D1 || j < 0 || j >= D2)
                throw std::out_of_range("Index out of bound");
            
            mMat[i*D2 + j] = f;
        }

        float _get(int i, int j) const
        {
            if(i < 0 || i >= D1 || j < 0 || j >= D2)
                throw std::out_of_range("Index out of bound");

            return mMat[i*D2 + j];
        }

        PM::vec<D2> row(int i) const
        {
            if(i < 0 || i >= D1)
                throw std::out_of_range("Index out of bound");
            return PM::pm_GetRow<PM::mat<D1,D2> >(*this, i);
        }

        PM::vec<D1> column(int i) const
        {
            if(i < 0 || i >= D2)
                throw std::out_of_range("Index out of bound");
            return PM::pm_GetColumn(*this, i);
        }

        void transpose()
        {
            assign(PM::pm_Transpose(*this));
        }

        float determinant()
        {
            return PM::pm_Determinant(*this);
        }

        void inverse()
        {
            assign(PM::pm_Inverse(*this));
        }

    private:
        void assign(const PM::mat<D1,D2>& m)
        {
            for(int i = 0; i < D1; ++i)
                for(int j = 0; j < D2; ++j)
                    mMat[i*D2 + j] = PM::pm_GetIndex(m, i, j);
        }
        float mMat[D1*D2];
    };

    template<int D1,int D2>
    bool eq(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                if(v1._get(i,j) != v2._get(i,j))
                    return false;

        return true;
    }

    template<int D1,int D2>
    bool neq(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        return !eq(v1,v2);
    }

    template<int D1,int D2>
    pymat<D1,D2> operator +(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) + v2._get(i,j));
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator -(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) - v2._get(i,j));
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator *(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) * v2._get(i,j));
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator /(const pymat<D1,D2>& v1, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) / v2._get(i,j));
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator *(const pymat<D1,D2>& v1, float f)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) * f);
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator /(const pymat<D1,D2>& v1, float f)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, v1._get(i,j) / f);
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator *(float f, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, f * v2._get(i,j));
        return r;
    }

    template<int D1,int D2>
    pymat<D1,D2> operator /(float f, const pymat<D1,D2>& v2)
    {
        pymat<D1,D2> r;
        for(int i = 0; i < D1; ++i)
            for(int j = 0; j < D2; ++j)
                r._set(i,j, f / v2._get(i,j));
        return r;
    }

    template<int D1, int D2>
    struct mat_to_pymat
    {
        static PyObject* convert(PM::mat<D1,D2> const& x)
        {
            return bpy::incref(bpy::object(pymat<D1,D2>(x)).ptr());
        }
    };

    template<int D1,int D2>
    void setup_mat_internal(const char* name)
    {
        auto cls = bpy::class_<pymat<D1,D2> >(name)
            .def(bpy::init<bpy::tuple>())
            .def("__len__", &pymat<D1,D2>::len)
            .def("__str__", &pymat<D1,D2>::str)
            .def("__eq__", &eq<D1,D2>)
            .def("__neq__", &neq<D1,D2>)
            .def("__getitem__", &pymat<D1,D2>::get)
            .def("__setitem__", &pymat<D1,D2>::set)
            .def(bpy::self + bpy::self)
            .def(bpy::self - bpy::self)
            .def(bpy::self * bpy::self)
            .def(bpy::self / bpy::self)
            .def(bpy::self * float())
            .def(bpy::self / float())
            .def(float() * bpy::self)
            .def(float() / bpy::self)
            .def("row", &pymat<D1,D2>::row)
            .def("column", &pymat<D1,D2>::column)
            .def("transpose", &pymat<D1,D2>::transpose)
            .def("determinant", &pymat<D1,D2>::determinant)
            .def("inverse", &pymat<D1,D2>::inverse)
        ;

        bpy::to_python_converter<PM::mat<D1,D2>, mat_to_pymat<D1,D2> >();
        bpy::implicitly_convertible<pymat<D1,D2>,PM::mat<D1,D2> >();
    }

    void setup_mat()
    {
        setup_mat_internal<2,2>("mat2");
        setup_mat_internal<3,3>("mat3");
        setup_mat_internal<4,4>("mat4");
    }
}