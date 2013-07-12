#ifndef __GRAVITYTEST_H
#define __GRAVITYTEST_H

class GravityMark
{
public:
    GravityMark( const char *file, int line ) { fprintf(stderr, "GravityMark: %s:%d\n", file, line ); }
    virtual ~GravityMark() {}
private:
    GravityMark();
};

#define GRAVITY_TEST_EQUALS(a,b) do { \
    if ( !( (a) == (b) ) ) \
        throw GravityMark(__FILE__, __LINE__); \
} while (0)

#define GRAVITY_TEST(a) do { \
    if ( !( a ) ) \
        throw GravityMark(__FILE__, __LINE__); \
} while (0)



#endif
