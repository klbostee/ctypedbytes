/* 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Python.h>

#define BOOL_TC   2
#define INT_TC    3
#define LONG_TC   4
#define STRING_TC 7
#define VECTOR_TC 8
#define LIST_TC   9
#define MAP_TC    10
#define MARKER_TC 255

static inline int
_fastb_read_bool(FILE *stream) {
  unsigned char c;
  fread(&c, sizeof(unsigned char), 1, stream);
  return (int) c;
}

static inline int 
_fastb_write_bool(FILE *stream, int b) {
  unsigned char c[2];
  c[0] = BOOL_TC; /* typecode */
  c[1] = b;
  return fwrite(&c, sizeof(unsigned char), 2, stream) == 2;
}

static inline long
_fastb_read_int(FILE *stream) {
  unsigned char c[4];
  fread(&c, sizeof(unsigned char), 4, stream);
  return ((c[0] & 255) << 24) + ((c[1] & 255) << 16) +
         ((c[2] & 255) <<  8) + ((c[3] & 255)      );
}

static inline int 
_fastb_write_int(FILE *stream, long i) {
  unsigned char c[5];
  c[0] = INT_TC; /* typecode */
  c[1] = (i >> 24) & 255;
  c[2] = (i >> 16) & 255;
  c[3] = (i >>  8) & 255;
  c[4] = (i      ) & 255;
  return fwrite(&c, sizeof(unsigned char), 5, stream) == 5;
}

static inline long long
_fastb_read_long(FILE *stream) {
  unsigned char c[8];
  fread(&c, sizeof(unsigned char), 8, stream);
  return (((long long) c[0] & 255) << 56) + 
         (((long long) c[1] & 255) << 48) +
         (((long long) c[2] & 255) << 40) + 
         (((long long) c[3] & 255) << 32) +
         ((c[4] & 255) << 24) + ((c[5] & 255) << 16) +
         ((c[6] & 255) <<  8) + ((c[7] & 255)      );
}

static inline int 
_fastb_write_long(FILE *stream, long long l) {
  unsigned char c[9];
  c[0] = LONG_TC; /* typecode */
  c[1] = (l >> 56) & 255;
  c[2] = (l >> 48) & 255;
  c[3] = (l >> 40) & 255;
  c[4] = (l >> 32) & 255;
  c[5] = (l >> 24) & 255;
  c[6] = (l >> 16) & 255;
  c[7] = (l >>  8) & 255;
  c[8] = (l      ) & 255;
  return fwrite(&c, sizeof(unsigned char), 9, stream) == 9;
}

static inline int
_fastb_read_string(FILE *stream, char *s, int len) {
  if (fread(s, sizeof(char), len, stream) != len)
    return 0;
  s[len] = '\0';
  return 1;
}

static inline int
_fastb_write_string(FILE *stream, char *s) {
  unsigned char c[5];
  int len;
  len = strlen(s);
  c[0] = STRING_TC; /* typecode */
  c[1] = (len >> 24) & 255;
  c[2] = (len >> 16) & 255;
  c[3] = (len >>  8) & 255;
  c[4] = (len      ) & 255;  
  if (fwrite(&c, sizeof(unsigned char), 5, stream) != 5)
    return 0;
  return fwrite(s, sizeof(char), len, stream) == len;
}

static inline int 
_fastb_write_vector_header(FILE *stream, int size) {
  unsigned char c[5];
  c[0] = VECTOR_TC; /* typecode */
  c[1] = (size >> 24) & 255;
  c[2] = (size >> 16) & 255;
  c[3] = (size >>  8) & 255;
  c[4] = (size      ) & 255;
  return fwrite(&c, sizeof(unsigned char), 5, stream) == 5;
}

static inline int 
_fastb_write_list_header(FILE *stream) {
  unsigned char c;
  c = LIST_TC; /* typecode */
  return fwrite(&c, sizeof(unsigned char), 1, stream) == 1;
}

static inline int
_fastb_write_list_footer(FILE *stream) {
  unsigned char c;
  c = MARKER_TC; /* typecode */
  return fwrite(&c, sizeof(unsigned char), 1, stream) == 1;  
}

