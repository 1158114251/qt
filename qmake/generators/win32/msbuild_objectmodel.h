/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the qmake application of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MSBUILD_OBJECTMODEL_H
#define MSBUILD_OBJECTMODEL_H

#include "project.h"
#include "xmloutput.h"
#include "msvc_objectmodel.h"
#include <qatomic.h>
#include <qlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE


class VCXConfiguration;
class VCXProject;

class VCXCLCompilerTool : public VCToolBase
{
public:
    // Functions
    VCXCLCompilerTool();
    virtual ~VCXCLCompilerTool(){}
    bool parseOption(const char* option);

    // Variables
    QStringList             AdditionalIncludeDirectories;
    QStringList             AdditionalOptions;
    QStringList             AdditionalUsingDirectories;
    QString                 AlwaysAppend;
    QString                 AssemblerListingLocation;
    QString                 AssemblerOutput;
    QString                 BasicRuntimeChecks;
    triState                BrowseInformation;
    QString                 BrowseInformationFile;
    triState                BufferSecurityCheck;
    QString                 CallingConvention;
    QString                 CompileAs;
    QString                 CompileAsManaged;
    triState                CreateHotpatchableImage;
    QString                 DebugInformationFormat;
    triState                DisableLanguageExtensions;
    QStringList             DisableSpecificWarnings;
    QString                 EnableEnhancedInstructionSet;
    triState                EnableFiberSafeOptimizations;
    triState                EnablePREfast;
    QString                 ErrorReporting;
    QString                 ExceptionHandling;
    triState                ExpandAttributedSource;
    QString                 FavorSizeOrSpeed;
    triState                FloatingPointExceptions;
    QString                 FloatingPointModel;
    triState                ForceConformanceInForLoopScope;
    QStringList             ForcedIncludeFiles;
    QStringList             ForcedUsingFiles;
    triState                FunctionLevelLinking;
    triState                GenerateXMLDocumentationFiles;
    triState                IgnoreStandardIncludePath;
    QString                 InlineFunctionExpansion;
    triState                IntrinsicFunctions;
    triState                MinimalRebuild;
    triState                MultiProcessorCompilation;
    QString                 ObjectFileName;
    QStringList             ObjectFiles;
    triState                OmitDefaultLibName;
    triState                OmitFramePointers;
    triState                OpenMPSupport;
    QString                 Optimization;
    QString                 PrecompiledHeader;
    QString                 PrecompiledHeaderFile;
    QString                 PrecompiledHeaderOutputFile;
    triState                PreprocessKeepComments;
    QStringList             PreprocessorDefinitions;
    QString                 PreprocessOutputPath;
    triState                PreprocessSuppressLineNumbers;
    triState                PreprocessToFile;
    QString                 ProgramDataBaseFileName;
    QString                 ProcessorNumber;
    QString                 RuntimeLibrary;
    triState                RuntimeTypeInfo;
    triState                ShowIncludes;
    triState                SmallerTypeCheck;
    triState                StringPooling;
    QString                 StructMemberAlignment;
    triState                SuppressStartupBanner;
    QString                 TreatSpecificWarningsAsErrors;
    triState                TreatWarningAsError;
    triState                TreatWChar_tAsBuiltInType;
    triState                UndefineAllPreprocessorDefinitions;
    QStringList             UndefinePreprocessorDefinitions;
    triState                UseFullPaths;
    triState                UseUnicodeForAssemblerListing;
    QString                 WarningLevel;
    triState                WholeProgramOptimization;
    QString                 XMLDocumentationFileName;

    VCXConfiguration*       config;
};

class VCXLinkerTool : public VCToolBase
{
public:
    // Functions
    VCXLinkerTool();
    virtual ~VCXLinkerTool(){}
    bool parseOption(const char* option);

