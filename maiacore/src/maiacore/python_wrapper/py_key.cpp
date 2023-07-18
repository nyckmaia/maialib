#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "maiacore/key.h"
namespace py = pybind11;

void KeyClass(const py::module& m) {
    m.doc() = "Key class binding";

    // bindings to Key class
    py::class_<Key> cls(m, "Key");
    cls.def(py::init<int, bool>(), py::arg("fifthCircle") = 0, py::arg("isMajorMode") = true);
    cls.def(py::init<const std::string&>(), py::arg("key"));

    cls.def("setFifthCircle", &Key::setFifthCircle, py::arg("fifthCircle"));
    cls.def("getFifthCircle", &Key::getFifthCircle);

    cls.def("setIsMajorMode", &Key::setIsMajorMode, py::arg("isMajorMode"));
    cls.def("isMajorMode", &Key::isMajorMode);

    cls.def("getName", &Key::getName);

    cls.def("getRelativeKeyName", &Key::getRelativeKeyName);

    // cls.def("toXML", &Key::toXML, py::arg("identSize") = 2);

    // Default Python 'print' function:
    cls.def("__repr__", [](const Key& key) { return "<Key " + key.getName() + ">"; });

    cls.def("__hash__", [](const Key& key) { return std::hash<std::string>{}(key.getName()); });

    cls.def("__sizeof__", [](const Key& key) { return sizeof(key); });
}