static inline int
_fastb_write_map_header(FILE *stream, int size) {
  unsigned char c[5];
  c[0] = MAP_TC; /* typecode */
  c[1] = (size >> 24) & 255;
  c[2] = (size >> 16) & 255;
  c[3] = (size >>  8) & 255;
  c[4] = (size      ) & 255;
  return fwrite(&c, sizeof(unsigned char), 5, stream) == 5;
}


static PyObject *
_fastb_read_pyobj(FILE *stream, PyObject *fallback);

static int
_fastb_write_pyobj(FILE *stream, PyObject *pyobj, PyObject *fallback);

static PyObject *
_fastb_read_pyobj_fallback(FILE *stream, int typecode, PyObject *fallback) {
  PyObject *args, *handler, *ret;
  args = Py_BuildValue("(i)", typecode);
  handler = PyEval_CallObject(fallback, args);
  Py_DECREF(args);
  args = PyTuple_Pack(0);
  ret = PyEval_CallObject(handler, args);
  Py_DECREF(args);
  Py_DECREF(handler);
  return ret;
}

static int
_fastb_write_pyobj_fallback(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  PyObject *type, *args, *handler, *ret;
  type = PyObject_Type(pyobj);
  args = PyTuple_Pack(1, type);
  handler = PyEval_CallObject(fallback, args);
  Py_DECREF(args);
  Py_DECREF(type);
  args = PyTuple_Pack(1, pyobj);
  ret = PyEval_CallObject(handler, args);
  Py_DECREF(args);
  Py_DECREF(handler);
  if (ret == NULL)
    return 0;
  Py_DECREF(ret);
  return 1;
}

static PyObject *
_fastb_read_pyobj_bool(FILE *stream, PyObject *fallback) {
  return PyBool_FromLong(_fastb_read_bool(stream));
}

static int
_fastb_write_pyobj_bool(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  return _fastb_write_bool(stream, (unsigned char) PyInt_AS_LONG(pyobj));
}

static PyObject *
_fastb_read_pyobj_int(FILE *stream, PyObject *fallback) {
  return PyInt_FromLong(_fastb_read_int(stream));
}

static int 
_fastb_write_pyobj_int(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  long l = PyInt_AS_LONG(pyobj);
  if (-2147483647L <= l && l <= 2147483647L) {
    return _fastb_write_int(stream, l);
  } else {
    return _fastb_write_long(stream, l);
  }
}

static PyObject *
_fastb_read_pyobj_long(FILE *stream, PyObject *fallback) {
  return PyLong_FromLongLong(_fastb_read_long(stream));
}

static int
_fastb_write_pyobj_long(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  long long l = PyLong_AsLongLong(pyobj);
  if (-9223372036854775807LL <= l && l <= 9223372036854775807LL) {
    return _fastb_write_long(stream, l);
  } else {
    return _fastb_write_pyobj_fallback(stream, pyobj, fallback);
  }
}

static PyObject *
_fastb_read_pyobj_string(FILE *stream, PyObject *fallback) {
  PyStringObject *string;
  Py_ssize_t len;
  len = _fastb_read_int(stream);
  string = (PyStringObject *) PyString_FromStringAndSize(NULL, len);
  if (!_fastb_read_string(stream, string->ob_sval, len))
    return NULL;
  return (PyObject *) string;
}

static int 
_fastb_write_pyobj_string(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  return _fastb_write_string(stream, PyString_AS_STRING(pyobj));
}

static PyObject *
_fastb_read_pyobj_tuple(FILE *stream, PyObject *fallback) {
  PyObject *tuple;
  Py_ssize_t i, size;
  size = _fastb_read_int(stream);
  tuple = PyTuple_New(size);
  for (i = 0; i < size; i++) {
    PyTuple_SET_ITEM(tuple, i, _fastb_read_pyobj(stream, fallback));
  }
  return tuple;
}

static int 
_fastb_write_pyobj_tuple(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  Py_ssize_t i, size;
  size = PyTuple_GET_SIZE(pyobj);
  _fastb_write_vector_header(stream, size);
  for (i = 0; i < size; i++) {
    if (!_fastb_write_pyobj(stream, PyTuple_GET_ITEM(pyobj, i), fallback))
      return 0;
  }
  return 1;
}

