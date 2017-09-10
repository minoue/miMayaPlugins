#ifndef __UVCHECKER_H__
#define __UVCHECKER_H__


#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>

class UvChecker : public MPxCommand
{
public:
    UvChecker();
    virtual ~UvChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
private:
    MDagPath mDagPath;
};

#endif /* defined(__UVCHECKER_H__) */
