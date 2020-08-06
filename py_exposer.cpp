#include <boost/python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/module_init.hpp>
#include <boost/python/def.hpp>
#include <boost/python/call_method.hpp>
#include <boost/ref.hpp>
#include <boost/utility.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/numpy.hpp>

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
#include "jbufferout.h"
#include "notifier_impl.h"

using namespace boost::python;

namespace p = boost::python;
namespace np = boost::python::numpy;

class GILLock
{
public:
    GILLock()  { state_ = PyGILState_Ensure(); }
    void release()
    {
        if (!released_)
        {
            PyGILState_Release(state_);
            released_ = true;
        }
    }

    ~GILLock() {  release();  }
private:
    PyGILState_STATE state_;
    bool released_ = false;
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
        ScopedGILRelease scope;
        Vrok::ThreadPool::JoinThreads();
    }

};
int arr[] = {1,2,3,4};
class DriverPyOutImpl : public Vrok::DriverJBufferOut::Events, public wrapper<Vrok::DriverJBufferOut::Events>
{
public:
    void OnThreadStart()
    {
        Vrok::Notify::GetInstance()->NotifyInformation("onTh start");

    }
    void OnThreadEnd()
    {
        Vrok::Notify::GetInstance()->NotifyInformation("onTh end");
    }
    void OnBuffer(double* buffer, int frames, StreamType type) override
    {

        for (int f=0;f<_frames;f++)
        {
            for (int i=0;i<_channels;i++)
            {
                _buffer[_frames*i + f] = buffer[_channels * f + i];
            }
        }


        GILLock lock;
        try {
        np::ndarray ndArray = np::from_data(_buffer,
                                        np::dtype::get_builtin<double>(),
                                        p::make_tuple(_channels, _frames),
                                        p::make_tuple(_frames * sizeof(double), sizeof(double)),
                                        p::object());
          this->get_override("OnBuffer")(ndArray);
        }
        catch (boost::python::error_already_set& e)
        {
            PyErr_Print();
        }


    }
    void OnBufferConfigChange(int frames, int samplerate, int channels) override
    {

        _channels = channels;
        _frames = frames;
        _samplerate = samplerate;
        delete[] _buffer;
        _buffer = new double[_channels * _frames];

        GILLock lock;
        try
        {
            this->get_override("OnBufferConfigChange")(frames, samplerate, channels);
        }
        catch (boost::python::error_already_set& e)
        {
            PyErr_Print();
        }

    }
private:
    int _channels=0;
    int _frames=0;
    int _samplerate=0;
    double* _buffer=nullptr;
};

