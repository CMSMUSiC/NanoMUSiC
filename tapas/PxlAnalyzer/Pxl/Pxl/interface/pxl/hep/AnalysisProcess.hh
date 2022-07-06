//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_ANALYSIS_PROCESS_HH
#define PXL_HEP_ANALYSIS_PROCESS_HH
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

#include "Pxl/Pxl/interface/pxl/core/ObjectManager.hh"
#include "Pxl/Pxl/interface/pxl/core/weak_ptr.hh"
#include "Pxl/Pxl/interface/pxl/core/Event.hh"

namespace pxl
{

/**
 This class is designed as a base class to assist the analyzer in the
 evolution of different combinatorial hypotheses of an event according
 to a certain hep process.
 This class provides virtual methods to be called at the beginning and end
 of a job, at the beginning and end of a run, and, of course, at event analysis
 and event finishing time (just as needed in a stand-alone analysis framework,
 for instance). When inheriting from this class, the analyst can
 place user code in the according reimplementations of these methods.
 @deprecated use the Module system instead
 */
class PXL_DLL_EXPORT AnalysisProcess : public ObjectManager
{
public:
	AnalysisProcess() :
		ObjectManager()
	{
	}
	AnalysisProcess(const AnalysisProcess& original) :
		ObjectManager(original)
	{
	}
	explicit AnalysisProcess(const AnalysisProcess* original) :
		ObjectManager(original)
	{
	}
	virtual ~AnalysisProcess()
	{
	}

	inline virtual const Id& getTypeId() const
	{
		return getStaticTypeId();
	}

	static const Id& getStaticTypeId()
	{
		static const Id id("36c128e0-b14a-4f35-a317-d972d28f1802");
		return id;
	}

	virtual void serialize(const OutputStream &out) const
	{
		ObjectManager::serialize(out);
	}

	virtual void deserialize(const InputStream &in)
	{
		ObjectManager::deserialize(in);
	}

	/// This method can be reimplemented to build/destroy
	/// a static template for user-defined tree creation. \p mode is a freely usable parameter.
	virtual void buildTemplate(int mode = 0)
	{
	}

	/// This method can be reimplemented to hold hep analysis code executed at the begin of a computing job
	/// (as needed for histogram booking etc.).
	/// The optional parameter \p input is a const pointer to a pxl::Serializable instance (which can carry any 
	/// PXL object and may e.g. provide general information for the further processing).
	virtual void beginJob(const Serializable* input = 0)
	{
	}
	/// This method can be reimplemented to hold hep analysis code executed at the begin of a run.
	/// The optional parameter \p input is a const pointer to a pxl::Serializable instance (which can carry any 
	/// PXL object and may e.g. provide general information for the further processing).
	virtual void beginRun(const Serializable* input = 0)
	{
	}
	/// This method can be reimplemented to hold hep analysis code executed for the actual event analysis.
	/// The optional parameter \p event is a const pointer to a pxl::Event, the current event being analysed.
	virtual void analyseEvent(const Event* event = 0)
	{
	}
	/// This method can be reimplemented to hold hep analysis code executed at the end of each event.
	/// The optional parameter \p event is a const pointer to a pxl::Event, the current event being analysed.
	/// By default, this method deletes all objects owned by this AnalysisProcess.
	virtual void finishEvent(const Event* event = 0)
	{
		clearObjects();
	}
	/// This method can be reimplemented to hold hep analysis code executed at the end of a run.
	/// The optional parameter \p input is a const pointer to a pxl::Serializable instance (which can carry any 
	/// PXL object and may e.g. provide general information for the processing).
	virtual void endRun(const Serializable* input = 0)
	{
	}
	/// This method can be reimplemented to hold hep analysis code executed at the end of a computing job
	/// (as needed for histogram storing etc.).
	/// The optional parameter \p input is a const pointer to a pxl::Serializable instance (which can carry any 
	/// PXL object and may e.g. provide general information for the processing).
	virtual void endJob(const Serializable* input = 0)
	{
	}

	virtual Serializable* clone() const;

	virtual WkPtrBase* createSelfWkPtr()
	{
		return new weak_ptr<AnalysisProcess>(this);
	}

	virtual std::ostream& print(int level=1, std::ostream& os=std::cout, int pan=0) const;

private:
	AnalysisProcess& operator=(const AnalysisProcess& original)
	{
		return *this;
	}
};

} // namespace pxl


#endif // PXL_HEP_ANALYSIS_PROCESS_HH
