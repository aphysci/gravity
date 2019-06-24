/**
 * Contains util functions for unit tests.
 */

/**
 * Container must have an operator[] overload that results in the same type as T
 */
template <typename Container, typename T>
void check_equality(const Container& expected, const T* result, size_t size) {
  for(int i = 0; i < size; ++i) {
    CAPTURE(i);
    CHECK(expected[i] == result[i]);
  }  
}