BOOST_PYTHON_MODULE(vrok)
{

    np::initialize();

    PyEval_InitThreads();


    Vrok::Notify::GetInstance()->SetNotifier(new CNotifier());

    class_<std::vector<Vrok::VUMeter*>>("VUMeterList")
            .def(vector_indexing_suite<std::vector<Vrok::VUMeter*>>());

    class_<BufferGraph::Point, boost::noncopyable>("Point",boost::python::no_init);
    class_<Runnable, boost::noncopyable>("Runnable",boost::python::no_init);
    class_<Vrok::Decoder, boost::noncopyable>("Decoder",boost::python::no_init);
    class_<PlayerEventsImpl, boost::noncopyable>("PlayerEvents" ,boost::python::init<>())
            .def("QueueNext", pure_virtual(&Vrok::Player::Events::QueueNext));

    class_<DriverPyOutImpl, boost::noncopyable>("DriverPyOut" ,boost::python::init<>())
            .def("OnBuffer", pure_virtual(&Vrok::DriverJBufferOut::Events::OnBuffer))
            .def("OnBufferConfigChange", pure_virtual(&Vrok::DriverJBufferOut::Events::OnBufferConfigChange));

    class_<Vrok::Component,  boost::noncopyable>("Component" ,boost::python::no_init);
    class_<Vrok::PropertyBase,boost::noncopyable>("PropertyBase" ,boost::python::no_init);

    class_<Vrok::Property<float>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyFloat",boost::python::init<>());
    class_<Vrok::Property<double>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyDouble",boost::python::init<>());
    class_<Vrok::Property<int>,bases<Vrok::PropertyBase>, boost::noncopyable>("PropertyInteger",boost::python::init<>());

    class_<Vrok::Driver::DeviceInfo, boost::noncopyable>("DeviceInfo", boost::python::no_init)
            .def_readonly("name", &Vrok::Driver::DeviceInfo::name);

    class_<std::vector<Vrok::Driver::DeviceInfo>>("DeviceInfoList")
            .def(vector_indexing_suite<std::vector<Vrok::Driver::DeviceInfo>>());
    Vrok::PropertyBase* (Vrok::ComponentManager::*get_property_func)(Vrok::Component*, std::string);
    get_property_func = &Vrok::ComponentManager::GetProperty;
    //    void SetProperty(Component *component, PropertyBase *property, std::string value);
    void (Vrok::ComponentManager::*set_property_func)(Vrok::Component*, Vrok::PropertyBase*, std::string);
    set_property_func = &Vrok::ComponentManager::SetProperty;


    class_<Vrok::ComponentManager,boost::noncopyable >("ComponentManager", boost::python::no_init)
            .def("GetSingleton", &Vrok::ComponentManager::GetSingleton, return_value_policy<reference_existing_object>())
            .def("GetComponent", &Vrok::ComponentManager::GetComponent, return_value_policy<reference_existing_object>())
            .def("SetProperty", set_property_func)
            .def("GetProperty", get_property_func, return_value_policy<reference_existing_object>())
            .staticmethod("GetSingleton");

    class_<Vrok::Resource, boost::noncopyable>("Resource", boost::python::init<>())
            .def_readwrite("filename", &Vrok::Resource::_filename);
    class_<Vrok::DecoderFFMPEG, bases<Vrok::Decoder>, boost::noncopyable>("DecoderFFMPEG", boost::python::init<>())
            .def("Open",&Vrok::DecoderFFMPEG::Open)
            .def("Close",&Vrok::DecoderFFMPEG::Close)
            .def("Create",&Vrok::DecoderFFMPEG::Create, return_value_policy<reference_existing_object>())
            .staticmethod("Create");

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
            .def("Preallocate",&Vrok::DriverAlsa::Preallocate)
            .def("SetVolume",&Vrok::DriverAlsa::SetVolume)
            .def("RegisterSource", &Vrok::DriverAlsa::RegisterSource)
            .def("SetDevice", &Vrok::DriverAlsa::SetDevice)
            .def("GetDeviceInfo", &Vrok::DriverAlsa::GetDeviceInfo)
            .def("GetDefaultDevice", &Vrok::DriverAlsa::GetDefaultDevice);

    class_<Vrok::DriverJBufferOut, bases<BufferGraph::Point, Vrok::Component, Runnable>, boost::noncopyable>("DriverJBufferOut",boost::python::init<>())
            .def("Flush", &Vrok::DriverJBufferOut::Flush)
            .def("Preallocate",&Vrok::DriverJBufferOut::Preallocate)
            .def("SetVolume",&Vrok::DriverJBufferOut::SetVolume)
            .def("RegisterSource", &Vrok::DriverJBufferOut::RegisterSource)
            .def("SetVolume",&Vrok::DriverJBufferOut::SetVolume)
            .def("SetDevice", &Vrok::DriverJBufferOut::SetDevice)
            .def("GetDeviceInfo", &Vrok::DriverJBufferOut::GetDeviceInfo)
            .def("GetDefaultDevice", &Vrok::DriverJBufferOut::GetDefaultDevice)
            .def("SetEvents", &Vrok::DriverJBufferOut::SetEvents);


//    class_<Vrok::VUMeter>("VUMeter");

}