    // Variables
    QStringList             AdditionalDependencies;
    QStringList             AdditionalLibraryDirectories;
    QStringList             AdditionalManifestDependencies;
    QStringList             AdditionalOptions;
    QStringList             AddModuleNamesToAssembly;
    triState                AllowIsolation;
    triState                AssemblyDebug;
    QStringList             AssemblyLinkResource;
    QString                 BaseAddress;
    QString                 CLRImageType;
    QString                 CLRSupportLastError;
    QString                 CLRThreadAttribute;
    QString                 CLRUnmanagedCodeCheck;
    QString                 CreateHotPatchableImage;
    triState                DataExecutionPrevention;
    QStringList             DelayLoadDLLs;
    triState                DelaySign;
    QString                 Driver;
    QStringList             EmbedManagedResourceFile;
    triState                EnableCOMDATFolding;
    triState                EnableUAC;
    QString                 EntryPointSymbol;
    triState                FixedBaseAddress;
    QString                 ForceFileOutput;
    QStringList             ForceSymbolReferences;
    QString                 FunctionOrder;
    triState                GenerateDebugInformation;
    triState                GenerateManifest;
    triState                GenerateMapFile;
    qlonglong               HeapCommitSize;
    qlonglong               HeapReserveSize;
    triState                IgnoreAllDefaultLibraries;
    triState                IgnoreEmbeddedIDL;
    triState                IgnoreImportLibrary;
    QStringList             IgnoreSpecificDefaultLibraries;
    triState                ImageHasSafeExceptionHandlers;
    QString                 ImportLibrary;
    QString                 KeyContainer;
    QString                 KeyFile;
    triState                LargeAddressAware;
    triState                LinkDLL;
    QString                 LinkErrorReporting;
    triState                LinkIncremental;
    triState                LinkStatus;
    QString                 LinkTimeCodeGeneration;
    QString                 ManifestFile;
    triState                MapExports;
    QString                 MapFileName;
    QString                 MergedIDLBaseFileName;
    QString                 MergeSections;
    QString                 MidlCommandFile;
    QString                 ModuleDefinitionFile;
    QString                 MSDOSStubFileName;
    triState                NoEntryPoint;
    triState                OptimizeReferences;
    QString                 OutputFile;
    triState                PreventDllBinding;
    QString                 Profile;
    QString                 ProfileGuidedDatabase;
    QString                 ProgramDatabaseFile;
    triState                RandomizedBaseAddress;
    triState                RegisterOutput;
    qlonglong               SectionAlignment;
    triState                SetChecksum;
    QString                 ShowProgress;
    QString                 SpecifySectionAttributes;
    QString                 StackCommitSize;
    QString                 StackReserveSize;
    QString                 StripPrivateSymbols;
    QString                 SubSystem;
    triState                SupportNobindOfDelayLoadedDLL;
    triState                SupportUnloadOfDelayLoadedDLL;
    triState                SuppressStartupBanner;
    triState                SwapRunFromCD;
    triState                SwapRunFromNet;
    QString                 TargetMachine;
    triState                TerminalServerAware;
    triState                TreatLinkerWarningAsErrors;
    triState                TurnOffAssemblyGeneration;
    QString                 TypeLibraryFile;
    qlonglong               TypeLibraryResourceID;
    QString                 UACExecutionLevel;
    triState                UACUIAccess;
    QString                 Version;


    VCXConfiguration*       config;
};

class VCXMIDLTool : public VCToolBase
{
public:
    // Functions
    VCXMIDLTool();
    virtual ~VCXMIDLTool(){}
    bool parseOption(const char* option);

    // Variables
    QStringList             AdditionalIncludeDirectories;
    QStringList             AdditionalOptions;
    triState                ApplicationConfigurationMode;
    QString                 ClientStubFile;
    QString                 CPreprocessOptions;
    QString                 DefaultCharType;
    QString                 DLLDataFileName;
    QString                 EnableErrorChecks;
    triState                ErrorCheckAllocations;
    triState                ErrorCheckBounds;
    triState                ErrorCheckEnumRange;
    triState                ErrorCheckRefPointers;
    triState                ErrorCheckStubData;
    QString                 GenerateClientFiles;
    QString                 GenerateServerFiles;
    triState                GenerateStublessProxies;
    triState                GenerateTypeLibrary;
    QString                 HeaderFileName;
    triState                IgnoreStandardIncludePath;
    QString                 InterfaceIdentifierFileName;
    qlonglong               LocaleID;
    triState                MkTypLibCompatible;
    QString                 OutputDirectory;
    QStringList             PreprocessorDefinitions;
    QString                 ProxyFileName;
    QString                 RedirectOutputAndErrors;
    QString                 ServerStubFile;
    QString                 StructMemberAlignment;
    triState                SuppressCompilerWarnings;
    triState                SuppressStartupBanner;
    QString                 TargetEnvironment;
    QString                 TypeLibFormat;
    QString                 TypeLibraryName;
    QStringList             UndefinePreprocessorDefinitions;
    triState                ValidateAllParameters;
    triState                WarnAsError;
    QString                 WarningLevel;

