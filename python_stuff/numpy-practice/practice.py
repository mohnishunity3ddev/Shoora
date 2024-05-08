import numpy as np

a = np.array([1,2,3], dtype='int32')
print(a)

b = np.array([[9.0, 8.0, 7.0], [6.0, 5.0, 4.0]])
print(b)

# Get Dimensions
print(f"a's dimensions {a.ndim}")
print(f"b's dimensions {b.ndim}")

# Get Shape
print(f"a's shape: {a.shape}")
print(f"b's shape: {b.shape}")

# Get Data Type
print(f"a's dtype: {a.dtype}")
print(f"b's dtype: {b.dtype}")

# Get Item Size
print(f"a's itemSize: {a.itemsize}")
print(f"b's itemSize: {b.itemsize}")

# Get total Size
print(f"a's totalSize: {a.nbytes}")
print(f"b's totalSize: {b.nbytes}")


# Create two sample matrices
A = np.array([[1,2], [3,4], [5,6]])
B = np.array([[7, 8, 9], [10, 11, 12]])

# Multiply the matrices using matmul
C = np.matmul(A, B)

# Print the resulting matrix
print(C)