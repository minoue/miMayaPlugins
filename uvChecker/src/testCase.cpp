#include "testCase.h"
#include "uvPoint.h"
#include "uvEdge.h"

#include <maya/MString.h>
#include <maya/MGlobal.h>


TestCase::TestCase()
{
}

TestCase::~TestCase()
{
}

void TestCase::checkEdgeIntersection() {
    MGlobal::displayInfo("running test");

    UvPoint p1(0.11, 0.09);
    UvPoint p2(0.1, 0.5);
    UvPoint p3(0.1, 0.1);
    UvPoint p4(0.4, 0.1);
    UvEdge e1;
    e1.begin = p1;
    e1.end = p2;
    UvEdge e2;
    e2.begin = p3;
    e2.end = p4;

    UvPoint p5(0.4, 0.3);
    UvPoint p6(0.8, 0.6);
    UvPoint p7(0.6, 0.2);
    UvPoint p8(0.6, 0.3);
    UvEdge e3;
    e3.begin = p5;
    e3.end = p6;
    UvEdge e4;
    e4.begin = p7;
    e4.end = p8;


    bool result1 = e1.isIntersected(e2);
    bool result2 = e3.isIntersected(e4);
    MString resultStr;
    resultStr.set(result1);
    MGlobal::displayInfo(resultStr);
    resultStr.set(result2);
    MGlobal::displayInfo(resultStr);
}
