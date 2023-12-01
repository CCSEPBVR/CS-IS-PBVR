/* 
 * File:   wrapper_vtk_macros.h
 * Author: Martin Andersson, V.I.C Visible Information Center
 *
 * Created on November 18, 2015, 5:14 PM
 */

#ifndef WRAPPER_VTK_TYPES_H
#define WRAPPER_VTK_TYPES_H

/*
 WRAP_VTK_CTOR, wraps simple constructors with no args
 */
#define WRAP_VTK_CTOR(T)   _ ## T  T ## _ ## New(){ return T::New();}
#define DECLARE_VTK_CTOR(T)  _ ## T  T ## _ ## New();
/*
 WRAP_VTK_DTOR, wraps destructors
 */
#define WRAP_VTK_DTOR(Obj) void Obj ## _ ## Delete(_ ## Obj o){o->Delete();}
#define DECLARE_VTK_DTOR(Obj) void Obj ## _ ## Delete(_ ## Obj o);
/*
 WRAP_VTK_GETTER, wraps simple  getter method with return value
 */
#define WRAP_VTK_GETTER(Obj,getter,ret_T) ret_T Obj ## _ ## getter(_ ## Obj o){ return (ret_T)o->getter();}
#define DECLARE_VTK_GETTER(Obj,getter,ret_T) ret_T Obj ## _ ## getter(_ ## Obj o);
/*
 WRAP_VTK_GETTER_1ARG, wraps getter methods with 1 argument and return value
 */
#define WRAP_VTK_GETTER_1ARG(Obj,getter,ret_T,arg_T) ret_T Obj ## _ ## getter(_ ## Obj o, arg_T arg){ return (ret_T)o->getter(arg);}
#define DECLARE_VTK_GETTER_1ARG(Obj,getter,ret_T,arg_T) ret_T Obj ## _ ## getter(_ ## Obj o,arg_T arg);
/*
 WRAP_VTK_GETTER_1ARG, wraps getter methods with 1 argument and return value
 */
#define WRAP_VTK_GETTER_2ARG(Obj,getter,ret_T,arg_T1,arg_T2) ret_T Obj ## _ ## getter(_ ## Obj o, arg_T1 arg1, arg_T2 arg2){ return (ret_T)o->getter(arg1,arg2);}
#define DECLARE_VTK_GETTER_2ARG(Obj,getter,ret_T,arg_T1,arg_T2) ret_T Obj ## _ ## getter(_ ## Obj o,arg_T1 arg1, arg_T2 arg2);
/*
 WRAP_VTK_SETTER_1ARG, wraps setter methods with 1 argument returning void
 */
#define WRAP_VTK_SETTER_1ARG(Obj,setter,ret_T, arg_T) ret_T Obj ## _ ## setter(_ ## Obj o, arg_T arg){ o->setter(arg);}
#define DECLARE_VTK_SETTER_1ARG(Obj,setter,ret_T, arg_T) ret_T Obj ## _ ## setter(_ ## Obj o,arg_T arg);
/*
 WRAP_VTK_TYPE macro defines _vtkType as void* in Std C, and as vtkType in c++
 */
#ifdef __cplusplus
#define WRAP_VTK_TYPE(T) typedef T*  _ ## T;
#else
#define WRAP_VTK_TYPE(T) typedef void*  _ ## T;
#endif


#endif /* WRAPPER_VTK_TYPES_H */

