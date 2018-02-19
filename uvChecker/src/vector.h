#ifndef __VECTOR2D__
#define __VECTOR2D__

class Vector {
public:
    Vector();
    Vector(float u, float v);
    ~Vector();

    float u;
    float v;

    float dot(Vector& other);

private:
};

#endif /* defined(__VECTOR2D_H__) */