    VCXConfiguration*       config;
};

class VCXLibrarianTool : public VCToolBase
{
public:
    // Functions
    VCXLibrarianTool();
    virtual ~VCXLibrarianTool(){}
    bool parseOption(const char*){ return false; };

    // Variables
    QStringList             AdditionalDependencies;
    QStringList             AdditionalLibraryDirectories;
    QStringList             AdditionalOptions;
    QString                 DisplayLibrary;
    QString                 ErrorReporting;
    QStringList             ExportNamedFunctions;
    QStringList             ForceSymbolReferences;
    triState                IgnoreAllDefaultLibraries;
    QStringList             IgnoreSpecificDefaultLibraries;
    triState                LinkTimeCodeGeneration;
    QString                 ModuleDefinitionFile;
    QString                 Name;
    QString                 OutputFile;
    QStringList             RemoveObjects;
    QString                 SubSystem;
    triState                SuppressStartupBanner;
    QString                 TargetMachine;
    triState                TreatLibWarningAsErrors;
    triState                Verbose;

};

class VCXCustomBuildTool : public VCToolBase
{
public:
    // Functions
    VCXCustomBuildTool();
    virtual ~VCXCustomBuildTool(){}
    bool parseOption(const char*){ return false; };

    // Variables
    QStringList             AdditionalDependencies;
    QStringList             CommandLine;
    QString                 Description;
    QStringList             Outputs;
    QString                 ToolName;
    QString                 ToolPath;
    QString                 ConfigName;
};

class VCXResourceCompilerTool : public VCToolBase
{
public:
    // Functions
    VCXResourceCompilerTool();
    virtual ~VCXResourceCompilerTool(){}
    bool parseOption(const char*){ return false; };

    // Variables
    QStringList             AdditionalIncludeDirectories;
    QString                 AdditionalOptions;
    QString                 Culture;
    triState                IgnoreStandardIncludePath;
    triState                NullTerminateStrings;
    QStringList             PreprocessorDefinitions;
    QString                 ResourceOutputFileName;
    triState                ShowProgress;
    triState                SuppressStartupBanner;
    QString                 TrackerLogDirectory;
    QString                 UndefinePreprocessorDefinitions;
};

class VCXDeploymentTool
{
public:
    // Functions
    VCXDeploymentTool();
    virtual ~VCXDeploymentTool() {}

    // Variables
    QString                 DeploymentTag;
    QString                 RemoteDirectory;
    QString                 AdditionalFiles;
};

class VCXEventTool : public VCToolBase
{
protected:
    // Functions
    VCXEventTool() : UseInBuild(unset){};
    virtual ~VCXEventTool(){}
    bool parseOption(const char*){ return false; };

public:
    // Variables
    QString                 CommandLine;
    QString                 Description;
    triState                UseInBuild;
    QString                 EventName;
    QString                 ToolPath;
};

class VCXPostBuildEventTool : public VCXEventTool
{
public:
    VCXPostBuildEventTool();
    ~VCXPostBuildEventTool(){}
};

class VCXPreBuildEventTool : public VCXEventTool
{
public:
    VCXPreBuildEventTool();
    ~VCXPreBuildEventTool(){}
};

class VCXPreLinkEventTool : public VCXEventTool
{
public:
    VCXPreLinkEventTool();
    ~VCXPreLinkEventTool(){}
};

class VCXConfiguration
{
public:
    // Functions
    VCXConfiguration();
    ~VCXConfiguration(){}

    // Variables
    triState                ATLMinimizesCRunTimeLibraryUsage;
    triState                BuildBrowserInformation;
    QString                 CharacterSet;
    QString                 ConfigurationType;
    QString                 DeleteExtensionsOnClean;
    QString                 ImportLibrary;
    QString                 IntermediateDirectory;
    QString                 Name;
    QString                 ConfigurationName;
    QString                 OutputDirectory;
    QString                 PrimaryOutput;
    QString                 ProgramDatabase;
    triState                RegisterOutput;
    QString                 TargetName;
    QString                 UseOfATL;
    QString                 UseOfMfc;
    triState                WholeProgramOptimization;

