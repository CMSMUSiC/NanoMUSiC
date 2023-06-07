// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME TEventClassDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "include/TEventClass.hpp"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_TEventClass(void *p = nullptr);
   static void *newArray_TEventClass(Long_t size, void *p);
   static void delete_TEventClass(void *p);
   static void deleteArray_TEventClass(void *p);
   static void destruct_TEventClass(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TEventClass*)
   {
      ::TEventClass *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TEventClass >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("TEventClass", ::TEventClass::Class_Version(), "TEventClass.hpp", 31,
                  typeid(::TEventClass), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TEventClass::Dictionary, isa_proxy, 4,
                  sizeof(::TEventClass) );
      instance.SetNew(&new_TEventClass);
      instance.SetNewArray(&newArray_TEventClass);
      instance.SetDelete(&delete_TEventClass);
      instance.SetDeleteArray(&deleteArray_TEventClass);
      instance.SetDestructor(&destruct_TEventClass);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TEventClass*)
   {
      return GenerateInitInstanceLocal((::TEventClass*)nullptr);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::TEventClass*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr TEventClass::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *TEventClass::Class_Name()
{
   return "TEventClass";
}

//______________________________________________________________________________
const char *TEventClass::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TEventClass*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int TEventClass::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TEventClass*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TEventClass::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TEventClass*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TEventClass::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TEventClass*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void TEventClass::Streamer(TBuffer &R__b)
{
   // Stream an object of class TEventClass.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TEventClass::Class(),this);
   } else {
      R__b.WriteClassBuffer(TEventClass::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TEventClass(void *p) {
      return  p ? new(p) ::TEventClass : new ::TEventClass;
   }
   static void *newArray_TEventClass(Long_t nElements, void *p) {
      return p ? new(p) ::TEventClass[nElements] : new ::TEventClass[nElements];
   }
   // Wrapper around operator delete
   static void delete_TEventClass(void *p) {
      delete ((::TEventClass*)p);
   }
   static void deleteArray_TEventClass(void *p) {
      delete [] ((::TEventClass*)p);
   }
   static void destruct_TEventClass(void *p) {
      typedef ::TEventClass current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::TEventClass

namespace ROOT {
   static TClass *vectorlEvectorlEdoublegRsPgR_Dictionary();
   static void vectorlEvectorlEdoublegRsPgR_TClassManip(TClass*);
   static void *new_vectorlEvectorlEdoublegRsPgR(void *p = nullptr);
   static void *newArray_vectorlEvectorlEdoublegRsPgR(Long_t size, void *p);
   static void delete_vectorlEvectorlEdoublegRsPgR(void *p);
   static void deleteArray_vectorlEvectorlEdoublegRsPgR(void *p);
   static void destruct_vectorlEvectorlEdoublegRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<vector<double> >*)
   {
      vector<vector<double> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<vector<double> >));
      static ::ROOT::TGenericClassInfo 
         instance("vector<vector<double> >", -2, "vector", 423,
                  typeid(vector<vector<double> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEvectorlEdoublegRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<vector<double> >) );
      instance.SetNew(&new_vectorlEvectorlEdoublegRsPgR);
      instance.SetNewArray(&newArray_vectorlEvectorlEdoublegRsPgR);
      instance.SetDelete(&delete_vectorlEvectorlEdoublegRsPgR);
      instance.SetDeleteArray(&deleteArray_vectorlEvectorlEdoublegRsPgR);
      instance.SetDestructor(&destruct_vectorlEvectorlEdoublegRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<vector<double> > >()));

      ::ROOT::AddClassAlternate("vector<vector<double> >","std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<vector<double> >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEvectorlEdoublegRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<vector<double> >*)nullptr)->GetClass();
      vectorlEvectorlEdoublegRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEvectorlEdoublegRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEvectorlEdoublegRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<double> > : new vector<vector<double> >;
   }
   static void *newArray_vectorlEvectorlEdoublegRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<vector<double> >[nElements] : new vector<vector<double> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEvectorlEdoublegRsPgR(void *p) {
      delete ((vector<vector<double> >*)p);
   }
   static void deleteArray_vectorlEvectorlEdoublegRsPgR(void *p) {
      delete [] ((vector<vector<double> >*)p);
   }
   static void destruct_vectorlEvectorlEdoublegRsPgR(void *p) {
      typedef vector<vector<double> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<vector<double> >

namespace ROOT {
   static TClass *vectorlEdoublegR_Dictionary();
   static void vectorlEdoublegR_TClassManip(TClass*);
   static void *new_vectorlEdoublegR(void *p = nullptr);
   static void *newArray_vectorlEdoublegR(Long_t size, void *p);
   static void delete_vectorlEdoublegR(void *p);
   static void deleteArray_vectorlEdoublegR(void *p);
   static void destruct_vectorlEdoublegR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<double>*)
   {
      vector<double> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<double>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<double>", -2, "vector", 423,
                  typeid(vector<double>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEdoublegR_Dictionary, isa_proxy, 0,
                  sizeof(vector<double>) );
      instance.SetNew(&new_vectorlEdoublegR);
      instance.SetNewArray(&newArray_vectorlEdoublegR);
      instance.SetDelete(&delete_vectorlEdoublegR);
      instance.SetDeleteArray(&deleteArray_vectorlEdoublegR);
      instance.SetDestructor(&destruct_vectorlEdoublegR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<double> >()));

      ::ROOT::AddClassAlternate("vector<double>","std::vector<double, std::allocator<double> >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const vector<double>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEdoublegR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const vector<double>*)nullptr)->GetClass();
      vectorlEdoublegR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEdoublegR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEdoublegR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<double> : new vector<double>;
   }
   static void *newArray_vectorlEdoublegR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) vector<double>[nElements] : new vector<double>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEdoublegR(void *p) {
      delete ((vector<double>*)p);
   }
   static void deleteArray_vectorlEdoublegR(void *p) {
      delete [] ((vector<double>*)p);
   }
   static void destruct_vectorlEdoublegR(void *p) {
      typedef vector<double> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class vector<double>

namespace ROOT {
   static TClass *setlEstringgR_Dictionary();
   static void setlEstringgR_TClassManip(TClass*);
   static void *new_setlEstringgR(void *p = nullptr);
   static void *newArray_setlEstringgR(Long_t size, void *p);
   static void delete_setlEstringgR(void *p);
   static void deleteArray_setlEstringgR(void *p);
   static void destruct_setlEstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const set<string>*)
   {
      set<string> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(set<string>));
      static ::ROOT::TGenericClassInfo 
         instance("set<string>", -2, "set", 94,
                  typeid(set<string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &setlEstringgR_Dictionary, isa_proxy, 0,
                  sizeof(set<string>) );
      instance.SetNew(&new_setlEstringgR);
      instance.SetNewArray(&newArray_setlEstringgR);
      instance.SetDelete(&delete_setlEstringgR);
      instance.SetDeleteArray(&deleteArray_setlEstringgR);
      instance.SetDestructor(&destruct_setlEstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Insert< set<string> >()));

      ::ROOT::AddClassAlternate("set<string>","std::set<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const set<string>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *setlEstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const set<string>*)nullptr)->GetClass();
      setlEstringgR_TClassManip(theClass);
   return theClass;
   }

   static void setlEstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_setlEstringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) set<string> : new set<string>;
   }
   static void *newArray_setlEstringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) set<string>[nElements] : new set<string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_setlEstringgR(void *p) {
      delete ((set<string>*)p);
   }
   static void deleteArray_setlEstringgR(void *p) {
      delete [] ((set<string>*)p);
   }
   static void destruct_setlEstringgR(void *p) {
      typedef set<string> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class set<string>

namespace ROOT {
   static TClass *maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_Dictionary();
   static void maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p = nullptr);
   static void *newArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p);
   static void deleteArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p);
   static void destruct_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,vector<vector<double> > >*)
   {
      map<string,vector<vector<double> > > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,vector<vector<double> > >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,vector<vector<double> > >", -2, "map", 100,
                  typeid(map<string,vector<vector<double> > >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,vector<vector<double> > >) );
      instance.SetNew(&new_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR);
      instance.SetDelete(&delete_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,vector<vector<double> > > >()));

      ::ROOT::AddClassAlternate("map<string,vector<vector<double> > >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::vector<std::vector<double, std::allocator<double> >, std::allocator<std::vector<double, std::allocator<double> > > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,vector<vector<double> > >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,vector<vector<double> > >*)nullptr)->GetClass();
      maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<vector<double> > > : new map<string,vector<vector<double> > >;
   }
   static void *newArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<vector<double> > >[nElements] : new map<string,vector<vector<double> > >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p) {
      delete ((map<string,vector<vector<double> > >*)p);
   }
   static void deleteArray_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p) {
      delete [] ((map<string,vector<vector<double> > >*)p);
   }
   static void destruct_maplEstringcOvectorlEvectorlEdoublegRsPgRsPgR(void *p) {
      typedef map<string,vector<vector<double> > > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,vector<vector<double> > >

namespace ROOT {
   static TClass *maplEstringcOvectorlEdoublegRsPgR_Dictionary();
   static void maplEstringcOvectorlEdoublegRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOvectorlEdoublegRsPgR(void *p = nullptr);
   static void *newArray_maplEstringcOvectorlEdoublegRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOvectorlEdoublegRsPgR(void *p);
   static void deleteArray_maplEstringcOvectorlEdoublegRsPgR(void *p);
   static void destruct_maplEstringcOvectorlEdoublegRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,vector<double> >*)
   {
      map<string,vector<double> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,vector<double> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,vector<double> >", -2, "map", 100,
                  typeid(map<string,vector<double> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOvectorlEdoublegRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,vector<double> >) );
      instance.SetNew(&new_maplEstringcOvectorlEdoublegRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOvectorlEdoublegRsPgR);
      instance.SetDelete(&delete_maplEstringcOvectorlEdoublegRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOvectorlEdoublegRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOvectorlEdoublegRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,vector<double> > >()));

      ::ROOT::AddClassAlternate("map<string,vector<double> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::vector<double, std::allocator<double> >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::vector<double, std::allocator<double> > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,vector<double> >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOvectorlEdoublegRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,vector<double> >*)nullptr)->GetClass();
      maplEstringcOvectorlEdoublegRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOvectorlEdoublegRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOvectorlEdoublegRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<double> > : new map<string,vector<double> >;
   }
   static void *newArray_maplEstringcOvectorlEdoublegRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,vector<double> >[nElements] : new map<string,vector<double> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOvectorlEdoublegRsPgR(void *p) {
      delete ((map<string,vector<double> >*)p);
   }
   static void deleteArray_maplEstringcOvectorlEdoublegRsPgR(void *p) {
      delete [] ((map<string,vector<double> >*)p);
   }
   static void destruct_maplEstringcOvectorlEdoublegRsPgR(void *p) {
      typedef map<string,vector<double> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,vector<double> >

namespace ROOT {
   static TClass *maplEstringcOunsignedsPintgR_Dictionary();
   static void maplEstringcOunsignedsPintgR_TClassManip(TClass*);
   static void *new_maplEstringcOunsignedsPintgR(void *p = nullptr);
   static void *newArray_maplEstringcOunsignedsPintgR(Long_t size, void *p);
   static void delete_maplEstringcOunsignedsPintgR(void *p);
   static void deleteArray_maplEstringcOunsignedsPintgR(void *p);
   static void destruct_maplEstringcOunsignedsPintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,unsigned int>*)
   {
      map<string,unsigned int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,unsigned int>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,unsigned int>", -2, "map", 100,
                  typeid(map<string,unsigned int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOunsignedsPintgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,unsigned int>) );
      instance.SetNew(&new_maplEstringcOunsignedsPintgR);
      instance.SetNewArray(&newArray_maplEstringcOunsignedsPintgR);
      instance.SetDelete(&delete_maplEstringcOunsignedsPintgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOunsignedsPintgR);
      instance.SetDestructor(&destruct_maplEstringcOunsignedsPintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,unsigned int> >()));

      ::ROOT::AddClassAlternate("map<string,unsigned int>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, unsigned int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, unsigned int> > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,unsigned int>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOunsignedsPintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,unsigned int>*)nullptr)->GetClass();
      maplEstringcOunsignedsPintgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOunsignedsPintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOunsignedsPintgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,unsigned int> : new map<string,unsigned int>;
   }
   static void *newArray_maplEstringcOunsignedsPintgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,unsigned int>[nElements] : new map<string,unsigned int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOunsignedsPintgR(void *p) {
      delete ((map<string,unsigned int>*)p);
   }
   static void deleteArray_maplEstringcOunsignedsPintgR(void *p) {
      delete [] ((map<string,unsigned int>*)p);
   }
   static void destruct_maplEstringcOunsignedsPintgR(void *p) {
      typedef map<string,unsigned int> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,unsigned int>

namespace ROOT {
   static TClass *maplEstringcOstringgR_Dictionary();
   static void maplEstringcOstringgR_TClassManip(TClass*);
   static void *new_maplEstringcOstringgR(void *p = nullptr);
   static void *newArray_maplEstringcOstringgR(Long_t size, void *p);
   static void delete_maplEstringcOstringgR(void *p);
   static void deleteArray_maplEstringcOstringgR(void *p);
   static void destruct_maplEstringcOstringgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,string>*)
   {
      map<string,string> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,string>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,string>", -2, "map", 100,
                  typeid(map<string,string>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOstringgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,string>) );
      instance.SetNew(&new_maplEstringcOstringgR);
      instance.SetNewArray(&newArray_maplEstringcOstringgR);
      instance.SetDelete(&delete_maplEstringcOstringgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOstringgR);
      instance.SetDestructor(&destruct_maplEstringcOstringgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,string> >()));

      ::ROOT::AddClassAlternate("map<string,string>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,string>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOstringgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,string>*)nullptr)->GetClass();
      maplEstringcOstringgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOstringgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOstringgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,string> : new map<string,string>;
   }
   static void *newArray_maplEstringcOstringgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,string>[nElements] : new map<string,string>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOstringgR(void *p) {
      delete ((map<string,string>*)p);
   }
   static void deleteArray_maplEstringcOstringgR(void *p) {
      delete [] ((map<string,string>*)p);
   }
   static void destruct_maplEstringcOstringgR(void *p) {
      typedef map<string,string> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,string>

namespace ROOT {
   static TClass *maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_Dictionary();
   static void maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p = nullptr);
   static void *newArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p);
   static void deleteArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p);
   static void destruct_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,map<string,map<string,TH1F*> > >*)
   {
      map<string,map<string,map<string,TH1F*> > > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,map<string,map<string,TH1F*> > >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,map<string,map<string,TH1F*> > >", -2, "map", 100,
                  typeid(map<string,map<string,map<string,TH1F*> > >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,map<string,map<string,TH1F*> > >) );
      instance.SetNew(&new_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR);
      instance.SetDelete(&delete_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,map<string,map<string,TH1F*> > > >()));

      ::ROOT::AddClassAlternate("map<string,map<string,map<string,TH1F*> > >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > > > > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > > > > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,map<string,map<string,TH1F*> > >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,map<string,map<string,TH1F*> > >*)nullptr)->GetClass();
      maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,map<string,TH1F*> > > : new map<string,map<string,map<string,TH1F*> > >;
   }
   static void *newArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,map<string,TH1F*> > >[nElements] : new map<string,map<string,map<string,TH1F*> > >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p) {
      delete ((map<string,map<string,map<string,TH1F*> > >*)p);
   }
   static void deleteArray_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p) {
      delete [] ((map<string,map<string,map<string,TH1F*> > >*)p);
   }
   static void destruct_maplEstringcOmaplEstringcOmaplEstringcOTH1FmUgRsPgRsPgR(void *p) {
      typedef map<string,map<string,map<string,TH1F*> > > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,map<string,map<string,TH1F*> > >

namespace ROOT {
   static TClass *maplEstringcOmaplEstringcOTH2FmUgRsPgR_Dictionary();
   static void maplEstringcOmaplEstringcOTH2FmUgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p = nullptr);
   static void *newArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p);
   static void deleteArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p);
   static void destruct_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,map<string,TH2F*> >*)
   {
      map<string,map<string,TH2F*> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,map<string,TH2F*> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,map<string,TH2F*> >", -2, "map", 100,
                  typeid(map<string,map<string,TH2F*> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOmaplEstringcOTH2FmUgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,map<string,TH2F*> >) );
      instance.SetNew(&new_maplEstringcOmaplEstringcOTH2FmUgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR);
      instance.SetDelete(&delete_maplEstringcOmaplEstringcOTH2FmUgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOmaplEstringcOTH2FmUgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,map<string,TH2F*> > >()));

      ::ROOT::AddClassAlternate("map<string,map<string,TH2F*> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH2F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH2F*> > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH2F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH2F*> > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,map<string,TH2F*> >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOmaplEstringcOTH2FmUgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,map<string,TH2F*> >*)nullptr)->GetClass();
      maplEstringcOmaplEstringcOTH2FmUgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOmaplEstringcOTH2FmUgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,TH2F*> > : new map<string,map<string,TH2F*> >;
   }
   static void *newArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,TH2F*> >[nElements] : new map<string,map<string,TH2F*> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p) {
      delete ((map<string,map<string,TH2F*> >*)p);
   }
   static void deleteArray_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p) {
      delete [] ((map<string,map<string,TH2F*> >*)p);
   }
   static void destruct_maplEstringcOmaplEstringcOTH2FmUgRsPgR(void *p) {
      typedef map<string,map<string,TH2F*> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,map<string,TH2F*> >

namespace ROOT {
   static TClass *maplEstringcOmaplEstringcOTH1FmUgRsPgR_Dictionary();
   static void maplEstringcOmaplEstringcOTH1FmUgRsPgR_TClassManip(TClass*);
   static void *new_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p = nullptr);
   static void *newArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR(Long_t size, void *p);
   static void delete_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p);
   static void deleteArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p);
   static void destruct_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,map<string,TH1F*> >*)
   {
      map<string,map<string,TH1F*> > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,map<string,TH1F*> >));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,map<string,TH1F*> >", -2, "map", 100,
                  typeid(map<string,map<string,TH1F*> >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOmaplEstringcOTH1FmUgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,map<string,TH1F*> >) );
      instance.SetNew(&new_maplEstringcOmaplEstringcOTH1FmUgRsPgR);
      instance.SetNewArray(&newArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR);
      instance.SetDelete(&delete_maplEstringcOmaplEstringcOTH1FmUgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR);
      instance.SetDestructor(&destruct_maplEstringcOmaplEstringcOTH1FmUgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,map<string,TH1F*> > >()));

      ::ROOT::AddClassAlternate("map<string,map<string,TH1F*> >","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,map<string,TH1F*> >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOmaplEstringcOTH1FmUgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,map<string,TH1F*> >*)nullptr)->GetClass();
      maplEstringcOmaplEstringcOTH1FmUgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOmaplEstringcOTH1FmUgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,TH1F*> > : new map<string,map<string,TH1F*> >;
   }
   static void *newArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,map<string,TH1F*> >[nElements] : new map<string,map<string,TH1F*> >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p) {
      delete ((map<string,map<string,TH1F*> >*)p);
   }
   static void deleteArray_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p) {
      delete [] ((map<string,map<string,TH1F*> >*)p);
   }
   static void destruct_maplEstringcOmaplEstringcOTH1FmUgRsPgR(void *p) {
      typedef map<string,map<string,TH1F*> > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,map<string,TH1F*> >

namespace ROOT {
   static TClass *maplEstringcOintgR_Dictionary();
   static void maplEstringcOintgR_TClassManip(TClass*);
   static void *new_maplEstringcOintgR(void *p = nullptr);
   static void *newArray_maplEstringcOintgR(Long_t size, void *p);
   static void delete_maplEstringcOintgR(void *p);
   static void deleteArray_maplEstringcOintgR(void *p);
   static void destruct_maplEstringcOintgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,int>*)
   {
      map<string,int> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,int>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,int>", -2, "map", 100,
                  typeid(map<string,int>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOintgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,int>) );
      instance.SetNew(&new_maplEstringcOintgR);
      instance.SetNewArray(&newArray_maplEstringcOintgR);
      instance.SetDelete(&delete_maplEstringcOintgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOintgR);
      instance.SetDestructor(&destruct_maplEstringcOintgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,int> >()));

      ::ROOT::AddClassAlternate("map<string,int>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, int, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, int> > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,int>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOintgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,int>*)nullptr)->GetClass();
      maplEstringcOintgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOintgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOintgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,int> : new map<string,int>;
   }
   static void *newArray_maplEstringcOintgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,int>[nElements] : new map<string,int>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOintgR(void *p) {
      delete ((map<string,int>*)p);
   }
   static void deleteArray_maplEstringcOintgR(void *p) {
      delete [] ((map<string,int>*)p);
   }
   static void destruct_maplEstringcOintgR(void *p) {
      typedef map<string,int> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,int>

namespace ROOT {
   static TClass *maplEstringcOdoublegR_Dictionary();
   static void maplEstringcOdoublegR_TClassManip(TClass*);
   static void *new_maplEstringcOdoublegR(void *p = nullptr);
   static void *newArray_maplEstringcOdoublegR(Long_t size, void *p);
   static void delete_maplEstringcOdoublegR(void *p);
   static void deleteArray_maplEstringcOdoublegR(void *p);
   static void destruct_maplEstringcOdoublegR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,double>*)
   {
      map<string,double> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,double>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,double>", -2, "map", 100,
                  typeid(map<string,double>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOdoublegR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,double>) );
      instance.SetNew(&new_maplEstringcOdoublegR);
      instance.SetNewArray(&newArray_maplEstringcOdoublegR);
      instance.SetDelete(&delete_maplEstringcOdoublegR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOdoublegR);
      instance.SetDestructor(&destruct_maplEstringcOdoublegR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,double> >()));

      ::ROOT::AddClassAlternate("map<string,double>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, double, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, double> > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,double>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOdoublegR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,double>*)nullptr)->GetClass();
      maplEstringcOdoublegR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOdoublegR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOdoublegR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,double> : new map<string,double>;
   }
   static void *newArray_maplEstringcOdoublegR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,double>[nElements] : new map<string,double>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOdoublegR(void *p) {
      delete ((map<string,double>*)p);
   }
   static void deleteArray_maplEstringcOdoublegR(void *p) {
      delete [] ((map<string,double>*)p);
   }
   static void destruct_maplEstringcOdoublegR(void *p) {
      typedef map<string,double> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,double>

namespace ROOT {
   static TClass *maplEstringcOTH2FmUgR_Dictionary();
   static void maplEstringcOTH2FmUgR_TClassManip(TClass*);
   static void *new_maplEstringcOTH2FmUgR(void *p = nullptr);
   static void *newArray_maplEstringcOTH2FmUgR(Long_t size, void *p);
   static void delete_maplEstringcOTH2FmUgR(void *p);
   static void deleteArray_maplEstringcOTH2FmUgR(void *p);
   static void destruct_maplEstringcOTH2FmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,TH2F*>*)
   {
      map<string,TH2F*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,TH2F*>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,TH2F*>", -2, "map", 100,
                  typeid(map<string,TH2F*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOTH2FmUgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,TH2F*>) );
      instance.SetNew(&new_maplEstringcOTH2FmUgR);
      instance.SetNewArray(&newArray_maplEstringcOTH2FmUgR);
      instance.SetDelete(&delete_maplEstringcOTH2FmUgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOTH2FmUgR);
      instance.SetDestructor(&destruct_maplEstringcOTH2FmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,TH2F*> >()));

      ::ROOT::AddClassAlternate("map<string,TH2F*>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH2F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH2F*> > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,TH2F*>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOTH2FmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,TH2F*>*)nullptr)->GetClass();
      maplEstringcOTH2FmUgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOTH2FmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOTH2FmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,TH2F*> : new map<string,TH2F*>;
   }
   static void *newArray_maplEstringcOTH2FmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,TH2F*>[nElements] : new map<string,TH2F*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOTH2FmUgR(void *p) {
      delete ((map<string,TH2F*>*)p);
   }
   static void deleteArray_maplEstringcOTH2FmUgR(void *p) {
      delete [] ((map<string,TH2F*>*)p);
   }
   static void destruct_maplEstringcOTH2FmUgR(void *p) {
      typedef map<string,TH2F*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,TH2F*>

namespace ROOT {
   static TClass *maplEstringcOTH1FmUgR_Dictionary();
   static void maplEstringcOTH1FmUgR_TClassManip(TClass*);
   static void *new_maplEstringcOTH1FmUgR(void *p = nullptr);
   static void *newArray_maplEstringcOTH1FmUgR(Long_t size, void *p);
   static void delete_maplEstringcOTH1FmUgR(void *p);
   static void deleteArray_maplEstringcOTH1FmUgR(void *p);
   static void destruct_maplEstringcOTH1FmUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<string,TH1F*>*)
   {
      map<string,TH1F*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<string,TH1F*>));
      static ::ROOT::TGenericClassInfo 
         instance("map<string,TH1F*>", -2, "map", 100,
                  typeid(map<string,TH1F*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplEstringcOTH1FmUgR_Dictionary, isa_proxy, 0,
                  sizeof(map<string,TH1F*>) );
      instance.SetNew(&new_maplEstringcOTH1FmUgR);
      instance.SetNewArray(&newArray_maplEstringcOTH1FmUgR);
      instance.SetDelete(&delete_maplEstringcOTH1FmUgR);
      instance.SetDeleteArray(&deleteArray_maplEstringcOTH1FmUgR);
      instance.SetDestructor(&destruct_maplEstringcOTH1FmUgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<string,TH1F*> >()));

      ::ROOT::AddClassAlternate("map<string,TH1F*>","std::map<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, TH1F*, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const, TH1F*> > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<string,TH1F*>*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplEstringcOTH1FmUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<string,TH1F*>*)nullptr)->GetClass();
      maplEstringcOTH1FmUgR_TClassManip(theClass);
   return theClass;
   }

   static void maplEstringcOTH1FmUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplEstringcOTH1FmUgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,TH1F*> : new map<string,TH1F*>;
   }
   static void *newArray_maplEstringcOTH1FmUgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<string,TH1F*>[nElements] : new map<string,TH1F*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplEstringcOTH1FmUgR(void *p) {
      delete ((map<string,TH1F*>*)p);
   }
   static void deleteArray_maplEstringcOTH1FmUgR(void *p) {
      delete [] ((map<string,TH1F*>*)p);
   }
   static void destruct_maplEstringcOTH1FmUgR(void *p) {
      typedef map<string,TH1F*> current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<string,TH1F*>

namespace ROOT {
   static TClass *maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_Dictionary();
   static void maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_TClassManip(TClass*);
   static void *new_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p = nullptr);
   static void *newArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(Long_t size, void *p);
   static void delete_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p);
   static void deleteArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p);
   static void destruct_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<long,pair<string,set<string> > >*)
   {
      map<long,pair<string,set<string> > > *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<long,pair<string,set<string> > >));
      static ::ROOT::TGenericClassInfo 
         instance("map<long,pair<string,set<string> > >", -2, "map", 100,
                  typeid(map<long,pair<string,set<string> > >), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_Dictionary, isa_proxy, 0,
                  sizeof(map<long,pair<string,set<string> > >) );
      instance.SetNew(&new_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR);
      instance.SetNewArray(&newArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR);
      instance.SetDelete(&delete_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR);
      instance.SetDeleteArray(&deleteArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR);
      instance.SetDestructor(&destruct_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR);
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<long,pair<string,set<string> > > >()));

      ::ROOT::AddClassAlternate("map<long,pair<string,set<string> > >","std::map<long, std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::set<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > >, std::less<long>, std::allocator<std::pair<long const, std::pair<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::set<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >, std::less<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > >, std::allocator<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > > > > > >");
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const map<long,pair<string,set<string> > >*)nullptr); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal((const map<long,pair<string,set<string> > >*)nullptr)->GetClass();
      maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_TClassManip(theClass);
   return theClass;
   }

   static void maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p) {
      return  p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<long,pair<string,set<string> > > : new map<long,pair<string,set<string> > >;
   }
   static void *newArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(Long_t nElements, void *p) {
      return p ? ::new((::ROOT::Internal::TOperatorNewHelper*)p) map<long,pair<string,set<string> > >[nElements] : new map<long,pair<string,set<string> > >[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p) {
      delete ((map<long,pair<string,set<string> > >*)p);
   }
   static void deleteArray_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p) {
      delete [] ((map<long,pair<string,set<string> > >*)p);
   }
   static void destruct_maplElongcOpairlEstringcOsetlEstringgRsPgRsPgR(void *p) {
      typedef map<long,pair<string,set<string> > > current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class map<long,pair<string,set<string> > >

namespace {
  void TriggerDictionaryInitialization_libTEventClass_Impl() {
    static const char* headers[] = {
"include/TEventClass.hpp",
nullptr
    };
    static const char* includePaths[] = {
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC",
"/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include",
"/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include",
"/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include/rapidjson",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC/external",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-LightSkimmer/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Classification/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Validation/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-HeavyValidation/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Pxl/Pxl/src",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Tools",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/tools/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Tools/PXL",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Main",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Utils/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass/./",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC",
"/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include",
"/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include",
"/cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/include/rapidjson",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC/external",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-LightSkimmer/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Classification/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Validation/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-HeavyValidation/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Pxl/Pxl/src",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Tools",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/tools/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Tools/PXL",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/PxlAnalyzer/Main",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/MUSiC-Utils/include",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass/./",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass/include",
"/cvmfs/sft.cern.ch/lcg/releases/ROOT/6.26.08-34ede/x86_64-centos7-gcc12-opt/include/",
"/.automount/home/home__home1/institut_3a/esper/esper/NanoMUSiC/NanoMUSiC/TEventClass/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libTEventClass dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$include/TEventClass.hpp")))  TEventClass;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libTEventClass dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "include/TEventClass.hpp"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"TEventClass", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libTEventClass",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libTEventClass_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libTEventClass_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libTEventClass() {
  TriggerDictionaryInitialization_libTEventClass_Impl();
}
