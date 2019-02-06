#ifndef __SYMMETRYTABLE_H__
#define __SYMMETRYTABLE_H__


#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>

class SymmetryTable : public MPxCommand
{
public:
    SymmetryTable();
    virtual ~SymmetryTable();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
private:
    MDagPath mDagPath;
};

#endif /* defined(__SYMMETRYTABLE_H__) */
