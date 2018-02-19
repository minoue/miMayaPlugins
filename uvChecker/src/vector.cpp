#include "vector.h"

Vector::Vector()
{
}

Vector::Vector(float u, float v)
{
    this->u = u;
    this->v = v;
}

Vector::~Vector()
{
}

float Vector::dot(Vector& other)
{
    float dot = (this->u * other.u) + (this->v * other.v);
    return dot;
}
