/** @file
 * @author Jan Gamec
 * @date 24 May 2015
 * @brief File containing python wrapper for a C MLP with RPROP
 *
 * This file serves as a wrapper for functionality in rprop.c . After building this script, it can be
 * imported as a standalone python module. The module can be build by following command:
 *     python setup.py build_ext --inplace
 *
 * The setup.py file **is required** to be in the same directory as this script!.
 * In order to change configuration of network, please see documentation for rprop.c file.
 * @see rprop.c   
 */

#include <Python.h>
#include <numpy/arrayobject.h>
#include "rprop.c"

static char module_docstring[] =
"This module provides an implementation of multilayer perceptron learned by RPROP algorithm.";

static char learn_docstring[] = 
"Learns network specified by weights matrix using given pattern set";

static char run_docstring[] =
"Runs specific network described with weights matrix using given pattern";

static char init_docstring[] =
"Initializes the network with random weights";

static PyObject* rprop_learn2(PyObject *self, PyObject *args);

static PyObject* rprop_run2(PyObject *self, PyObject *args);

static PyObject* rprop_init(PyObject *self, PyObject *args);

/**
 * Definition of all functions available after the build.
 */
static PyMethodDef module_methods[] = {
  {"learn", rprop_learn2, METH_VARARGS, learn_docstring},
  {"run", rprop_run2, METH_VARARGS, run_docstring},
  {"init", rprop_init, METH_VARARGS, init_docstring},
  {NULL, NULL, 0, NULL}
};


/**
 * @brief Initializes python wrapping functions
 *
 */
PyMODINIT_FUNC init_rprop(void)
{
  PyObject *m = Py_InitModule3("_rprop", module_methods, module_docstring);
  if (m == NULL)
    return;

  /* Load `numpy` functionality. */
  import_array();
}

/**
 * @brief Function wrapping a learning process of a neural network.
 *
 * Function in python accepts following arguments:
 * - **num_of_epochs** Number of training epochs
 * - **patternSet** A numpy array of training patterns
 * - **weights** A numpy array of neural network weights
 * @param self Object pointer
 * @param args Holds all arguments that can be passed to function in python
 * @return A numpy array holding new weights
 */
static PyObject* rprop_learn2(PyObject *self, PyObject *args) {
  int num_of_epochs;
  double **weights, **patternSet;
  ann_t ann[1];
  PyObject *weights_obj, *pattern_obj, *res;
  PyArray_Descr *descr;
  npy_intp weights_dims[2], patternSet_dims[2];
  descr = PyArray_DescrFromType(NPY_DOUBLE);

  if (!PyArg_ParseTuple(args, "iOO", &num_of_epochs, &pattern_obj ,&weights_obj))
    return NULL;

  if (PyArray_AsCArray(&weights_obj, (void **)&weights, weights_dims, 2, descr) < 0) {
    PyErr_SetString(PyExc_TypeError, "error converting to c array");
    return NULL;
  }

  if (PyArray_AsCArray(&pattern_obj, (void **)&patternSet, patternSet_dims, 2, descr) < 0) {
    PyErr_SetString(PyExc_TypeError, "error converting to c array");
    return NULL;
  }

  ann_init(ann, weights);
  rprop_learn(ann, num_of_epochs, (int)patternSet_dims[0], patternSet);

  if((res = PyArray_SimpleNew(2, weights_dims, NPY_DOUBLE)) == NULL) {
    return NULL;
  }

  memcpy(PyArray_DATA(res), ann->w, sizeof(ann->w));
  return res;
}

/**
 * @brief Function wrapping a function that runs a neural network forward.
 *
 * It runs network forward and return values on output neurons.
 * Function in python accepts following arguments:
 * - **pattern** A numpy array representing one input pattern
 * - **weights** A numpy array of neural network weights
 * @param self Object pointer
 * @param args Holds all arguments that can be passed to function in python
 * @return An Q-value on the output neuron with double precission
 */
static PyObject* rprop_run2(PyObject *self, PyObject *args) {
  double** weights;
  ann_t ann[1];
  // int pattern_length;
  
  PyObject *weights_obj, *pattern_obj;
  PyArray_Descr *descr;
  npy_intp dims[2];
  descr = PyArray_DescrFromType(NPY_DOUBLE); // define numpy array type

  if (!PyArg_ParseTuple(args, "OO", &pattern_obj ,&weights_obj)) {
    return NULL;
  } // parse python objects from args into their c representation

  // convert python object to c numpy array
  PyObject *pattern_array = PyArray_FROM_OTF(pattern_obj, NPY_DOUBLE, NPY_IN_ARRAY);

  // if conversion failed, destroy object and return exception
  if (pattern_array == NULL) {
    Py_XDECREF(pattern_array);
    return NULL;
  }

  // get number of elements in pattern
  // pattern_length  = (int)PyArray_DIM(pattern_array, 0);

  // convert c numpy array into native type
  double *pattern = (double*)PyArray_DATA(pattern_array);

  // convert multi-dimensional numpy weights array into 2D double array
  if (PyArray_AsCArray(&weights_obj, (void **)&weights, dims, 2, descr) < 0) {
    PyErr_SetString(PyExc_TypeError, "error converting to c array");
    return NULL;
  }

  // if dimensions of input weights matrix doesn't correspond to network size
  // throw an exception
  if (dims[0] != Nx || dims[1] != Nx) {
    return NULL; 
  }

  ann_init(ann, weights); // set network weights
  double* result = rprop_run(ann, pattern);
  PyObject *ret = Py_BuildValue("d", result[0]);
  return ret;
}


/**
 * @brief Function wrapping a initialization of a network
 *
 * Function in python has no input arguments. It just creates new network with random weights initialization
 * and return this weights as a numpy array.
 * @param self Object pointer
 * @param args Holds all arguments that can be passed to function in python
 * @return A numpy array holding new weights of the created network
 */
static PyObject *rprop_init(PyObject* self, PyObject *args) {
  ann_t ann[1];
  PyObject *res;
  npy_intp dims[2] = {Nx, Nx};

  // initialize network with random weights
  ann_rndinit(ann,-0.1,0.1);
  // create empty c numpy array
  if((res = PyArray_SimpleNew(2, dims, NPY_DOUBLE)) == NULL) {
    return NULL;
  }
  // copy weights matrix into created numpy array
  memcpy(PyArray_DATA(res), ann->w, sizeof(ann->w));
  return res;
}