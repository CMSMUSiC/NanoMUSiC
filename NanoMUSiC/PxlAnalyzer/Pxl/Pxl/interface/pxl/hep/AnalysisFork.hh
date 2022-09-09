//-------------------------------------------
// Project: Physics eXtension Library (PXL) -
//      http://vispa.physik.rwth-aachen.de/ -
// Copyright (C) 2006-2015 Martin Erdmann   -
//               RWTH Aachen, Germany       -
// Licensed under a LGPL-2 or later license -
//-------------------------------------------

#ifndef PXL_HEP_ANALYSIS_FORK_HH
#define PXL_HEP_ANALYSIS_FORK_HH

#include "Pxl/Pxl/interface/pxl/core/Event.hh"
#include "Pxl/Pxl/interface/pxl/core/ObjectManager.hh"
#include "Pxl/Pxl/interface/pxl/core/macros.hh"

namespace pxl
{

/**
 This class is designed as a base class to assist the analyzer in the
 parallel evolution of different hep process hypotheses or the analysis of different
 (instrumental) aspects of an event.
 When calling beginRun() or corresponding methods, the beginRun() methods of all
 contained objects of type AnalysisFork and AnalysisProcess will be called.
 @deprecated use the Module system instead
 */
class PXL_DLL_EXPORT AnalysisFork : public ObjectManager
{
  public:
    AnalysisFork() : ObjectManager()
    {
    }
    AnalysisFork(const AnalysisFork &original) : ObjectManager(original)
    {
    }
    explicit AnalysisFork(const AnalysisFork *original) : ObjectManager(original)
    {
    }
    virtual ~AnalysisFork()
    {
    }

    inline virtual const Id &getTypeId() const
    {
        return getStaticTypeId();
    }

    static const Id &getStaticTypeId()
    {
        static const Id id("91b6a6ec-4ecf-490f-ba92-47d20e42bc16");
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

    /// This method can be reimplemented to hold hep analysis code executed at the begin of a computing job.
    /// The optional parameter \p input is a const pointer to a pxl::Serializable (which can be any serializable PXL
    /// object). By default, this method invokes the corresponding method of all managed pxl::AnalysisProcess instances,
    /// passing the parameter \p input to the according method of each instance.
    virtual void beginJob(const Serializable *input = 0);

    /// This method can be reimplemented to hold hep analysis code executed at the begin of a run.
    /// The optional parameter \p input is a const pointer to a pxl::Serializable (which can be any serializable PXL
    /// object). By default, this method invokes the corresponding method of all managed pxl::AnalysisProcess instances,
    /// passing the parameter \p input to the according method of each instance.
    virtual void beginRun(const Serializable *input = 0);

    /// This method can be reimplemented to hold hep analysis code executed for the actual event analysis.
    /// The optional parameter \p input is a const pointer to a pxl::Event (that might carry the reconstructed event
    /// data or generator information). By default, this method invokes the corresponding method of all managed
    /// pxl::AnalysisProcess instances, passing the parameter \p event to the according method of each instance.
    virtual void analyseEvent(const Event *event = 0);

    /// This method can be reimplemented to hold hep analysis code executed at the end of each event.
    /// The optional parameter \p input is a const pointer to a pxl::Event (that might carry the reconstructed event
    /// data or generator information). By default, this method invokes the corresponding method of all managed
    /// pxl::AnalysisProcess instances, passing the parameter \p event to the according method of each instance.
    virtual void finishEvent(const Event *event = 0);

    /// This method can be reimplemented to hold hep analysis code executed at the end of a run.
    /// The optional parameter \p input is a const pointer to a pxl::Serializable (which can be any serializable PXL
    /// object). By default, this method invokes the corresponding method of all managed pxl::AnalysisProcess instances,
    /// passing the parameter \p input to the according method of each instance.
    virtual void endRun(const Serializable *input = 0);

    /// This method can be reimplemented to hold hep analysis code executed at the end of a computing job
    /// (as needed for histogram storing etc.).
    /// The optional parameter \p input is a const pointer to a pxl::Serializable (which can be any serializable PXL
    /// object). By default, this method invokes the corresponding method of all managed pxl::AnalysisProcess instances,
    /// passing the parameter \p input to the according method of each instance.
    virtual void endJob(const Serializable *input = 0);

    virtual Serializable *clone() const;

    virtual std::ostream &print(int level = 1, std::ostream &os = std::cout, int pan = 0) const;

    virtual WkPtrBase *createSelfWkPtr()
    {
        return new weak_ptr<AnalysisFork>(this);
    }

  private:
    AnalysisFork &operator=(const AnalysisFork &original)
    {
        return *this;
    }
};

} // namespace pxl

#endif // PXL_HEP_ANALYSIS_FORK_HH
