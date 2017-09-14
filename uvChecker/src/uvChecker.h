#ifndef __UVCHECKER_H__
#define __UVCHECKER_H__

#include <maya/MDagPath.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>

class UvChecker : public MPxCommand {
public:
    UvChecker();
    virtual ~UvChecker();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
    static MSyntax newSyntax();

    MStatus findOverlaps();

    enum Check {
        OVERLAPS,
        UDIM
    };

private:
    MDagPath mDagPath;
    bool verbose;
    unsigned int checkNumber;
};

#endif /* defined(__UVCHECKER_H__) */
