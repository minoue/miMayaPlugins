#ifndef __SAMPLECOMMAND_H__
#define __SAMPLECOMMAND_H__


#include <maya/MPxCommand.h>
#include <maya/MDagPath.h>

class SampleCommand : public MPxCommand
{
public:
    SampleCommand();
    virtual ~SampleCommand();
    MStatus doIt(const MArgList& argList);
    MStatus undoIt();
    MStatus redoIt();
    bool isUndoable() const;
    static void* creater();
private:
    MDagPath mDagPath;
};

#endif /* defined(__SAMPLECOMMAND_H__) */