static PyObject *
_fastb_read_pyobj_list(FILE *stream, PyObject *fallback) {
  PyObject *list, *item;
  list = PyList_New(0);
  while ((item = _fastb_read_pyobj(stream, fallback))) {
    PyList_Append(list, item);
  }
  return list;
}

static int 
_fastb_write_pyobj_list(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  Py_ssize_t i, size;
  size = PyList_GET_SIZE(pyobj);
  _fastb_write_list_header(stream);
  for (i = 0; i < size; i++) {
    if (!_fastb_write_pyobj(stream, PyList_GET_ITEM(pyobj, i), fallback))
      return 0;
  }
  _fastb_write_list_footer(stream);
  return 1;
}

static PyObject *
_fastb_read_pyobj_dict(FILE *stream, PyObject *fallback) {
  PyObject *dict;
  Py_ssize_t i, size;
  size = _fastb_read_int(stream);
  dict = PyDict_New();
  for (i = 0; i < size; i++) {
    PyObject *key, *value;
    key = _fastb_read_pyobj(stream, fallback);
    value = _fastb_read_pyobj(stream, fallback);
    PyDict_SetItem(dict, key, value);
  }
  return dict;
}

static int
_fastb_write_pyobj_dict(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  PyObject *key, *value;
  Py_ssize_t pos = 0;
  _fastb_write_map_header(stream, PyDict_Size(pyobj));
  while (PyDict_Next(pyobj, &pos, &key, &value)) {
    if (!_fastb_write_pyobj(stream, key, fallback))
      return 0;
    if (!_fastb_write_pyobj(stream, value, fallback))
      return 0;
  }
  return 1;
}

typedef PyObject * (*_fastb_read_cb) (FILE *, PyObject *);

static _fastb_read_cb _read_cb_table[256];

static void
_fastb_init_read_cb_table(void) {
  int i;
  for (i = 0; i < 256; i++) {
    _read_cb_table[i] = NULL;
  }
  _read_cb_table[BOOL_TC] = _fastb_read_pyobj_bool;
  _read_cb_table[INT_TC] = _fastb_read_pyobj_int;
  _read_cb_table[LONG_TC] = _fastb_read_pyobj_long;
  _read_cb_table[STRING_TC] = _fastb_read_pyobj_string;
  _read_cb_table[VECTOR_TC] = _fastb_read_pyobj_tuple;
  _read_cb_table[LIST_TC] = _fastb_read_pyobj_list;
  _read_cb_table[MAP_TC] = _fastb_read_pyobj_dict;
}

static PyObject *
_fastb_read_pyobj(FILE *stream, PyObject *fallback) {
  unsigned char typecode;
  _fastb_read_cb callback;
  if (!fread(&typecode, sizeof(unsigned char), 1, stream))
    return NULL;
  if (typecode == MARKER_TC)
    return NULL;
  callback = _read_cb_table[typecode];
  if (callback == NULL)
    return _fastb_read_pyobj_fallback(stream, typecode, fallback);
  return callback(stream, fallback);
}

static int
_fastb_write_pyobj(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  if (PyBool_Check(pyobj)) {
    return _fastb_write_pyobj_bool(stream, pyobj, fallback);
  } else if (PyInt_CheckExact(pyobj)) {   
    return _fastb_write_pyobj_int(stream, pyobj, fallback);
  } else if (PyLong_CheckExact(pyobj)) {
    return _fastb_write_pyobj_long(stream, pyobj, fallback);  
  } else if (PyString_CheckExact(pyobj)) {
    return _fastb_write_pyobj_string(stream, pyobj, fallback);
  } else if (PyTuple_CheckExact(pyobj)) {
    return _fastb_write_pyobj_tuple(stream, pyobj, fallback);
  } else if (PyList_CheckExact(pyobj)) {
    return _fastb_write_pyobj_list(stream, pyobj, fallback);
  } else if (PyDict_CheckExact(pyobj)) {
    return _fastb_write_pyobj_dict(stream, pyobj, fallback);
  } else {
    return _fastb_write_pyobj_fallback(stream, pyobj, fallback);
  }
}


typedef struct {
  PyObject_HEAD
  FILE *stream;
  PyObject *fallback;
} readiterobject;