    // XML sub-parts
    VCXCLCompilerTool        compiler;
    VCXLibrarianTool         librarian;
    VCXLinkerTool            linker;
    VCXMIDLTool              idl;
    VCXResourceCompilerTool  resource;
    VCXCustomBuildTool       custom;
    VCXDeploymentTool        deployment;     // Not likely to be supported: http://msdn.microsoft.com/en-us/library/sa69he4t.aspx
    VCXPostBuildEventTool    postBuild;
    VCXPreBuildEventTool     preBuild;
    VCXPreLinkEventTool      preLink;
};

struct VCXFilterFile
{
    VCXFilterFile()
    { excludeFromBuild = false; }
    VCXFilterFile(const QString &filename, bool exclude = false )
    { file = filename; excludeFromBuild = exclude; }
    VCXFilterFile(const QString &filename, const QString &additional, bool exclude = false )
    { file = filename; excludeFromBuild = exclude; additionalFile = additional; }
    bool operator==(const VCXFilterFile &other){
        return file == other.file
               && additionalFile == other.additionalFile
               && excludeFromBuild == other.excludeFromBuild;
    }

    bool                    excludeFromBuild;
    QString                 file;
    QString                 additionalFile; // For tools like MOC
};

class VcxprojGenerator;
class VCXFilter
{
public:
    // Functions
    VCXFilter();
    ~VCXFilter(){};

    void addFile(const QString& filename);  // equal
    void addFile(const VCXFilterFile& fileInfo);// equal
    void addFiles(const QStringList& fileList);// equal
    bool addExtraCompiler(const VCXFilterFile &info); //equal
    void modifyPCHstage(QString str); //almost equal -> file comment .vcxproj, PrecompiledHeader property

    // Variables
    QString                 Name;
    QString                 Filter;
    QString                 Guid;
    triState                ParseFiles;
    VcxprojGenerator*       Project;
    VCXConfiguration*       Config;
    QList<VCXFilterFile>    Files;

    customBuildCheck        CustomBuild;

    bool                    useCustomBuildTool;
    VCXCustomBuildTool      CustomBuildTool;

    bool                    useCompilerTool;
    VCXCLCompilerTool       CompilerTool;
};

typedef QList<VCXFilter> VCXFilterList;
class VCXProjectSingleConfig
{
public:
    enum FilterTypes {
        None,
        Source,
        Header,
        Generated,
        LexYacc,
        Translation,
        Resources,
        Extras
    };
    // Functions
    VCXProjectSingleConfig(){};
    ~VCXProjectSingleConfig(){}

    // Variables
    QString                 Name;
    QString                 Version;
    QString                 ProjectGUID;
    QString                 Keyword;
    QString                 SccProjectName;
    QString                 SccLocalPath;
    QString                 PlatformName;

    // XML sub-parts
    VCXConfiguration        Configuration;
    VCXFilter               RootFiles;
    VCXFilter               SourceFiles;
    VCXFilter               HeaderFiles;
    VCXFilter               GeneratedFiles;
    VCXFilter               LexYaccFiles;
    VCXFilter               TranslationFiles;
    VCXFilter               FormFiles;
    VCXFilter               ResourceFiles;
    VCXFilterList           ExtraCompilersFiles;

    bool                    flat_files;

    // Accessor for extracompilers
    VCXFilter               &filterForExtraCompiler(const QString &compilerName);
    // ### this function is the same as in VCProjectSingleConfig ----> DIE DIE DIE
};



// Tree & Flat view of files --------------------------------------------------
class VCXFilter;
class XNode
{
public:
    virtual ~XNode() { }
    void addElement(const VCXFilterFile &file) {
        addElement(file.file, file);
    }
    virtual void addElement(const QString &filepath, const VCXFilterFile &allInfo) = 0;
    virtual void removeElements()= 0;
    virtual void generateXML(XmlOutput &xml, XmlOutput &xmlFilter, const QString &tagName, VCXProject &tool, const QString &filter) = 0;
    virtual bool hasElements() = 0;
};

class XTreeNode : public XNode
{
    typedef QMap<QString, XTreeNode*> ChildrenMap;
    VCXFilterFile info;
    ChildrenMap children;

public:
    virtual ~XTreeNode() { removeElements(); }

