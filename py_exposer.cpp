#include <boost/python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/module_init.hpp>
#include <boost/python/def.hpp>
#include <boost/python/call_method.hpp>
#include <boost/ref.hpp>
#include <boost/utility.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "common.h"
#include "player.h"
#include "effect.h"
#include "audioout.h"
#include "alsa.h"
#include "preamp.h"
#include "eq.h"
#include "fir.h"
#include "ffmpeg.h"
#include "threadpool.h"
#include "componentmanager.h"

using namespace boost::python;

class GILLock
{
public:
    GILLock()  { state_ = PyGILState_Ensure(); }
    ~GILLock() { PyGILState_Release(state_);   }
private:
    PyGILState_STATE state_;
};

class PlayerEventsImpl : public Vrok::Player::Events, public wrapper<Vrok::Player::Events>
{
public:
    void QueueNext()
    {
        GILLock lock;
        this->get_override("QueueNext")();
    }
};

class ScopedGILRelease
{
public:
    inline ScopedGILRelease()
    {
        m_thread_state = PyEval_SaveThread();
    }

    inline ~ScopedGILRelease()
    {
        PyEval_RestoreThread(m_thread_state);
        m_thread_state = NULL;
    }

private:
    PyThreadState * m_thread_state;
};

class ThreadPool_PyWrap : public Vrok::ThreadPool
{
public:
    ThreadPool_PyWrap(size_t size) :
        Vrok::ThreadPool(size)
    {}

    void JoinThreads()
    {
        std::cout<<"542352"<<std::endl;
        ScopedGILRelease scope;
        Vrok::ThreadPool::JoinThreads();
    }

};
BOOST_PYTHON_MODULE(vrok)
{
    PyEval_InitThreads();

    class_<std::vector<Vrok::VUMeter*>>("VUMeterList")
            .def(vector_indexing_suite<std::vector<Vrok::VUMeter*>>());

    class_<BufferGraph::Point, boost::noncopyable>("Point",boost::python::no_init);
    class_<Runnable, boost::noncopyable>("Runnable",boost::python::no_init);
    class_<Vrok::Decoder, boost::noncopyable>("Decoder",boost::python::no_init);
    class_<PlayerEventsImpl, boost::noncopyable>("PlayerEvents" ,boost::python::init<>())
            .def("QueueNext", pure_virtual(&Vrok::Player::Events::QueueNext));

    class_<Vrok::Component,  boost::noncopyable>("Component" ,boost::python::no_init);
    class_<Vrok::PropertyBase,boost::noncopyable>("PropertyBase" ,boost::python::no_init);

    class_<Vrok::Property<float>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyFloat",boost::python::init<>());
    class_<Vrok::Property<double>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyDouble",boost::python::init<>());
    class_<Vrok::Property<int>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyInteger",boost::python::init<>());

    class_<Vrok::ComponentManager,boost::noncopyable >("ComponentManager", boost::python::no_init)
            .def("GetSingleton", &Vrok::ComponentManager::GetSingleton, return_value_policy<reference_existing_object>())
            .def("GetComponent", &Vrok::ComponentManager::GetComponent, return_value_policy<reference_existing_object>())
            .def("SetProperty", &Vrok::ComponentManager::SetProperty, return_value_policy<reference_existing_object>())
            .def("GetProperty", &Vrok::ComponentManager::GetProperty, return_value_policy<reference_existing_object>())
            .staticmethod("GetSingleton");

    class_<Vrok::Resource, boost::noncopyable>("Resource", boost::python::init<>())
            .def_readwrite("filename", &Vrok::Resource::_filename);
    class_<Vrok::DecoderFFMPEG, bases<Vrok::Decoder>, boost::noncopyable>("DecoderFFMPEG", boost::python::init<>())
            .def("Open",&Vrok::DecoderFFMPEG::Open)
            .def("Close",&Vrok::DecoderFFMPEG::Close);
    class_<Vrok::Player, bases<BufferGraph::Point,Vrok::Component, Runnable>,boost::noncopyable>("Player",boost::python::init<>())
            .def("SubmitForPlayback",&Vrok::Player::SubmitForPlayback)
            .def("Resume",&Vrok::Player::Resume)
            .def("Pause",&Vrok::Player::Pause)
            .def("Stop",&Vrok::Player::Stop)
            .def("Skip",&Vrok::Player::Skip)
            .def("Flush", &Vrok::Player::Flush)
            .def("SetEvents",&Vrok::Player::SetEvents)
            .def("Preallocate",&Vrok::Player::Preallocate)
            .def("RegisterSink", &Vrok::Player::RegisterSink);

    class_<ThreadPool_PyWrap, boost::noncopyable>("ThreadPool",boost::python::init<std::size_t>())
            .def("RegisterWork",&ThreadPool_PyWrap::RegisterWork)
            .def("StopThreads",&ThreadPool_PyWrap::StopThreads)
            .def("CreateThreads",&ThreadPool_PyWrap::CreateThreads)
            .def("JoinThreads",&ThreadPool_PyWrap::JoinThreads);

    class_<Vrok::EffectFIR, bases<BufferGraph::Point,Vrok::Component, Runnable>, boost::noncopyable>("EffectFIR",boost::python::init<>())
            .def("Flush", &Vrok::EffectSSEQ::Flush)
            .def("GetMeters", &Vrok::EffectFIR::GetMeters)
            .def("Preallocate",&Vrok::Player::Preallocate)
            .def("RegisterSink", &Vrok::EffectFIR::RegisterSink)
            .def("RegisterSource", &Vrok::EffectFIR::RegisterSource);
    class_<Vrok::EffectSSEQ, bases<BufferGraph::Point, Vrok::Component,Runnable>, boost::noncopyable>("EffectSSEQ",boost::python::init<>())
            .def("Flush", &Vrok::EffectSSEQ::Flush)
            .def("GetMeters", &Vrok::EffectSSEQ::GetMeters)
            .def("Preallocate",&Vrok::Player::Preallocate)
            .def("RegisterSink", &Vrok::EffectSSEQ::RegisterSink)
            .def("RegisterSource", &Vrok::EffectSSEQ::RegisterSource);
    class_<Vrok::VUMeter, boost::noncopyable>("VUMeter",boost::python::init<std::string>())
            .def("GetValue", &Vrok::VUMeter::GetValue)
            .def("GetClip", &Vrok::VUMeter::GetClip);
    class_<Vrok::DriverAlsa, bases<BufferGraph::Point, Vrok::Component, Runnable>, boost::noncopyable>("DriverAlsa",boost::python::init<>())
            .def("Flush", &Vrok::DriverAlsa::Flush)
            .def("GetMeters", &Vrok::DriverAlsa::GetMeters)
            .def("Preallocate",&Vrok::Player::Preallocate)
            .def("RegisterSource", &Vrok::DriverAlsa::RegisterSource);


//    class_<Vrok::VUMeter>("VUMeter");

}
