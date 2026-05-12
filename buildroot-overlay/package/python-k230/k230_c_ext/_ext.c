#include <Python.h>
#include <stdio.h>

static PyObject* hello_hello(PyObject* self, PyObject* args) {
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    printf("wwwwwwwHelloWorld from C Extension!\n");
    Py_RETURN_NONE;
}

static PyMethodDef HelloMethods[] = {
    {"hello", hello_hello, METH_VARARGS, "Print HelloWorld"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef hellomodule = {
    PyModuleDef_HEAD_INIT,
    "k230_ext",
    NULL,
    -1,
    HelloMethods
};

PyMODINIT_FUNC PyInit_k230_ext(void) {
    return PyModule_Create(&hellomodule);
}
