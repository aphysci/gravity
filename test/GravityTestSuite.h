#ifndef __GRAVITYTESTSUITE_H
#define __GRAVITYTESTSUITE_H

class GravityAbort 
{
public:
    GravityAbort( const char *file, int line ) { fprintf(stderr, "GravityAbort: %s:%d\n", file, line ); }
    virtual ~GravityAbort() {}
private:
    GravityAbort();
};

#define GRAVITY_ASSERT_EQUALS(a,b) do { \
    if ( !( (a) == (b) ) ) \
        throw GravityAbort(__FILE__, __LINE__); \
} while (0)

#define GRAVITY_ASSERT(a) do { \
    if ( !( a ) ) \
        throw GravityAbort(__FILE__, __LINE__); \
} while (0)



#endif
