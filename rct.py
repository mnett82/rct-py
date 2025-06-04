from ctypes import c_float, c_int, c_ulong, c_void_p, POINTER
import ctypes
import numpy as np
import os


class RCT:
    _c_lib = None

    def __init__(self):
        if RCT._c_lib == None:
            if os.name == 'posix':
                _libext = 'so'
            elif os.name == 'nt':
                _libext = 'dll'
            else:
                raise RuntimeError("Unsupported operating system for shared library loading")

            try:
                RCT._c_lib = ctypes.CDLL(f'librct.{_libext}')
            except OSError as e:
                printf(f"Error loading librct.{_libext}: {e}")
                raise

            RCT._c_lib.rct_build.argtypes = [c_ulong, c_int, c_float, c_float, c_void_p, c_int, c_int]
            RCT._c_lib.rct_build.restype = c_void_p
            RCT._c_lib.rct_destroy.argtypes = [c_void_p]
            RCT._c_lib.rct_destroy.restype = None
            RCT._c_lib.rct_find_near.argtypes = [c_void_p, c_void_p, c_int, POINTER(c_int)]
            RCT._c_lib.rct_find_near.restype = c_int

        self._rct = None
        self._data = None
        self._seed = 0
        self._verbosity = 2
        self._coverage = 8.0
        self._sample_rate = None
        
    def __del__(self):
        if self._rct != None:
            RCT._c_lib.rct_destroy(self._rct)
            self._rct = None

    def set_coverage(self, coverage: float):
        self._coverage = coverage
    
    def fit(self, X: np.ndarray):
        if not isinstance(X, np.ndarray):
            raise TypeError("Input X must be a array.")
        if X.ndim != 2:
            raise ValueError("Input X must be a 2-dimensional array.")
        if X.dtype != np.float32:
            raise TypeError("Input X must be a float32 array.")

        self._data = X

        rows, cols = X.shape
        self._sample_rate = rows ** (1/3)

        self._rct = RCT._c_lib.rct_build(self._seed,
                                         self._verbosity,
                                         self._coverage,
                                         self._sample_rate,
                                         self._data.ctypes.data_as(c_void_p),
                                         rows,
                                         cols)

    def query(self, q: np.array, n: int) -> np.array:
        if q.ndim != 1:
            raise ValueError("Input q must be a 1-dimensional array.")
        if q.dtype != np.float32:
            raise TypeError("Input q must be a float32 array.")

        q_ptr = q.ctypes.data_as(c_void_p)

        results = np.zeros(n, dtype=np.int32)
        results_ptr = results.ctypes.data_as(POINTER(c_int))

        RCT._c_lib.rct_find_near(self._rct, q_ptr, n, results_ptr)

        return results

if __name__ == "__main__":
    data_float32 = np.array([[1.0, 2.0, 3.0],
                             [4.0, 5.0, 6.0],
                             [7.0, 8.0, 9.0]], dtype=np.float32)
    rct = RCT()
    rct.fit(data_float32)

    neighbors = rct.query(np.array([0.0, 0.0, 0.0], dtype=np.float32), 2)
    print(neighbors)