static PyTypeObject readitertype;

PyObject *
readiter_new(FILE *stream, PyObject *fallback)
{
  readiterobject *rio;
  rio = PyObject_GC_New(readiterobject, &readitertype);
  if (rio == NULL)
    return NULL;
  rio->stream = stream;
  Py_INCREF(fallback);
  rio->fallback = fallback;
  _PyObject_GC_TRACK(rio);
  return (PyObject *) rio;
}

static void
readiter_dealloc(readiterobject *rio)
{
  _PyObject_GC_UNTRACK(rio);
  Py_XDECREF(rio->fallback);
  PyObject_GC_Del(rio);
}

static int
readiter_traverse(readiterobject *rio, visitproc visit, void *arg)
{
  Py_VISIT(rio->fallback);
  return 0;
}

static PyObject *
readiter_iternext(readiterobject *rio)
{
  return _fastb_read_pyobj(rio->stream, rio->fallback);
}

static PyTypeObject readitertype = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "fastb.readiter",                        /* tp_name */
  sizeof(readiterobject),                  /* tp_basicsize */
  0,                                       /* tp_itemsize */
  (destructor)readiter_dealloc,            /* tp_dealloc */
  0,			                   /* tp_print */
  0,				           /* tp_getattr */
  0,				           /* tp_setattr */
  0,				           /* tp_compare */
  0,			                   /* tp_repr */
  0,			                   /* tp_as_number */
  0,				           /* tp_as_sequence */
  0,				           /* tp_as_mapping */
  0,				           /* tp_hash */
  0,				           /* tp_call */
  0,			                   /* tp_str */
  PyObject_GenericGetAttr,   	           /* tp_getattro */
  0,				           /* tp_setattro */
  0,				           /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
  0, 				           /* tp_doc */
  (traverseproc)readiter_traverse,         /* tp_traverse */
  0,				           /* tp_clear */
  0,				           /* tp_richcompare */
  0,				           /* tp_weaklistoffset */
  PyObject_SelfIter,		           /* tp_iter */
  (iternextfunc)readiter_iternext,         /* tp_iternext */
  0,				           /* tp_methods */
};


static PyObject *
_fastb_read_pyiter(FILE *stream, PyObject *fallback) {
  return readiter_new(stream, fallback);
}

static int 
_fastb_write_pyiter(FILE *stream, PyObject *pyobj, PyObject *fallback) {
  PyObject *iterator, *item;
  iterator = PyObject_GetIter(pyobj);
  if (iterator == NULL)
    return 0;
  while ((item = PyIter_Next(iterator))) {
    if (!_fastb_write_pyobj(stream, item, fallback)) {
      Py_DECREF(item);
      return 0;
    }
    Py_DECREF(item);
  }
  Py_DECREF(iterator);
  return 1;
}


static PyObject *
reads(PyObject *self, PyObject *args)
{
  FILE *stream;
  PyObject *fallback;
  if (PyTuple_GET_SIZE(args) != 2)
    return NULL;
  stream = PyFile_AsFile(PyTuple_GET_ITEM(args, 0));
  fallback = PyTuple_GET_ITEM(args, 1);
  return _fastb_read_pyiter(stream, fallback);
}

static PyObject *
writes(PyObject *self, PyObject *args)
{                          
  FILE *stream;
  PyObject *pyobj, *fallback;
  if (PyTuple_GET_SIZE(args) != 3)   
    return NULL;
  stream = PyFile_AsFile(PyTuple_GET_ITEM(args, 0));
  pyobj = PyTuple_GET_ITEM(args, 1);
  fallback = PyTuple_GET_ITEM(args, 2);
  _fastb_write_pyiter(stream, pyobj, fallback);
  Py_INCREF(Py_None);
  return Py_None;  
}


static PyMethodDef FastbMethods[] = {
  {"reads", reads, METH_VARARGS,
   "Read python objects."},
  {"writes", writes, METH_VARARGS,      
   "Write python objects."},
  {NULL, NULL, 0, NULL} /* sentinel */
};

PyMODINIT_FUNC
initfastb(void) {
  Py_InitModule("fastb", FastbMethods);
  _fastb_init_read_cb_table();
}