    int pathIndex(const QString &filepath) {
        int Windex = filepath.indexOf("\\");
        int Uindex = filepath.indexOf("/");
        if (Windex != -1 && Uindex != -1)
            return qMin(Windex, Uindex);
        else if (Windex != -1)
            return Windex;
        return Uindex;
    }

    void addElement(const QString &filepath, const VCXFilterFile &allInfo){
        QString newNodeName(filepath);

        int index = pathIndex(filepath);
        if (index != -1)
            newNodeName = filepath.left(index);

        XTreeNode *n = children.value(newNodeName);
        if (!n) {
            n = new XTreeNode;
            n->info = allInfo;
            children.insert(newNodeName, n);
        }
        if (index != -1)
            n->addElement(filepath.mid(index+1), allInfo);
    }

    void removeElements() {
        ChildrenMap::ConstIterator it = children.constBegin();
        ChildrenMap::ConstIterator end = children.constEnd();
        for( ; it != end; it++) {
            (*it)->removeElements();
            delete it.value();
        }
        children.clear();
    }

    void generateXML(XmlOutput &xml, XmlOutput &xmlFilter, const QString &tagName, VCXProject &tool, const QString &filter);
    bool hasElements() {
        return children.size() != 0;
    }
};

class XFlatNode : public XNode
{
    typedef QMap<QString, VCXFilterFile> ChildrenMapFlat;
    ChildrenMapFlat children;

public:
    virtual ~XFlatNode() { removeElements(); }

    int pathIndex(const QString &filepath) {
        int Windex = filepath.lastIndexOf("\\");
        int Uindex = filepath.lastIndexOf("/");
        if (Windex != -1 && Uindex != -1)
            return qMax(Windex, Uindex);
        else if (Windex != -1)
            return Windex;
        return Uindex;
    }

    void addElement(const QString &filepath, const VCXFilterFile &allInfo){
        QString newKey(filepath);

        int index = pathIndex(filepath);
        if (index != -1)
            newKey = filepath.mid(index+1);

        // Key designed to sort files with same
        // name in different paths correctly
        children.insert(newKey + "\0" + allInfo.file, allInfo);
    }

    void removeElements() {
        children.clear();
    }

    void generateXML(XmlOutput &xml, XmlOutput &xmlFilter, const QString &tagName, VCXProject &proj, const QString &filter);
    bool hasElements() {
        return children.size() != 0;
    }
};
// ----------------------------------------------------------------------------

class VCXProject
{
public:
    // Variables
    QString                 Name;
    QString                 Version;
    QString                 ProjectGUID;
    QString                 Keyword;
    QString                 SccProjectName;
    QString                 SccLocalPath;
    QString                 PlatformName;

    // Single projects
    QList<VCXProjectSingleConfig>  SingleProjects;

    // List of all extracompilers
    QStringList             ExtraCompilers;
};

class VCXProjectWriter : public VCProjectWriter
{
public:
    // ### replace the X classes with the standard names!
    void write(XmlOutput &, VCXProjectSingleConfig &);
    void write(XmlOutput &, VCXProject &);

    void write(XmlOutput &, const VCXCLCompilerTool &);
    void write(XmlOutput &, const VCXLinkerTool &);
    void write(XmlOutput &, const VCXMIDLTool &);
    void write(XmlOutput &, const VCXCustomBuildTool &);
    void write(XmlOutput &, const VCXLibrarianTool &);
    void write(XmlOutput &, const VCXResourceCompilerTool &);
    void write(XmlOutput &, const VCXEventTool &);
    void write(XmlOutput &, const VCDeploymentTool &);
    void write(XmlOutput &, const VCXConfiguration &);
    void write(XmlOutput &, VCFilter &);

private:
    static void addFilters(VCXProject &project, XmlOutput &xmlFilter, const QString &filterName);
    static void outputFilter(VCXProject &project, XmlOutput &xml, XmlOutput &xmlFilter, const QString &filtername);
    static void outputFileConfigs(VCXProject &project, XmlOutput &xml, XmlOutput &xmlFilter, const VCXFilterFile &info, const QString &filtername);
    static bool outputFileConfig(VCXFilter &filter, XmlOutput &xml, XmlOutput &xmlFilter, const QString &filename, const QString &filtername, bool fileAllreadyAdded);

    friend class XTreeNode;
    friend class XFlatNode;
};

QT_END_NAMESPACE

#endif // MSVC_OBJECTMODEL_H
