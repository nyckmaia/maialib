#pragma once

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>

#include <iostream>

namespace maiacore_python {

namespace py = pybind11;

// Python's sys.stdout or sys.stderr ('name') behind a writer that does not fail on a character
// the stream cannot encode, such as a file name in Japanese on a stream in the Windows ANSI code
// page: that text is written with backslash escapes instead. pybind11's redirection writes the
// C++ output to the stream from a destructor too, where an exception ends the process.
inline py::object tolerantStream(const char* name) {
    const py::object stream = py::module_::import("sys").attr(name);
    const py::object write = stream.attr("write");
    const py::cpp_function tolerantWrite([stream, write](const py::str& text) {
        try {
            write(text);
        } catch (py::error_already_set& error) {
            if (!error.matches(PyExc_UnicodeEncodeError)) {
                throw;
            }
            const py::object encoding = py::getattr(stream, "encoding", py::none());
            const py::str codec = encoding.is_none() ? py::str("ascii") : py::str(encoding);
            write(text.attr("encode")(codec, "backslashreplace").attr("decode")(codec));
        }
    });
    return py::module_::import("types").attr("SimpleNamespace")(
        py::arg("write") = tolerantWrite, py::arg("flush") = stream.attr("flush"));
}

// The call guard of the bindings that load scores: while the call runs, std::cout and std::cerr
// go to Python's sys.stdout and sys.stderr through tolerantStream().
class ConsoleRedirect {
   public:
    ConsoleRedirect()
        : _out(std::cout, tolerantStream("stdout")), _err(std::cerr, tolerantStream("stderr")) {}

   private:
    py::scoped_ostream_redirect _out;
    py::scoped_ostream_redirect _err;
};

}  // namespace maiacore_python
