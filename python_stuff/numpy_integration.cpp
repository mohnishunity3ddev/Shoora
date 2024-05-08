#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
#include <Eigen/Dense>

#include <iterator>

namespace py = pybind11;

template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>
ToEigen(const py::array_t<T> &arr)
{
    // Get dimensions from NumPy array
    auto shape = arr.shape();
    int rows = shape[0];
    int cols = shape[1];

    // Create Eigen matrix
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> eigen_mat(rows, cols);

    // Copy data from NumPy array to Eigen matrix
    auto ptr = arr.data();
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            eigen_mat(i, j) = *ptr++;
        }
    }

    return eigen_mat;
}

template <typename T, std::size_t R, std::size_t C>
py::array_t<T>
createArray(T val[R][C], const py::module &numpy)
{
    py::list values;
    for (int i = 0; i < R; ++i)
    {
        py::list row;
        for (int j = 0; j < C; ++j)
        {
            row.append(val[i][j]);
        }
        values.append(row);
    }

    py::object result = numpy.attr("array")(values);

    // Convert result to a NumPy array
    py::array_t<T> arr = py::cast<py::array_t<T>>(result);

    return arr;
}

// Function to multiply two NumPy arrays using matmul
template <typename T>
py::array_t<T>
multiplyMatrices(const py::array_t<T> &arr1, const py::array_t<T> &arr2, const py::module &numpy)
{
    // Check if matrix multiplication is possible (compatible shapes)
    if (arr1.shape(1) != arr2.shape(0))
    {
        PyErr_SetString(PyExc_ValueError, "Incompatible matrix shapes for multiplication");
        throw py::error_already_set();
    }

    py::object result = numpy.attr("matmul")(arr1, arr2);

    // Convert the result to a NumPy array (optional)
    py::array_t<T> product_arr = py::cast<py::array_t<T>>(result);

    return product_arr;
}

template <typename T>
void
displayArray(py::array_t<T> &arr)
{
    // Get a pointer to the underlying data buffer
    int *data_ptr = arr.mutable_data();

    // Print the NumPy array shape
    auto shape = arr.shape();

    std::cout << "Shape of the NumPy array: ";
    for (size_t i = 0; i < arr.ndim(); ++i)
    {
        std::cout << shape[i] << " ";
    }
    std::cout << std::endl;

    // Print the NumPy array values
    std::cout << "NumPy array values:" << std::endl;
    for (int i = 0; i < shape[0]; ++i)
    {
        for (int j = 0; j < shape[1]; ++j)
        {
            std::cout << data_ptr[i * shape[1] + j] << " ";
        }
        std::cout << std::endl;
    }
}

int
main()
{
    py::scoped_interpreter guard{}; // Start and stop the interpreter

    int a[3][2] = {{1,2}, {3,4}, {5,6}};
    int b[2][3] = {{7, 8, 9}, {10, 11, 12}};

    // Import NumPy module
    py::module numpy = py::module::import("numpy");

    // Convert result to a NumPy array
    auto arr1 = createArray<int, 3, 2>(a, numpy);
    auto arr2 = createArray<int, 2, 3>(b, numpy);

    displayArray(arr1);
    displayArray(arr2);

    try
    {
        auto product = multiplyMatrices<int>(arr1, arr2, numpy);
        auto eigenMat = ToEigen(product);

        std::cout << "Product Matrix: \n" << eigenMat << " \n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
